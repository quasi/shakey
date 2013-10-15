/*
 *
 *       quasi HTTP web server "Shaky/1.0"
 *
 *       Ripul
 *       Abhijit
 *       Pradeep
 *       Amit
 *
 *       Jan 2001
 *
 *
 *
 *************************************************************/

#include"quasihttp.h"

#define _MULTI_THREADED

static void *childThread(void*);

char *documentroot;
char *defaultpage;
char *userroot;
char* error301;
char *error403;
char *error400;
char *error404;
char *error500;
char *error501;
char *hostname;
char *port;
int port1;


int
main(int argc, char **argv)
{
  int listenFD;
  socklen_t addrlen, len;
  struct sockaddr_in  myaddress, *clientAdd;
  pthread_t hThread;
  struct cInfo *clientinfo;

  initilise();
  listenFD = Socket(AF_INET,SOCK_STREAM,0);
  sscanf(port,"%d",&port1);
  printf("%d\n",port1);
  myaddress.sin_family = AF_INET;
  myaddress.sin_port = htons(port1);
  myaddress.sin_addr.s_addr = htonl(INADDR_ANY);

  len = sizeof(clientAdd);

  bzero(&(myaddress.sin_zero),8);

  Bind(listenFD,(struct sockaddr *) &myaddress,sizeof(struct sockaddr));
  Listen(listenFD,5);

  clientinfo = (struct cInfo*)malloc(sizeof(struct cInfo));
  clientAdd = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

  for(;;)
    {
      len = addrlen;
      printf("\nWaiting for connections\n");
      clientinfo->connFD = accept(listenFD,clientAdd,(socklen_t *)&addrlen);
      clientinfo->len = addrlen;
      printf("Accepted : %d\n",clientinfo->connFD);
      clientinfo->remoteAddr = clientAdd;
      pthread_create(&hThread,NULL, &childThread, (void *) clientinfo);
      printf("\nThread created\n");
    }

}


static void *
childThread(void *temp)
{
  char buffer[1500];
  int i,datalen,retval,flag=0;
  fd_set rfds;
  struct timeval tv;
  struct cInfo *clientinfo;
  struct request * client;


  pthread_detach(pthread_self());

  client = (struct request *)malloc(sizeof(struct request));

  clientinfo = temp;

  while(1)
    {
      bzero(buffer,1500);
      FD_ZERO(&rfds);
      FD_SET((int)clientinfo->connFD,&rfds);
      tv.tv_sec = 10;
      tv.tv_usec = 0;
      retval = select((int)(clientinfo->connFD + 1),&rfds,NULL,NULL,&tv);
      if (retval)
        {
          datalen = recv((int)clientinfo->connFD,buffer,port1,0);
          if(datalen == 0)
            {
              printf("Exiting.............");
              break;
            }
          if(datalen < 0)
            {
              printf("Error receiving data......");
              pthread_exit((int*)1);
            }
          else
            {
              printf("%d\n",strlen(buffer));
              for(i=0;buffer[i] != 0;i++)
                {
                  printf("%c",buffer[i]);
                  if((buffer[i] == 13) && (buffer[i+1] == 10) && (buffer[i+2] == 13) &&(buffer[i+3]==10))
                    {
                      printf("%d\n",i+4);
                      break;
                    }
                }
              if(!flag)
                {
                  client->method = buffer;
                  client->url = getNextparameter(buffer);
                  if(!strcmp(client->method,"GET"))
                    {
                      client->data = getQuery(client->url);
                      if(client->data!=NULL)
                        client->protocol = getNextparameter(client->data);
                      else
                        client->protocol = getNextparameter(client->url);

                    }
                  else
                    client->protocol = getNextparameter(client->url);
                  client->head = getHeader(client->protocol);
                  printf("I got the protocol: %s\n",client->protocol);
                  client->errorcode = 200;
                  client->socket = clientinfo->connFD;
                  service(client,clientinfo);
                  if(client->errorcode == 200)
                    {
                      if(strcmp(client->method,"GET")==0 && strcmp(findextention(client->url),"cgi") == 0)
                        {
                          printf("I am going to collect query data\n");
                          querytofile(client);
                          printf("I am going to execute\n");
                          execute(client);
                        }
                      else if(!strcmp(client->method,"POST"))
                        {
                          printf("I am in post\n");
                          if(strcmp(findextention(client->url),"cgi"))
                            {
                              client->errorcode=501;
                              senddata(client);
                              pthread_exit((int*)1);
                            }
                          else
                            {
                              printf("I am going to collect post data\n");
                              getPost(client);
                              printf("I am about to execute cgi\n");
                              execute(client);
                              printf("I executed the client\n");
                            }
                        }
                      else
                        {
                          senddata(client);
                        }
                      close((int) clientinfo->connFD);
                    }
                  else
                    {
                      senddata(client);
                      close((int) clientinfo->connFD);
                    }
                  break;

                }
            }
        }
      else
        {
          close((int)clientinfo->connFD);
          pthread_exit((int*)1);
        }
      bzero(buffer,1024);
    }
  pthread_exit((int*)1);
}


/* enters here when it recieves a request*/
void
service(struct request * client, struct cInfo * clientsocket)
{
  FILE *fp;
  char *parsedurl,rhost[24];
  int i;
  inet_ntop(AF_INET,clientsocket->remoteAddr,rhost,clientsocket->len);
  printf("%d\n",strcmp(client->protocol,"HTTP/1.1"));

  if(strcmp(client->protocol,"HTTP/1.1")>0)
    {
      client->errorcode = 400;
      return;
    }

  if( (strcmp(client->method,"GET")!=0) && (strcmp(client->method,"POST")!=0) && (strcmp(client->method,"HEAD")!=0))
    {
      client->errorcode = 501;
      return;
    }

  client->errorcode = parseurl(client->url,&parsedurl);
  if(!client->errorcode)
    client->url = parsedurl;
  printf("\nURL:%s\n",client->url);
  return;
}


int
parseurl(char *url,char **modifiedurl)
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
    {
      length += strlen(defaultpage)+1;
      return 301;
    }
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

char*
finduser(char *url)
{
  int i;
  char *user;
  for(i=2;url[i] != '/'&&url[i] != '\0';i++);
  user = malloc(i-1);
  for(i=2;url[i] != '/'&&url[i] != '\0';user[i-2]=url[i],i++);
  user[i-2] = '\0';
  return user;
}


int
isdir(char *url)
{
  int i;
  for(i = strlen(url) - 1;url[i] != '.' && url[i] != '/'; i--);
  if(url[i] == '.')
    return 0;
  else
    return 1;
}


void
senddata(struct request * client)
{
  int i,count=0;
  char c;
  FILE *fp;
  time_t tim;
  struct stat size;
  char buffer[1024];
  bzero(buffer,1024);
  printf("I am send Data \n");

  if(client->errorcode == 301)
    {
      printf("before 301\n");
      sendheader(client,buffer);
      fp = fopen(client->url,"rb");
    }
  else if(client->errorcode == 400)
    {
      sendheader(client,buffer);
      fp = fopen(client->url,"rb");
    }
  else if(client->errorcode == 501)
    {
      sendheader(client,buffer);
      fp = fopen(client->url,"rb");
    }
  else
    {
      printf("\nfile is :%s\n",client->url);
      fp = fopen(client->url,"rb");
      if(fp == NULL)
        {
          if(errno == 13)
            {
              client->errorcode = 403;
              sendheader(client,buffer);
              printf("Not autharised\n");
              fp = fopen(client->url,"rb");
            }
          else if(errno == 2)
            {
              client->errorcode = 404;
              sendheader(client,buffer);
              perror("fopen");
              fp = fopen(client->url,"rb");
            }
        }
      else
        sendheader(client,buffer);
    }


  write(client->socket,buffer,strlen(buffer));
  if(fp != NULL && strcmp(client->method,"HEAD")!=0 )
    {
      printf("\nfile %s\n",client->url);
      stat(client->url,&size);
      printf("%d\n",size.st_size);
      while(count < size.st_size)
        {
          bzero(buffer,1024);
          for(i = 0; i< 1024 && count < size.st_size; i++,count++)
            buffer[i] = fgetc(fp);

          write(client->socket,buffer,i);
        }
      fclose(fp);
    }
  printf("End\n");

}


void
sendheader(struct request* client,char *buffer)
{
  time_t tim;
  int i,datalen;
  struct stat size;
  char *ext, charsize[20];

  bzero(buffer,1024);


  if(client->errorcode == 200)
    {
      ext = findextention(client->url);
      strcpy(buffer,"HTTP/1.1 200 OK\nDate: ");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      if(strcmp(ext,"cgi")!=0)
        {
          strcat(buffer,"Content-Type: ");
          getcontent(buffer,ext);
        }
      strcat(buffer,"\nAccept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(client->url,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
    }
  else if(client->errorcode == 301)
    {
      strcpy(buffer,"HTTP/1.1 301 Moved Permanently\n");
      printf("in 301\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Location: http://");
      strcat(buffer,hostname);
      strcat(buffer,client->url);
      strcat(buffer,"/\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error301,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      client->url = error301;
    }
  else if(client->errorcode == 403)
    {
      strcpy(buffer,"HTTP/1.1 403 Forbidden\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error403,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      client->url = error403;
    }
  else if(client->errorcode == 400)
    {
      strcpy(buffer,"HTTP/1.1 400 Bad Request\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error400,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      client->url = error400;
    }
  else if(client->errorcode == 404)
    {
      strcpy(buffer,"HTTP/1.1 404 Not Found\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error404,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      client->url = error404;
    }
  else if(client->errorcode == 501)
    {
      strcpy(buffer,"HTTP/1.1 501 Method Not Implemented\n");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      if(strcmp(ext,"cgi"))
        strcat(buffer,"Allow: GET, HEAD, POST\n");
      else
        strcat(buffer,"Allow: GET, HEAD\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"Content-Type: text/html\n");
      strcat(buffer,"Accept-Ranges: bytes\n");
      strcat(buffer,"Content-Length: ");
      stat(error501,&size);
      sprintf(charsize,"%d\n\n",size.st_size);
      strcat(buffer,charsize);
      client->url = error501;
    }

  puts(buffer);
  printf(">>>> Length of header : %d\n",strlen(buffer));
}


void
getcontent(char *buffer,char *ext)
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
  close((int)fp);
}

execute(struct request * client)
{
  char filename[25],filename1[25],env[300],buffer[1024],tempfile[28];
  char *envr[100];
  struct headerfield * temp;
  int i,returnval,count=0,tempstdout,tempstdin;
  time_t tim;
  struct stat size;
  sscanf(env,"REQUEST_METHOD=%s",client->method);
  envr[0] = (char *)malloc(strlen(env)+1);
  strcpy(envr[0],env);
  sscanf(env,"CONTENT_LENGTH=%s",client->length);
  envr[1] = (char *)malloc(strlen(env)+1);
  strcpy(envr[1],env);
  sscanf(env,"HOST=%s",hostname);
  envr[2] = (char *)malloc(strlen(env)+1);
  strcpy(envr[2],env);
  for(i = 3,temp = client->head ; temp!=NULL; temp = temp->next , i++)
    {
      sscanf(env,"HTTP_%s=%s",temp->key,temp->value);
      envr[i] = (char *)malloc(strlen(env)+1);
      strcpy(envr[i],env);
    }
  envr[i] = NULL;

  printf("I am Executing the CGI\n");
  sprintf(filename,"input%d",client->socket);
  client->inputfile = fopen(filename,"rb");
  sprintf(filename1,"output%d",client->socket);
  client->outputfile = fopen(filename1,"wb");
  dup2(1,tempstdout);
  dup2(0,tempstdin);
  dup2(client->socket,1);
  dup2(fileno(client->inputfile),0);
  returnval = execve(client->url,NULL,envr);
  dup2(tempstdout,1);
  dup2(tempstdin,2);
  printf(" i just executed the cgi\n");
  fclose(client->outputfile);
  fclose(client->inputfile);
  sscanf(tempfile,"rm %s",filename);
  system(tempfile);
  bzero(buffer,1024);
  if(returnval!=-1)
    {
      strcpy(buffer,"HTTP/1.1 200 OK\nDate: ");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"\nAccept-Ranges: bytes\n");
    }
  else
    {
      strcpy(buffer,"HTTP/1.1 500 Inernal Server Error\nDate: ");
      tim = time(&tim);
      strcat(buffer,ctime(&tim));
      strcat(buffer,"Server :Shaky/1.0 (Unix)\n");
      strcat(buffer,"Connection: close\n");
      strcat(buffer,"\nAccept-Ranges: bytes\n");
      strcat(buffer,"Content-Type: text/html\n\n");
      strcpy(filename1,error500);
    }
  write(client->socket,buffer,strlen(buffer));
  client->outputfile = fopen(filename1,"rb");
  if(client->outputfile!=NULL)
    {
      printf("\nfile %s\n",filename1);
      stat(filename1,&size);
      printf("%d\n",size.st_size);
      while(count < size.st_size)
        {
          bzero(buffer,1024);
          for(i = 0; i< 1024 && count < size.st_size; i++,count++)
            buffer[i] = fgetc(client->outputfile);

          write(client->socket,buffer,i);
        }
      fclose(client->outputfile);
    }
  printf("End\n");

  return;
}

void
initilise()
{

  documentroot = config("documentroot");
  defaultpage = config("defaultpage");
  userroot = config("userroot");
  error301 = config("error301");
  error403 = config("error403");
  error400 = config("error400");
  error404 = config("error404");
  error500 = config("error500");
  error501 = config("error501");
  hostname = config("hostname");
  port = config("port");
}
