#ifndef __ICMP_H__
#define __ICMP_H__ 1

#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "list.h"

#define BUFSIZE 1500

struct lu{
	unsigned int nsent;	/* icmp packet recv seq id */
	struct timeval tv;	/* icmp packet recv timeval */
};

struct icmp_log{
	unsigned short recv_pkg_nr;	/* total recved icmp packet number */
	struct lu lu[255];
	unsigned int delay_sum;
	unsigned int delay_avg;
	unsigned int lost;
	unsigned int health; 
};

struct icmp_mgr{
	int fd;				/* socket fd */
	char alias[256];
	char dname[256];
	in_addr_t serv_addr;
	unsigned short tm_out;
	struct list_head pack_head;
	unsigned int nsent;		/* icmp packet seq */
	int poll;			/* epoll fd */		
	unsigned short pack_nr;			/* number of packet to send */
	int magic;			/* icmp packet id, same as tid */
	struct epoll_event ev;
	struct icmp_pack *icmp_packs;	/* icmp packets to be send */
	struct icmp_log *icmp_log;	/* icmp recv log */
	pthread_t tid;
	struct list_head mh;
};

struct icmp_pack{
	struct icmp *icmp;
	struct list_head list;
	struct icmp_mgr *mgr;
	struct timeval tv;	/* icmp packet send timeval */
	char buf[BUFSIZE];	/* icmp packet send buffer */
};


void icmp_gen(struct icmp_mgr *mgr);
void icmp_send(struct icmp_mgr *mgr);
void icmp_poll(struct icmp_mgr *mgr);
void icmp_clean(struct icmp_mgr *mgr);
void icmp_acc(struct icmp_mgr *mgr);

extern struct list_head mgr_list;

#endif
