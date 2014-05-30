#ifndef __ICMP_H__
#define __ICMP_H__ 1

#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "dpu.h"
#include "list.h"

#define BUFSIZE 1500

struct lu{
	unsigned int nsent;
	struct timeval tv;
};

struct icmp_log{
	unsigned short recv_pkg_nr;
	struct lu lu[255];
};

struct icmp_mgr{
	int fd;
	struct list_head pack_head;
	unsigned int nsent;
	int poll;
	int pack_nr;
	int magic;
	struct epoll_event ev;
	struct icmp_pack *icmp_packs;
	struct icmp_log *icmp_log;
	struct dpu *dpu;
};

struct icmp_pack{
	struct icmp *icmp;
	struct list_head list;
	struct icmp_mgr *mgr;
	struct timeval tv;
	char buf[BUFSIZE];
};


struct icmp_mgr * icmp_gen(struct dpu *dpu);
void icmp_send(struct icmp_mgr *mgr);
void icmp_poll(struct icmp_mgr *mgr);
void icmp_clean(struct icmp_mgr *mgr);
void icmp_acc(struct icmp_mgr *mgr);

#endif
