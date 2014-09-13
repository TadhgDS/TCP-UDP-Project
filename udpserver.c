/* A simple server in the internet domain using TCP
   The port number is passed as an argument !*/


// tcp.port==5000 || (arp  && eth.src == 00:22:19:e9:23:99 && arp.dst.proto_ipv4 == 192.168.1.15 && !(arp.src.proto_ipv4 == 192.168.1.254)) && !vnc 
// tcp.port==5001 || (arp  && eth.src == 00:22:19:e9:23:99 && arp.dst.proto_ipv4 == 192.168.1.15 && !(arp.src.proto_ipv4 == 192.168.1.254)) && !vnc 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <time.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sysctl.h>
//#include "settings.h"


int globalval[8];

#define LISTEN 0
#define TIMER 1
#define DATA 2

//#define TCP_USER_TIMEOUT 18 //300000
#define TCPMAXBUFFER 16384
#define NUM_PACKETS 10000
#define PACKET_PAYLOAD 512


int sockfd=-1, newsockfd=-1;
int pollReturn;
int dataLost = 0;
static volatile int running=1; 
////ssssd
unsigned long timestamp_millis(){
     struct timespec temp;
     clock_gettime(CLOCK_REALTIME,&temp);
     unsigned long time_in_mill = (temp.tv_sec) * 1000 + (temp.tv_nsec) / 1000000;
     return time_in_mill;
   }
void timestmp(){
     printf("<t> %lu ms\n",timestamp_millis());
}

void sigHandler() {
    close(sockfd);
    sockfd = -1; //
    close(newsockfd);
    newsockfd = -1; //
    running = 0;
}
 
void error(const char *msg) {
    perror(msg);
    exit(1);
}
 

// Check for a poll() event
bool event(unsigned short events, int eventMask)
{
    return ((events & eventMask)!=0);
}


 
int main(int argc, char *argv[])
{

    signal(SIGINT, sigHandler);
    int portno;              //portno stores the port number on which the server accepts connections.
    socklen_t clilen;                            //clilen stores the size of the address of the client. This is needed for the accept system call.
    char buffer[PACKET_PAYLOAD];                            //server reads characters from the socket connection into this buffer.
    struct sockaddr_in serv_addr, cli_addr;
    int clientaddrlen = sizeof(cli_addr);

    int n;
    char data='A';
    int timerfd;
   // struct itimerspec interval ={{0,1000000000L},{0,1000000000L}};
    struct itimerspec interval ={{1,0},{1,0}};
    int count=NUM_PACKETS;  // NUMBER OF PACKETS TO SEND
    int status;
		 

    if (argc < 2) {
    	fprintf(stderr,"ERROR, no port provided\n");
        timestmp();
        exit(1);
    }
 
    if (argc >= 4) {
    	count = atoi(argv[3]);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
  	}
 
    bzero((char *) &serv_addr, sizeof(serv_addr));
 
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
	}

    listen(sockfd,5);        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    newsockfd = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &clientaddrlen);

    // set the reuse address socket option
    int opt=SO_REUSEADDR;
    status = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(int));
    if (status==-1){
        error("Enable to set SO_REUSEADDR");
    }

    timerfd = timerfd_create( 0, CLOCK_REALTIME );
    if (timerfd==-1){
        error("Create timer failed");
    }
    status = timerfd_settime( timerfd, 0, &interval, NULL );
    if (status==-1){
        error("Setup timer failed");
    }
    clilen = sizeof(cli_addr);


 
    //Setting up poll 
    struct pollfd pollfds[3];
    pollfds[LISTEN].fd = sockfd;
    pollfds[LISTEN].events = POLLIN;
    pollfds[LISTEN].revents = 0;
 
    pollfds[TIMER].fd = timerfd;
    pollfds[TIMER].events = POLLIN;
    pollfds[TIMER].revents = 0;

    pollfds[DATA].fd = -1;
    pollfds[DATA].events = 0;
    pollfds[DATA].revents = 0;
 
    int nfds=2;

    while(running){
		
        int i;
        for (i=0; i<nfds; i++) {
            pollfds[i].revents = 0; // clear the returned events
        }

        pollReturn = poll(pollfds,nfds,1000); 
 
        if(pollReturn == -1) {
            printf("ERROR");
            timestmp();
        }
        if(pollReturn == 0) {
            printf("Nothing yet.. \n");
            timestmp();
        }
 


        // IF AN EVENT OCCURED via poll()
        if(pollReturn > 0) {

/*            // Debugging
            int i;
            for (i=0; i<nfds; i++) {
                printf("Poll(%d) returned %08X\n", i, pollfds[i].revents);
                timestmp();
                printf("nfds: %d i: %d \n",nfds,i);
            }  */


            // Check for errors
            for (i=0; i<nfds; i++) {
               if(event(pollfds[i].revents, POLLERR)) {
                   error("POLLERR\n");
                   timestmp();
               }
               if(event(pollfds[1].revents, POLLHUP)) {
                   error("POLLHUP\n");
                   timestmp();
               }
               if(event(pollfds[1].revents, POLLNVAL)) {
                   error("POLLNVAL\n");
                   timestmp();
               }
            }


            // IF EVENT IS ON LISTENING SOCKET
            if(event(pollfds[0].revents, POLLIN)) {
                //// Accepting new connection                //////////////////////////////////////////////////////////////////////////////////////////////////
               // newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                newsockfd = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &clientaddrlen);

                if (newsockfd < 0) {
                    printf("\n --------------  sockfd: %d, newsockfd: %d \n",sockfd,newsockfd);
                    error("ERROR on accept");
                }
                else {
                    printf("Connection accepted\n");
                    pollfds[DATA].fd = newsockfd;
                    nfds = 3;
                }
                // set the nodelay option - send data immediately on every write  
                opt=TCP_NODELAY;
                status = setsockopt(newsockfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt,sizeof(int));
                if (status==-1) error("Enable to set TCP_NODELAY");

		

                // Set Nonblocking
          		int stat;
          		int on = 1;
          		int flags;
          		//stat = ioctl(newsockfd, FIONBIO, (char*)&on);
          		flags = fcntl( newsockfd, F_GETFL, 0 );
          		if(stat==-1){
          		   printf("Error from fcntl/getfl");
                             exit(1);
          		}
          		stat = fcntl( newsockfd, F_SETFL, flags | O_NONBLOCK );
          		if(stat==-1){
          		    printf("Error from fcntl/setfl");
                              exit(1);
          		}
          		printf("%08x \n", stat);
            }



            // IF EVENT IS ON DATA SOCKET
            if (event(pollfds[TIMER].revents, POLLIN)) {
            	   
                // read the timer to acknowledge it

                unsigned char temp[8];
                int status = read( timerfd, temp, 8 );

				
               	// if we've sent all the packets, close the program
               	if (count==0) {
                  //running = 0; CHANGE////////////////////
                   
                    close(newsockfd);
                    newsockfd = -1;

                    pollReturn = 0;     
                    nfds = 2;     
                    count = NUM_PACKETS;
               }

               // check socket is open
               					
                else if (newsockfd>0) {


                  	// ### CREATE DATA
                  	char payload[PACKET_PAYLOAD]; 
                  	memset(&payload, data, PACKET_PAYLOAD); 
                  	if (++data>'Z'){ 
                         data='A';
                    }

                    /// ### ADD PACKET NUMBER AND TIMESTAMP TO PAYLOAD
                 	const int k = snprintf(NULL, 0, "%lu", timestamp_millis());
	              	char timestamp[k+1];
              		int c = snprintf(timestamp, k+1+9, "<t> %lu ms: ", timestamp_millis());
              		
              		const int d = (NULL, 0, "%d", count);
              		char packno[d+1];
              		int j = snprintf(packno, d+1+3, "[%d] ", count);
              		
              		strncpy(payload, timestamp, c);
              		strncpy(payload, packno, j);

                    // ### PRINT CURRENT SOCKET QUEUE LENGTH
                    int status, value;
                    status = ioctl(newsockfd, SIOCOUTQ, &value);   
                    printf("Socket Queue length BEFORE write: %d \n", value); 


                   /// ### SEND DATA
                  	n = 0;
                  	//n = write(newsockfd,payload,PACKET_PAYLOAD); ///////////////////////////////////////////////////////////////////////////////////////
                    n = sendto(sockfd, payload, PACKET_PAYLOAD, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));


                  	if (n < 0){
                    	if ((errno==EAGAIN)||(errno==EWOULDBLOCK)) {
                    		printf("Write incomplete - nothing sent\n");
                    		timestmp();

                        int i = dataLost; //if dataLost is unchanging, stop printing
					              dataLost += PACKET_PAYLOAD;

					              if(dataLost>i){
                          printf("DATA LOST: %d \n", dataLost);
                        }
      			         	}
              				else {				
               					error("ERROR writing to socket:");
                        timestmp();  
              				}   
                  	}
                  	else if (n == PACKET_PAYLOAD) {
                      printf("Write successful\n");
                      timestmp();
                  	}
                  	else {
                    	printf("Write incomplete: %d\n", n);
                    	timestmp();
       				        dataLost += (PACKET_PAYLOAD - n);
		        	        printf("DATA LOST: %d \n", dataLost);	
                  	}

                  
      		   			if(value>TCPMAXBUFFER){
      		   				printf("LOOK %d\n",value);
      		    			dataLost += PACKET_PAYLOAD;
      					   	printf("DATA LOST: %d \n", dataLost);
      				    }

                  status = ioctl(newsockfd, SIOCOUTQ, &value);   
                  printf("Socket Queue length AFTER write: %d \n", value); 

                	count--;
                	printf("count : %d \n",count);
                	timestmp();
               	}
               	else {
                 // printf("Waiting..\n");
                    //timestmp();
               	}
				
      				if(dataLost > 1){
      					printf("DATA LOST: %d \n", dataLost);	
      				}
            }

        }

    }

    if (count>0){
        printf("Program interrupted\n");
        timestmp();
    }
    else{
        printf("Program finished\n");
        timestmp();
    }

   if (sockfd>-1) close(sockfd);
   if (newsockfd>-1) close(newsockfd);


 return 0;
}
