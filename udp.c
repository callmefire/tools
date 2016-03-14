#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <getopt.h>
#include <signal.h>

typedef struct {
	unsigned int id;
	unsigned int cnt;
	unsigned int size;
	unsigned int last;
} tag_t;

tag_t tags[256];

unsigned int family = 4;
unsigned int mode = 0;
unsigned int total = 0;				/* pkt count */
unsigned int interval = 1000;			/* us */
unsigned int size = 100;			/* pkt size */
struct in_addr ipaddr;
struct in6_addr ipaddr6;
unsigned short port = 6000;
unsigned char tag = 0;
char buf[65536];
unsigned int id;

void usage(void)
{
	printf("Usage:\n");
	printf("\t-a <family:4 or 6>\n");
	printf("\t-m <mode>\n");
	printf("\t\t server - run in server mode\n");
	printf("\t\t client - run in client mode\n");
	printf("\t-d <destination ip address>\n");
	printf("\t-p <port number>\n");
	printf("\t-i <interval us>\n");
	printf("\t-c <packet count>\n");
	printf("\t-s <packet size: 8 - 65535>\n");
	printf("\t-t <packet tag>\n");
	printf("\t-h help\n");
	printf("\t-v version\n");
}

void client(void)
{
	struct sockaddr_in sr;
	struct timeval tv1,tv2;
	int sd;
	unsigned int cnt = 0;
	unsigned int dur_time;
	unsigned int delta;
	int ret;

	memset(buf,0,sizeof(buf));
	*(char *)(buf+4) = tag;
	
	sd = socket(AF_INET,SOCK_DGRAM,0);

	sr.sin_family = AF_INET;
	sr.sin_port = htons(port);
	sr.sin_addr.s_addr = ipaddr.s_addr;
	bzero(&sr.sin_zero,8);
	
	if (connect(sd,(struct sockaddr *)&sr,sizeof(sr)) < 0) {
		perror("connect error\n");
		exit(1);
	}

	gettimeofday(&tv1,NULL);
	usleep(0);
	gettimeofday(&tv2,NULL);
	delta = (tv2.tv_usec - tv1.tv_usec) * 2;


	gettimeofday(&tv1,NULL);	
	while( (cnt < total) || (total == 0)) {
		*(unsigned int *)buf = htonl(cnt+1);
		ret = sendto(sd,buf,size,0,(struct sockaddr *)&sr,sizeof(sr));
		gettimeofday(&tv2,NULL);
		if (tv1.tv_sec == tv2.tv_sec) {
			dur_time = tv2.tv_usec - tv1.tv_usec + delta;
		} else {
			dur_time = 1000000 - tv1.tv_usec + tv2.tv_usec + delta;
		}
		if (dur_time < interval) {
			usleep(interval - dur_time);
		}
		gettimeofday(&tv1,NULL);
		cnt++;
	}
	close(sd);
	printf("Send %u packets with size %u Bytes to %s\n", cnt, size, inet_ntoa(ipaddr));
}

void client6(void)
{
	struct sockaddr_in6 sr;
	struct timeval tv1,tv2;
	int sd;
	unsigned int cnt = 0;
	unsigned int dur_time;
	unsigned int delta;
	int ret;
	char ip6str[INET6_ADDRSTRLEN];

	memset(buf,0,sizeof(buf));
	*(char *)(buf+4) = tag;
	
	sd = socket(AF_INET6,SOCK_DGRAM,0);

	sr.sin6_family = AF_INET6;
	sr.sin6_port = htons(port);
	sr.sin6_addr = ipaddr6;
	
	if (connect(sd,(struct sockaddr *)&sr,sizeof(sr)) < 0) {
		perror("connect error\n");
		exit(1);
	}

	gettimeofday(&tv1,NULL);
	usleep(0);
	gettimeofday(&tv2,NULL);
	delta = (tv2.tv_usec - tv1.tv_usec) * 2;

	gettimeofday(&tv1,NULL);	
	while( (cnt < total) || (total == 0)) {
		*(unsigned int *)buf = htonl(cnt+1);
		ret = sendto(sd,buf,size,0,(struct sockaddr *)&sr,sizeof(sr));
		gettimeofday(&tv2,NULL);
		if (tv1.tv_sec == tv2.tv_sec) {
			dur_time = tv2.tv_usec - tv1.tv_usec + delta;
		} else {
			dur_time = 1000000 - tv1.tv_usec + tv2.tv_usec + delta;
		}
		if (dur_time < interval) {
			usleep(interval - dur_time);
		}
		gettimeofday(&tv1,NULL);
		cnt++;
	}
	close(sd);
	printf("Send %u packets with size %u Bytes to %s\n", cnt, size, inet_ntop(AF_INET6,&ipaddr6,ip6str,INET6_ADDRSTRLEN));
}


static void alarm_handler(int signo)
{
	int i;
	static unsigned int sec = 0;
	unsigned int mbs = 0;

	system("clear");
	sec += 2;
			printf("\n################################### %08u s ##################################\n",sec);
			printf("#                                                                               #\n");
	for(i=0; i<256; i++) {
		if (tags[i].cnt) {
			printf("#  %s stream %03d: received/total %010u/%010u packets, size %5u Bytes #\n",(tags[i].cnt == tags[i].last)?" ":"*",
					i,tags[i].cnt,tags[i].id,tags[i].size);
		}

		if (tags[i].cnt != tags[i].last) {
			mbs += ((tags[i].cnt - tags[i].last) * tags[i].size) >> 11; /* 2KB */
			tags[i].last = tags[i].cnt;
		}
	}
			printf("#                                                                               #\n");
			printf("#############################  Total %8u KB/s ##############################\n",mbs);

	alarm(2);
}

void server(void)
{
	int sd;
	struct sockaddr_in sl;
	int ret;
	int len;

	memset(buf,0,sizeof(buf));
	memset(tags,0,sizeof(tags));
	
	sd = socket(AF_INET,SOCK_DGRAM,0);

	sl.sin_family = AF_INET;
	sl.sin_port = htons(port);
	sl.sin_addr.s_addr = INADDR_ANY;
	bzero(&sl.sin_zero,8);

	if (bind(sd,(struct sockaddr *)&sl,sizeof(sl)) < 0) {
		perror("bind error");
		exit(1);
	}

	if (signal(SIGALRM,alarm_handler) == SIG_ERR) {
       perror("register signal ALARM failed\n");
       exit(1);
    }

	alarm(2);

	len = sizeof(sl);
	while (1) {
		if((ret = recvfrom(sd,buf,sizeof(buf),0,(struct sockaddr *)&sl,&len)) < 0) {
			continue;
		}
		tag = *(unsigned char *)(buf+4);
		tags[tag].id = ntohl(*(unsigned int *)buf);
		if (tags[tag].id == 1) {	/* reset */
			tags[tag].cnt = 0;
			tags[tag].size = 0;
			tags[tag].last = 0;
		}
		tags[tag].cnt++;
		tags[tag].size = ret;
	}	

	close(sd);
}

void server6(void)
{

}

int main(int argc,char **argv)
{

	struct option long_options[] = {
           {"family",   1, 0, 'a'},
           {"port",     1, 0, 'p'},
           {"mode",     1, 0, 'm'},
           {"interval", 1, 0, 'i'},
           {"size",     1, 0, 's'},
           {"count",    1, 0, 'c'},
           {"dest",     1, 0, 'd'},
           {"tag",      1, 0, 't'},
           {"help",     0, 0, 'h'},
           {"version",  0, 0, 'v'},
           {0,          0, 0, 0  }
	};
	int option_index = 0;
	signed char c;

	while ( (c = getopt_long(argc,argv,"a:m:p:i:s:c:d:t:hv",long_options,&option_index)) > 0) {
		char *p;
	
		switch (c) {
			case 'a':
				if (optarg[0] == '4') {
					family = 4;
				} else if (optarg[0] == '6') {
					family = 6;
				}
				break;
			case 'm':
				if (optarg[0] == 's') {
					mode = 1;
				} else if (optarg[0] == 'c') {
					mode = 0;
				} else {
					printf("'mode' must be appointed\n");
					exit(1);
				}
				break;
			case 'i':
				if ( (interval = atoi(optarg)) == 0) {
					printf("Invalid interval value\n");
					exit(1);
				}
				break;
			case 'p':
				if ( (atoi(optarg) <= 0) || (atoi(optarg) > 65535) ) {
					printf("Invalid port number\n");
					exit(1);
				}
				port = atoi(optarg);
				break;
			case 's':
				if ( (size = atoi(optarg)) == 0) {
					printf("Invalid pakcet size\n");
					exit(1);
				}
				break;
			case 'c':
				if ( (total = atoi(optarg)) == 0) {
					printf("Invalid pakcet count\n");
					exit(1);
				}
				break;
			case 'd':
				if (family == 4) {
					if (inet_aton(optarg,&ipaddr) == 0) {
						perror("Invalid ip address");
						exit(1);
					}
				} else {	
					if (inet_pton(AF_INET6, optarg, &ipaddr6) <= 0) {
						perror("Invalid ipv6 address");
						exit(1);
					}
				} 
				break;
			case 't':
				if (  (atoi(optarg) < 0) || (atoi(optarg) > 255)) {
					printf("Invalid tag value\n");
					exit(1);
				}
				tag = (unsigned char)atoi(optarg);
				break;
			case 'v':
				printf("udp suite v1.0\n");
				return 0;
			case 'h':
			default:
				usage();
				exit(0);
		}
	}
	
	if (mode) {
		if (family == 4) 
			server();
		else
			server6();
	} else {
		if (family == 4) 
			client();
		else
			client6();
	}

	return 0;
}

