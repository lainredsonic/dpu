#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "operation.h"
#include "icmp.h"
#include "dpu.h"


void * dpu_task2(void *args){
	struct sockaddr_in addr;
	inet_pton(AF_INET, "192.168.13.1", &addr.sin_addr);

	struct icmp_mgr *mgr = (struct icmp_mgr *)args;
	icmp_send(mgr, addr.sin_addr.s_addr);
	icmp_poll(mgr);
	return mgr;
}

void probe_loop2(){
	pthread_t tid;
	int i=0;
	int nr = 1;
	struct icmp_mgr *mgr;

	for(;i<2;i++){
		mgr = icmp_gen(nr); 
		pthread_create(&tid, NULL, dpu_task2, (void *)mgr);	
		pthread_detach(tid);
	}
}

void * dpu_task(void *args){
	in_addr_t s_addr = *((in_addr_t *)args);
	struct icmp_mgr *mgr = NULL;
	mgr = icmp_gen(1);
	icmp_send(mgr, s_addr);
	icmp_poll(mgr);
//	printf("parm:%s, pid:%u\n", p, (unsigned int)pthread_self());
	return 0;
}

void probe_loop(){
	pthread_t tid;
	struct dpu *tmp_dpu;
	in_addr_t s_addr[255];
//	struct in_addr addr[255];
	int i=0;
/*
	for(;i<2;i++){
		inet_pton(AF_INET, "192.168.13.1", &addr[i]);
		pthread_create(&tid, NULL, dpu_task, (void *)&(addr[i].s_addr));	
		pthread_detach(tid);
	}
*/

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
	probe_loop();
	sleep(20);
	return 0;
}
