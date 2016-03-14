#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
	int pid=0;
	int clisock = 0;
	int i,j;
	char serveraddr[INET_ADDRSTRLEN];
	char clientaddr[INET_ADDRSTRLEN];
	struct sockaddr_in	servername;
	struct sockaddr_in	clientname;
	int count;
	int port;
	char c;

	opterr = 0;
	while ( (c=getopt(argc,argv,"p:c:")) != -1) {
		switch (c) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'c':
				count = atoi(optarg);
				break;
			default:
				printf("Usage:-%c option is not surport\n",optopt);
				exit(1);
		}
	}
	
	if (opterr > 0 || argc < 3) {
		printf("Usage:\n");
		printf("\ttestclient -c count -p port dst\n");
		exit(1);
	}
	
	if (optind < argc) 
		strcpy(serveraddr,argv[optind]);
	
	bzero(&servername, sizeof(servername));
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(serveraddr);
	servername.sin_port=htons(port);

	i = 0;
	while (i < count ) {
		
		clisock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (clisock < 0) {

			pid = fork();
			if ( pid < 0 ) {
				printf("fork new process failed\n");
				exit(1);
			}else if ( pid > 0 ) {
				wait(NULL);
				printf("process %d finished\n",(int)getpid());
				exit(1);
			}else {
				int fd;
				printf("create new process\n");
				for (fd = 3; fd < 1024;fd++)
					close(fd);
				continue;	
			}
		}
			
		if (connect(clisock, (struct sockaddr*)&servername,sizeof(servername)) != 0) {
			printf("connect socket failure!\nmay be the server runs down\n");
			sleep(1);
			continue;
		}

		printf("sock id = %d -- conn num = %d\n", clisock, ++i);
	}

	while (1) {
		printf("%d conns finished\nCtrl+C to Abort.\n",i);
		sleep(10);
	}

	return 0;
}
