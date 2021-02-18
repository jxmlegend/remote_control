#include "base.h"
#include "rtsp.h"

sem_t rtspd_semop;
sem_t rtspd_accept_lock;

pthread_cond_t rtspd_cond;
pthread_mutex_t rtspd_mutex;

struct rtsp_buffer *rtsp[MAX_CONN];

static void rtspd_free()
{

}

int rtsp_terardown()
{

}

int rtsp_play()
{

}

int rtsp_setup()
{

}

int rtsp_describe()
{


	//send()
}


int set_option_reply()
{
#if 0	
	sprintf();	

	strcat();
	strcat();
	send();
#endif
}

int rtsp_options(int cur_conn_num)
{
#if 0
	if(get_rtsp_cseg() == SUCCESS)
	{

	}	
#endif
}

int rtsp_cmd_match(int method, int cur_conn_num)
{
	switch(method)
	{
		case OPTIONS:
		{
			if(rtsp_options(cur_conn_num) <= 0)
			{

			}
		}
		break;
		case DESCRIBE:
		{
			if(rtsp_options(cur_conn_num) <= 0)
			{

			}
		}
		break;
		case SETUP:
		{
			if(rtsp_options(cur_conn_num) <= 0)
			{

			}
		}
		break;
		case PLAY:
		{
			if(rtsp_options(cur_conn_num) <= 0)
			{

			}
		}
		break;
		case TEARDOWN:
			break;
		case UNKNOWN:
			break;
	}
}



int get_rtsp_method(int cur_conn_num)
{
	char method[32];

	sscanf(rtsp[cur_conn_num]->in_buf, "%3ls", method);

	if(strcmp(method, "OPTIONS") == 0)
		return OPTIONS;
	
	if(strcmp(method, "DESCRIBE") == 0)
		return DESCRIBE;

	if(strcmp(method, "SETUP") == 0)
		return SETUP;

	if(strcmp(method, "PLAY") == 0)
		return PLAY;

	if(strcmp(method, "TEARDOWN") == 0)
		return TEARDOWN;

	return UNKNOWN;
}

int rtspd_proc()
{
	int method;
	method = get_rtsp_method(1);
	if(rtsp_cmd_match(method, 1) < 0)
	{
		return ERROR;
	}
	else
	{

		return SUCCESS;
	}	
}

void rtspd_tcp_loop(int sockfd)
{
	
}

int set_framerate(int rate, int free_chn)
{
	switch(rate)
	{
		case 15:
			rtsp[free_chn]->frame_rate_step = 3600;
			break;
		case 25:
			rtsp[free_chn]->frame_rate_step = 3600;
			break;
		case 30:
			rtsp[free_chn]->frame_rate_step = 3000;
			break;
		default:
			rtsp[free_chn]->frame_rate_step = 3600;
			break;
	}
}



int init_rtspd()
{
	int ret, i;
	int sockfd = -1;
	int free_chn;
	
	sem_init(&rtspd_semop, 0, 1);
	sem_init(&rtspd_accept_lock, 0, 0);

	ret = pthread_mutex_init(&rtspd_mutex, NULL);
	if(ret != 0)
	{
		DEBUG("");
		return ERROR;
	}
	pthread_cond_init(&rtspd_cond, NULL);
	
	sockfd = create_tcp();
	if(sockfd == INVALID_SOCKET)
	{
		DEBUG("");
		return ERROR;
	}

	ret = bind_socket(NULL, sockfd,  5554);
	if(ret != SUCCESS)
	{
		DEBUG("");
		return ERROR;
	}
	
	for(i = 0; i < MAX_CONN; i++)
	{
		rtsp[i] = calloc(1, sizeof(struct rtsp_buffer));
		if(!rtsp[i])
		{
			DEBUG("");
			return ERROR;
		}
		//sem_init(&rtspd_lock[i], 0, 0);
	}

	set_framerate(25, free_chn);

	rtspd_tcp_loop(sockfd);	
	rtspd_free();
	return SUCCESS;
}



