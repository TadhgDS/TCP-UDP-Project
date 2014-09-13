.PHONY : all
.PHONY : clean

all:	tcpserver tcpclient udpclient udpserver

tcpserver:	tcpserver.c
	gcc -o tcpserver tcpserver.c -lrt

tcpclient:	tcpclient.c
	gcc -o tcpclient tcpclient.c -lrt

udpserver:	udpserver.c
	gcc -o udpserver udpserver.c -lrt

udpclient:	udpclient.c
	gcc -o udpclient udpclient.c -lrt

clean:
	-rm tcpclient
	-rm tcpserver
	-rm udpclient
	-rm udpserver

