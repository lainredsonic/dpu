#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "operation.h"
#include "icmp.h"
#include "dpu.h"


void * dpu_task(void *args){
	struct dpu *dpu = (struct dpu *)args;
	struct icmp_mgr *mgr = NULL;
	mgr = icmp_gen(dpu->pkg_nr);
	icmp_send(mgr, dpu->serv_addr);
	icmp_poll(mgr);
	icmp_clean(mgr);
	return 0;
}

void probe_loop(){
	struct dpu *tmp_dpu;

	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		pthread_create(&tmp_dpu->tid, NULL, dpu_task, (void *)tmp_dpu);
//		pthread_detach(tmp_dpu->tid);
	}
	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		pthread_join(tmp_dpu->tid, NULL);
	}
}

int main(int argc,char **argv){

	dpu_init();

/* 		alias	|	ip	|dname|	nr|tmout|threas*/
//	dpu_add("group2", "114.114.114.114", "www.google.com", 2, 10, 50);
	dpu_add("group1", "192.168.6.51", "www.baidu.com", 2, 30, 40);
	dpu_add("group3", "192.168.6.52", "www.baidu.com", 2, 30, 40);
//	dpu_add("group4", "8.8.4.4", "www.baidu.com", 20, 20, 40);
//	dpu_add("group5", "8.8.8.8", "www.baidu.com", 20, 30, 40);
//	dpu_add("group6", "192.168.6.33", "www.baidu.com", 100, 30, 40);
	probe_loop();
	return 0;
}
