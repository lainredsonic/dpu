#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <libgen.h>

#include "dpu.h"
#include "operation.h"
#include "dns_d.h"
#include "clib/log.h"

void usage(char *cmd){
	char *cmdname = basename(cmd);
	printf("usage: %s [-c count] [-t timeout] [-i interval] [-l loops] <-n domain_name> <name servers>\n", cmdname);
	exit(0);
}

void * dpu_task(void *args){
	struct dns_mgr *mgr = (struct dns_mgr *)args;
	dns_gen(mgr);
	dns_send(mgr);
	dns_poll(mgr);
	dns_acc(mgr);
	dns_clean(mgr);
	return 0;
}

void probe_loop(){
	struct dpu *dpu;
	struct dns_mgr *mgr = NULL;
	struct list_head *pos, *n;
	struct tm *tm;
	time_t ct;
	char tmstr[64];

	DPU_LOCK(&dpu_mutex);
	list_for_each_entry(dpu, &dpu_list, lh){
		if(!(mgr = (struct dns_mgr *)malloc(sizeof(struct dns_mgr)))){
			error_at_line(1, errno, __FILE__, __LINE__, "[error] alloc dpu mgr failed");
		}
		strcpy(mgr->alias, dpu->alias);
		strcpy(mgr->dname, dpu->dname);
		mgr->serv_addr = dpu->serv_addr;
		mgr->tm_out = dpu->tm_out;
		mgr->pack_nr = dpu->pkg_nr;
		mgr->ns_type = dpu->ns_type;
		list_add(&mgr->mh, &dns_mgr_list);
	}
	DPU_UNLOCK(&dpu_mutex);

	list_for_each_entry(mgr, &dns_mgr_list, mh){
		pthread_create_or_die(&mgr->tid, NULL, dpu_task, (void *)mgr);
//		pthread_detach(mgr->tid);
	}

	list_for_each_safe(pos, n, &dns_mgr_list){
		mgr = list_entry(pos, struct dns_mgr, mh);
		pthread_join_or_warn(mgr->tid, NULL);
		time(&ct);
		tm = localtime(&ct);
		strftime(tmstr, 64, "%F %T",tm);
		printf("[%-10s] %-15s lost:%8u, delay:%8u, health:%8u\n", tmstr, mgr->alias, mgr->dns_log->lost, mgr->dns_log->delay_avg, mgr->dns_log->health);
		LOG(LOG_LEVEL_TRACE, "[%-10s] %-15s lost:%8u, delay:%8u, health:%8u", tmstr, mgr->alias, mgr->dns_log->lost, mgr->dns_log->delay_avg, mgr->dns_log->health);

		DPU_LOCK(&dpu_mutex);
		list_for_each_entry(dpu, &dpu_list, lh){
			if(!strcmp(dpu->dname, mgr->dname)){
				dpu->health = mgr->dns_log->health;
			}
		}
		DPU_UNLOCK(&dpu_mutex);
		list_del(&mgr->mh);
		free(mgr);
	}

}

int main(int argc,char **argv){

	int count=1, opt;
	int tmout=5, inv=0, loops=0;
	int server_count;
	int i;
	char dname[255];
	int dname_none=1;

	while ((opt = getopt(argc, argv, "c:t:i:l:n:"))!=-1){
		switch(opt){
			case 'c':
				count = atoi(optarg);
				break;
			case 't':
				tmout = atoi(optarg);
				break;
			case 'i':
				inv = atoi(optarg);
				break;
			case 'l':
				loops = atoi(optarg);
				break;
			case 'n':
				strcpy(dname, optarg);
				dname_none=0;
				break;
			default:
				usage(argv[0]);
		}
	}
	if(dname_none)
		usage(argv[0]);

	server_count = argc - optind;
	if(server_count <= 0)
		usage(argv[0]);


	g_default_log = log_new_stdout(LOG_LEVEL_TRACE);
	const char *log_prefix = "dpu";
	char log_tmp[PATH_MAX] = {};
	snprintf(log_tmp, PATH_MAX, "%s/%s%d",
		"/var/log/dpu",
		log_prefix, 1
	);
	g_default_log = log_new_rolling_file(LOG_LEVEL_TRACE, log_tmp,
		10,
		5000 * 1024
	);



	dpu_init();
	INIT_LIST_HEAD(&dns_mgr_list);

/* 		alias	|	ip	|dname|	nr|tmout|threas*/
//	dpu_add("group2", "114.114.114.114", "www.google.com", NS_TYPE_A, 20, 2, 50);
//	dpu_add("group7", "8.8.8.8", "www.google.com", NS_TYPE_A, 20, 5 ,50);
////	dpu_add("group1", "192.168.6.51", "www.baidu.com", NS_TYPE_A, 20, 5, 40);
////	dpu_add("group3", "192.168.6.52", "www.baidu.com", NS_TYPE_A, 20, 5, 40);
//	dpu_add("group4", "8.8.4.4", "www.baidu.com", NS_TYPE_A, 20, 5, 40);
////	dpu_add("group5", "8.8.8.5", "www.baidu.com", NS_TYPE_A, 20, 5, 40);
//	dpu_add("group6", "192.168.6.179", "www.baidu.com", NS_TYPE_A, 100, 5, 40);
////	dpu_add("group8", "192.168.13.1", "www.baidu.com", NS_TYPE_A, 100, 5, 40);
	for(i=0; i<server_count; i++)
		if(dpu_add(argv[i+optind], argv[i+optind], dname, NS_TYPE_A, count, tmout, 2000)){
			error_at_line(1, errno, __FILE__, __LINE__, "[error] dpu add dname");
		}

	if(loops == 0){
		for(;;){
			probe_loop();
			sleep(inv);
		}
	}
	else{
		for(i=0; i<loops; i++){
			probe_loop();
			sleep(inv);
		}
	}
	dpu_destory();
	return 0;
}
