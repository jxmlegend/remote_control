#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#ifdef _WIN32
    #define LOG_DIR  "c://Users//Public//Documents//YZYEduClient//Log"
    #define LOG_ERR_FILE "c://Users//Public//Documents//YZYEduClient//Log//remote_monitor_err.log"
#else
    #define LOG_DIR  "./log"
    #define LOG_ERR_FILE "./log/remote_monitor_err.log"
#endif

#define TTF_DIR "./font/msyh.ttf"

#define MAX_FILENAMELEN 256
/* config */
#define CONFIG_FILE "config.ini"

#define BASE_SECTION "base"
#define BASE_TYPE_KEY "type"

#define SERVER_SECTION "server"
#define SERVER_CLIENT_PORT_KEY "client_port"
#define SERVER_CONTROL_PORT_KEY "control_port"
#define SERVER_H264_PORT_KEY "h264_port"
#define SERVER_WINDOW_FLAG_KEY "window_flag"
#define SERVER_WINDOW_SIZE_KEY "window_size"            //2 = 2 * 2 4个窗口显示

#define CLIENT_SECTION "client"
#define CLIENT_IP_KEY "server_ip"
#define CLIENT_PORT_KEY "server_port"
#define CLIENT_QUALITY_KEY "quality"
#define CLIENT_FPS_KEY "fps"

#define DEFAULT_TYPE "client"

#define DEFAULT_CLIENT_PORT_VALUE 22000
#define DEFAULT_CONTROL_PORT_VALUE 23000
#define DEFAULT_H264_PORT_VALUE 23001
#define DEFAULT_WINDOW_FLAG_VALUE 0
#define DEFAULT_WINDOW_SIZE_VALUE 1             //2 = 2 * 2 4个窗口显示

#define DEFAULT_QUALITY_VALUE 60
#define DEFAULT_FPS_VALUE 25

#define DEFAULT_IP_VALUE "192.169.27.243"
#define DEFAULT_PORT_VALUE 22000


extern int server_flag;
extern int window_flag;
extern const char program_name[];

#endif
