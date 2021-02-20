#include "base.h"
#include "configs.h"

/* config.ini */
int server_flag = 1;
int client_port = -1, control_port = -1, h264_port = -1,  window_flag = 1, window_size = 2;
int default_fps = 12; 
int default_size = 3;
int default_bps = 20000000;

int server_port = -1; 
char *server_ip = NULL;
const char program_name[] = "remote_monitor";

//int major_ver;

void init_configs()
{
    int ret;
    char buf[128] = {0};
        
    ret = read_profile_string(BASE_SECTION, BASE_TYPE_KEY, buf, sizeof(buf), DEFAULT_TYPE, CONFIG_FILE);
	if(ret && STRPREFIX(buf, "server"))
    {   
        server_flag = 1;
    }
    else
    {    
         server_flag = 0;
    }
	server_flag = 1;
    if(server_flag)     //服务端程序
    {   
        client_port = read_profile_int(SERVER_SECTION, SERVER_CLIENT_PORT_KEY, DEFAULT_CLIENT_PORT_VALUE, CONFIG_FILE);
        control_port = read_profile_int(SERVER_SECTION, SERVER_CONTROL_PORT_KEY, DEFAULT_CONTROL_PORT_VALUE, CONFIG_FILE);
        h264_port = read_profile_int(SERVER_SECTION, SERVER_H264_PORT_KEY, DEFAULT_H264_PORT_VALUE, CONFIG_FILE);
        window_flag = read_profile_int(SERVER_SECTION, SERVER_WINDOW_FLAG_KEY, DEFAULT_WINDOW_FLAG_VALUE, CONFIG_FILE);
        window_size = read_profile_int(SERVER_SECTION, SERVER_WINDOW_SIZE_KEY, DEFAULT_WINDOW_SIZE_VALUE, CONFIG_FILE);
        
        DEBUG("\nprograme server: \n client_port %d, control_port %d, h264_port %d, window_flag %d, window_size %d,",
                 client_port, control_port, h264_port, window_flag, window_size);
    }
    else                //客户端程序
    {   
        ret = read_profile_string(CLIENT_SECTION, CLIENT_IP_KEY, buf, sizeof(buf), DEFAULT_IP_VALUE, CONFIG_FILE);
		if(!server_ip)
        	server_ip = strdup(buf);
        server_port = read_profile_int(CLIENT_SECTION, CLIENT_PORT_KEY, DEFAULT_PORT_VALUE, CONFIG_FILE);
        //default_fps = read_profile_int(CLIENT_SECTION, CLIENT_FPS_KEY, DEFAULT_FPS_VALUE, CONFIG_FILE);
        DEBUG("\nprograme client: \n server_ip %s, server_port %d, window_flag %d, default_fps %d",
                server_ip, server_port, window_flag, default_fps);
    }
}


void save_configs()
{
#if 0
    char buf[128] = {0};
        
    terminal_info *terminal = &(conf.terminal);
    netcard_param *net = &(conf.netcard);
    server_info *server = &(conf.server);
            
    /* base */
    sprintf(buf, "%d", conf.install_flag);
    write_profile_string(BASE_SECTION, BASE_INSTALL_KEY, buf, config_file);
            
    conf.config_ver += 1;

    /* terminal */
    sprintf(buf, "%d", terminal->id);
    write_profile_string(TERMINAL_SECTION, TM_ID_KEY, buf, config_file);
    write_profile_string(TERMINAL_SECTION, TM_NAME_KEY, terminal->name, config_file);
    write_profile_string(TERMINAL_SECTION, TM_PLATFORM_KEY, terminal->platform, config_file);
    sprintf(buf, "%d", terminal->desktop_type);
    write_profile_string(TERMINAL_SECTION, TM_DESKTOP_TYPE_KEY, buf, config_file);
    sprintf(buf, "%d", terminal->auto_desktop);
    write_profile_string(TERMINAL_SECTION, TM_AUTO_DESKTOP_KEY, buf, config_file);
    sprintf(buf, "%d", conf.config_ver);
    write_profile_string(TERMINAL_SECTION, TM_CONFIG_VER_KEY, buf, config_file);

    /* network */
    sprintf(buf, "%d", net->is_dhcp);
    write_profile_string(NET_SECTION, NET_DHCP_KEY, buf, config_file);
    write_profile_string(NET_SECTION, NET_IP_KEY, net->ip, config_file);

    write_profile_string(NET_SECTION, NET_NETMASK_KEY, net->netmask, config_file);

    write_profile_string(NET_SECTION, NET_GATEWAY_KEY, net->gateway, config_file);
    write_profile_string(NET_SECTION, NET_DNS1_KEY, net->dns1, config_file);
    write_profile_string(NET_SECTION, NET_DNS2_KEY, net->dns2, config_file);
    write_profile_string(NET_SECTION, NET_MAC_KEY, net->mac, config_file);

    /* server info */
    write_profile_string(SERVER_SECTION, SERVER_IP_KEY, server->ip, config_file);
        
    sprintf(buf, "%d", 50007);
    write_profile_string(SERVER_SECTION, SERVER_PORT_KEY, buf, config_file);

    /* version */
    sprintf(buf, "%d", conf.major_ver);
    write_profile_string(VERSION_SECTION, VER_MAJOR_KEY, buf, config_file);
    sprintf(buf, "%d", conf.minor_ver);
    write_profile_string(VERSION_SECTION, VER_MINOR_KEY, buf, config_file);

    sprintf(buf, "v%d.%d", conf.major_ver, conf.minor_ver);
    write_profile_string(VERSION_SECTION, VER_LINUX_KEY, buf, config_file);

    DEBUG("save config success");
#endif
}







