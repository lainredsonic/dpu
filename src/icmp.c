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


uint16_t
in_cksum(uint16_t *addr, int len)
{
	int				nleft = len;
	uint32_t		sum = 0;
	uint16_t		*w = addr;
	uint16_t		answer = 0;
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
	free(icmp_mgr);
}

struct icmp_mgr * icmp_gen(int nr){
	int i;
	struct icmp_mgr *icmp_mgr = malloc(sizeof(struct icmp_mgr));
	if(!icmp_mgr){
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
	return icmp_mgr;
}

void icmp_send(struct icmp_mgr *mgr, in_addr_t serv_addr){
	struct icmp_pack *pack;
	struct sockaddr_in addr;
	int len;
	list_for_each_entry_reverse(pack, &mgr->pack_head, list){
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = serv_addr;
		len = 8 + DATALEN;
		gettimeofday((struct timeval *) pack->icmp->icmp_data, NULL);
		pack->icmp->icmp_cksum = 0;
		pack->icmp->icmp_cksum = in_cksum((u_short *)pack->icmp, len);
		sendto(pack->mgr->fd, (char *)(pack->icmp), len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));
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
	while(pack_nr > 0){
		recv = 0;
		event_nr = epoll_wait(mgr->poll, (struct epoll_event *)&ev, 100, 1000);
		err_num = errno;
		if (event_nr < 0){
			printf("epoll_wait error:%d,%s\n", err_num,strerror(err_num));
			if(err_num == EINTR)
				continue;
		}else if(event_nr == 0){
			printf(".");
			fflush(stdout);
		}
//		printf("\tev nr:%d\n", event_nr);
		for(i=0;i<event_nr;i++){
			ev_mgr = (struct icmp_mgr *)((ev[i]).data.ptr);
			n = read(ev_mgr->fd, &buf, BUFSIZE);
			err_num = errno;
			if (n == 0) {
				printf("Read nothing");
			}else if(n<0){
				printf("read error:%s\n", strerror(err_num));
			}else{
				id = *((unsigned short *)&buf[0x18]);
				if(id == ev_mgr->magic){
					printf("icmp thread0x%x,id:0x%x\n", ev_mgr->magic,id);
					recv++;
				}
/*
				for(j=0;j<56;j++){
					printf("%u:id:0x%x\n", (unsigned int)pthread_self(), (mgr->recvbuf)[j]&0xff);
				}
*/
			}
		}
		pack_nr -= recv;
	}
	epoll_ctl(mgr->poll, EPOLL_CTL_DEL, mgr->fd, NULL);
	close(mgr->fd);
	close(mgr->poll);
}
