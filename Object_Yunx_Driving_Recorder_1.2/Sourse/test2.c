#include "font.h"
#include"showtime.h"
#include "camera.h"
#include <pthread.h>

//为了提高程序的可移植性，LCD的设备文件的路径通过宏定义实现
#define  LCD_DEV   "/dev/fb0"

//为了提高程序的可移植性，映射内存的大小通过宏定义实现
#define  LCD_MAPSIZE   800*480*4
// 天气结构体

    unsigned int *lcd_mp;

// 全局变量声明

static pthread_mutex_t lcd_mutex = PTHREAD_MUTEX_INITIALIZER;  // LCD显示互斥锁，用于防止多线程同时访问显示设备
static volatile bool program_running = true;  // 程序运行状态标志，volatile确保多线程能即时看到变量更新
 char timebuffer[80];
char *show_time();
  extern int Get_weather2(weather1 *wea_p);
 char my_pic_pictr_path[256] ={0}; //用于存储转换好的图像路径
struct LcdDevice *lcd;  // 声明为全局变量
struct LcdDevice *init_lcd(const char *device)
{
	//申请空间
	struct LcdDevice* lcd = malloc(sizeof(struct LcdDevice));//为字体申请空间
	if(lcd == NULL)
	{
		perror("malloc failed");
		return NULL;
	} 

	//1打开设备
	lcd->fd = open(device, O_RDWR);
	if(lcd->fd < 0)
	{
		perror("open lcd fail");
		free(lcd);
		return NULL;
	}

	//映射
	lcd->mp = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd->fd,0);

	return lcd;
}
// // void display_text();
// int main()
// {	

// 	op_dve();
	
// 	printf("------------1\n");
//     //初始化Lcd
// 	  lcd = init_lcd("/dev/fb0");  // 全局变量初始化
// 	if (lcd == NULL) {
// 		return -1;
// 	}
// 	printf("------------2\n");	
// 	display_text();
// return 0;
// }

void* weather_time_thread(void* arg)
{	
	 // 将void指针转换为unsigned int指针
    unsigned int *lcd_mp = (unsigned int*)arg;
//打开字体	
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");

	printf("%p\n",f);

	//字体大小的设置
	fontSetSize(f,20);


	bitmap *bm;
	char my_pic_pictr_path[256] ={0}; //用于存储转换好的图像路径

	// 构造天气
	weather1 wea;
	

	 while(program_running)
	{
	
	 pthread_mutex_lock(&lcd_mutex);
	// sprintf(my_pic_pictr_path,"/tmp/Object_Yunx_Driving_Recorder2/Data/runV1.bmp");//图片路径
												
    	// Bmp_Decode(my_pic_pictr_path,lcd->mp);									//转换图片像素,显示一张主界面图片	

		bitmap *bm = createBitmapWithInit(150,20,4,getColor(50,255,255,0)); //构造时间输入框,也可使用createBitmapWithInit函数，改变画板颜色
		bitmap *bw = createBitmapWithInit(30,20,4,getColor(50,255,255,0)); //构造天气输入框,

		// char *str = show_time();   											//获取时间
		// char * Temtur=weg_p->Temperature;
	
		// fontPrint(f,bw,0,0,Temtur,getColor(0,28,106,224),0);		//显示天气到输入框
		// fontPrint(f,bm,0,0,str,getColor(0,28,106,224),0);	
		//显示时间到输入框
		show_font_to_lcd(lcd_mp,150,80,bm);

		show_font_to_lcd(lcd_mp,150,100,bw);
	
		printf("-------------7\n");  
		destroyBitmap(bm);                                      //刷新缓存
		destroyBitmap(bw);
		
		// 释放LCD互斥锁
        pthread_mutex_unlock(&lcd_mutex);
	
		sleep(1);
	
	}

	fontUnload(f);
//编译test.c testtime.c op_Boot_anim.c get_weathe2.c  -o main_ok   -I ./ -L./Library -lfont  -lm 
	return NULL;
}