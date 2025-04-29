#include "font.h"
#include<time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

time_t rawtime;
struct tm *info;
char timebuffer[80];

char *show_time(void)
{
   
   time( &rawtime );

   info = localtime( &rawtime );
  
   strftime(timebuffer,80,"%Y/%m/%e %H:%M:%S", info);//以年月日_时分秒的形式表示当前时间
  printf("%s", timebuffer);
 
  return(timebuffer);
    
}
// int main ()
// {
//     for(;;)
//     {
//         show_time();
//         printf("%s\n", timebuffer );
//         sleep(1);
//     }
//    return(0);
// }