#ifndef __DPU_H__
#define __DPU_H__ 1

#include <netinet/in.h>
#include <pthread.h>
#include <linux/types.h>
#include <error.h>
#include <errno.h>

#include "list.h"

/* Dns Probe Unit
 * only activity for interface
 * all dpu operation must be protect
 */

struct dpu{
	char alias[256];	/* alias name for target server */
	char dname[256];	/* human readable domain name */
	in_addr_t serv_addr;	/* target server ipv4 addr */
	__be16 ns_type;
	unsigned short pkg_nr;	/* number of probe packet */ 
	unsigned short tm_out;	/* timeout of probe response */
	int health;		/* health value */
	int health_thres;
	struct list_head lh;
};

extern pthread_mutexattr_t mutex_attr;
extern struct list_head dpu_list; /* globle dpu list */
extern pthread_mutex_t dpu_mutex;

static inline void pthread_mutex_lock_or_die(pthread_mutex_t * mutex)
{
	if((pthread_mutex_lock(mutex))){
		error_at_line(1, errno, __FILE__, __LINE__, "dpu_lock");	
	}
}

static inline void pthread_mutex_unlock_or_die(pthread_mutex_t * mutex)
{
	if((pthread_mutex_unlock(mutex))){
		error_at_line(1, errno, __FILE__, __LINE__, "dpu_unlock");	
	}
}

static inline void pthread_create_or_die(pthread_t *thread, const pthread_attr_t *attr, void *(*start_route)(void *), void *arg)
{
	if(pthread_create(thread, attr, start_route, arg)){
		error_at_line(1, errno, __FILE__, __LINE__, "dpu mgr task create");
	}
}

static inline void pthread_join_or_warn(pthread_t thread, void **retval)
{
	if(pthread_join(thread, retval)){
		error_at_line(0, errno, __FILE__, __LINE__, "dpu mgr task not found");
	}
}

#define DPU_LOCK(a) pthread_mutex_lock_or_die(a)
#define DPU_UNLOCK(a) pthread_mutex_unlock_or_die(a)


#endif
