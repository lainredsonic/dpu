#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include "config_tool.h"
#include "server.h"


int main(int argc, char **argv)
{
	struct payload config;
	int sockfd;
	struct sockaddr_un serveraddr;
	struct sockaddr_un clientaddr;
	
	struct in_addr bindaddr;
	void *buf = &config;

	if(argc != 4){
		printf("%s <opt> <dname> <server ip>\n", basename(argv[0]));
		exit(1);
	}
	
	unlink(SOCK_CLIENT_PATH);
	
//	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);

	memset(&clientaddr, 0, sizeof(struct sockaddr_un));
	clientaddr.sun_family = AF_LOCAL;
	strcpy(clientaddr.sun_path, SOCK_CLIENT_PATH);

	if(bind(sockfd, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr_un))<0){
		error_at_line(1, errno, __FILE__, __LINE__, "[ERROR] config_server bind");
	}


	memset(&serveraddr, 0, sizeof(struct sockaddr_un));
	serveraddr.sun_family = AF_LOCAL;
	strcpy(serveraddr.sun_path, SOCK_PATH);
	
//	connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_un));

	strcpy(config.alias, argv[3]);
	inet_pton(AF_INET, argv[3], &bindaddr);

	config.type = htons(1);
	config.tmout = 1;
	config.nr = 5;
	config.thres = 1000;
	strcpy(config.dname, argv[2]);
	config.ipaddr = bindaddr.s_addr;

	if(!strcmp(argv[1],"add")){
		config.opt = DPU_ADD;
	}else if(!(strcmp(argv[1], "del"))){
		config.opt = DPU_DEL;
	}else if(!(strcmp(argv[1], "get"))){
		config.opt = DPU_GET;
	}else{
		printf("unknown opt type\n");
		exit(1);
	}

	if(sendto(sockfd, buf, sizeof(struct payload), 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_un))<0){
		error_at_line(1, errno, __FILE__, __LINE__, "[ERROR] config_server send failed");
	}
	if(config.opt == DPU_GET){
		if(recvfrom(sockfd, buf, sizeof(struct payload), 0, NULL, NULL)<0){
			error_at_line(1, errno, __FILE__, __LINE__, "[ERROR] config_server recv failed");
		}
		printf("health: %d\n", config.health);
	}
	return 0;
}
