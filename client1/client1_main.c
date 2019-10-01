
#include     <stdlib.h>
#include     <string.h>
#include     <inttypes.h>
#include     "../errlib.h"
#include     "../sockwrap.h"
#include     "../mylib.h"

#define TIMEOUT   10 /*TIMEOUT IN SECONDS - SET TO 0 FOR INFINITE TIME*/
#define BUFLEN	128 /* BUFFER LENGTH */

/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);
void showAddr(char *str, struct sockaddr_in *a);
int iscloseorstop(char *buf);

/* GLOBAL VARIABLES */
char *prog_name;


int main(int argc, char *argv[])
{
	uint32_t sizef=0, lastm=0;
    char     	   buf[BUFLEN];		/* transmission buffer */
    char	   rbuf[BUFLEN];	/* reception buffer */
    uint16_t	   tport_n, tport_h;	/* server port number (net/host ord) */

    int		   nfile, s;
    int		   result;
    
    FILE *f;
    struct sockaddr_in	saddr;		/* server address structure */
    struct in_addr	sIPaddr; 	/* server IP addr. structure */
    struct timeval tv;
	
    prog_name = argv[0];

   /* input IP address and port of server */
	
	if(argc<3){
		err_quit("Usage ./%s <ip address> <port number> <file name>\n", prog_name);
	}    

    result = inet_aton(argv[1], &sIPaddr);
    if (!result)
	err_quit("Invalid address");

	 /* create the socket */
    printf("Creating socket\n");
    s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("done. Socket fd number: %d\n",s);
	
    if (sscanf(argv[2], "%" SCNu16, &tport_h)!=1)
		err_quit("Invalid port number");
    tport_n = htons(tport_h);

    /* prepare address structure */
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port   = tport_n;
    saddr.sin_addr   = sIPaddr;

    /* connect */
    showAddr("Connecting to target address", &saddr);
    Connect(s, (struct sockaddr *) &saddr, sizeof(saddr));
    printf("done.\n");
	
    /*set timeout for any operation*/
    if(TIMEOUT!=0){
	    tv.tv_sec = TIMEOUT;
	    tv.tv_usec = 0;
	    setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    }

    /* main client loop */
    printf("Enter line 'QUIT' to close connection and stop client.\n");
	nfile = 3;
	size_t	len;
	
    while(nfile<argc){ 
	
		strcpy(buf, "GET ");
		strcat(buf, argv[nfile]);
		strcat(buf,"\r\n");
		len = strlen(buf);

			if(writen(s, buf, len) != len){
				printf("Write error\n");
				break;
			}
			
			printf("waiting for response...\n");
			result = readn(s, rbuf, 5);
			len = strlen(rbuf);
			
			if (result <= 0){
				 printf("Read error/Connection closed\n");
				 close(s);
				 exit(1);
			}
			else{
				rbuf[result-1] = '\0';
				printf("Received response from socket %03u : \n%s\n", s, rbuf);
				
				 
				if (strstr(rbuf, "+OK")!=NULL) {
					printf("OK received\n");
					//GET DIMENSION OF FILE
					result = readn(s, rbuf, 4);
					if (result <= 0){
						 printf("Read error/Connection closed\n");
						 close(s);
						 exit(1);
					}else{
						memcpy(&sizef, rbuf, 4);
						sizef = Endian_UInt32_Conversion(sizef);
						printf("file dimension: [%u]\n", sizef);
					}
					//GET LAST MODIFICATION OF FILE
					result = readn(s, rbuf, 4);
					if (result <= 0){
						 printf("Read error/Connection closed\n");
						 close(s);
						 exit(1);
					}else{
						memcpy(&lastm, rbuf, 4);
						lastm = Endian_UInt32_Conversion(lastm);
						printf("data dimension: [%u]\n", lastm);
					}
					if( !(f = fopen(argv[nfile], "wb+"))){
						printf("open file error!\n");
					}else//ok open file
					{
						printf("file opened\n");
						rcvfile(f, sizef, s);
					}//ok open file				
				}//ok received
				else{
					close(s);
					printf("Error received from server\n");
					exit(0);
					
					
				}
			}//received answer
		printf("===========================================================\n");
		nfile = nfile+1;
    }//for cycle
    strcpy(buf, "QUIT\r\n");
	len = strlen(buf);
	
	if(writen(s, buf, len) != len){
		printf("Write error\n");
	}
    close(s);
    exit(0);
}//main
