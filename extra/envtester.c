#include<stdlib.h>

main()
{

  char* state;
  setenv("QUASI_STATE","active",1);

  if( (state=getenv("QUASI_STATE"))==NULL)puts("Variable not set");

  puts(state);
}
