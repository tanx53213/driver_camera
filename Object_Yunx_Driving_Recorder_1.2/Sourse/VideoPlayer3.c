// 添加必要的头文件
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include"camera.h"
// 定义录像文件夹路径
// #define RECORD_PATH "/tmp/camera_records"
#define RECORD_PATH2 "/tmp/Object_Yunx_Driving_Recorder2/Data/camera_records"
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








// 修复 YUV 到 BMP 的转换函数
void yuyv_to_bmp3(char *yuvbuf, const char* bmp_file) {
    if (!yuvbuf || !bmp_file) {
        printf("Invalid input parameters\n");
        return;
    }

    FILE *bmp_fp = fopen(bmp_file, "wb");
    if (!bmp_fp) {
        printf("创建BMP文件失败\n");
        return;
    }
fseek(bmp_fp,54,SEEK_SET);
    size_t i = 0,j = 0;

    
    // 存储RGB数据的行缓冲区
    unsigned char *buf_line = malloc(640 * 3);
    if (!buf_line) {
        fclose(bmp_fp);
        return;
    }
    
    // 转换每一行数据并写入
    for (int y = 479; y >= 0; y--) {
        for (int x = 0; x < 640; x++) {
            int i = y * 640 + x;
            // 安全地访问YUV数据
            unsigned char Y = yuvbuf[i*2];
            unsigned char U = (x > 0) ? yuvbuf[i*2-1] : yuvbuf[i*2+1];
            unsigned char V = (x < 639) ? yuvbuf[i*2+1] : yuvbuf[i*2-1];
            
            unsigned int rgb = pixel_yuv2rgb(Y, U, V);
            
            // 写入RGB数据
            buf_line[x*3] = (rgb) & 0xFF;         // B
            buf_line[x*3+1] = (rgb >> 8) & 0xFF;  // G
            buf_line[x*3+2] = (rgb >> 16) & 0xFF; // R
        }
        fwrite(buf_line, 1, 640*3, bmp_fp);
    }

    free(buf_line);
    fclose(bmp_fp);
}










// 录像功能实现
void record_video(void) {
    char timestamp[32];
    char filepath[256];
    char dir_path[256];
    time_t now;
    struct tm *timeinfo;
    bool first_frame = true;  // 用于标记是否是第一帧
    
    // 初始化摄像头
    open_device();
    init_device();
    start_capturing();
    
    // 创建录像文件夹
    mkdir(RECORD_PATH2, 0777);
    
    // 获取当前时间作为文件名
      // 以时间创建子目录
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    snprintf(dir_path, sizeof(dir_path), "%s/%s", RECORD_PATH, timestamp);
    mkdir(dir_path, 0777);
    
    // 打开文件
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        printf("Failed to create record file\n");
        return;
    }
    
    // 录制5秒视频
    int frame_count = 0;
    while (frame_count < 150 && current_state == STATE_RECORD) {  // 30帧/秒，录5秒
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

         char bmp_file[512];
        snprintf(bmp_file, sizeof(bmp_file), "%s/frame_%03d.bmp", dir_path, frame_count);
        yuyv_to_bmp3(buffers[buf.index].start, bmp_file);
        // 写入数据到文件
        fwrite(buffers[buf.index].start, 1, buf.bytesused, fp);
        
        // 重新入队缓冲区
        if (-1 == ioctl(camera_fd, VIDIOC_QBUF, &buf)) {
            break;
        }
        
        frame_count++;
        printf("\r录制进度: %d%%", (frame_count * 100) / 150);
        fflush(stdout);
    }
    
    printf("\n录制完成\n");
    fclose(fp);
    
    // 停止采集并关闭设备
    // stop_capturing();
    // close_device();
}

// 6. 显示历史记录
void show_history(void) {
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
    while (1) {
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
        if (count >= MAX_RECORDS) {
            dir = opendir(RECORD_PATH);
            time_t oldest_time = time(NULL);
            char oldest_file[256] = {0};
            
            if (dir) {
                while ((ent = readdir(dir)) != NULL) {
                    if (ent->d_name[0] == '.') continue;
                    
                    char filepath[512];
                    struct stat st;
                    snprintf(filepath, sizeof(filepath), "%s/%s", RECORD_PATH, ent->d_name);
                    
                    if (stat(filepath, &st) == 0) {
                        if (st.st_mtime < oldest_time) {
                            oldest_time = st.st_mtime;
                            strcpy(oldest_file, filepath);
                        }
                    }
                }
                closedir(dir);
                
                if (oldest_file[0]) {
                    remove(oldest_file);
                }
            }
        }
        
        // 录制新的视频
        record_video();
    }
}

// 修改handleButtonPress函数，添加新功能
void handleButtonPress(Button* button, int buttonIndex) {
    if (button == NULL || buttonIndex < 0 || buttonIndex >= buttonCount) {
        printf("无效的按钮索引\n");
        return;
    }

    switch(buttonIndex) {
        case 0:  // 后视
            open_device();
            init_device();
            start_capturing();
            mainloop();
            break;
        case 1:  // 录像
            printf("按下录像按钮\n");
            record_video();
            break;
        case 2:  // 历史
            printf("按下历史按钮\n");
            show_history();
            break;
        case 3:  // 返回
            printf("按下返回按钮\n");
            // 返回主界面的代码
            break;
        case 4:  // 守护进程
            printf("按下守护进程按钮\n");
            circular_recording();
            break;
        case 5:  // 电源
            printf("按下电源按钮\n");
            // 电源相关操作
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


// YUV转BMP
void yuyv_to_bmp(char *yuvbuf, const char* bmp_file) {
    FILE *bmp_fp = fopen(bmp_file, "wb");
    if (!bmp_fp) {
        printf("创建BMP文件失败\n");
        return;
    }

    // 写入54字节的空文件头
    char header[54] = {0};
    fwrite(header, 1, 54, bmp_fp);
    
    // 跳过文件头
    fseek(bmp_fp, 54, SEEK_SET);

    // 存储RGB数据的行缓冲区
    char buf_line[640*3] = {0};  // 一行的RGB数据
    
    // 转换每一行数据并写入
    for (int y = 479; y >= 0; y--) {  // BMP格式从下到上存储
        for (int x = 0; x < 640; x++) {
            int i = y * 640 + x;
            // 使用已有的YUV到RGB转换函数
            unsigned int rgb = pixel_yuv2rgb(yuvbuf[i*2], yuvbuf[i*2+1], yuvbuf[i*2+3]);
            
            // 按BGR顺序写入缓冲区
            buf_line[x*3] = (rgb) & 0xFF;         // B
            buf_line[x*3+1] = (rgb >> 8) & 0xFF;  // G
            buf_line[x*3+2] = (rgb >> 16) & 0xFF; // R
        }
        // 写入这一行的数据
        fwrite(buf_line, 1, 640*3, bmp_fp);
    }

    fclose(bmp_fp);
}