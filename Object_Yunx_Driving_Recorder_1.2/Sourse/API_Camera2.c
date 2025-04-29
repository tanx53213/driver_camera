/*******************************************************************************************
 *      @name   camera.c
 *      @brief  这是系统IO的关于文件访问接口实现对linux系统下的摄像头设备的访问，用于获取图像
 *      @date   2025/01/13 
 *      @author cecelmx@126.com 
 *      @note
 *          1. ARM开发板搭载的是linux系统，而linux系统下有关于视频设备的V4L2驱动框架
 *          2. V4L2提供了关于视频捕捉设备的设备文件 ARM开发板下的设备文件的路径"/dev/video7" 
 *        
 * 
 *      @version  xx.xx.xx   主版本号.次版本号.修正版本号
 * 
 *      CopyRight (c)   2024-2025   Your Name     All Right Reserved
 * 
 * ****************************************************************************************/

#include "camera.h"


int lcd_fd;
unsigned int *lcd_mp;



//用于记录摄像头的设备文件的文件描述符
int camera_fd = -1;

unsigned int n_buffers= 0; //用于记录缓冲区的编号



struct buffer *buffers = NULL; //用于记录申请的缓冲区信息



//转换一个像素点  YUV --> RGB
unsigned int pixel_yuv2rgb(char Y,char Cb,char Cr)
{
    unsigned int pixel = 0; //用于存储转换后的可以在LCD上显示的像素点

    unsigned int R;
    unsigned int G;
    unsigned int B;

    //转换算法
    R =  (255/219)*(Y-16) + 1.402*(127/112)*(Cr-128);
    G =  (255/219)*(Y-16) - 0.344*(127/112)*(Cb-128) - 0.714*(127/112)*(Cr-128);
    B =  (255/219)*(Y-16) + 1.772*(127/112)*(Cb-128);

    //边界处理
    if(R>255) R = 255;
    if(G>255) G = 255;
    if(B>255) B = 255;

    if(R<0)   R=0;
    if(G<0)   G=0;
    if(B<0)   B=0;
   
   pixel = R<<16 | G<<8 | B;

   return pixel;
}
//
// unsigned int pixel_yuv2rgb2(char Y,char Cb,char Cr)
// {
//     unsigned int pixel = 0; //用于存储转换后的可以在LCD上显示的像素点

//     unsigned int R;
//     unsigned int G;
//     unsigned int B;

//     //转换算法
//     R =  (255/219)*(Y-16) + 1.402*(127/112)*(Cr-128);
//     G =  (255/219)*(Y-16) - 0.344*(127/112)*(Cb-128) - 0.714*(127/112)*(Cr-128);
//     B =  (255/219)*(Y-16) + 1.772*(127/112)*(Cb-128);

//     //边界处理
//     if(R>255) R = 255;
//     if(G>255) G = 255;
//     if(B>255) B = 255;

//     if(R<0)   R=0;
//     if(G<0)   G=0;
//     if(B<0)   B=0;
   
//  fput(B,bmp_fp);
//  fput(G,bmp_fp);
//  fput(R,bmp_fp);

//    return pixel;
// }
// // int Bmpopen_funtion()                                             
// //    {//打开BMP图片文件 并把文件光标向后偏移54字节
// //    char buf_line[800*3]={0};
// //     FILE *bmp_fp = fopen("./1.bmp","r+");
// //     //int bmp_fd = open(picname, O_RDWR);
    

// // fseek(bmp_fp,54,SEEK_SET);
// // for(int y=0;y<800;y++)
// // {
// //     fread(buf_line,3*800,1,bmp_fp);//读取一行颜色分量
// //    // fread(buf_line,3*800,1,);
// //     for(int x=0;x<480;x++)
// //     {
// //         //将BMP颜色分量写入LCD屏
// //        *( lcd_mp+800*(480-y-1)+x) = buf_line[x*3+2]<<16 | buf_line[x*3+1]<<8 | buf_line[x*3];//RGB
// //     }
// // }

// //以行为单位循环读取BMP图片的颜
//    }

//把YUV格式的图像显示在电脑上
void yuyv_to_lcd2(char *yuvbuf)
{   
    //打开BMP图片文件 并把文件光标向后偏移54字节
   char buf_line[800*3]={0};
    FILE *bmp_fp = fopen("./1.bmp","a+");
    //int bmp_fd = open(picname, O_RDWR);
    

fseek(bmp_fp,54,SEEK_SET);
    size_t i = 0,j = 0;
   
    unsigned int rgbbuf[320*240] = {0}; //相当于是缓冲区

    //YUYV是用4个字节表示2个像素点 
    for (i = 0,j = 0; i < 320*240; i+=2,j+=4)
    {
       pixel_yuv2rgb(yuvbuf[j+0],yuvbuf[j+1],yuvbuf[j+3]);  //Y1 U V -- ARGB
       pixel_yuv2rgb(yuvbuf[j+2],yuvbuf[j+1],yuvbuf[j+3]);  //Y2 U V -- ARGB 
    }
    
    size_t x = 0,y = 0;

    for (size_t y = 0; y <240; y++)
    {
         fread(buf_line,3*800,1,bmp_fp);//读取一行颜色分量
        for (size_t x = 0; x < 320; x++)
        {
            lcd_mp[y*800+x] = rgbbuf[y*320+x]; 
        } 
    } 
}

//把YUV格式的图像显示在LCD上
void yuyv_to_lcd(char *yuvbuf)
{   
    size_t i = 0,j = 0;
   
    unsigned int rgbbuf[640*480] = {0}; //相当于是缓冲区

    //YUYV是用4个字节表示2个像素点 
    for (i = 0,j = 0; i < 640*480; i+=2,j+=4)
    {
        rgbbuf[i+0] = pixel_yuv2rgb(yuvbuf[j+0],yuvbuf[j+1],yuvbuf[j+3]);  //Y1 U V -- ARGB
        rgbbuf[i+1] = pixel_yuv2rgb(yuvbuf[j+2],yuvbuf[j+1],yuvbuf[j+3]);  //Y2 U V -- ARGB 
    }
    
    size_t x = 0,y = 0;

//     for (size_t y = 0; y <480; y++)
//     {
//         for (size_t x = 0; x < 640; x++)
//         {
//             lcd_mp[y*800+x] = rgbbuf[y*640+x]; 
//         } 
//     } 
// }
   for (size_t y = 0; y <480; y++)
    {
        for (size_t x = 0; x < 640; x++)
        {
            lcd_mp[y*800+x+80] = rgbbuf[y*640+x]; 
        } 
    } 
}

//打开摄像头
void open_device(void)
{
      
    //打开摄像头，以非阻塞的方式打开摄像头，read或者write不会阻塞
    camera_fd = open (CAMERA_PATH, O_RDWR | O_NONBLOCK, 0);

    //错误处理
    if (-1 == camera_fd) 
    {
            fprintf (stderr, "Cannot open '%s': %d, %s\n",
                                                CAMERA_PATH, errno, strerror (errno));
            exit (-1); //终止程序
    }
}


//初始化设备
void init_device(void)
{
    //定义摄像头的相关类
    struct v4l2_cropcap cropcap;    //用于记录视频设备的裁剪功能和缩放功能信息
    struct v4l2_format fmt;         //用于记录视频设备的捕捉画面的格式信息
    struct v4l2_requestbuffers req; //用于记录申请的缓冲区的信息

    //1.应用程序应该设置视频设备的数据流的类型，如果需要捕获图像，应该设置V4L2_BUF_TYPE_VIDEO_CAPTURE
	memset (&cropcap,0,sizeof(cropcap));        //清空内存
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; //设置参数
    ioctl(camera_fd, VIDIOC_S_CROP, &cropcap);  //发送请求
    
    
    //2.设置摄像头的采集的图像的格式(帧宽度、帧高度、图像格式...)
	memset (&fmt,0,sizeof(fmt));                            //清空内存
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;  //数据流的类型
    fmt.fmt.pix.width       = 640;                          //图像的宽度，目前使用的摄像头支持(640,480) or (320,240)
    fmt.fmt.pix.height      = 480;                          //图像的高度，目前使用的摄像头支持(640,480) or (320,240)
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;            //目前使用的摄像头只支持YUYV格式(像素点的颜色分量编码格式)
    ioctl(camera_fd, VIDIOC_S_FMT, &fmt);                   //发送请求

    //3.为了提高摄像头采集的速率，所以为摄像头申请缓冲区，用于存储采集的图像的颜色分量，一般申请4块缓冲区
    memset (&req,0,sizeof(req));                            //清空内存
    req.count               = 4;                            //缓冲区的数量
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;  //数据流的类型
    req.memory              = V4L2_MEMORY_MMAP;             //内存映射方式
    ioctl(camera_fd, VIDIOC_REQBUFS, &req);                 //发送请求

    buffers = calloc(req.count, sizeof (*buffers));    //申请内存，用于记录申请到的缓冲区的信息

    //循环申请用于存储图像的缓冲区，并把每个缓冲区的信息都存储到刚才申请的堆空间
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
    {
        //用于记录申请成功的缓冲区
        struct v4l2_buffer buf;

        memset (&buf,0,sizeof(buf));                        //清空内存
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;      //数据流的类型
        buf.memory      = V4L2_MEMORY_MMAP;                 //内存映射方式
        buf.index       = n_buffers;                        //缓冲区的编号
        ioctl (camera_fd, VIDIOC_QUERYBUF, &buf);           //发送请求

        buffers[n_buffers].length = buf.length;             //记录信息
        buffers[n_buffers].start  = mmap(                   //内存映射
                                            NULL,
                                            buf.length,
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED ,
                                            camera_fd, 
                                            buf.m.offset
                                        );
    }
}   


//开始采集
void start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    //循环把缓冲区入队，此时摄像头才会把采集的画面存储到缓冲区中
    for (i = 0; i < n_buffers; ++i) 
    {
        struct v4l2_buffer buf;

        memset (&buf,0,sizeof(buf));                    //清空内存
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;  //数据流的类型
        buf.memory      = V4L2_MEMORY_MMAP;             //内存映射方式
        buf.index       = i;                            //缓冲区的编号
        ioctl (camera_fd, VIDIOC_QBUF, &buf);           //把缓冲区入队

    }
    
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;                 //数据流的类型
    ioctl (camera_fd, VIDIOC_STREAMON, &type);          //开始进行捕获
}



void mainloop(void)      //主循环(缓冲区出队+进行像素转换+显示在LCD上+缓冲区入队)
{
	unsigned int count;
    struct v4l2_buffer buf;
	unsigned int i;

    count = 100;

    while (count-- > 0 && current_state == STATE_BACKUP) 
    {
        //死循环
        for (;;) 
        {
            //添加了超时机制，去检测文件描述符的读状态
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO (&fds);
            FD_SET (camera_fd, &fds);

            //超时时间是2s，也就是如果超过2s摄像头的读状态没发生变化，说明不可读
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select (camera_fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) 
            {
                if (EINTR == errno)
                        continue;
            }

            if (0 == r) {
                    fprintf (stderr, "select timeout\n");
                    exit (EXIT_FAILURE);
            }

            //如果缓冲区中存储画面，则把缓冲区出队
            for(int i =0;i < n_buffers; ++i)
            {
                struct v4l2_buffer buf;
                memset (&buf,0,sizeof(buf));                    //清空内存
                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;  //数据流的类型
                buf.memory      = V4L2_MEMORY_MMAP;             //内存映射方式
                ioctl(camera_fd, VIDIOC_DQBUF, &buf);           //把缓冲区出队


                //应该用户把出队的缓冲区的像素进行转换，并显示在LCD上
                yuyv_to_lcd(buffers[i].start);
                Draw_line();

                //处理完成后，则把缓冲区入队
                ioctl (camera_fd, VIDIOC_QBUF, &buf);           //把缓冲区入队
            }          
          if (current_state != STATE_BACKUP)
          {
            break;  // 如果状态改变，退出循环
        }
        
        }
    
    }
}


void mainloop3(void)      //主循环(缓冲区出队+进行像素转换+显示在LCD上+缓冲区入队)
{
	unsigned int count;
    struct v4l2_buffer buf;
	unsigned int i;

    count = 100;

    while (count-- > 0 && current_state == STATE_BACKUP) 
    {
        //死循环
        for (;;) 
        {
            //添加了超时机制，去检测文件描述符的读状态
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO (&fds);
            FD_SET (camera_fd, &fds);

            //超时时间是2s，也就是如果超过2s摄像头的读状态没发生变化，说明不可读
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select (camera_fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) 
            {
                if (EINTR == errno)
                        continue;
            }

            if (0 == r) {
                    fprintf (stderr, "select timeout\n");
                    exit (EXIT_FAILURE);
            }

            //如果缓冲区中存储画面，则把缓冲区出队
            for(int i =0;i < n_buffers; ++i)
            {
                struct v4l2_buffer buf;
                memset (&buf,0,sizeof(buf));                    //清空内存
                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;  //数据流的类型
                buf.memory      = V4L2_MEMORY_MMAP;             //内存映射方式
                ioctl(camera_fd, VIDIOC_DQBUF, &buf);           //把缓冲区出队


                //应该用户把出队的缓冲区的像素进行转换，并显示在LCD上
                yuyv_to_lcd(buffers[3].start);
                

                //处理完成后，则把缓冲区入队
                ioctl (camera_fd, VIDIOC_QBUF, &buf);           //把缓冲区入队
            }          
          if (current_state != STATE_BACKUP)
          {
            break;  // 如果状态改变，退出循环
        }
        
        }
    
    }
}




















// int main()
// {
//     //0.初始化LCD(打开LCD+MMAP内存映射)
//     lcd_fd = open(LCD_DEV,O_RDWR);

//     //错误处理
//     if(-1 == lcd_fd)
//     {
//         fprintf(stderr,"open lcd error,errno = %d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }

//     lcd_mp = (unsigned int *)mmap(
//                                     NULL        ,               //由系统内核选择合适的位置
//                                     LCD_MAPSIZE ,               //指的是要映射的内存的大小
//                                     PROT_READ | PROT_WRITE,     //以可读可写的方式访问
//                                     MAP_SHARED,                 //选择共享映射空间
//                                     lcd_fd,                     //指的是待映射内存的文件描述符
//                                     0                           //不偏移内存
//                                 );    
//     //错误处理
//     if(lcd_mp == MAP_FAILED)
//     {
//         fprintf(stderr,"mmap for lcd error,errno = %d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }

//  // 绘制辅助线条
 
//     // //
//     //  // 绘制狙击镜准星
//     // int centerX = 400; // 圆心X坐标
//     // int centerY = 240; // 圆心Y坐标
//     // int radius = 20;   // 圆的半径

//     // for (y = -radius; y <= radius; y++)
//     // {
//     //     int xOffset = (int)sqrt(radius * radius - y * y); // 计算x偏移
//     //     *(lcd_mp + 800 * (centerY + y) + (centerX + xOffset)) = 0xFFFFFF; // 右侧
//     //     *(lcd_mp + 800 * (centerY + y) + (centerX - xOffset)) = 0xFFFFFF; // 左侧
//     // }
//     //1.打开摄像头
//     open_device();

//     //2.初始化设备
//     init_device();

//     //3.开始捕获
//     start_capturing();

//     //4.开始转换
//     mainloop();

 
// }



