#include "base.h"
#include "rtsp.h"
#include "client.h"

sem_t rtspd_semop;
sem_t rtspd_accept_lock;

pthread_cond_t rtspd_cond;
pthread_mutex_t rtspd_mutex;

struct rtsp_buffer *rtsp[MAX_CONN];

char *get_stat(int code)
{
    struct {
        char *token;
        int code;
    } status[] = { 
        {   
        "Continue", 100}, {
        "OK", 200}, {
        "Created", 201}, {
        "Accepted", 202}, {
        "Non-Authoritative Information", 203}, {
        "No Content", 204}, {
        "Reset Content", 205}, {
        "Partial Content", 206}, {
        "Multiple Choices", 300}, {
        "Moved Permanently", 301}, {
        "Moved Temporarily", 302}, {
        "Bad Request", 400}, {
        "Unauthorized", 401}, {
        "Payment Required", 402}, {
        "Forbidden", 403}, {
        "Not Found", 404}, {
        "Method Not Allowed", 405}, {
        "Not Acceptable", 406}, {
        "Proxy Authentication Required", 407}, {
        "Request Time-out", 408}, {
        "Conflict", 409}, {
        "Gone", 410}, {
        "Length Required", 411}, {
        "Precondition Failed", 412}, {
        "Request Entity Too Large", 413}, {
        "Request-URI Too Large", 414}, {
        "Unsupported Media Type", 415}, {
        "Bad Extension", 420}, {
        "Invalid Parameter", 450}, {
        "Parameter Not Understood", 451}, {
        "Conference Not Found", 452}, {
        "Not Enough Bandwidth", 453}, {
        "Session Not Found", 454}, {
        "Method Not Valid In This State", 455}, {
        "Header Field Not Valid for Resource", 456}, {
        "Invalid Range", 457}, {
        "Parameter Is Read-Only", 458}, {
        "Unsupported transport", 461}, {
        "Internal Server Error", 500}, {
        "Not Implemented", 501}, {
        "Bad Gateway", 502}, {
        "Service Unavailable", 503}, {
        "Gateway Time-out", 504}, {
        "RTSP Version Not Supported", 505}, {
        "Option not supported", 551}, {
        "Extended Error:", 911}, {
        NULL, -1}
    };
    int i;
    for (i = 0; status[i].code != code && status[i].code != -1; ++i);
    return status[i].token;
}

void send_reply(int code, struct client *cli)
{
	memset(cli->rtsp_buf, 0, sizeof(cli->rtsp_buf));
	sprintf(cli->rtsp_buf, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL, RTSP_VER, code, (char *)get_stat(code), cli->rtsp_cseq);
	strcat(cli->rtsp_buf, RTSP_EL);
	//send_msg();
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


int set_option_reply(struct client *cli, int code)
{
	//sprintf(cli->rtsp_buf, 
	sprintf(cli->rtsp_buf, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL, RTSP_VER, code, (char *)get_stat(code), cli->rtsp_cseq);	
	strcat(cli->rtsp_buf, "Public: OPTIONS,DESCRIBE,SETUP,PLAY,PAUSE,TEARDOWN"RTSP_EL);
	strcat(cli->rtsp_buf, RTSP_EL);
	
	return send_msg(cli->fd, cli->rtsp_buf, strlen(cli->rtsp_buf));
}

int get_rtsp_cseg(struct client *cli)
{
	char *p;
	char trash[255] = {0};
	
	if((p = strstr(cli->rtsp_buf, "CSeq")) !=  NULL)
	{
		if(sscanf(p, "%254s %d", trash, &cli->rtsp_cseq) == 2)
		{
			DEBUG("Cseq %d", cli->rtsp_cseq);
			return SUCCESS;
		}
	}
	send_reply(400, cli);
	return ERROR;
}

int rtsp_options(struct client *cli)
{
	if(get_rtsp_cseg(cli) != SUCCESS)
		return ERROR;

	return set_option_reply(cli, 200);
}

int rtsp_cmd_match(struct client *cli)
{
	int ret;
	int method = get_rtsp_method(cli);
	switch(method)
	{
		case OPTIONS:
			ret = rtsp_options(cli);
			break;
		case DESCRIBE:
			ret = rtsp_options(cli);
			break;
		case SETUP:
			ret = rtsp_options(cli);
			break;
		case PLAY:
			ret = rtsp_options(cli);
			break;
		case TEARDOWN:
			ret = ERROR;
			break;
		case UNKNOWN:
			ret = ERROR;
			break;
	}
	return ret;
}



int get_rtsp_method(struct client *cli)
{
	char method[32];

	sscanf(cli->rtsp_buf, "%31s", method);

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
	//rtspd_free();
	return SUCCESS;
}



