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
#include"camera.h"
//为了提高程序的可移植性，触摸屏设备文件的路径通过宏定义实现
#define  TS_DEV   "/dev/input/event0"





// struct ts_pix *OnTimets = NULL;  // 只在全局声明

//    Button buttons[6] = 
//     {
//         {2, 480, 150, 50, "后视"},
//         {220, 480, 150, 50, "录像"},
//         {400, 480, 150, 50, "历史"},
//         {600, 480, 150, 50, "返回"},
//         {800, 480, 150, 50, "守护进程"},
//         {950, 510, 72, 71, "电源"}
//     };
// int buttonCount = sizeof(buttons) / sizeof(Button);//按键计数

// // 检测点是否在按钮区域内
// bool isPointInButton(int x, int y, Button* btn) {
//     return (x >= btn->x && x <= (btn->x + btn->width) &&
//             y >= btn->y && y <= (btn->y + btn->height));
// }
extern int buttonCount;


// 按钮点击处理函数
void handleButtonPress(Button* button, int buttonIndex) //将i作为常量索引
{
    if (button == NULL || buttonIndex < 0 || buttonIndex >= buttonCount) {
        printf("无效的按钮索引\n");
        return;
    }

    // printf("按下%s\n", button->text);
    
    // 根据按钮索引执行不同操作
    switch(buttonIndex) {
        case 0:  // 后视
    
                                            //1.打开摄像头
                                            open_device();

                                            //2.初始化设备
                                            init_device();

                                            //3.开始捕获
                                            start_capturing();

                                            //4.开始转换
                                            mainloop();
            printf("按下后视按钮\n");
            break;
        case 1:  // 录像
            printf("按下录像按钮\n");
            break;
        case 2:  // 历史
            printf("按下历史按钮\n");
            break;
        case 3:  // 返回
            printf("按下返回按钮\n");
            break;
        case 4:  // 守护进程
            printf("按下守护进程按钮\n");
            break;
        case 5:  // 电源
            printf("按下电源按钮\n");
            break;
        default:
            printf("未知按钮\n");
            break;
    }
}
// 处理触摸事件
void handleTouchEvent(ts_pix* event, Button* buttons, int buttonCount)
 {
    if (!event->isPressed) {
        return;  // 未检测到按压，直接返回
    }
    
    // // 遍历所有按钮检查是否被点击
    for (int i = 0; i < buttonCount; i++) {
        if (isPointInButton(event->pix_x, event->pix_y, &buttons[i]))
         {
          
            // 按钮的具体处理逻辑
            handleButtonPress(&buttons[i],i);

            break;
        }
    

     }
}

// int main()
// {
//     int ts_x,ts_y;
//     struct input_event ts_event;
// printf("计数等于%d",buttonCount);
//     // 将内存分配移到这里
//     OnTimets = malloc(sizeof(struct ts_pix));
//     if (OnTimets == NULL)
//     {
//         fprintf(stderr, "内存分配失败\n");
//         exit(-1);
//     }
// bzero(OnTimets,(sizeof(struct ts_pix)));
//     //1.打开触摸屏，linux系统下把硬件设备抽象为文件，所以可以访问硬件设备的设备文件
//     int ts_fd = open(TS_DEV,O_RDWR);
    

//     //错误处理
//     if(-1 == ts_fd)
//     {
//         fprintf(stderr,"open touch screen error,errno = %d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }
//     for(;;)
//     {
//         // 从触摸屏的设备文件中读取输入设备的类的属性并存储到类相关的变量中
//         read(ts_fd, &ts_event, sizeof(ts_event)); // read函数默认的属性是带有阻塞属性

//         // 检查事件类型
//         switch(ts_event.type) // 使用switch语句处理事件类型
//         {
//             case EV_ABS: // 处理绝对坐标事件
//                 switch(ts_event.code) // 进一步判断坐标轴
//                 {
//                     case ABS_X: // 说明是触摸屏的X轴
//                         ts_x = ts_event.value;
//                         // break;
//                     case ABS_Y: // 说明是触摸屏的Y轴
//                         ts_y = ts_event.value;
//                         // break;
//                 }
//                 break;

//             case EV_KEY: // 处理键盘事件和触摸状态
//                 if(ts_event.code == BTN_TOUCH) // 检查触摸状态
//                 {
//                     if(ts_event.value == 1) // 1表示触摸按下
//                     {
//                         // printf("按下\n", ts_event.code);// 表示按键按下
//                         // 只有在坐标值被更新后才输出
//                         printf("x = %d\t,y = %d\n", ts_x, ts_y);
//                         printf("--------------1\n");
//                         OnTimets->pix_x=ts_x;
//                         OnTimets->pix_y=ts_y;
//                         OnTimets->isPressed = true;
//                         handleTouchEvent(OnTimets, buttons, buttonCount);


//                          // 处理完后清零，避免重复触发
//                 ts_x = 0;
//                 ts_y = 0;
//                 break;
                
//                     }

//                      else if(ts_event.value == 0) // 0表示按键释放
//                     {
//                         printf("松开\n", ts_event.code);
//                         OnTimets->isPressed = false;
//                     }
//                 }
            
//                 break;//结束switch语句
//         }

        
//         // 4.输出结果
        
//          printf("Event: type=%d, code=%d, value=%d\n",ts_event.type, ts_event.code, ts_event.value);
//     }
   
   

//     return 0;
// }
