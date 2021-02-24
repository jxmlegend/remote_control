#include "base.h"
#include "rtsp.h"

sem_t rtspd_semop;
sem_t rtspd_accept_lock;

pthread_cond_t rtspd_cond;
pthread_mutex_t rtspd_mutex;

//struct rtsp_buffer *rtsp[MAX_CONN];
struct rtsp_cli *rtsp;

QUEUE *vids_queue;
QUEUE *audio_queue;

void *thread_ffmpeg_video_decode(void *param);
void *thread_ffmpeg_audio_decode(void *param);

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

#if 0
void send_reply(int code, struct client *cli)
{
	memset(cli->rtsp_buf, 0, sizeof(cli->rtsp_buf));
	sprintf(cli->rtsp_buf, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL, RTSP_VER, code, (char *)get_stat(code), cli->rtsp_cseq);
	strcat(cli->rtsp_buf, RTSP_EL);
	send_msg(cli->fd, cli->rtsp_buf, strlen(cli->rtsp_buf));
}

static int parse_url(const char *url, char *server, uint16_t *port, char *file_name)
{
	char *token, *port_str;
	char temp[128] = {0};
	
	strcpy(temp, url);
	if(strncmp(temp, "rtsp://", 7) == 0)
	{
		token = strtok(&temp[7], " :/\t\n");
		strcpy(server, token);
		
		port_str = strtok(&temp[strlen(server) + 7 + 1], " /\t\n");
		
		if(port_str)
			*port = (uint16_t)atol(port_str);
		else
			*port = 554;
		
		token = strtok(NULL, " ");
		
		if(token)
			strcpy(file_name, token);
		else
			file_name[0] = '\0';

		return SUCCESS;
	}
	return ERROR;
}

int check_rtsp_url(struct client *cli)
{
	int port;
	char url[128];
	char object[128], server[128];	
	
	if(!sscanf(cli->rtsp_buf, " %*s %254s ", url))
	{
		send_reply(400, cli);
		return ERROR;
	}
	/* validate the url */
	if(parse_url(url, server, &port, object) != SUCCESS)
	{
		send_reply(400, cli);
		return ERROR;
	}
	DEBUG("server %s port %d object %s", server, port, object);
	strcpy(cli->host_name, server);
	if(strstr(object, "trackID") || strcmp(object, "") == 0)
	{
		strcpy(object, cli->file_name);
	}
	else
	{
		strcpy(cli->file_name, object);
	}
	return SUCCESS;	
}

int check_rtsp_filename(struct client *cli)
{
	
	return SUCCESS;
}

int rtsp_terardown()
{

}

int send_play_reply(struct client *cli, int code)
{
	char temp[255];
	
  	/* build a reply message */
    sprintf(cli->rtsp_buf, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL,
		 RTSP_VER, code, (char *)get_stat(code), cli->rtsp_cseq, PACKAGE,VERSION);
    add_time_stamp(cli->rtsp_buf, 0); 
	strcat(cli->rtsp_buf, "Range: npt=0.000-");
	strcat(cli->rtsp_buf, RTSP_EL);
    strcat(cli->rtsp_buf, "Session: ");
    sprintf(temp, "%d", cli->session_id);
    strcat(cli->rtsp_buf, temp);
	strcat(cli->rtsp_buf, ";timeout=60");
    strcat(cli->rtsp_buf, RTSP_EL);
    strcat(cli->rtsp_buf, RTSP_EL);
	send_msg(cli->fd, cli->rtsp_buf, strlen(cli->rtsp_buf));

	
	rtp_send_packet(cli->host_name, cli->rtp_port, cli->is_tcp);
}

int rtsp_play(struct client *cli)
{
	char *p = NULL;
	char trash[255];

	if(get_rtsp_cseg(cli))
		return ERROR;

	if((p = strstr(cli->rtsp_buf, HDR_SESSION)) != NULL)
	{
		if(sscanf(p, "%254s %d", trash, &cli->session_id) == 2)
		{
			DEBUG("cli->session_id %d", cli->session_id);
			return send_play_reply(cli, 200);
		}
		else
		{
			send_reply(cli, 454);
			return ERROR;
		}
	}
	send_reply(cli, 400);
	return ERROR;
}

int send_setup_reply(struct client *cli, int code)
{
    char temp[30];
    char ttl[4];

    /* build a reply message */
    sprintf(cli->rtsp_buf, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL,
		RTSP_VER, code, (char *)get_stat(code), cli->rtsp_cseq, PACKAGE,VERSION);
    add_time_stamp(cli->rtsp_buf, 0); 
	strcat(cli->rtsp_buf, "Range: npt=0.000-");
    strcat(cli->rtsp_buf, RTSP_EL);
	
	
    strcat(cli->rtsp_buf, "Session: ");
    sprintf(temp, "%d", cli->session_id);
    strcat(cli->rtsp_buf, temp);
    strcat(cli->rtsp_buf, RTSP_EL);
    /**** unicast  ****/
	if(cli->is_tcp)
	{
    	strcat(cli->rtsp_buf, "Transport: RTP/AVP/TCP;unicast;interleaved=0-1");
    	strcat(cli->rtsp_buf, RTSP_EL);
	}
	else
	{
    	strcat(cli->rtsp_buf, "Transport: RTP/AVP;unicast;client_port=");
		
    sprintf(temp, "%d", cli->rtp_port);
    strcat(cli->rtsp_buf, temp);
    strcat(cli->rtsp_buf, "-");
    sprintf(temp, "%d", cli->rtcp_port);
    strcat(cli->rtsp_buf, temp);
    strcat(cli->rtsp_buf, ";server_port=");
    sprintf(temp, "%d", cli->rtp_port + 1);
    strcat(cli->rtsp_buf, temp);
    strcat(cli->rtsp_buf, "-");
    sprintf(temp, "%d", cli->rtcp_port + 1);
    strcat(cli->rtsp_buf, temp);
    //sprintf(temp, ";ssrc=%u", 1234);/*xxx*/
    //strcat(cli->rtsp_buf, temp);   
    //strcat(cli->rtsp_buf,";ttl=");
    //sprintf(ttl,"%d",(int32_t)DEFAULT_TTL);
    //strcat(cli->rtsp_buf, ttl);
    strcat(cli->rtsp_buf, RTSP_EL);
	}
    strcat(cli->rtsp_buf, RTSP_EL);


	return send_msg(cli->fd, cli->rtsp_buf, strlen(cli->rtsp_buf));
}

int rtsp_setup(struct client *cli)
{
	char *p = NULL;
	char trash[255], line[255];
	
	if(check_rtsp_url(cli))
		return ERROR;

	if(check_rtsp_filename(cli))
		return ERROR;
	
	if(get_rtsp_cseg(cli))
		return ERROR;
	
	/* udp */
	if((p = strstr(cli->rtsp_buf, "client_port")) == NULL &&
		strstr(cli->rtsp_buf, "multicast") == NULL)
	{
		/* Not Acceptable */
		/* tcp */
		//send_reply(406, cli);
		//return ERROR;
		cli->is_tcp = cli->fd;
	}
	
	if((p = strstr(cli->rtsp_buf, HDR_TRANSPORT)) == NULL)
	{
		/* Not Acceptable */
		send_reply(406, cli);
		return ERROR;
	}
	
	if(sscanf(p, "%10s%255s", trash, line) != 2)
	{
		/* Bad Request */
		send_reply(400, cli);
		return ERROR;
	}
	
	/* get client rtp and rtcp port */
	if(strstr(line, "client_port") != NULL)
	{
		p = strstr(line, "client_port");
		p = strstr(p, "=");
		sscanf(p + 1, "%d", &cli->rtp_port);
		p = strstr(p, "-");
		sscanf(p + 1, "%d", &cli->rtcp_port);
	}
	DEBUG("cli->rtp_port %d cli->rtcp_port %d", cli->rtp_port, cli->rtcp_port);
	//get_server_port();	
	//cli->seq = get_randdom_seq();
	//cli->ssrc = 0x69257765;
	//cli->timestamp = random32(0);
	return send_setup_reply(cli, 200);
}

char *get_sdp_user_name(char *buf)
{
	strcpy(buf, PACKAGE);
	return buf;
}

float ntp_time(time_t t)
{
	return (float)t + 2208988800U;
}

char *get_sdp_session_id(char *buf)
{
	sprintf(buf, "%.0f", ntp_time(time(NULL)));
	return buf;
}

char *get_sdp_version(char *buf)
{
	sprintf(buf, "%.0f", ntp_time(time(NULL)));
	return buf;
}

int get_describe_sdp(struct client *cli)
{
	char buf[30] = {0};
	
	strcpy(cli->sdp_buf, "v=0"SDP_EL);
	strcat(cli->sdp_buf, "o=");
	strcat(cli->sdp_buf, get_sdp_user_name(buf));
	strcat(cli->sdp_buf, " ");
	strcat(cli->sdp_buf, get_sdp_session_id(buf));
	strcat(cli->sdp_buf, " ");
	strcat(cli->sdp_buf, get_sdp_version(buf));
	strcat(cli->sdp_buf, SDP_EL);
	strcat(cli->sdp_buf, "c=IN IP4 ");
	strcat(cli->sdp_buf, cli->host_name);
	strcat(cli->sdp_buf, SDP_EL);
	strcat(cli->sdp_buf, "m=video 0 RTP/AVP ");
	cli->payload_type = 96;
	sprintf(cli->sdp_buf + strlen(cli->sdp_buf), "%d"SDP_EL, cli->payload_type);

	if(cli->payload_type >= 96)
	{
		strcat(cli->sdp_buf, "a=rtpmap:");
		sprintf(cli->sdp_buf + strlen(cli->sdp_buf), "%d ", cli->payload_type);
		strcat(cli->sdp_buf, "H264/90000");
		strcat(cli->sdp_buf, SDP_EL);
		//strcat(cli->sdp_buf, "a=fmtp:96 packetization-mode=1;profile-level-id=1EE042;sprop-parameter-sets=QuAe2gLASRA=,zjCkgA==");
		//strcat(cli->sdp_buf, SDP_EL);
		strcat(cli->sdp_buf, "a=control:");
		sprintf(cli->sdp_buf + strlen(cli->sdp_buf), "rtsp://%s/%s/trackID=0", cli->host_name, cli->file_name);
		strcat(cli->sdp_buf, SDP_EL);
	}
	strcat(cli->sdp_buf, SDP_EL);
	return SUCCESS;
}

void add_time_stamp(char *buf, int crlf)
{
	struct tm *t;
	time_t now;
		
	now = time(NULL);
	t = gmtime(&now);
	strftime(buf + strlen(buf), 38, "Date: %a, %d %b %Y %H:%M:%S GMT"RTSP_EL, t);
		
	if(crlf)
		strcat(buf, "\r\n");
}


int send_describe_reply(struct client *cli, int code)
{
	sprintf(cli->rtsp_buf, "%s %d %s"RTSP_EL"CSeq: %d"RTSP_EL"Server: %s/%s"RTSP_EL,
		RTSP_VER, code, (char *)get_stat(code), cli->rtsp_cseq, PACKAGE, VERSION);
	add_time_stamp(cli->rtsp_buf, 0);
	strcat(cli->rtsp_buf, "Content-Type: application/sdp"RTSP_EL);
	//sprintf(cli->rtsp_buf + strlen(cli->rtsp_buf), "Content-Base: rtsp://%s/%s/"
	//		RTSP_EL, cli->host_name, cli->file_name);	
	sprintf(cli->rtsp_buf + strlen(cli->rtsp_buf), "Content-Length: %d"RTSP_EL, 
			strlen(cli->sdp_buf));
	
	strcat(cli->rtsp_buf, RTSP_EL);
	strcat(cli->rtsp_buf, cli->sdp_buf);
	
	DEBUG("cli->sdp_buf %s", cli->sdp_buf);
	return send_msg(cli->fd, cli->rtsp_buf, strlen(cli->rtsp_buf));
}

int rtsp_describe(struct client *cli)
{
	if(check_rtsp_url(cli))
		return ERROR;

	if(check_rtsp_filename(cli))
		return ERROR;

	/* get the description format SDP is recomended */
	if(strstr(cli->rtsp_buf, HDR_ACCEPT) != NULL)
	{
		if(strstr(cli->rtsp_buf, "application/sdp") == NULL)
		{
			send_reply(551, cli);
			return ERROR;
		}
	}

	if(get_rtsp_cseg(cli) != SUCCESS)
		return ERROR;
	
	if(get_describe_sdp(cli) != SUCCESS)
		return ERROR;
		
	return send_describe_reply(cli, 200);
}


int set_option_reply(struct client *cli, int code)
{
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
			ret = rtsp_describe(cli);
			break;
		case SETUP:
			ret = rtsp_setup(cli);
			break;
		case PLAY:
			ret = rtsp_play(cli);
			break;
		case TEARDOWN:
			//ret = rtsp_teardown(cli);
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
#endif

void rtspd_tcp_loop(int sockfd)
{
	
}

#if 0
int set_framerate(int rate, int free_chn)
{
	switch(rate)
	{
		case 15:
			rtsp[free_chn].frame_rate_step = 3600;
			break;
		case 25:
			rtsp[free_chn].frame_rate_step = 3600;
			break;
		case 30:
			rtsp[free_chn].frame_rate_step = 3000;
			break;
		default:
			rtsp[free_chn].frame_rate_step = 3600;
			break;
	}
}
#endif

int rtspd_get_freechn()
{
	int i;
	for(i = 0; i < max_conn; i++)
	{
		if(rtsp[i].conn_status == 0)
		{
			rtsp[i].conn_status = 1;
			return i;
		}
	}
	return -1;
}

int rtspd_freechn(int chn)
{
	rtsp[chn].conn_status = 0;
	rtsp[chn].cli = NULL;
	rtsp[chn].is_running = 0;
}

int free_rtspd()
{
	int i;
	void *tret;
	if(vids_queue)
		free(vids_queue);
	
	for(i = 0; i < max_conn; i++)
	{
		free(rtsp[i].vids_buf);
		free(rtsp[i].frame_buf);
	
		rtsp[i].vids_buf = NULL;
		rtsp[i].frame_buf = NULL;

		pthread_cancel(rtsp[i].pthread_video_decode);
		pthread_join(rtsp[i].pthread_video_decode, &tret); 
		pthread_cancel(rtsp[i].pthread_audio_decode);
		pthread_join(rtsp[i].pthread_audio_decode, &tret); 
		
		close_fd(rtsp[i].fd);
	}	
	free(rtsp);
	rtsp = NULL;
}

int rtspd_chn_stop(int chn)
{
	void *tret;
	if(rtsp[chn].is_running)
	{
		pthread_cancel(rtsp[chn].pthread_video_decode);
		pthread_join(rtsp[chn].pthread_video_decode, &tret); 
		pthread_cancel(rtsp[chn].pthread_audio_decode);
		pthread_join(rtsp[chn].pthread_audio_decode, &tret); 
		rtsp[chn].is_running = 0;
        rtsp[chn].cli = NULL;
	}
	rtsp[chn].conn_status = 0;
    clear_texture();
}

int rtspd_chn_init()
{
	int i, j, ret, chn;
	max_conn = window_size * window_size;
	rtsp = (struct rtsp_cli *)malloc(sizeof(struct rtsp_cli) * max_conn);
	vids_queue = (QUEUE *)malloc(sizeof(QUEUE) * max_conn);
	get_window_size(&screen_width, &screen_height);

    vids_width = screen_width / window_size;
    vids_height = screen_height / window_size;

	DEBUG("screen_width %d screen_height %d vids_width %d vids_height %d", screen_width, screen_height,
		vids_width, vids_height);

	if(!rtsp || !vids_queue)	
		return ERROR;

	memset(rtsp, 0, sizeof(struct rtsp_cli) * max_conn);
	
	for(i = 0; i < window_size; i++)
	{
		for(j = 0; j < window_size; j++)
		{
			chn = i + j * window_size;
			rtsp[chn].chn = chn;
			rtsp[chn].x = i * vids_width;
			rtsp[chn].y = j * vids_height;
			rtsp[chn].w = vids_width;
			rtsp[chn].h = vids_height;
			rtsp[chn].video_port = chn + h264_port + 1;
			//rtsp[chn].audio_port = chn + h264_port + 1 + 50; 
			rtsp[chn].vids_buf = (uint8_t *)malloc(MAX_VIDSBUFSIZE * sizeof(unsigned char));
			if(!rtsp[chn].vids_buf)
				return ERROR;
			init_queue(&(vids_queue[chn]), rtsp[chn].vids_buf, MAX_VIDSBUFSIZE);

			rtsp[chn].frame_buf = (uint8_t *)malloc(MAX_BUFLEN);
			rtsp[chn].frame_pos = 0;
			rtsp[chn].frame_size = 0;

			if(!rtsp[chn].frame_buf)
				return ERROR;
		}		
	}
	return SUCCESS;
}

int init_rtspd()
{
#if 0
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
#endif
}



