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
		exit_client();
	}
}

static void parse_options(int argc, char *argv[])
{   
    int ret = 0;
    char buf[126] = {0};
    switch(argc)
    {   
        case 3:
            server_port = atoi(argv[2]);
        case 2:
            server_ip = strdup(argv[1]);
        default:
            break;
    }   

    if(!server_ip || server_port >= 65535 || server_port <= 0)
    {   
        ret = read_profile_string(CLIENT_SECTION, CLIENT_IP_KEY, buf, sizeof(buf), DEFAULT_IP_VALUE, CONFIG_FILE);
        if(server_ip)
            free(server_ip);
    
        server_ip = strdup(buf);
        server_port = read_profile_int(CLIENT_SECTION, CLIENT_PORT_KEY, DEFAULT_PORT_VALUE, CONFIG_FILE);
    }   
}

int main(int argc, char *argv[])
{
	int ret;

	(void)time(&current_time);

	init_logs();
	init_signals();	
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
