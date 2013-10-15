#include"quasihttp.h"

# define BUFFERSIZE 200

char*
findextention(char *url)
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



char*
getNextparameter(char *buffer)
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

char*
getQuery(char *buffer)
{
  int i;
  char *ptr;
  ptr = buffer;
  for(i=0 ; i<1024 && buffer[i] != '?' ; i++)
    {
      if(buffer[i] == '\0'||buffer[i] == 32)
        return NULL;
      ptr++;
    }
  *ptr='\0';
  ptr++;
  return ptr;
}

void
Pthread_detach(pthread_t tid)
{
  int		n;

  if ( (n = pthread_detach(tid)) == 0)
    return;
  errno = n;
  perror("pthread_detach error");
}

int
Socket( int family, int type, int protocol )
{
  int des;
  des = socket( family,type,protocol);
  if(des < 0)
    {
      perror("Error defining socket");
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
      perror("Error Binding");
      exit(1);
    }
}

int Listen( int des, int backlog)
{
  int temp;
  temp = listen(des,backlog);
  if(temp < 0)
    {
      perror("Error listening");
      exit(1);
    }
  return temp;
}

int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
  int		n;

 again:
  if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
    if (errno == EPROTO || errno == ECONNABORTED)
#else
      if (errno == ECONNABORTED)
#endif
        goto again;
      else
        {
          perror("accept error");
          _exit(1);
        }
  }
  return(n);
}

struct headerfield * getHeader(char * buffer)
{
  struct headerfield *head, *tail, *temp;
  char *ptr,*ptr1;
  int i;
  head = NULL;
  tail = NULL;
  ptr1 = buffer;
  while(1)
    {
      for(ptr = ptr1 ; *ptr != 13 ; ptr++)
        {
          if(*ptr == 13)
            if(*(ptr+1) == 10 && (*(ptr+2)) == 13 && (*(ptr+3)) == 10)
              {
                *ptr = '\0';
                return head;
              }
          if(*ptr=='\0')
            {
              printf("Going to return\n");
              return head;
            }
        }
      *ptr='\0';
      ptr+=2;
      ptr1 = ptr;
      for(ptr1 = ptr ; *ptr1 != 32 ; ptr1++)
        {
          if(*ptr == 10)
            if((*(ptr+1)) == 13 && (*(ptr+2)) == 10)
              {
                *ptr = '\0';
                return head;
              }
          if(*ptr1=='\0')
            return head;
        }
      *(ptr1-1)='\0';
      ptr1++;
      temp = (struct headerfield *)malloc(sizeof(struct headerfield));
      temp->key = ptr;
      temp->value = ptr1;
      temp->next = NULL;
      if(head==NULL)
        {
          head = temp;
          tail = temp;
          temp = NULL;
        }
      else
        {
          tail->next = temp;
          tail = temp;
          temp = NULL;
        }
    }
}
char * findvalue(char * key,struct request * client)
{
  int i;
  struct headerfield *temp;
  for(temp=client->head;temp!=NULL;temp = temp->next)
    {
      if(!strcmp(temp->key,key))
        {
          return temp->value;
        }
    }
  return NULL;
}


void getPost(struct request * client)
{
  int length,i,j;
  char *ptr,*buffer,*content,filename[11],readbuffer[1024];
  FILE *fp;
  struct headerfield * temp;
  content = findvalue("Content-Length",client);
  if(content != NULL)
    sscanf(content,"%d",&length);
  client->length = length;
  for(temp = client->head;temp->next != NULL&&temp!=NULL;temp = temp->next);
  buffer = temp->value;
  ptr = buffer;
  for(i=0 ; buffer[i] != '\0' || buffer[i+1] != 10 || buffer[i+2] != 13 || buffer[i+3]!=10 ; i++)
    {
      ptr++;
      if(buffer[i] == '\0'||buffer[i] == 32)
        {
          client->data = NULL;
          printf("%s",client->data);
          return ;
        }
    }
  ptr+=3;
  *ptr='\0';
  ptr++;
  client->data = ptr;
  sprintf(filename,"input%d",client->socket);
  fp = fopen(filename,"wb");
  if(fp!=NULL)
    client->inputfile = fp;
  for(i=0;i<length&&client->data[i]!='\0';i++)
    fputc(client->data[i],client->inputfile);
  while(i<length)
    {
      recv(client->socket,readbuffer,client->port,0);
      for(j=0;j<1024&& i<length;i++,j++)
        fputc(readbuffer[j],client->inputfile);
    }
  fclose(fp);
  return;
}
void querytofile(struct request * client)
{
  char filename[25];
  int i;
  sprintf(filename,"input%d",client->socket);
  client->inputfile = fopen(filename,"wb");
  if(client->inputfile == NULL)
    pthread_exit((int*)1);
  printf("Hey i opened the file\n");
  if(client->data != NULL)
    for(i=0;client->data[i] != '\0';i++)
      {
        fputc(client->data[i],client->inputfile);
        printf("%c\n",client->data[i]);
      }
  fclose(client->inputfile);
  return;
}


char * config(char* msg)
{
  char confvarValue[200];
  char confvar[200];
  char * returnValue;
  char firstchar;
  FILE * p;

  p = fopen("shaky.config","r");
  while(1)
    {
      fscanf(p, "%c", &firstchar);
      if(firstchar == EOF)
        {
          printf("I should be returning\n");
          return NULL;
        }
      else if(firstchar == '#')
        {
          char buffer[BUFFERSIZE];
          fgets(buffer, BUFFERSIZE, p);
          continue;
        }
      else
        {
          fseek(p, -1, SEEK_CUR);
        }

      fscanf(p,"%s %s\n", confvar, confvarValue);

      if(!strcmp(confvar,msg))
        {
          returnValue = (char *)malloc((strlen(confvarValue) + 1)*sizeof(char));
          strcpy(returnValue,confvarValue);
          return returnValue;
        }
    }
}
