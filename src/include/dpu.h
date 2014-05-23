#ifndef __DPU_H__
#define __DPU_H__ 1

#include <netinet/in.h>
#include "list.h"

struct dpu{
	char alias[256];
	char dname[256];
	in_addr_t serv_addr;
	unsigned short pkg_nr;
	unsigned short tm_out;
	int health;
	int health_thres;
	struct list_head lh;
};

extern struct list_head dpu_list;

#endif
