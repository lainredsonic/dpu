#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "list.h"
#include "dpu.h"
#include "operation.h"

struct list_head dpu_list;
//pthread_mutex_t dpu_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dpu_mutex;

void dpu_init(){
	INIT_LIST_HEAD(&dpu_list);
	pthread_mutex_init(&dpu_mutex, NULL);
}

int dpu_add(char *alias, char *serv_addr,
		char *dname, unsigned short pkg_nr,
		unsigned short timeout, int health_thres)
{
	struct dpu *tmp_dpu;
	struct in_addr in_addr;
	inet_pton(AF_INET, serv_addr, &in_addr);
	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		if(in_addr.s_addr == tmp_dpu->serv_addr){
//			return -1;
		}
	}
	tmp_dpu = (struct dpu *)malloc(sizeof(struct dpu));
	INIT_LIST_HEAD(&tmp_dpu->lh);
	strcpy(tmp_dpu->alias, alias);
	tmp_dpu->serv_addr = in_addr.s_addr;
	strcpy(tmp_dpu->dname, dname);
	tmp_dpu->pkg_nr = pkg_nr;
	tmp_dpu->tm_out = timeout;
	tmp_dpu->health_thres = health_thres;
	list_add(&tmp_dpu->lh, &dpu_list);
	return 0;
}

int dpu_del(char *serv_addr){
	struct dpu *tmp_dpu, *find_dpu=NULL;
	in_addr_t addr;
	inet_pton(AF_INET, serv_addr, &addr);
	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		if(tmp_dpu->serv_addr == addr){
			find_dpu=tmp_dpu;
			break;
		}
	}
	if(find_dpu){
		list_del(&find_dpu->lh);
		free(find_dpu);
		return 0;
	}else
		return -1;
}

int dpu_set(char *serv_addr, unsigned short type){
	struct dpu *tmp_dpu, *find_dpu=NULL;
	in_addr_t addr;
	inet_pton(AF_INET, serv_addr, &addr);
	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		if(tmp_dpu->serv_addr == addr){
			find_dpu=tmp_dpu;
			break;
		}
	}
	if(!find_dpu)
		return -1;

	switch(type){
	case DPU_DNAME:
//		......
		break;
	case DPU_PKGNR:
//		......
		break;
	case DPU_TMOUT:
//		......
		break;
	case DPU_THRES:
//		......
		break;
	}
	return 0;
}

int dpu_health(char *serv_addr){
	struct dpu *tmp_dpu, *find_dpu=NULL;
	in_addr_t addr;
	inet_pton(AF_INET, serv_addr, &addr);
	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		if(tmp_dpu->serv_addr == addr){
			find_dpu=tmp_dpu;
			break;
		}
	}
	if(!find_dpu)
		return -1;
	return find_dpu->health;
}
