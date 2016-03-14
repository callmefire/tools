#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int svrsock, pid;
	int clisock;
	char c;
	int len;
	struct sockaddr_in servername, clientname;
	int i=0;
	int one=1;
	int port=8888;

	pid = fork();

	if ( pid < 0 )
		exit(1);
	else if ( pid > 0 )
		exit(0);

	if ( setsid() < 0) {
		printf("can't daemonise\n");
		exit(1);
	}
		
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
		
			printf("accept failed\n");	
			sleep(1);
			continue;
		}
	
		pid = fork();

		if ( pid < 0 ) {
			printf("fork error!\n");
			sleep(1);
			continue;
		}

		if ( pid == 0 ) {		// child process
			dup2(clisock, 0);
			dup2(clisock, 1);

			close(svrsock);
			
			while (1) {
				sleep(100);
			}
		}
	
		close(clisock); 
		printf("sock fd = %d  -- conn succ times %d!\n",clisock,++i);
	}

	return 0;
}

