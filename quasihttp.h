# include<sys/socket.h>
# include<stdio.h>
# include<sys/types.h>
# include<malloc.h>
# include<arpa/inet.h>
# include<string.h>
# include<dirent.h>
# include<stdlib.h>
# include<time.h>
# include<pwd.h>
# include<pthread.h>
# include<errno.h>
# include<sys/stat.h>
# include<sys/time.h>

struct headerfield
{
  char *key;
  char *value;
  struct headerfield *next;
};

struct cInfo
{
  int connFD;
  int len;
  struct sockaddr_in *remoteAddr;
};

struct request
{
  char * method;
  char * url;
  char * protocol;
  struct headerfield * head;
  char * data;
  int length;
  int port;
  int socket;
  int errorcode;
  FILE *inputfile;
  FILE *outputfile;
};

char* getNextparameter(char *buffer);
void service( struct request *buffer, struct cInfo *clientsocket);
void senddata(struct request * client);
int parseurl(char *url,char **modifiedurl);
int isdir(char *);
char* finduser(char*);
char* findextention(char *);
void getcontent(char *,char *);
void sendheader(struct request * client,char *);
struct headerfield * getHeader(char *);
char * getQuery(char *);
void getPost(struct request *);
char *config(char*);
void initilise();
