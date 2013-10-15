# include<stdio.h>
# include<string.h>
# define BUFFERSIZE 200


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

      fscanf(p,"%s %s", confvar, confvarValue);

      if(!strcmp(confvar,msg))
        {
          returnValue = (char *)malloc((strlen(confvarValue) + 1)*sizeof(char));
          strcpy(returnValue,confvarValue);
          return returnValue;
        }
    }
}
