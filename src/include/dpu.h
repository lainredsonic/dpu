#ifndef __DPU_H__
#define __DPU_H__ 1

#include <netinet/in.h>
#include <pthread.h>

#include "list.h"

/* Dns Probe Unit
 * only activity for interface
 * all dpu operation must be protect
 */

struct dpu{
	char alias[256];	/* alias name for traget server */
	char dname[256];	/* human readable domain name */
	in_addr_t serv_addr;	/* target server ipv4 addr */
	unsigned short pkg_nr;	/* number of probe packet */ 
	unsigned short tm_out;	/* timeout of probe response */
	int health;		/* health value */
	int health_thres;
	struct list_head lh;
};

extern struct list_head dpu_list; /* globle dpu list */
extern pthread_mutex_t dpu_mutex;


#endif
