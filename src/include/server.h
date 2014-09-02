#ifndef __SERVER_H__
#define __SERVER_H__

#include <linux/types.h>
#define SOCK_PATH "/tmp/.dpu.sock"

enum{
	DPU_ADD=0,
	DPU_DEL,
	DPU_GET,
};

struct payload
{
	unsigned short opt;
	unsigned int ipaddr;
	unsigned short type;
	unsigned short nr;
	unsigned short tmout;
	unsigned short thres;
	int health;
	char dname[512];
	char alias[32];
}__attribute__ ((packed));

void do_work(int connfd);
void config_server_init();
void config_server_destory();

#endif
