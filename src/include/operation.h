#ifndef __OPERATION_H__
#define __OPERATION_H__ 1

enum{
	DPU_DNAME,
	DPU_PKGNR,
	DPU_TMOUT,
	DPU_THRES,
};

void dpu_init();
int dpu_add(char *alias, char *ipaddr,
		char *dname, unsigned short pkg_nr,
		unsigned short timeout, int health_thres
	);
int dpu_del(char *ipaddr);
int dpu_set(char *ipaddr, unsigned short type);
int dpu_health(char *ipaddr);

#endif
