CC = gcc

all: tcpclient tcpserver1 tcpserver   

tcpclient:tcpclient.c
	gcc -g -o $@ $<

tcpserver:tcpserver.c
	gcc -g -o $@ $<

tcpserver1:tcpserver1.c
	gcc -g -o $@ $<

clean:
	@rm tcpclient tcpserver tcpserver1 -f 

