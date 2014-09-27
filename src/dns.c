#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <syscall.h>
#include <errno.h>
#include <pthread.h>
#include <error.h>
#include <errno.h>

#include "utils.h"
#include "dns_d.h"

#define DNS_DATALEN 1500

struct list_head dns_mgr_list;

void dns_clean(struct dns_mgr *dns_mgr)
{
	if(!dns_mgr)
		return;
	free(dns_mgr->dns_packs);
//	free(dns_mgr->dns_log);
}

void dns_gen(struct dns_mgr *dns_mgr)
{
	int i;
	__be16 id;
	int nr = dns_mgr->pack_nr;
	struct dns_pack *dns_pack = NULL;

	dns_mgr->dns_log = malloc(sizeof(struct dns_log));
	if(!dns_mgr->dns_log){
		printf("nomem\n");
		exit(-ENOMEM);
	}
	dns_pack = calloc(nr, sizeof(struct dns_pack));
	if(!dns_pack){
		printf("nocmem\n");
		exit(-ENOMEM);
	}
	dns_mgr->dns_packs = dns_pack;
	INIT_LIST_HEAD(&dns_mgr->pack_head);
	dns_mgr->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(-1 == dns_mgr->fd){
		error_at_line(1, errno, __FILE__, __LINE__, "[error] create dpu mgr fd failed" );
	}
	dns_mgr->epoll = epoll_create(nr);
	if(-1 == dns_mgr->epoll){
		error_at_line(1, errno, __FILE__, __LINE__, "[error] create dpu mgr epoll failed" );
	}
	dns_mgr->nsent = 0;
//	dns_mgr->pack_nr = nr;

	dns_mgr->ev.events = EPOLLIN;
	(dns_mgr->ev).data.ptr = dns_mgr;
	if (-1 == (epoll_ctl(dns_mgr->epoll, EPOLL_CTL_ADD, dns_mgr->fd, &dns_mgr->ev))){
		error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu mgr epoll_ctl failed" );
	}
	for(i=0; i<nr; i++){
		struct dns_pack *pack = dns_pack+i;
		INIT_LIST_HEAD(&pack->list);
		pack->dns = (struct dnshdr_s *)(pack->buf);
		pack->mgr = dns_mgr;
		id = pack->mgr->magic + pack->mgr->nsent++;
		pack->len = dns_fill_query(pack->dns, dns_mgr->dname, id, NS_FLAG_RD, NS_CLASS_IN, dns_mgr->ns_type);
		list_add(&pack->list, &pack->mgr->pack_head);
	}

}

void dns_send(struct dns_mgr *mgr)
{
	struct dns_pack *pack;
	struct sockaddr_in addr;
//	int len;
	list_for_each_entry_reverse(pack, &mgr->pack_head, list){
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = mgr->serv_addr;
		addr.sin_port = htons(53);
		if(gettimeofday(&pack->tv, NULL)){
			error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu pack timestamp failed" );
		}
		if(connect(pack->mgr->fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))){
			error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu pack connect" );
		}
//		sendto(pack->mgr->fd, (char *)(pack->dns), len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));
try:	
		if(-1 == write(pack->mgr->fd, (char *)(pack->dns), pack->len)){
			if(errno == EAGAIN || errno == EINTR){
				goto try;
			}else{
				error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu pack write" );
			}
		}
	}
}

void dns_poll(struct dns_mgr *mgr)
{
	int event_nr, i, n;
//	int j;
//	unsigned short id;
	struct epoll_event ev[65536];
	struct dns_mgr *ev_mgr;
	int recv;
	int pack_nr = mgr->pack_nr;
	char buf[DNS_BUFSIZE];
	struct dnshdr_s *dns=(struct dnshdr_s *)buf;
	struct sockaddr serv_addr;
	socklen_t sl = sizeof(struct sockaddr_in);
	int timeout = mgr->tm_out*1000;
	unsigned int matched = 0;

	while(pack_nr > 0){
		event_nr = epoll_wait(mgr->epoll, (struct epoll_event *)&ev, 100, timeout);
		if(event_nr < 0){
			if(errno == EAGAIN || errno == EINTR){
				continue;
			}else{
				error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu epoll wait failed");
			}
		}else if(event_nr == 0){
//			printf(".");
//			fflush(stdout);
//			printf("%s timeout\n", mgr->dpu->alias);

			break;
		}
//		printf("\tev nr:%d\n", event_nr);
		recv = 0;
		for(i=0;i<event_nr;i++){
			ev_mgr = (struct dns_mgr *)((ev[i]).data.ptr);
reread:			
			n =  recvfrom(ev_mgr->fd, &buf, DNS_BUFSIZE,  0, &serv_addr, &sl);
			if (n == 0) {
				printf("Read nothing");
			}else if(n<0){
				if(errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
					goto reread;
				else
					error_at_line(0, errno, __FILE__, __LINE__, "[warn] dpu epoll wait failed"); //############
			}else{
//				printf("dns thread0x%x,id:0x%x, seq:%d\n", ev_mgr->magic, id, dns->dns_seq);
				ev_mgr->dns_log->lu[matched].id=dns->id;
				gettimeofday(&ev_mgr->dns_log->lu[matched].tv,NULL);
				matched++;
				recv++;
			}
		}
		pack_nr -= recv;
	}
	mgr->dns_log->recv_pkg_nr=matched;
	if(-1 == epoll_ctl(mgr->epoll, EPOLL_CTL_DEL, mgr->fd, NULL)){
		error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu mgr epoll_ctl failed" );
	}
try1:
	if(-1 == close(mgr->fd)){
		if(errno == EAGAIN || errno == EINTR){
			goto try1;
		}else{
			error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu close fd failed");
		}
	}
try2:
	if(-1 == close(mgr->epoll)){
		if(errno == EAGAIN || errno == EINTR){
			goto try2;
		}else{
			error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu close epoll failed");
		}
	}
}

void dns_acc(struct dns_mgr *mgr)
{
	struct dns_log *log = mgr->dns_log;
	unsigned short lost = mgr->pack_nr - log->recv_pkg_nr;
	struct dns_pack *dns_pack = mgr->dns_packs;
	unsigned int diff;
	unsigned int delay_sum = 0;
	unsigned int delay_avg = 0;
	unsigned int health = 0;

	int i, j;
	for(j=0; j<mgr->pack_nr; j++)
		for(i=0 ; i<log->recv_pkg_nr; i++){
			if((dns_pack+j)->dns->id == (log->lu[i]).id){
				diff = tv_diff(&(log->lu[i].tv),&(dns_pack+j)->tv);
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
	mgr->dns_log->delay_sum = delay_sum;
	mgr->dns_log->delay_avg = delay_avg;
	mgr->dns_log->lost = lost;
	mgr->dns_log->health = health;
}
