#include <stdio.h>
#include <string.h>



int main_te1(int argc, char *argv[])
{
  int i;
  char str[100];

 
  while(strcmp(str,"exit")!=0){

    /* Make question and read user's input */
    printf("A> Hi. What do you want?\nY> ");

    fgets(str, 100, stdin);
    i = strlen(str)-1;
    if( str[ i ] == '\n') str[i] = '\0';


    /* Decision Logic */
    printf("A> Sure!\n\n");

  }

  
  return 0;
}

