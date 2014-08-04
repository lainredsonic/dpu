#ifndef __OPERATION_H__
#define __OPERATION_H__ 1

#include "dpu.h"

enum{
	DPU_DNAME,
	DPU_PKGNR,
	DPU_TMOUT,
	DPU_THRES,
};

void dpu_init();
int dpu_add(char *alias, char *ipaddr,
		char *dname, __be16 ns_type, unsigned short pkg_nr,
		unsigned short timeout, int health_thres
	);
int dpu_del(char *ipaddr);
int dpu_set(char *ipaddr, unsigned short type);
int dpu_health(char *ipaddr);
void dpu_destory(void);

#endif
