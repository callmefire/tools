#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
	int svrsock, pid;
	int clisock;
	char c;
	int len;
	struct sockaddr_in servername, clientname;
	int i=0;
	int one=1;
	int port;

	opterr = 0;		
	while ( (c=getopt(argc,argv,"p:")) !=-1) {
		switch (c) {
			case 'p':
				if (optarg)
					port = atoi(optarg);
				else {
					printf("Usage:\n");
					printf("\ttestserver -p port\n");
					exit(1);
				}
				break;
			default:
				printf("Usage:-%c is not surported\n");
				exit(1);
		}
	}

	if (argc < 2) {
		printf("Usage:\n");
		printf("\ttestserver -p port\n");
	}
	
	pid = fork();

	if ( pid < 0 )
		exit(1);
	else if ( pid > 0 )
		exit(0);

	if ( setsid() < 0) {
		printf("can't daemonise\n");
		exit(1);
	}
	
	pid = fork();

	if ( pid < 0 )
		exit(1);
	else if ( pid > 0 )
		exit(0);
		
	svrsock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (svrsock < 0)
	{
		printf("create server socket failure!\n");
		exit(0);
	}

	if (setsockopt(svrsock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(one)) < 0) {
		printf("Cannot set SO_REUSEADDR option\n");
		exit(1);
	}
	
    bzero(&servername, sizeof(servername));
	servername.sin_family = AF_INET;
	servername.sin_port=htons(port);

	if (bind(svrsock, (struct sockaddr*)&servername, sizeof(servername))<0) {
		close(svrsock);
		printf("bind server socket error!\n");
		exit(0);
	}

	if (listen(svrsock, 5)<0) {
		close(svrsock);
		printf("server listen error!\n");
		exit(0);
	}

	while(1)
	{
		bzero(&clientname, sizeof(clientname));
		len = sizeof(clientname);
		clisock = accept(svrsock, (struct sockaddr*)&clientname, &len);
		if ( clisock < 0 ) {
			
			pid = fork();

			if ( pid < 0 ) {
				printf("fork new process failed\n");
				exit(1);
			} else if ( pid == 0 ) { 
				while (1) {
					close(svrsock);
				       	sleep(1000);
				}
			} else {
				int fd;
				printf("create new process %d\n",pid);
				for (fd=3;fd<1024;fd++) {
					if (fd != svrsock)
						close(fd);
				}
				continue;
			}
		}
		
		printf("sock fd = %d  -- conn succ times %d\n",clisock,++i);
	}

	return 0;
}

