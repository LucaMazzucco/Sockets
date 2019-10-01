#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <inttypes.h>
#include    "errlib.h"
#include    "sockwrap.h"
#include    <time.h>
#include    <sys/stat.h>
#include    <sys/types.h>

#define RBUFLEN		1024 /* Buffer length */


int sendfile(FILE *f, int s){
	int tot, n, sizef;
	char buf[RBUFLEN];

	tot = 0;
	fseek(f, 0L, SEEK_END);
	sizef = ftell(f);
	fseek(f, 0, SEEK_SET);
	while(tot<sizef){
		
		n = fread(buf, 1, RBUFLEN-10, f);
		//printf("valore n: %d\n", n);
		//n = strlen(buf);
		tot+=n;
		if(tot > sizef)
			n -= (tot-sizef);
		if(writen(s, buf, n) != n){
		       printf("Write error while replying\n");
		       return -1;
	    }
		//else
		       //printf("Reply sent byte sent:%d on %d\n",tot, sizef);

		
		}
	fclose(f);
	return 1;

}

int rcvfile(FILE *f, int sizef, int s){
	int tot, result, flagend=0;
	char rbuf[RBUFLEN];
 	tot = sizef;
    	while(flagend==0){
			if(tot< RBUFLEN-10){
				result = readn(s, rbuf, tot);
				
			}
			else
				result = readn(s, rbuf, RBUFLEN-10);
			
			tot -= result;
			if (result <= 0)
			{
				printf("Read error/Connection closed\n");
				close(s);
				return 0;	
			}
			else
			{
				if(tot>=0){
					
					fwrite(rbuf, 1, result, f);	
					//printf("Received response from socket %03u (byte left:%d)\n", s, tot);
				}
				if(tot<=0){
					fclose(f);
					flagend = 1;	
					printf("file received tot: %d\n", tot);			
				}
			}
	}//while
	return 1;
}

/* Utility function to display a string str
   followed by an internet address a, written
   in decimal notation
*/
void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	printf("%s %s",str,p);
	printf("!%u\n", ntohs(a->sin_port));
}

/* Checks if the content of buffer buf equals the "close" o "stop" line */
int iscloseorstop(char *buf)
{
	return (!strcmp(buf, "QUIT\r\n"));
}


/* Gets a line of text from standard input after having printed a prompt string 
   Substitutes end of line with '\0'
   Empties standard input buffer but stores at most maxline-1 characters in the
   passed buffer
*/
int mygetline(char *line, size_t maxline, char *prompt)
{
	char	ch;
	size_t  i;

	printf("%s", prompt);
	for (i=0; i< maxline-1 && (ch = getchar()) != '\n' && ch != EOF; i++)
		*line++ = ch;
	*line = '\0';
	while (ch != '\n' && ch != EOF)
		ch = getchar();
	if (ch == EOF)
		return(EOF);
	else    return(1);
}
void SendMessage(int socket,void* messageBuffer, size_t messageSize){
	Send(socket,messageBuffer,messageSize,0);
}
//Set timeout to 0 for infinite wait
//Return value <0 is timeout
ssize_t ReceiveMessage(int socket,void* messageBuffer,size_t messageSize,int timeout){
	struct timeval tv;
	if(timeout==0){
		return recv(socket,messageBuffer,messageSize,0);
	}
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
	return recv(socket,messageBuffer,messageSize,0);
}

uint Endian_UInt32_Conversion(uint value){
   return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
}

