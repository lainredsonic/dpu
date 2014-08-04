#ifndef __DNS_D_H__
#define __DNS_D_H__ 1

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "list.h"
#include "dns.h"


#define DNS_BUFSIZE 1500
struct dns_lu{
	unsigned int id;	/* dns packet recv seq id */
	struct timeval tv;	/* dns packet recv timeval */
};

struct dns_log{
	unsigned short recv_pkg_nr;	/* total recved dns packet number */
	struct dns_lu lu[255];
	unsigned int delay_sum;
	unsigned int delay_avg;
	unsigned int lost;
	unsigned int health; 
};

struct dns_mgr{
	int fd;				/* socket fd */
	int epoll;			/* epoll fd */		
	pthread_t tid;
	in_addr_t serv_addr;
	int magic;			/* dns packet id */
	char alias[256];
	char dname[256];		/* domain name */
	__be16 ns_type;			/* domain type */
	unsigned short tm_out;
	unsigned short pack_nr;		/* number of packet to send */
	unsigned int nsent;		/* dns packet seq */
	struct list_head pack_head;
	struct epoll_event ev;
	struct dns_pack *dns_packs;	/* dns packets to be send */
	struct dns_log *dns_log;	/* dns recv log */
	struct list_head mh;
};

struct dns_pack{
	struct dnshdr_s *dns;
	struct list_head list;
	struct dns_mgr *mgr;
	struct timeval tv;	/* dns packet send timeval */
	char buf[DNS_BUFSIZE];	/* dns packet send buffer */
	size_t len;		/* dns payload length */
};


void dns_gen(struct dns_mgr *mgr);
void dns_send(struct dns_mgr *mgr);
void dns_poll(struct dns_mgr *mgr);
void dns_clean(struct dns_mgr *mgr);
void dns_acc(struct dns_mgr *mgr);

extern struct list_head dns_mgr_list;


#endif
