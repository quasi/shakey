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
# include<sys/stat.h>
# include<errno.h>
# include<pthread.h>
# include<sys/time.h>

int Socket( int family, int type, int protocol );
int Bind( int socket_fd, const struct sockaddr *myaddress, socklen_t address_length);
int Listen( int des, int backlog);
int test(char *buffer,char *str);
char* getNextparameter(char *buffer);
char verify(char *, char *);
void service(char *,int );
void senddata(char* ,int,int);
int parseurl(char *,char **);
int isdir(char *);
char* finduser(char*);
char* findextention(char *);
void sendheader(int ,char **,int);
char* findextention(char *);
void getcontent(char *,char *);
static void* doit(void *);




char documentroot[] = "/home/httpd/html/";
char defaultpage[] = "index.html";
char userroot[] = "public_html/";
char error301[] = "error301.html";
char error403[] = "error403.html";
char error400[] = "error400.html";
char hostname[] = "yugandhar:1500";


struct tobesent
{
  struct sockaddr_in *clientaddress;
  int clientsoc;
};

int main(int argc, char **argv)
{
  struct dirent **namelist;
  int socketdes,clientsocket,list,len,datalen,i,n,dummy,c,flag;
  char *buffer,*client,*filename, *user, *pass,result;
  struct sockaddr_in  myaddress,clientadd ;
  struct tobesent clientinfo;
  pthread_t hThread;
  client = (char *)malloc(16);
  socketdes = Socket(AF_INET,SOCK_STREAM,0);
  myaddress.sin_family = AF_INET;
  myaddress.sin_port = htons(1500);
  myaddress.sin_addr.s_addr = htonl(INADDR_ANY);
  len = sizeof(clientadd);
  bzero(&(myaddress.sin_zero),8);
  Bind(socketdes,(struct sockaddr *) &myaddress,sizeof(struct sockaddr));
  Listen(socketdes,5);
  while(1)
    {
      clientsocket = accept(socketdes,(struct sockaddr *)&clientadd,(socklen_t *)&len);
      printf("Accepted\n");
      if(clientsocket  < 0)
        {
          printf("Error accepting........");
          exit(1);
        }
      clientinfo.clientaddress = &clientadd;
      clientinfo.clientsoc = clientsocket;
      pthread_create(&hThread,NULL,&doit,(void *) clientinfo.clientsoc);
    }
}

static void *
doit(void *clientinfo)
{
  char buffer[1024];
  int i,datalen,retval;
  fd_set rfds;
  struct timeval tv;
  pthread_detach(pthread_self());
  while(1)
    {
      bzero(buffer,1024);
      FD_ZERO(&rfds);
      FD_SET((int)clientinfo,&rfds);
      tv.tv_sec = 10;
      tv.tv_usec = 0;
      retval = select((int)(clientinfo+1),&rfds,NULL,NULL,&tv);
      if (retval != 0)
        {
          datalen = recv((int)clientinfo,buffer,1024,0);
          if(datalen == 0)
            {
              printf("Exiting.............");
              break;
            }
          if(datalen < 0)
            {
              printf("Error receiving data......");
              exit(1);
            }
          else
            {
              for(i=0;buffer[i] != 0;i++)
                {
                  printf("%c",buffer[i]);
                }
              if(buffer[datalen-4] == 13 && buffer[datalen-3] == 10 && buffer[datalen-2] == 13 && buffer[datalen-1] == 10)
                {
                  service(buffer, (int) clientinfo);
                  close((int) clientinfo);
                  break;
                }
            }
        }
      else
        {
          close((int)clientinfo);
          pthread_exit(0);
        }
      bzero(buffer,1024);
    }
}

void service(char *buffer, int clientsocket)
{
  FILE *fp;
  char *url, *protocol, *parsedurl;
  int redirect,i;

  url=getNextparameter(buffer);
  protocol=getNextparameter(url);

  redirect = parseurl(url,&parsedurl);
  if(redirect)
    {
      senddata(url,clientsocket,redirect);
      printf("\nURL:%s\n",url);
    }
  else
    {
      senddata(parsedurl,clientsocket,redirect);
      printf("\nURL:%s\n",parsedurl);
    }
}
int parseurl(char *url,char **modifiedurl)
{
  int i,length;
  char *user, *home;
  struct passwd *pass;
  if(strlen(url)==1)
    {
      *modifiedurl = malloc(strlen(documentroot) + strlen(defaultpage)+1);
      strcpy(*modifiedurl,documentroot);
      strcat(*modifiedurl,defaultpage);
      return 0;
    }
  if(url[1] == '~')
    {
      user = finduser(url);
      pass = getpwnam(user);
      length = strlen(pass->pw_dir);
      length += strlen(userroot);
      length += strlen(url);
    }
  else
    {
      length = strlen(documentroot);
      length += strlen(url);
    }
  if(isdir(url) && url[strlen(url)-1]!='/')
    return 1;
  else if(isdir(url) && url[strlen(url)-1]=='/')
    length += strlen(defaultpage);

  *modifiedurl = malloc(length +1);
  if(url[1] == '~')
    {
      strcpy(*modifiedurl,pass->pw_dir);
      strcat(*modifiedurl,"/");
      strcat(*modifiedurl,userroot);
      i = strlen(user) + 3;
      length = strlen(*modifiedurl) ;
      printf("I am in is ~\n");
      if(url[i] != '\0')
        {
          for(;url[i] != '\0'; i++,length++)
            {

              (*modifiedurl)[length] = url[i];
            }
          (*modifiedurl)[length] = '\0';
        }
    }
  else
    {
      strcpy(*modifiedurl,documentroot);
      (*modifiedurl)[strlen(*modifiedurl)-1] = '\0';
      strcat(*modifiedurl,url);
    }
  if(isdir(url))
    {
      strcat(*modifiedurl,defaultpage);
    }
  return 0;
}


char* finduser(char *url)
{
  int i;
  char *user;
  for(i=2;url[i] != '/'&&url[i] != '\0';i++);
  user = malloc(i-1);
  for(i=2;url[i] != '/'&&url[i] != '\0';user[i-2]=url[i],i++);
  user[i-2] = '\0';
  return user;
}


int isdir(char *url)
{
  int i;
  for(i = strlen(url) - 1;url[i] != '.' && url[i] != '/'; i--);
  if(url[i] == '.')
    return 0;
  else
    return 1;
}


void senddata(char *url,int clientsocket,int redirect)
{
  int i,count=0;
  char c;
  FILE *fp, *fp1;
  time_t tim;
  struct stat size;
  char buffer[1024];
  bzero(buffer,1024);
  fp1 = fopen("logs","a");
  printf("I am send Data \n");
  if(redirect != 1)
    {
      fp = fopen(url,"rb");
      printf("\nfile %s\n",url);
      if(fp == NULL)
        {
          if(errno == 13)
            {
              sendheader(403,&url,clientsocket);
              printf("Not autharised\n");
              fp = fopen(url,"rb");
            }
          else if(errno == 2)
            {
              sendheader(400,&url,clientsocket);
              printf("Could not find file %s %d\n",url,errno);
              perror("fopen");
              fp = fopen(url,"rb");
            }
        }
      else
        sendheader(200,&url,clientsocket);
    }
  else
    {
      sendheader(301,&url,clientsocket);
      fp = fopen(url,"rb");
    }
  printf("\nfile %s\n",url);
  stat(url,&size);
  printf("%d\n",size.st_size);
  while(count < size.st_size)
    {
      for(i = 0; i< 1023&&count < size.st_size; i++,count++)
        {
          buffer[i] = fgetc(fp);
          fprintf(fp1,"%c",c);
        }
      write(clientsocket,buffer,strlen(buffer));
      bzero(buffer,1024);

    }
  fclose(fp);
  fclose(fp1);
  printf("End\n");

}

void sendheader(int Errcode,char **url,int clientsocket)
{
  time_t tim;
  int i,datalen;
  struct stat size;
  char *ext, buffer[1024],charsize[20];
  printf("I am send header \n");
  if(Errcode == 200)
    {
      ext = findextention(*url);
      strcpy(buffer,"HTTP/1.1 200 OK\nDate: ");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :AsUWish/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: ");
      getcontent(buffer,ext);
      strcat(buffer,"\nAccept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(*url,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
    }
  else if(Errcode == 301)
    {
      strcpy(buffer,"HTTP/1.1 301 Moved Permanently\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :AsUWish/1.0 (Unix)\n");
      strcat(buffer,"Location: http://");
      strcat(buffer,hostname);
      strcat(buffer,*url);
      strcat(buffer,"/\n");
      printf("I am going to add the location to the header\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error301,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      printf("I am about to change url\n");
      *url = error301;
    }
  else if(Errcode == 403)
    {
      strcpy(buffer,"HTTP/1.1 403 Forbidden\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :AsUWish/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error403,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      printf("I am about to change url\n");
      *url = error403;
    }
  else if(Errcode == 400)
    {
      strcpy(buffer,"HTTP/1.1 400 Bad Request\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :AsUWish/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error403,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      printf("I am about to change url\n");
      *url = error400;
    }
  puts(buffer);
  write(clientsocket,buffer,strlen(buffer));
}

void getcontent(char *buffer,char *ext)
{
  char c=0,readbuf[200];
  FILE *fp;
  int i,j,flag,same;
  fp = fopen("/etc/mime.types","r");
  if(fp ==NULL)
    {
      printf("Unable to open file\n");
      exit(1);
    }
  while(c != EOF)
    {
      for(i=0;i<200&&c!='\n'&&c!=EOF;i++)
        {
          c = fgetc(fp);
          readbuf[i] = c;
        }
      readbuf[i] = '\n';
      flag=0;
      for(i=0;readbuf[i]!='\n'&&i<200;i++)
        {
          if(readbuf[i] == '\t' || readbuf[i] == ' ')
            {
              flag++;
              j=0;
              same = 1;
              continue;
            }
          if(flag>0)
            {
              if(ext[j] != readbuf[i])
                {
                  same = 0;
                  flag = 0;
                }
              else
                {
                  j++;
                }
              if(j == strlen(ext))
                {
                  for(i=0;readbuf[i]!='\t';i++);
                  readbuf[i] = '\0';
                  strcat(buffer,readbuf);
                  break;
                }
            }
        }
      if(c=='\n')	c=0;
    }
  fclose(fp);
}

char* findextention(char *url)
{
  int i,j;
  char *ext;
  for(i=strlen(url)-1;url[i] != '.';i--);
  ext = malloc(strlen(url)-i);
  i++;
  for(j=0;url[i]!='\0';i++,j++)
    ext[j] = url[i];
  ext[j] = '\0';
  printf("%s\n",ext);
  return ext;
}

int Socket( int family, int type, int protocol )
{
  int des;
  des = socket( family,type,protocol);
  if(des < 0)
    {
      printf("Error defining socket......\n");
      exit(1);
    }
  return des;
}

int Bind( int socket_fd, const struct sockaddr *myaddress, socklen_t address_length)
{
  int temp;
  temp =  bind(socket_fd,myaddress,address_length);
  if(temp < 0)
    {
      printf("Error Binding......\n");
      exit(1);
    }
}

int Listen( int des, int backlog)
{
  int temp;
  temp = listen(des,backlog);
  if(temp < 0)
    {
      printf("Error listening......\n");
      exit(1);
    }
  return temp;
}

char* getNextparameter(char *buffer)
{
  int i;
  char *ptr;
  ptr = buffer;
  for(i=0 ; i<1024 && buffer[i] != 32 ; i++)
    {
      ptr++;
      if(buffer[i] == '\0')
        return NULL;
    }
  *ptr='\0';
  ptr++;
  return ptr;
}
