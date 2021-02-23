#include "base.h"

#ifdef _WIN32
#include "dll.h"

HWND hwnd = NULL;
static stop_callback stop_cb;
static int init_flag = 0;
static int running = 0;
/*
启动启动监控服务 （教师端调用）
@param clientPort 
@param controlPort j接收端地址,组播地址
@param dataPort 接收端端口
@param winStyleFlag 图像质量0-1 窗口或者全屏
@param pageSize 终端显示个数0-5  2*2 = 4
成功返回0, 否则返回对应错误号
*/

sem_t server_sem;

CAPTUREANDCAST_API int StartMonitorServer(const int clientPort, const int controlPort, const int dataPort, 
					const int winStyleFlag, const int pageSize, stop_callback call)
{
	int ret;

	
	server_flag = 1;

    client_port = clientPort;
    control_port = controlPort;
    h264_port = dataPort;
    window_flag = winStyleFlag;
    window_size = pageSize;
	stop_cb = call;

	if(running)
	{
		DEBUG("server already running");
		return ERROR;
	}

    init_logs();
    ret = load_wsa();
    if(SUCCESS != ret)
    {
        DEBUG("load wsa error ret:%d", ret);
        return ERROR;
    }
    ret = init_pipe();
    if(SUCCESS != ret)
    {
        DEBUG("init pipe error ret:%d", ret);
        return ERROR;
    }

    ret = init_SDL();
    if(SUCCESS != ret)
    {
        DEBUG("init SDL error ret:%d", ret);
        return ERROR;
    }

    if(window_size <= 0 || window_size > 5)
    {
        DEBUG("window size param is error window_size: %d", window_size);
        return ERROR;
    }

	if(!init_flag)
	{
		init_SDL();
		init_flag = 1;	
	}
	running = 1;
	return init_server();
}

/*
停止监控端服务（教师端端用）
@param NULL
成功返回0, 否则返回对应错误号
*/
CAPTUREANDCAST_API int StopMonitorServer()
{
	if(running)
	{
		sdl_window_quit();
		unload_wsa();
		close_pipe();
		close_logs();
		running = 0;
	}
	return SUCCESS;
}

/*
断开所以客户端（教师端端用）
@param NULL
成功返回0, 否则返回对应错误号
*/
CAPTUREANDCAST_API int DisconnectAllClient()
{
	//DEBUG("");
	//return close_clients();
	//send_pipe();
}

/*
取消控制模式切换到监控模式（教师端端用）
@param NULL
成功返回0, 否则返回对应错误号
*/
CAPTUREANDCAST_API int ExitControl()
{
	//DEBUG("");
	//send_pipe();
	//return convert_mode(0);
	
}

/*
启动被监控端连接（学生端用）
@param serverIp 服务端的ip
@param serverPort 服务端port 用于通信交互
@param clientFlag 客户端标志用于区分客户端类型 
成功返回0, 否则返回对应错误号
*/
CAPTUREANDCAST_API int StartMonitorClient(const char* serverIp, const int serverPort, const char* clientFlag)
{

}

/*
设置分页相关属性(教师端调用)
@param pageIndex 当前第几页
@param pageSize 一页显示多少个客户端
@param info 客户端的信息用于显示
@param length 
成功返回0, 否则返回对应错误号
*/
CAPTUREANDCAST_API int SetPageAttribute(const int pageIndex, const int pageSize, struct StudentInfo *info, const int length)
{
	return SUCCESS;
}

/*
获取当前页面客户端在线数量(教师端调用)
@param NULL
返回0-25, 客户端在线数量
*/
CAPTUREANDCAST_API int GetPageCount()
{
    return SUCCESS;
}

void stop_server()
{
    if(stop_cb)
        stop_cb();
    stop_cb = NULL;
	running = 0;
}
#endif	//_WIN32
