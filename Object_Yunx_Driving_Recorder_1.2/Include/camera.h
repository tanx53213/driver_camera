#ifndef __camera_h
#define __camera_h
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include<math.h>
#include<stdbool.h>
#include <linux/input.h>
#include <pthread.h>
#include"showtime.h"
//宏:

//为了提高程序的可移植性，LCD的设备文件的路径通过宏定义实现
#define  LCD_DEV   "/dev/fb0"

//为了提高程序的可移植性，映射内存的大小通过宏定义实现
#define  LCD_MAPSIZE   800*480*4

//摄像头的设备文件的路径 用户需要根据实际情况进行修改
#define CAMERA_PATH  "/dev/video7"  

//为了提高程序的可移植性，触摸屏设备文件的路径通过宏定义实现
#define  TS_DEV   "/dev/input/event0"

//GIF开机动画图片数量
#define GIF_NUM     104

#define WEATHER_TXTPATH "./weather.txt"

// #define RECORD_PATH "/tmp/camera_records"

#define RECORD_PATH "/tmp/Object_Yunx_Driving_Recorder2/Data/camera_records"


#define MAX_RECORDS 200  // 最大录像数量
//变量声明:

extern int lcd_fd;
extern unsigned int *lcd_mp;

//结构体

typedef struct
 {
    int x;          // 按钮左上角X坐标
    int y;          // 按钮左上角Y坐标
    int width;      // 按钮宽度
    int height;     // 按钮高度
    char* text;     // 按钮文本
} Button;

  typedef struct ts_pix
{
    unsigned int pix_x;
  
    unsigned int pix_y;

    bool isPressed;       //是否按压
} ts_pix;



// 天气结构体
typedef struct 
{
	char Temperature[128];			// 体感温度
	char Relative_Humidity[128];	// 相对湿度
	char Wind[128];					// 风向
	char Weather[128];				// 天气：小雨、大雨、大小雪等
	char Sky_conditions[128];		// 天气环境：阴、晴、阴转晴，晴转阴等
	
}weather1;


struct buffer //摄像头缓冲
{
        void * start;   //申请的缓冲区的地址
        size_t length;  //申请的缓冲求的大小
};


// 在代码开头定义程序状态枚举类型
typedef enum {
    STATE_MAIN,      // 主界面
    STATE_BACKUP,    // 后视状态
    STATE_RECORD,    // 录像状态
    STATE_HISTORY,   // 历史状态
    STATE_GUARD      // 守护进程状态
} ProgramState;

// 全局状态变量
extern ProgramState current_state;  // 在头文件中声明

// 函数声明:

extern bool Bmp_Decode(const char *bmp_path,unsigned int *lcd_mp);  //打开图片将转换BMP像素点Data播放于LCDLCD
extern void Draw_line(void);										//画倒车辅助线
extern int op_dve();   												//打开LCD屏幕设备
//extern int Get_weather(weather1 *wea_p);

extern int op_ts_dev();												//打开屏幕触控文件
extern bool isPointInButton(int x, int y, Button* btn);       // 检测点是否在按钮区域内
extern void handleButtonPress(Button* button, int buttonIndex);       // 按钮点击处理函数逻辑
extern void handleTouchEvent(ts_pix* event, Button* buttons, int buttonCount);   //遍历按钮处理
extern void open_device(void);                                     //打开摄像头
extern void init_device(void);                                //初始化设备
extern void start_capturing(void);                           //开始视频采集

extern void mainloop(void);//主循环(缓冲区出队+进行像素转换+显示在LCD上+缓冲区入队)
extern unsigned int pixel_yuv2rgb(char Y,char Cb,char Cr);//转换一个像素点  YUV --> RGB

extern void yuyv_to_lcd(char *yuvbuf);//把YUV格式的图像显示在LCD上
extern void* weather_time_thread(void* arg);
extern void record_video();
extern void show_history();
extern void circular_recording();
extern void mainloop3(void);



#endif  