#ifndef __showtime_h_
#define __showtime_h_

#include<stdio.h>
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

 extern char timebuffer[80];

 extern char *show_time(void);

 #endif