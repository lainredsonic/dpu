#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#include "server.h"
#include "dpu.h"
#include "list.h"
#include "operation.h"


int listenfd;
int connfd;
pthread_t config_server_tid;
int dpu_exit;

void do_work(int listenfd)
{
	char buf[512];
	struct payload *config = (struct payload *)buf;
	ssize_t len;

	memset(buf, 0 ,512);

again:
	if((len=recvfrom(listenfd, buf, sizeof(struct payload), 0, NULL, NULL))<0){
		if(errno == EINTR)
			goto again;
		else
			error_at_line(1, errno, __FILE__, __LINE__, "[ERROR] config_server accept");
	}else if(len==0){
		error_at_line(0, errno, __FILE__, __LINE__, "[WARN] config_server recv len 0");
		return ;
	}

	switch(config->opt){
		case DPU_ADD:
			DPU_LOCK(&dpu_mutex);
			dpu_add(config->alias, config->alias, config->dname, config->type, config->nr, config->tmout, config->thres);
			DPU_UNLOCK(&dpu_mutex);
			break;
		case DPU_DEL:
			DPU_LOCK(&dpu_mutex);
			dpu_del(config->alias);
			DPU_UNLOCK(&dpu_mutex);
			break;
		default:
			break;
	}
	
}

void * config_task(void *args)
{
	listen(listenfd, 10);
	
	for(;;){
		if(dpu_exit)
			break;
		if((connfd=accept(listenfd, NULL, NULL))<0){
			if(errno == EINTR)
				continue;
			else
				error_at_line(1, errno, __FILE__, __LINE__, "[ERROR] config_server accept");
		}
/*
		if(fork()==0){
			close(listenfd);
*/
			do_work(connfd);
/*
			exit(0);
		}else{
			close(connfd);
		}
*/
		close(connfd);
	}
	pthread_exit(NULL);
}

void * config_task_dg(void *args)
{
	for(;;){
		if(dpu_exit)
			break;
		do_work(listenfd);
	}
	pthread_exit(NULL);
}

void config_server_init()
{
	struct sockaddr_un server_addr;
//	listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	listenfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	
	unlink(SOCK_PATH);
	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_LOCAL;
	strcpy(server_addr.sun_path, SOCK_PATH);
	if(bind(listenfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un))<0){
		error_at_line(1, errno, __FILE__, __LINE__, "[ERROR] config_server bind");
	}
	pthread_create_or_die(&config_server_tid, NULL, config_task_dg, NULL);
}

void config_server_destory()
{
	dpu_exit = 1;
	close(listenfd);
//	close(connfd);
}
