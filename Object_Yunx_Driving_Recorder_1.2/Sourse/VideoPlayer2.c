// 添加必要的头文件
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include"camera.h"
// 定义录像文件夹路径
// #define RECORD_PATH "/tmp/camera_records"
#define MAX_RECORDS 200  // 最大录像数量

#define  TS_DEV   "/dev/input/event0"
int lcd_fd;
unsigned int *lcd_mp;

// ProgramState current_state = STATE_MAIN;

//用于记录摄像头的设备文件的文件描述符
static int camera_fd = -1;

static unsigned int n_buffers= 0; //用于记录缓冲区的编号



static struct buffer *buffers = NULL; //用于记录申请的缓冲区信息


extern int buttonCount;

/************* ***********/

// 停止摄像头函数
void stop_camera(void) {
    if (camera_fd >= 0) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(camera_fd, VIDIOC_STREAMOFF, &type);
        close(camera_fd);
        camera_fd = -1;
    }
}

// 返回主界面函数
void return_to_main(void) {
    // 根据当前状态执行清理操作
    switch(current_state)
     {
        case STATE_BACKUP:
            printf("退出后视功能\n");
            stop_camera();
            break;
            
        case STATE_RECORD:
            printf("退出录像功能\n");
            stop_camera();
            break;
            
        case STATE_HISTORY:
            printf("退出历史记录\n");
            break;
            
        case STATE_GUARD:
            printf("退出守护进程\n");
            stop_camera();
            break;
            
        default:
            break;
    }
    
    // 重绘主界面
    Bmp_Decode("/tmp/Object_Yunx_Driving_Recorder2/Data/runV1.bmp", lcd_mp);
    
    // 更新状态
    current_state = STATE_MAIN;
}







/****************************** */

// 录像功能实现
void record_video(void) 
{
    char timestamp[32];
    char filepath[256];
    time_t now;
    struct tm *timeinfo;
    bool first_frame = true;  // 用于标记是否是第一帧
    
    // 初始化摄像头
    open_device();
    init_device();
    start_capturing();
    
    // 创建录像文件夹
    mkdir(RECORD_PATH, 0777);
    
    // 获取当前时间作为文件名
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    snprintf(filepath, sizeof(filepath), "%s/%s.yuv", RECORD_PATH, timestamp);
    
    // 打开文件
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        printf("Failed to create record file\n");
        return;
    }
    
    // 录制5秒视频
    int frame_count = 0;
    while (frame_count < 150 && current_state == STATE_RECORD) 
    {  // 30帧/秒，录5秒
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        
        // 等待缓冲区
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(camera_fd, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(camera_fd + 1, &fds, NULL, NULL, &tv);
        if (r == -1) {
            if (errno == EINTR) continue;
            break;
        }

        if (r == 0) {
            printf("select timeout\n");
            break;
        }
        
        // 取出缓冲区
        if (-1 == ioctl(camera_fd, VIDIOC_DQBUF, &buf)) {
            break;
        }
        
        // 如果是第一帧，显示在LCD上
        if (first_frame) {
            yuyv_to_lcd(buffers[buf.index].start);
            first_frame = false;
            printf("显示第一帧\n");
        }
        
        // 写入数据到文件
        fwrite(buffers[buf.index].start, 1, buf.bytesused, fp);
        
        // 重新入队缓冲区
        if (-1 == ioctl(camera_fd, VIDIOC_QBUF, &buf)) {
            break;
        }
        
        frame_count++;
        printf("\r录制进度: %d%%", (frame_count * 100) / 150);
        fflush(stdout);
     // 检查状态是否改变
        if (current_state != STATE_RECORD)
         {
            break;
   
         }
    
        printf("\n录制完成\n");
        fclose(fp);
    
        // 停止采集并关闭设备
        stop_camera();
        // stop_capturing();
        // close_device();
    }
}

// 6. 显示历史记录
void show_history(void)
{
    DIR *dir;
    struct dirent *ent;
    char filepath[256];
    
    dir = opendir(RECORD_PATH);
    if (dir == NULL) {
        printf("No history records\n");
        return;
    }
    
    // 读取第一个录像文件
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        
        snprintf(filepath, sizeof(filepath), "%s/%s", RECORD_PATH, ent->d_name);
        FILE *fp = fopen(filepath, "rb");
        if (fp) {
            // 读取YUV数据并显示
            char *buffer = malloc(640 * 480 * 2);  // YUYV格式
            if (buffer) {
                size_t bytes_read = fread(buffer, 1, 640 * 480 * 2, fp);
                if (bytes_read > 0) {
                    yuyv_to_lcd(buffer);
                }
                free(buffer);
            }
            fclose(fp);
            break;  // 只显示第一帧
        }
    }
    closedir(dir);
}

// 7. 循环录像功能
void circular_recording(void) {
    while (1) 
    {
        // 检查录像数量
        DIR *dir = opendir(RECORD_PATH);
        int count = 0;
        struct dirent *ent;
        
        if (dir) {
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_name[0] != '.') count++;
            }
            closedir(dir);
        }
        
        // 如果超过最大数量，删除最旧的文件
        if (count >= MAX_RECORDS) 
        {
            dir = opendir(RECORD_PATH);
            time_t oldest_time = time(NULL);
            char oldest_file[256] = {0};
            
            if (dir)
            {
                while ((ent = readdir(dir)) != NULL)
                 {
                    if (ent->d_name[0] == '.') continue;
                    
                    char filepath[512];
                    struct stat st;
                    snprintf(filepath, sizeof(filepath), "%s/%s", RECORD_PATH, ent->d_name);
                    
                    if (stat(filepath, &st) == 0)
                     {
                        if (st.st_mtime < oldest_time) 
                        {
                            oldest_time = st.st_mtime;
                            strcpy(oldest_file, filepath);
                        }
                    }
                }
                closedir(dir);
                
                if (oldest_file[0])
                 {
                    // remove(oldest_file);//删除操作
                     char buf[512];
                    bzero(buf, sizeof(buf));
                    sprintf(buf, "rm %s", oldest_file);
                    system(buf);
                    printf("过期录像: %s已清理\n",oldest_file);
                }
            }
        }
        
        // 录制新的视频
        record_video();
    }
}

// 修改handleButtonPress函数，添加新功能
void handleButtonPress(Button* button, int buttonIndex) {
    if (button == NULL || buttonIndex < 0 || buttonIndex >= buttonCount) 
    {
        printf("无效的按钮索引\n");
        return;
    }
   // 电源键特殊处理 (buttonIndex == 5)

     switch(buttonIndex)
    {
        case 0:  // 后视
            if (current_state == STATE_MAIN) 
            {
                printf("进入后视功能\n");
                current_state = STATE_BACKUP;
                open_device();
                init_device();
                start_capturing();
                mainloop();
            }
            break;
            
        case 1:  // 录像
            if (current_state == STATE_MAIN) 
            {
                printf("进入录像功能\n");
                current_state = STATE_RECORD;
                 open_device();
                init_device();
                start_capturing();
                mainloop3();
            }
            break;
            
        case 2:  // 历史
            if (current_state == STATE_MAIN)
             {
                printf("进入历史记录\n");
                current_state = STATE_HISTORY;
                show_history();
            }
            break;
            
        case 3:  // 返回
            if (current_state != STATE_MAIN)
             {
                return_to_main();
            }
            break;
            
        case 4:  // 守护进程
            if (current_state == STATE_MAIN)
             {
                printf("进入守护进程\n");
                current_state = STATE_GUARD;
                circular_recording();
            }
            break;
            case5: 
             {
                    if (current_state == STATE_MAIN) 
                    {
                        // 在主界面按电源键退出程序
                        printf("程序退出\n");
                        stop_camera();  // 确保清理摄像头
                        exit(0);
                    } 
                    else 
                    {
                        // 在其他界面按电源键返回主界面
                        return_to_main();
                        return;
                    }   
                   
             }
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


