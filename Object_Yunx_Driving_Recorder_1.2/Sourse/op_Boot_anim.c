/*******************************************************************************************
 *      @name   ShowGIF.c
 *      @brief  这是开机动画,使用BMP图像编解码技术实现对某个指定路径下的所有BMP格式的图像的解码，显示LCD上
 *      @date   2025/01/10 
 *      @author cecelmx@126.com 
 *      @note
 *          1. ARM平台搭载的是Linux系统，采用面向对象的思想“一切皆文件”，BMP图像属于规则文件
 *          2. BMP图像由2部分组成：图像属性54字节(14字节的文件头+40字节的信息头) + 图像的颜色分量
 *          3. 只打算在LCD上显示固定大小的BMP图像，打开BMP图像之后可以选择把文件光标向后偏移54字节 
 *          4. 对于BMP图像采用的是小端存储，所以BMP图像的每个像素点在文件中都是以BGR的顺序存储
 *          5. BMP图像的像素点的存储顺序是从下到上，从左到右
 *          6. LCD屏的像素点采用的是ARGB格式存储，所以BMP像素点BGR进行转换  B | G<<8 | R<<16
 *          7. 由于是在ARM开发板的LCD上显示BMP，所以BMP图像是需要下载到ARM开发板
 *          
 *          8. 如果想要实现开机动画的效果，则需要若干帧BMP图像，图像的命名Frame0.bmp ~ Frame81.bmp
 *          9. 确保FPS>=24HZ 所以调用usleep 以微妙为单位  1ms = 1000us
 * 
 *      @version  xx.xx.xx   主版本号.次版本号.修正版本号
 * 
 *      CopyRight (c)   2024-2025   Your Name     All Right Reserved
 * 
 * ****************************************************************************************/

#include"camera.h"

#define LCD_DEV     "/dev/fb0"

#define MMAP_SIZE   800*480*4




 
/*******************************************************************************************
 *      @name   Bmp_Decode
 *      @brief  BMP格式图像的解码接口，会把解码图像显示在LCD上
 *      @note
 *              1.只能解码分辨率是800*480的24bit色深的BMP     
*               2.打开LCD_DEV       
 *      @param  bmp_path:待解码的bmp图像的路径，是一个字符串常量
 * 
 *      @param  lcd_mp  :指的是LCD屏幕内存映射的起始地址，应该是mmap函数的返回值
 * 
 *      @retval 解码成功 返回true  解码失败 返回false   
 * ****************************************************************************************/
int op_dve()   //打开设备
{

    
    int lcd_fd;
    unsigned int *lcd_mp; //lcd的内存映射声明
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
}






bool Bmp_Decode(const char *bmp_path,unsigned int *lcd_mp) //转换图片像素并读入LCD屏幕
{
    //1.打开BMP图像  以二进制只读的方式打开 "rb"
    FILE *bmp_fp = fopen(bmp_path,"rb");

    if( NULL == bmp_fp)
    {
        fprintf(stderr,"fopen [%s] error,errno=%d,%s\n",bmp_path,errno,strerror(errno));
        return false;
    }

    //2.由于不需要读取图像的属性信息，所以把文件的位置指示器进行设置，向后偏移54字节
    fseek(bmp_fp,54,SEEK_SET);
   
    //3.循环从BMP图像中读取一行数据并转化为ARGB格式，写入到LCD对应的像素点
    
    char bufline[800*3] = {0}; //用于存储一行像素点的颜色分量

    for (size_t y = 0; y < 480; y++)
    {
        //读取BMP图像的一行数据 
        fread(bufline,800*3,1,bmp_fp);

        //BMP图片的像素点是24bit色深，所以应该把bufline中存储的颜色分量以3个字节为一组,循环处理 
        for (size_t x = 0; x < 800; x++)
        {
            int data = 0; //用于存储转换之后的像素点

            //BMP是小端存储，所以3个字节的颜色分量的顺序是BGR,需要转换为LCD像素点的存储格式ARGB
            data |= bufline[3*x+0];     //B 蓝色分量
            data |= bufline[3*x+1]<<8;  //G 蓝色分量
            data |= bufline[3*x+2]<<16; //R 蓝色分量

            //把转换之后的像素点的颜色分量写入到LCD的映射内存，从下向上，从左向右的顺序
            *(lcd_mp+(800*(479-y))+x) = data;
        }
    }

    return true;
}


// int main(int argc,char *argv[])
// {
//     char gif_path[256] ={0}; //用于存储转换好的图像路径

//     //1.打开LCD
//     int lcd_fd = open(LCD_DEV,O_RDWR);

//     if(-1 == lcd_fd)
//     {
//         fprintf(stderr,"open lcd error,errno=%d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }

//     //2.进行内存映射，为了提高LCD的刷新效率
//     unsigned int *lcd_mp = (unsigned int *)mmap(
//                                                     NULL, 
//                                                     MMAP_SIZE,
//                                                     PROT_READ|PROT_WRITE,
//                                                     MAP_SHARED,
//                                                     lcd_fd,
//                                                     0
//                                                 );

//     if( NULL == lcd_mp)
//     {
//         fprintf(stderr,"mmap error,errno=%d,%s\n",errno,strerror(errno));
//         exit(-1);
//     }

//     //3.在LCD显示开机动画,用户需要自己修改路径
//     for (size_t i = 0; i <= GIF_NUM; i++)
//     {
//         sprintf(gif_path,"/tmp/Object_Yunx_Driving_Recorder/Data/RunV_OpBoot_Anim/runv%d.bmp",i);  //构造名称
//         Bmp_Decode(gif_path,lcd_mp);                 //图像显示
//         usleep(20*1000);                             //FPS=50HZ
//     }
      
//     return 0;
// }