/*******************************************************************************************
 *      @name   TouchScreen.c
 *      @brief  这是系统IO的关于文件访问接口实现对linux系统下的触摸屏设备的访问，用于获取坐标值
 *      @date   2025/01/10 
 *      @author cecelmx@126.com 
 *      @note
 *          1. ARM开发板搭载的是linux系统，而linux系统下有关于输入设备的子系统(输入子系统)
 *          2. 输入子系统提供了关于触摸屏的设备文件 设备文件的路径"/dev/input/event0" 
 *          3. ARM开发板搭载的触摸屏属于电容式触摸屏，触摸屏的坐标系分为两类
 *          4. 蓝色边框的触摸屏，坐标范围是(800,480 ),所以和LCD屏的分辨率一致，所以不需要转换
 *          5. 黑色边框的触摸屏，坐标范围是(1024,600),所以和LCD屏的分辨率不同，所以需要转换！！
 *          6. linux系统的输入设备采用的类一般都是定义在input.h头文件中  路径 <linux/input.h>
 * 
 *      @version  xx.xx.xx   主版本号.次版本号.修正版本号
 * 
 *      CopyRight (c)   2024-2025   Your Name     All Right Reserved
 * 
 * ****************************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>
//为了提高程序的可移植性，触摸屏设备文件的路径通过宏定义实现
#define  TS_DEV   "/dev/input/event0"
   int ts_x;
   int ts_y;
    struct input_event ts_event;
    
    struct ts_pix
{
    unsigned int pix_x;
  
    unsigned int pix_y;
} ;

struct ts_pix *OnTimets=NULL;
struct ts_pix *SetButtonts=NULL;


 int tch_Scrn(int fd);

  
// //按钮
// void button_ts(OnTimets)
// {
// int buff[]=0;
// }

// int LCD_botton_ts(ts_fd)
// {
//     tch_Scrn(ts_fd,OnTimets);
//     for(;;)
//     {

//     }
// }


int main()
{
int ts_fd;//触控

      //1.打开触摸屏，linux系统下把硬件设备抽象为文件，所以可以访问硬件设备的设备文件
   
    ts_fd = open(TS_DEV,O_RDWR); //打开触控屏幕

    //错误处理
    if(-1 == ts_fd)
    {
        fprintf(stderr,"open touch screen error,errno = %d,%s\n",errno,strerror(errno));
        exit(-1);
    }
    

  printf("=============1\n");

 
tch_Scrn(ts_fd);

 printf("=============6\n");
    return 0;
}

 int tch_Scrn(int ts_fd)
   {
  printf("=============2\n");
 for(;;)
    {
        
 printf("=============4\n");
        ts_x=0;//清空
        ts_y=0;//清空
        //2.从触摸屏的设备文件中读取输入设备的类的属性并存储到类相关的变量中
         printf("=============41\n");
        read(ts_fd,&ts_event,sizeof(ts_event)); //read函数默认的属性是带有阻塞属性
  printf("=============5\n");
        //3.分析输入设备的类型以及编码以及获取对应的数值
      

             printf("按下");
  if(ts_event.type == EV_ABS && ts_event.code == ABS_X)      //说明是触摸屏的X轴
        {
              printf("=============7\n");
            ts_x = ts_event.value;
        }
      else if(ts_event.type == EV_ABS && ts_event.code == ABS_Y) //说明是触摸屏的Y轴
        {
            ts_y = ts_event.value;
        }


        }
   
       
       printf("x = %d\t,y = %d\n",ts_x,ts_y);  //输出结果
       usleep(100);
    }
    
   
