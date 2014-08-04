#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "operation.h"
#include "icmp.h"
#include "dpu.h"


void * dpu_task(void *args){
	struct icmp_mgr *mgr = (struct icmp_mgr *)args;
	icmp_gen(mgr);
	icmp_send(mgr);
	icmp_poll(mgr);
	icmp_acc(mgr);
	icmp_clean(mgr);
	return 0;
}

void probe_loop(){
	struct dpu *dpu;
	struct icmp_mgr *mgr;
	struct list_head *pos, *n;

	pthread_mutex_lock(&dpu_mutex);
	list_for_each_entry(dpu, &dpu_list, lh){
		mgr = (struct icmp_mgr *)malloc(sizeof(struct icmp_mgr));
		strcpy(mgr->alias, dpu->alias);
		strcpy(mgr->dname, dpu->dname);
		mgr->serv_addr = dpu->serv_addr;
		mgr->tm_out = dpu->tm_out;
		mgr->pack_nr = dpu->pkg_nr;
		list_add(&mgr->mh, &mgr_list);
	}
	pthread_mutex_unlock(&dpu_mutex);

	list_for_each_entry(mgr, &mgr_list, mh){
		pthread_create(&mgr->tid, NULL, dpu_task, (void *)mgr);
//		pthread_detach(mgr->tid);
	}

	list_for_each_safe(pos, n, &mgr_list){
		mgr = list_entry(pos, struct icmp_mgr, mh);
		pthread_join(mgr->tid, NULL);
		printf("%s lost:%u, delay:%u, health:%u\n", mgr->alias, mgr->icmp_log->lost, mgr->icmp_log->delay_avg, mgr->icmp_log->health);
		pthread_mutex_lock(&dpu_mutex);
		list_for_each_entry(dpu, &dpu_list, lh){
			if(!strcmp(dpu->dname, mgr->dname)){
				dpu->health = mgr->icmp_log->health;
			}
		}
		pthread_mutex_unlock(&dpu_mutex);
		list_del(&mgr->mh);
		free(mgr);
	}

}

int main(int argc,char **argv){

	dpu_init();
	INIT_LIST_HEAD(&mgr_list);

/* 		alias	|	ip	|dname|	nr|tmout|threas*/
	dpu_add("group2", "114.114.114.114", "www.google.com", 20, 2, 50);
	dpu_add("group7", "8.8.8.8", "www.google.com", 20, 5 ,50);
//	dpu_add("group1", "192.168.6.51", "www.baidu.com", 20, 5, 40);
//	dpu_add("group3", "192.168.6.52", "www.baidu.com", 20, 5, 40);
	dpu_add("group4", "8.8.4.4", "www.baidu.com", 20, 5, 40);
	dpu_add("group5", "8.8.8.5", "www.baidu.com", 20, 5, 40);
//	dpu_add("group6", "192.168.6.179", "www.baidu.com", 100, 5, 40);
	dpu_add("group8", "192.168.13.1", "www.baidu.com", 100, 5, 40);
	probe_loop();
//	sleep(3600);
	dpu_destory();
	return 0;
}
