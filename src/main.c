#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "operation.h"
#include "icmp.h"
#include "dpu.h"


void * dpu_task(void *args){
	in_addr_t s_addr = *((in_addr_t *)args);
	struct icmp_mgr *mgr = NULL;
	mgr = icmp_gen(1);
	icmp_send(mgr, s_addr);
	icmp_poll(mgr);
	return 0;
}

void probe_loop(){
	pthread_t tid;
	struct dpu *tmp_dpu;
	in_addr_t s_addr[255];
	int i=0;

	list_for_each_entry(tmp_dpu, &dpu_list, lh){
		s_addr[i] = tmp_dpu->serv_addr;
		pthread_create(&tid, NULL, dpu_task, (void *)&s_addr[i]);	
		pthread_detach(tid);
		i++;
	}
}

int main(int argc,char **argv){

	dpu_init();
	dpu_add("group2", "114.114.114.114", "www.google.com", 10, 10, 50);
	dpu_add("group1", "192.168.6.51", "www.baidu.com", 20, 30, 40);
	dpu_add("group3", "192.168.6.52", "www.baidu.com", 20, 30, 40);
	dpu_add("group4", "8.8.4.4", "www.baidu.com", 20, 30, 40);
	dpu_add("group5", "8.8.8.8", "www.baidu.com", 20, 30, 40);
	probe_loop();
	sleep(20);
	return 0;
}
