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


int Get_weather2(weather1 *weg_p)
{	
    // 变量
	char weather1_buf[2048] = {0};
	char buf[256] = {0};
	int  i        = 0;
    char splicing_buf[256] = {0};

	// 3、对weather1.txt进行解析
	// a、打开文件(只读模式)
	FILE * fp = fopen(weather1_TXTPATH, "rb");
	if( fp == NULL)
	{
		perror("Msg_RecvMsg: fopen失败\n");	
		return -1;
	}
 
    
    // 1. 读取文件数据
    size_t ret = fread(weather1_buf, 1, sizeof(weather1_buf)-1, fp);
    if(ret == 0)
    {
        perror("Msg RecvMsg: fread失败\n");
        return -2;
    }
    weather1_buf[ret] = '\0';  // 确保读取的数据有结束符
    
    // 2. 查找温度数据
    char *p = strstr(weather1_buf, "Temperature:");
    if(p != NULL)
    {
        p = p + 12;  // 跳过"Temperature
        
        
        memset(weg_p->Temperature, 0, sizeof(weg_p->Temperature));
        
     
        int i = 0;
        while (i < sizeof(weg_p->Temperature)-1)  // -1 留给 \0
        {
            // 检查是否遇到结束标志
            if (p[i] == '\r' || p[i] == '\n' || p[i] == '\0')
                break;
                
            weg_p->Temperature[i] = p[i];
            i++;
        }
        weg_p->Temperature[i] = '\0';  // 添加字符串结束符
    }
    else
    {
        strncpy(weg_p->Temperature, "暂无数据!", sizeof(weg_p->Temperature)-1);
        weg_p->Temperature[sizeof(weg_p->Temperature)-1] = '\0';
    }
    
   // d、关闭文件
	fclose(fp);
}


	








