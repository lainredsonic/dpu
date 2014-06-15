#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <syscall.h>
#include <errno.h>
#include <pthread.h>

#include "icmp.h"


#define DATALEN 56

unsigned int nsent;

struct list_head mgr_list;


uint16_t
in_cksum(uint16_t *addr, int len)
{
	int		nleft = len;
	uint32_t	sum = 0;
	uint16_t	*w = addr;
	uint16_t	answer = 0;
	/*
	 * 	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 *  	 * sequential 16 bit words to it, and at the end, fold back all the
	 *  	 * carry bits from the top 16 bits into the lower 16 bits.
	 * 	 	 	 	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}
	/* 4mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
		sum += answer;
	}
	/* 4add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}


void icmp_clean(struct icmp_mgr *icmp_mgr){
	if(!icmp_mgr)
		return;
	free(icmp_mgr->icmp_packs);
	free(icmp_mgr->icmp_log);
}

void icmp_gen(struct icmp_mgr *icmp_mgr){
	int i;
	int nr = icmp_mgr->pack_nr;

	icmp_mgr->icmp_log = malloc(sizeof(struct icmp_log));
	if(!icmp_mgr->icmp_log){
		printf("nomem\n");
		exit(-ENOMEM);
	}

	struct icmp_pack *icmp_pack = calloc(nr, sizeof(struct icmp_pack));
	if(!icmp_pack){
		printf("nocmem\n");
		exit(-ENOMEM);
	}
	icmp_mgr->icmp_packs = icmp_pack;
	INIT_LIST_HEAD(&icmp_mgr->pack_head);
	icmp_mgr->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	icmp_mgr->poll = epoll_create(nr);
	icmp_mgr->nsent = 0;
	icmp_mgr->pack_nr = nr;
	icmp_mgr->magic = syscall(SYS_gettid);

	icmp_mgr->ev.events = EPOLLIN;
	(icmp_mgr->ev).data.ptr = icmp_mgr;
	epoll_ctl(icmp_mgr->poll, EPOLL_CTL_ADD, icmp_mgr->fd, &icmp_mgr->ev);
	
	for(i=0; i<nr; i++){
		struct icmp_pack *pack = icmp_pack+i;
		INIT_LIST_HEAD(&pack->list);
		pack->icmp = (struct icmp *)(pack->buf);
		pack->icmp->icmp_type = ICMP_ECHO;
		pack->icmp->icmp_code = 0;
		pack->mgr = icmp_mgr;
		pack->icmp->icmp_id = pack->mgr->magic;
		pack->icmp->icmp_seq = pack->mgr->nsent++;
		memset(pack->icmp->icmp_data, 0xab, DATALEN);
		list_add(&pack->list, &pack->mgr->pack_head);
	}
}

void icmp_send(struct icmp_mgr *mgr){
	struct icmp_pack *pack;
	struct sockaddr_in addr;
	int len;
	list_for_each_entry_reverse(pack, &mgr->pack_head, list){
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = mgr->serv_addr;
		addr.sin_port = 0;
		len = 8 + DATALEN;
//		gettimeofday((struct timeval *) pack->icmp->icmp_data, NULL);
		pack->icmp->icmp_cksum = 0;
		pack->icmp->icmp_cksum = in_cksum((u_short *)pack->icmp, len);
		gettimeofday(&pack->tv, NULL);
		connect(pack->mgr->fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
//		sendto(pack->mgr->fd, (char *)(pack->icmp), len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));
		write(pack->mgr->fd, (char *)(pack->icmp), len);
	}
}

void icmp_poll(struct icmp_mgr *mgr){
	int event_nr, i, n;
//	int j;
	unsigned short id;
	struct epoll_event ev[100];
	struct icmp_mgr *ev_mgr;
	int err_num;
	int recv;
	int pack_nr = mgr->pack_nr;
	char buf[BUFSIZE];
	struct icmp *icmp=(struct icmp *)&buf[20];
	struct sockaddr serv_addr;
	socklen_t sl = sizeof(struct sockaddr);
	int timeout = mgr->tm_out*1000;
	unsigned int matched = 0;

	while(pack_nr > 0){
		recv = 0;
		event_nr = epoll_wait(mgr->poll, (struct epoll_event *)&ev, 100, timeout);
		err_num = errno;
		if (event_nr < 0){
			printf("epoll_wait error:%d,%s\n", err_num,strerror(err_num));
			if(err_num == EINTR)
				continue;
		}else if(event_nr == 0){
//			printf(".");
//			fflush(stdout);
//			printf("%s timeout\n", mgr->dpu->alias);

			break;
		}
//		printf("\tev nr:%d\n", event_nr);
		for(i=0;i<event_nr;i++){
			ev_mgr = (struct icmp_mgr *)((ev[i]).data.ptr);
			n =  recvfrom(ev_mgr->fd, &buf, BUFSIZE,  0, &serv_addr, &sl);
			err_num = errno;
			if (n == 0) {
				printf("Read nothing");
			}else if(n<0){
				printf("read error:%s\n", strerror(err_num));
			}else{
//				id = *((unsigned short *)&buf[0x18]);
				id = icmp->icmp_id;
//				if(id == ev_mgr->magic && (((struct sockaddr_in *)&serv_addr)->sin_addr).s_addr == ev_mgr->dpu->serv_addr){
				if(id == ev_mgr->magic){
//					printf("icmp thread0x%x,id:0x%x, seq:%d\n", ev_mgr->magic, id, icmp->icmp_seq);
					ev_mgr->icmp_log->lu[matched].nsent=icmp->icmp_seq;
					gettimeofday(&ev_mgr->icmp_log->lu[matched].tv,NULL);
					matched++;
					recv++;
				}
			}
		}
		pack_nr -= recv;
	}
	mgr->icmp_log->recv_pkg_nr=matched;
	epoll_ctl(mgr->poll, EPOLL_CTL_DEL, mgr->fd, NULL);
	close(mgr->fd);
	close(mgr->poll);
}

unsigned int tv_diff(struct timeval *tv1, struct timeval *tv2)
{
	unsigned int udiff;
	unsigned int diff;
	udiff = (unsigned int)(tv1->tv_usec-tv2->tv_usec);
	diff = (unsigned int)(tv1->tv_sec-tv2->tv_sec);
	return (diff*1000000+udiff)/1000;
}

void icmp_acc(struct icmp_mgr *mgr){
	struct icmp_log *log = mgr->icmp_log;
	unsigned int lost = mgr->pack_nr-log->recv_pkg_nr;
	struct icmp_pack *icmp_pack = mgr->icmp_packs;
	unsigned int diff;
	unsigned int delay_sum = 0;
	unsigned int delay_avg = 0;
	unsigned int health = 0;


	int i, j;
	for(j=0; j<mgr->pack_nr; j++)
		for(i=0 ; i<log->recv_pkg_nr; i++){
			if((icmp_pack+j)->icmp->icmp_seq == (log->lu[i]).nsent){
				diff = tv_diff(&(log->lu[i].tv),&(icmp_pack+j)->tv);
			delay_sum += diff;
			}
		}
	if(log->recv_pkg_nr){
		delay_avg = delay_sum/log->recv_pkg_nr;
		health = delay_avg*25 + lost*3000;
	}else{
		delay_avg = -1;
		health = -1;
	}
	mgr->icmp_log->delay_sum = delay_sum;
	mgr->icmp_log->delay_avg = delay_avg;
	mgr->icmp_log->lost = lost;
	mgr->icmp_log->health = health;
}
