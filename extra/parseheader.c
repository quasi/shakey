# include<stdio.h>
# include "quasihttp.h"

struct headerfield * getHeader(char *);
void displayheader(struct headerfield *);

int main()
{
  FILE *fp;
  struct headerfield *head;
  char buffer[1024],c;
  int i;
  fp = fopen("header","r");
  if(fp == NULL)
    {
      printf("Could not open file\n");
      exit(1);
    }
  for(i = 0;c!=EOF;c=fgetc(fp),(buffer[i++] = c));
  head = getHeader(buffer);
  displayheader(head);
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
      for(ptr = ptr1 ; *ptr != '\n' ; ptr++)
        {
          if(*ptr=='\0')
            {
              printf("Going to return\n");
              return head;
            }
        }
      *ptr='\0';
      ptr++;
      ptr1 = ptr;
      for(ptr1 = ptr ; *ptr1 != 32 ; ptr1++)
        {
          if(*ptr1=='\0')
            return head;
        }
      *ptr1='\0';
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

void displayheader(struct headerfield * head)
{
  struct headerfield *temp;
  for(temp=head ; temp!=NULL ; temp = temp->next)
    {
      if(temp->key == NULL)
        printf("Could not print\n");
      printf("%s\n",temp->key);
      printf("%s\n",temp->value);
    }
}
