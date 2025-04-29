#include "camera.h"
#include <pthread.h>

pthread_mutex_t lcd_mutex = PTHREAD_MUTEX_INITIALIZER;  // LCD显示互斥锁，用于防止多线程同时访问显示设备
volatile bool program_running = true;  // 程序运行状态标志，volatile确保多线程能即时看到变量更新
ProgramState current_state = STATE_MAIN;

  // 定义按钮
struct ts_pix *OnTimets;  // 只在全局声明

   Button buttons[6] = 
    {
        {2, 480, 150, 50, "后视"},
        {220, 480, 150, 50, "录像"},
        {400, 480, 150, 50, "历史"},
        {600, 480, 150, 50, "返回"},
        {800, 480, 150, 50, "守护进程"},
        {950, 510, 72, 71, "电源"}
    };
int buttonCount = sizeof(buttons) / sizeof(Button);//按键计数

// 检测点是否在按钮区域内
bool isPointInButton(int x, int y, Button* btn) {
    return (x >= btn->x && x <= (btn->x + btn->width) &&
            y >= btn->y && y <= (btn->y + btn->height));
}

    int lcd_fd;
    unsigned int *lcd_mp;
    int ts_x,ts_y;
    struct input_event ts_event;
    char gif_path[256] ={0}; //用于存储转换好的图像路径
    char main_pictr_path[256] ={0}; //用于存储转换好的图像路径
    int ts_fd;

/****************************************************************************************/
 
 void* touch_thread(void* arg)
    {
                    
                        while(program_running)
                    {
                        
                        // 从触摸屏的设备文件中读取输入设备的类的属性并存储到类相关的变量中
                        read(ts_fd, &ts_event, sizeof(ts_event)); // read函数默认的属性是带有阻塞属性

                        // 检查事件类型
                        switch(ts_event.type) // 使用switch语句处理事件类型
                        {
                            case EV_ABS: // 处理绝对坐标事件
                                switch(ts_event.code) // 进一步判断坐标轴
                                {
                                    case ABS_X: // 说明是触摸屏的X轴
                                        ts_x = ts_event.value;
                                        // break;
                                    case ABS_Y: // 说明是触摸屏的Y轴
                                        ts_y = ts_event.value;
                                        // break;
                                }
                                break;

                            case EV_KEY: // 处理键盘事件和触摸状态
                                if(ts_event.code == BTN_TOUCH) // 检查触摸状态
                                {
                                    if(ts_event.value == 1) // 1表示触摸按下
                                    {
                                        // printf("按下\n", ts_event.code);// 表示按键按下
                                        // 只有在坐标值被更新后才输出
                                        printf("x = %d\t,y = %d\n", ts_x, ts_y);
                                        printf("--------------1\n");
                                        OnTimets->pix_x=ts_x;
                                        OnTimets->pix_y=ts_y;
                                        OnTimets->isPressed = true;
                                        handleTouchEvent(OnTimets, buttons, buttonCount);

                                        pthread_mutex_unlock(&lcd_mutex);  // 释放LCD互斥锁
                                        // 处理完后清零，避免重复触发
                                ts_x = 0;
                                ts_y = 0;
                                break;
                                
                                    }

                                    else if(ts_event.value == 0) // 0表示按键释放
                                    {
                                        printf("松开\n", ts_event.code);
                                        OnTimets->isPressed = false;
                                    }
                                }
                            
                                break;//结束switch语句
                        }

                        
                        // 4.输出值处理
                        usleep(10000);  // 短暂休眠，避免CPU占用过高
                        //  printf("Event: type=%d, code=%d, value=%d\n",ts_event.type, ts_event.code, ts_event.value);
                
                    }

                //编译TouchScreen4.c op_Boot_anim.c main.c
                        return NULL;
                
    }

int main()
{

   


    // op_dve();//打开屏幕LCD_DEV文件
   //0.初始化LCD(打开LCD+MMAP内存映射)
    lcd_fd = open(LCD_DEV,O_RDWR);

    //错误处理
    if(-1 == lcd_fd)
    {
        fprintf(stderr,"open lcd error,errno = %d,%s\n",errno,strerror(errno));
        exit(-1);
    }

    lcd_mp = (unsigned int *)mmap(
                                    NULL        ,               //由系统内核选择合适的位置
                                    LCD_MAPSIZE ,               //指的是要映射的内存的大小
                                    PROT_READ | PROT_WRITE,     //以可读可写的方式访问
                                    MAP_SHARED,                 //选择共享映射空间
                                    lcd_fd,                     //指的是待映射内存的文件描述符
                                    0                           //不偏移内存
                                );    
    //错误处理
    if(lcd_mp == MAP_FAILED)
    {
        fprintf(stderr,"mmap for lcd error,errno = %d,%s\n",errno,strerror(errno));
        exit(-1);
    }

    
  
    ts_fd = open(TS_DEV,O_RDWR);//打开触摸屏TS_DEV文件
 
  //错误处理
        if(-1 == ts_fd)
    {
        fprintf(stderr,"open touch screen error,errno = %d,%s\n",errno,strerror(errno));
        exit(-1);
    }



    /****************************************************************************************/
    // 为实时读取像素分配内存分配
    OnTimets = malloc(sizeof(struct ts_pix));
    if (OnTimets == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        exit(-1);
    }
    bzero(OnTimets,(sizeof(struct ts_pix)));


  //在LCD显示开机动画,用户需要自己修改路径
   for (size_t i = 1; i <= GIF_NUM; i++)
    {
        sprintf(gif_path,"/tmp/Object_Yunx_Driving_Recorder2/Data/RunV_OpBoot_Anim/runv%d.bmp",i);  //构造名称

        Bmp_Decode(gif_path,lcd_mp);                    //开机动画图像显示
       
        usleep(20*1000);                             //FPS=50HZ
    }
printf("--------------main1\n");


    //用于记录摄像头的设备文件的文件描述符
    int camera_fd = -1;

    unsigned int n_buffers= 0; //用于记录缓冲区的编号


    struct buffer *buffers = NULL; //用于记录申请的缓冲区信息

    /****************************************************************************************/

    //展示主界面
    sprintf(main_pictr_path,"/tmp/Object_Yunx_Driving_Recorder2/Data/runV1.bmp");  //构造名称

    Bmp_Decode(main_pictr_path,lcd_mp);                      //主界面图像显示
 printf("--------------main2\n");
    // tch_Scrn();
       // 创建线程
    pthread_t weather_tid, touch_tid;
   printf("--------------main3\n");
  // 创建时间天气显示线程
    if(pthread_create(&weather_tid, NULL, weather_time_thread, NULL) != 0) {
        printf("创建天气线程失败\n");
        return -1;

    }

    // 创建触摸检测线程
    if(pthread_create(&touch_tid, NULL, touch_thread, NULL) != 0) {
        printf("创建触摸线程失败\n");
        return -1;
    }
       printf("--------------main4\n");
    // 等待线程结束
    pthread_join(weather_tid, NULL);
    printf("--------------main5\n");
    pthread_join(touch_tid, NULL);
    printf("--------------main6\n");
    // 清理资源
    pthread_mutex_destroy(&lcd_mutex);

    return 0;
}
