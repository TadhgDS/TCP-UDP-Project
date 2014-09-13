#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <time.h>

static volatile int running=1; 
static volatile int sockfd;
#define PACKET_PAYLOAD 512


unsigned long timestamp_millis(){
    struct timespec temp;
    clock_gettime(CLOCK_REALTIME,&temp);
    unsigned long time_in_mill = (temp.tv_sec) * 1000 + (temp.tv_nsec) / 1000000;
    return time_in_mill;
}
void timestmp(){
    printf("<t> %lu ms\n\n",timestamp_millis());
}


void sigHandler() {
    close(sockfd);
    sockfd = -1;
    // running = 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int portno, n;			//The variables sockfd, portno, and n are all identical to those of the server. 
    struct sockaddr_in serv_addr; 		//The variable serv_addr will contain the address of the server to which we want to connect.
    struct hostent *server; 			//The variable server is a pointer to a structure of type hostent. This structure is defined in the header file netdb.h 

    char buffer[PACKET_PAYLOAD];				//All of this code is the same as that in the server.
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);

    signal(SIGINT, sigHandler);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
  		timestmp();
	}



    server = gethostbyname(argv[1]); 		//The variable argv[1] contains the name of a host on the Internet, e.g. cs.nuim.ie.
						//Takes such a name as an argument and returns a pointer to a hostent containing information about that host.
						//The field char *h_addr contains the IP address.
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
		timestmp();
        exit(0);
    }
 
    bzero((char *) &serv_addr, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");  
		timestmp();		
    }
						

    int packno;
    int count = 0;
    while (running) {
        bzero(buffer,PACKET_PAYLOAD);
        n = read(sockfd,buffer,PACKET_PAYLOAD);
        
        if(n > 0){
           	count++;
            printf("\n\n"); 
            printf("Reading %d bytes from ",n);
           	printf("packet %d \n",count);           	              
           	printf("%s\n", buffer);    

            char packnotemp[2];
            strncpy(packnotemp,buffer+1,2);
            packno = atoi(packnotemp);	
        }
        else if(n < 0){
          	error("ERROR reading from socket");
		   	timestmp();
        }
        else if(packno == 1){
            close(sockfd);
            printf("Socket closed by far end, exiting\n");  
            running = 0;
        }
        else{ 
            printf("Read returned 0\n");
            timestmp();
            
		    
        }
    }
    n = 0;
	
    return 0;
}









