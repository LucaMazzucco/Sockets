
//mylib.h
//created by luca in 26/08/2017

int sendfile(FILE *f, int s);
int rcvfile(FILE *f, int sizef, int s);
void showAddr(char *str, struct sockaddr_in *a);
int iscloseorstop(char *buf);
int mygetline(char *line, size_t maxline, char *prompt);
void SendMessage(int socket,void* messageBuffer, size_t messageSize);
ssize_t ReceiveMessage(int socket,void* messageBuffer,size_t messageSize,int timeout);
uint Endian_UInt32_Conversion(uint value);
