#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
int test(char *buffer,char *str);
char* getsecondparameter(char *buffer);




int main(int argc, char **argv)
{
  int socketdes, ret, datalen, i, flag,dummy,c;
  struct sockaddr_in server_name;
  char sendbuffer[1024], recvbuffer[1024];
  FILE *fp;
  sendbuffer[0] = '\0';
  if(argc != 2)
    {
      printf("usage : client <server IP>\n");
      exit(1);
    }
  datalen = strlen(sendbuffer);
  socketdes = socket(AF_INET, SOCK_STREAM, 0);

  if(socketdes < 0)
    exit(1);
  bzero(&server_name,sizeof(server_name));
  server_name.sin_family = AF_INET;
  server_name.sin_port = htons(1560);
  ret = inet_pton(AF_INET, argv[1] ,&server_name.sin_addr);
  if(ret < 0)
    exit(1);
  ret = connect(socketdes, &server_name, sizeof(server_name));
  if(ret < 0)
    {
      printf("Error connecting........\n");
      exit(1);
    }
  printf("Connected to server\n");
  bzero(sendbuffer,1024);
  strcpy(sendbuffer,"GET /~pankaj/join.gif HTTP/1.0");
  sendbuffer[strlen(sendbuffer)] = (char)13;
  sendbuffer[strlen(sendbuffer)] = (char)10;
  sendbuffer[strlen(sendbuffer)] = (char)13;
  sendbuffer[strlen(sendbuffer)] = (char)10;
  send(socketdes,sendbuffer,strlen(sendbuffer),0);
  fp = fopen("logs1","w b");
  while (1)
    {

      bzero(recvbuffer,1024);
      datalen = recv(socketdes,recvbuffer,1024,0);
      if(datalen != 0)
        {
          for(i=0;i<datalen;i++)
            {
              fputc(recvbuffer[i],fp);
              printf("%c",recvbuffer[i]);
            }
        }
      else
        break;
    }
  fclose(fp);
  close(socketdes);
}
