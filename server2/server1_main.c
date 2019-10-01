/*
 * 	File Server2.C
 *	ECHO TCP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - CONCURRENT SERVER based on UNIX process creation on demand
 *	Warning: this version does not clear zombie processes!
 *	Signal handling is necessary to fix this problem
 */


#include    <stdlib.h>
#include    <string.h>
#include    <stdint.h>
#include    <inttypes.h>
#include	<signal.h>
#include    "../sockwrap.h"
#include    "../errlib.h"
#include    "../mylib.h"
#include    <sys/stat.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <time.h>
#include	<byteswap.h>

#define RBUFLEN		128
//#define NCLIENTS         20

/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);
void showAddr(char *str, struct sockaddr_in *a);
void service(int);
void proc_counter(int sig);
void sigpipe_handler(int sig);
/* GLOBAL VARIABLES */

	 
char *prog_name;
//int 	pcount =0;		/* client process counter*/

int main(int argc, char *argv[])
{
    int		conn_request_skt;	/* passive socket */
    uint16_t 	lport_n, lport_h;	/* port used by server (net/host ord.) */
    int		bklog = 2;		/* listen backlog */
    int		childpid;		/* pid of child process */
    int		s;			/* connected socket */
    
    //int	        pstatus=0;
    socklen_t	addrlen;
    struct sockaddr_in	saddr, caddr;	/* server and client addresses */ 
    

    prog_name = argv[0];
	//signal(SIGUSR1, proc_counter);
	signal(SIGPIPE, sigpipe_handler);
    /* input server port number */
  
    if(argc!=2){
	err_quit("Usage ./%s <port number>\n", prog_name);
	}

    if (sscanf(argv[1], "%" SCNu16, &lport_h)!=1)
	err_sys("Invalid port number");
    lport_n = htons(lport_h);

    /* create the socket */
    printf("creating socket...\n");
    s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("done, int number %u\n",s);

    /* bind the socket to any local IP address */
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_port        = lport_n;
    saddr.sin_addr.s_addr = INADDR_ANY;
    showAddr("Binding to address", &saddr);
    Bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
    printf("done.\n");

    /* listen */
    printf ("Listening at socket %d with backlog = %d \n",s,bklog);
    Listen(s, bklog);
    printf("done.\n");

    conn_request_skt = s;

    /* main server loop */
    for (;;)
    {
	/* accept next connection */
	addrlen = sizeof(struct sockaddr_in);
	s = Accept(conn_request_skt, (struct sockaddr *) &caddr, &addrlen);
	
	showAddr("Accepted connection from", &caddr);
	printf("new socket: %u  \n",s);

	/* fork a new process to serve the client on the new connection */
	if((childpid=fork())<0) 
	{ 
	    err_msg("fork() failed");
	    close(s);
	}
	else if (childpid > 0)
	{ 
	    /* parent process */
	    //pcount++;
	    waitpid(-1, NULL, WNOHANG);
	    /*
	    if(pcount>=NCLIENTS){
			wait(&pstatus);
			pcount--;
	    }
	    * */
	    close(s);	/* close connected socket */
	}
	else
	{
	    /* child process */
	    //close(conn_request_skt);	/* close passive socket */
	    service(s);			/* serve client */
	    //kill(getppid(), SIGUSR1);
	    printf("service ended\n");
		exit(0);
	}
    }
}
/*
void proc_counter(int sig){
	printf("signal captured\n");
	if(sig == SIGUSR1)
		pcount--;
	return;
	}
*/

void sigpipe_handler(int sig){
	
	if(sig == SIGPIPE){
		printf("Client unexpected problem\n");
		
	}
	
	}

/* Provides echo service on the passed socket */
void service(int s)
{
    int	 n, okflag=0;
    uint32_t sizef, lastm;
    char buf[RBUFLEN];		 /* reception buffer */
    FILE *f;
    char *token;
    struct stat attr;
    for (;;)
    {
	okflag = 0;
	strcpy(buf, "\n");
	n=recv(s, buf, RBUFLEN-1, 0);
	if(iscloseorstop(buf))		
	     {
	       printf("Connection closed by party on socket %d\n",s);
	       close(s);
	       return;
	    }
	if (n < 0)
	{
	   printf("Read error\n");
	   close(s);
	   printf("Socket %d closed\n", s);
	   return;
	}
	else if (n==0)
	{
	   printf("Connection closed by party on socket %d\n",s);
	   close(s);
	   return;
	}
       	else{
	       printf("Received line from socket %03d :\n", s);
	       buf[n]=0;
	       printf("%s\n",buf);
	       
		if (strstr(buf, "GET ")!=NULL) {
		   
			token = strtok(buf, " ");
			token = strtok(NULL, " ");
			token = strtok(token, "\r\n");
		
			if( !(f = fopen(token, "r"))){
				printf("file not found!\n");
				strcpy(buf, "-ERR\r\n");
				if(writen(s, buf, n) != n)
				       printf("Write error while replying\n");
				else
				       printf("Reply sent\n");
				close(s);
				//exit(9);
			}
			else{	
				okflag = 1;
				
				stat(token, &attr);
				fseek(f, 0L, SEEK_END);
				sizef = ftell(f);
				fseek(f, 0, SEEK_SET);

				strcpy(buf, "+OK\r\n");
			        n = strlen(buf);
			        if(writen(s, buf, n) != n)
				       printf("Write error while replying\n");
				else
				       printf("Reply sent\n");
				
				//COPY ON BUFFER SIZE IN 32BIT LONG INTEGER AND WRITE ON SOCKET
				sizef = Endian_UInt32_Conversion(sizef);
				//memcpy(buf, &sizef, 4);
				
				n = strlen(buf);
				if(writen(s, &sizef, 4) != 4)
				       printf("Write error while replying\n");
				else
				       printf("Reply sent\n");

				//COPY ON BUFFER DATA LAST MOD IN 32BIT LONG INTEGER AND WRITE ON SOCKET
				lastm = time(&attr.st_mtime);
				
				//memcpy(buf, &lastm, 4);
				
				n = strlen(buf);
				printf("time last modification in seconds from 1970: %u\n", lastm);
				lastm = Endian_UInt32_Conversion(lastm);
				
				if(writen(s, &lastm, 4) != 4)
				       printf("Write error while replying\n");
				else
				       printf("Reply sent\n");

				
			     }   
		}//GET
		else {
				if (strstr(buf, "QUIT\r\n")!=NULL) {
					printf("Connection closed\n");
					close(s);
					return;
				}
				else{
					printf("-ERR\r\n");
					strcpy(buf, "-ERR\r\n");  
					n = strlen(buf);
					if(writen(s, buf, n) != n)
						   printf("Write error while replying\n");
					else
						   printf("Reply sent\n"); 
					close(s);
					return;
				}
		}

	       
	    }	//RECEIVED LINE
	    
	    if(okflag){
		  sendfile(f, s);
		 
	    }		
	   
	}
    }




