#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define MAXLINE 4096
#define BUFFSIZE 8192
#define LISTENQ 1024
#define SERV_PORT 8080
#define PCKT_LEN 8192

int main(){

	int rsfd;
	struct sockaddr_in rawip;
	struct sockaddr_in raddr;
	int rawiplen, udpiplen, raddrlen;

	char msg[2048]; int msglen;

	struct iphdr hdrip;
	int iphdrlen = sizeof(hdrip);

	if((rsfd = socket(AF_INET, SOCK_RAW, 10)) < 0){  
		perror("Raw socket error!"); exit(0);
	}

	int reuse = 1;
	if(setsockopt(rsfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0){
		perror("Raw sockopt error!"); exit(0);
	}

	rawip.sin_family = AF_INET;
	rawip.sin_addr.s_addr = inet_addr("127.0.0.1");
	rawiplen = sizeof(rawip);

	if(bind(rsfd, (struct sockaddr*)&rawip, rawiplen) < 0){
		perror("Raw bind error!"); exit(0);
	}

	int i=0;
	while(1){
		raddrlen = sizeof(raddr);
		msglen = recvfrom(rsfd, msg, 2048, 0, (struct sockaddr*)&raddr, &raddrlen);

		if(msglen <= 0)continue;

		hdrip = *((struct iphdr*)msg);

		printf("Raw socket: %d\n", i);
		printf("hl: %d, version: %d, ttl: %d, protocol: %d\n", hdrip.ihl, hdrip.version, hdrip.ttl, hdrip.protocol);
		printf("src: %s\n", inet_ntoa(*((struct in_addr*)&hdrip.saddr)));
		printf("dest: %s\n", inet_ntoa(*((struct in_addr*)&hdrip.daddr)));

        struct hostent * host_entry = gethostbyaddr((char *) &hdrip.saddr,sizeof(hdrip.saddr),AF_INET);
        printf("Message came from : %s\n\n",host_entry->h_name);
		printf("\n");
		i++;
	}
	close(rsfd);
	return 0;
}