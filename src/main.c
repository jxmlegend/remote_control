#include "base.h"

time_t current_time;

static void usage()
{
    DIE("\nYZY RESCUE LINUX:\n"
           "t,                  set tftp server ip\n"
           "s                   set YZY server ip\n"
           "n,                  set nfs server rootfs \n"
           "p,                  set http server ip \n"
           "help                show this helo\n"
           );  
}

static void do_exit()
{
	if(server_flag)
	{
		exit_server();
	}
	else
	{
		//exit_client();
	}
}

static void parse_options(int argc, char *argv[])
{   
#if 0
    int ch;
    while((ch = getopt(argc, argv, "t:s:n:p:h")) != -1)
    {   
        switch(ch)
        {   
            case 't':
                if(strlen(optarg) > 8)
                {   
                    DEBUG("tftp ip:%s", optarg);
                    strcpy(conf.tftp_ip, optarg);
                }
                break;
            case 's':
                if(strlen(optarg) > 8)
                {   
                    DEBUG("server ip:%s", optarg);
                    strcpy(conf.server.ip, optarg);
                }
                break;
            case 'n':
                if(strlen(optarg) > 8)
                {   
                    DEBUG("nfs ip:%s", optarg);
                    strcpy(conf.nfs_ip, optarg);
                }
                break;                  
            case 'p':                   //httpd
                if(strlen(optarg) > 8)
                {   
                    DEBUG("http ip:%s", optarg);
                    strcpy(conf.http_ip, optarg);
                }
                break;
            case 'h':
                usage();
                break;
            default:
                break;
        }
    }
#endif
}

int main(int argc, char *argv[])
{
	int ret;
	
	(void)time(&current_time);

	init_logs();
	//init_signals();	
	init_configs();	
		
	parse_options(argc, argv);

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

	if(server_flag)
	{
		DEBUG("init server ");
		ret = init_server();
	}
	else
	{
		DEBUG("init client");
		ret = init_client();
	}
	
	do_exit();	
	close_pipe();
	unload_wsa();
	destory_SDL();			
	save_configs();	
	close_logs();
	return ret;	
}
