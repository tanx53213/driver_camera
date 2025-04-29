#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define weather1_TXTPATH "./weather.txt"



// 天气结构体
typedef struct 
{
	char Temperature[128];			// 体感温度
	char Relative_Humidity[128];	// 相对湿度
	char Wind[128];					// 风向
	char weather1[128];				// 天气：小雨、大雨、大小雪等
	char Sky_conditions[128];		// 天气环境：阴、晴、阴转晴，晴转阴等
	
}weather1;

// 获取天气信息，并分析
// int Get_weather1(weather1 *wea_p, char *city)
int Get_weather(weather1 *wea_p)
{
	// 变量
	char weather1_buf[2048] = {0};
	char buf[256] = {0};
	int  i        = 0;
    char splicing_buf[256] = {0};

    // sprintf(splicing_buf, "touch %s",weather1_TXTPATH);
	// system(splicing_buf);		                            // 创建天气txt文件，并设置其权限(不然可能会出问题)
    // wait(NULL); 											// 等待(必须等它执行完，才继续下面的内容)

    // bzero(splicing_buf, sizeof(splicing_buf));
    // sprintf(splicing_buf, "chmod 777 %s",weather1_TXTPATH);
	// system(splicing_buf);
	// wait(NULL); 											// 等待(必须等它执行完，才继续下面的内容)
	
	
	// // 1、Ubuntu系统安装(weather1-util)：命令：sudo apt install weather1-util
	// // 2、使用重定向命令，将获取的天气信息，写入指定的weather1.txt中
	// sprintf(buf, "weather1-util %s > %s", city, weather1_TXTPATH);
	// system(buf); 			// 执行buf里的shell命令
	// wait(NULL); 			// 等待
	
	// 3、对weather1.txt进行解析
	// a、打开文件(只读模式)
	FILE * fp = fopen(weather1_TXTPATH, "rb");
	if( fp == NULL)
	{
		perror("Msg_RecvMsg: fopen失败\n");	
		return -1;
	}


	// b、从文件读取命令数据
	size_t ret = fread(weather1_buf, 1, 2048, fp);
	if(ret == 0)
	{
		perror("Msg_RecvMsg: fread失败\n");
		return -2;
	}

	// c、解析
	// 获取温度
	char *p = strstr(weather1_buf,  "Temperature:");
	if(p != NULL)
	{
		 p = p+12;
		
		i = 0;
		wea_p->Temperature[0] = *(p+0);
		wea_p->Temperature[1] = *(p+1);
		wea_p->Temperature[2] = *(p+2);
	}
	else
	{
		strcpy(wea_p->Temperature, "暂无数据！");
	}
		
	// 获取相对湿度
	p = strstr(weather1_buf,  "Relative_Humidity:");
	if(p != NULL)
	{
		 p = p+18;

		i = 0;
		wea_p->Relative_Humidity[0] = *(p+0);
		wea_p->Relative_Humidity[1] = *(p+1);
		wea_p->Relative_Humidity[2] = *(p+2);
	}
	else
	{
		strcpy(wea_p->Relative_Humidity, "暂无数据！");
	}
	
	// 获取风向
	p = strstr(weather1_buf,  "Wind: ");
	if(p != NULL)
	{
		p = p+19;

		i = 0;
		while(*p != '\n')
		{
			wea_p->Wind[i] = *p;
			i++;
			p++;
		}
	}
	else
	{
		strcpy(wea_p->Wind, "暂无数据！");
	}
	
	// 获取天气
	p = strstr(weather1_buf,  "weather1: ");
	if(p != NULL)
	{
		p = p+9;

		i = 0;
		while(*p != '\n')
		{
			wea_p->weather1[i] = *p;
			i++;
			p++;
		}
	}
	else
	{
		strcpy(wea_p->weather1, "暂无数据！");
	}
	
	// 获取天气情况
	p = strstr(weather1_buf,  "Sky conditions:");
	if(p != NULL)
	{
		p = p+15;

		i = 0;
		
		// wea_p->Sky_conditions[0] = *(p+0);
		// wea_p->Sky_conditions[1] = *(p+1);
		while(*p != '\n')
		{
			wea_p->Sky_conditions[i] = *p;
			
			i++;
			

			p++;
		
		}
	}
	else
	{
		strcpy(wea_p->Sky_conditions, "暂无数据！");
	}


	// d、关闭文件
	fclose(fp);



	// // e、删除文件
	// bzero(buf, sizeof(buf));
	// sprintf(buf, "rm %s", weather1_TXTPATH);
	// system(buf);

	return 0;
	
}



//main函数
// int main(void)
// {
// 	// 1、天气结构体
// 	weather1 wea;
	
// 	// 2、获取天气信息
// 	// Get_weather1(&wea, "ShenZhen");
// 	Get_weather1(&wea);
	
// 	printf("体感温度  = %s\n", wea.Temperature);
// 	printf("相对湿度  = %s\n", wea.Relative_Humidity);
// 	// printf("风向      = %s\n", wea.Wind);
// 	// printf("天气      = %s\n", wea.weather1);
// 	printf("天气环境  = %s\n", wea.Sky_conditions);
	
// }