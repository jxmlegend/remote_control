#include "base.h"
#include "client.h"
#include "server.h"

static int server_s = -1;
static pthread_t pthread_tcp, pthread_encode_video, pthread_audio;

void *thread_ffmpeg_audio_encode(void *param);
void *thread_ffmpeg_video_encode(void *param);

struct client m_client;

void exit_client()
{
    void *tret = NULL;

    pthread_join(pthread_tcp, &tret);  //等待线程同步
    DEBUG("pthread_exit %d tcp", (int)tret);
}

static int recv_done(struct client *cli)
{   

	return SUCCESS;
}

int send_done(struct client *cli)
{   
    cli->status = READY;
	cli->send_size = 0;
    set_request_head(cli->send_head, 0, DONE_MSG, cli->send_size);
    return send_request(cli);
}

int send_control(struct client *cli, int code)
{   
    cli->send_size = sizeof(struct request);
    cli->send_buf = (unsigned char *)malloc(cli->send_size);
    if(!cli->send_buf)
        return ERROR;

    struct request *req = (struct request *)cli->send_buf;

    req->code = code;
    cli->status = CONTROL;
    set_request_head(cli->send_head, 0, CONTROL_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_control(struct client *cli)
{   
	int ret;
	void *tret;
    if(cli->status != READY)
	{
		pthread_cancel(pthread_encode_video);
    	pthread_join(pthread_encode_video, &tret);  //等待线程同步
	}

    rtp_format *fmt = (rtp_format *)cli->recv_buf;

	DEBUG("fmt->width %d fmt->video_code %d fmt->control_flag %d", fmt->width, fmt->video_code, fmt->control_flag);
	ret = pthread_create(&pthread_encode_video, NULL, thread_ffmpeg_video_encode, NULL);
	if(ret != SUCCESS)
	{

	}
	ret = send_control(cli, 200);
	return SUCCESS;
}

int send_play(struct client *cli, int code)
{   
    cli->send_size = sizeof(struct request);
    cli->send_buf = (unsigned char *)malloc(cli->send_size);
    if(!cli->send_buf)
        return ERROR;

    struct request *req = (struct request *)cli->send_buf;

    req->code = code;
    cli->status = PLAY;
    set_request_head(cli->send_head, 0, PLAY_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_play(struct client *cli)
{   
	int ret;
	void *tret;
    if(cli->status != READY)
	{
		pthread_cancel(pthread_encode_video);
    	pthread_join(pthread_encode_video, &tret);  //等待线程同步
	}

    rtp_format *fmt = (rtp_format *)cli->recv_buf;

	DEBUG("fmt->width %d fmt->video_code %d fmt->control_flag %d", fmt->width, fmt->video_code, fmt->control_flag);
	ret = pthread_create(&pthread_encode_video, NULL, thread_ffmpeg_video_encode, NULL);
	if(ret != SUCCESS)
	{

	}
	ret = send_play(cli, 200);
	return SUCCESS;
}

static int recv_options(struct client *cli)
{
	struct request *req = (struct request *)cli->recv_buf;
	DEBUG("option code %d", req->code);
	if(req->code == 200)
	{
		cli->status = READY;
		return SUCCESS;
	}
	else
	{
		DEBUG("option error code %d msg %s", req->code, req->msg);
		return ERROR;
	}
}

static int send_options(struct client *cli)
{   
	/* video format audio format ffmpeg code option yuv rgb width, height  */
	cli->send_size = sizeof(rtp_format) + 1;
	cli->send_buf = malloc(cli->send_size);
	if(!cli->send_buf)
		return ERROR;

	rtp_format *fmt = (rtp_format *)cli->send_buf;
	
	fmt->width = screen_width;
	fmt->height = screen_width;
	fmt->video_code = H264;
	fmt->audio_code = PCM;
	fmt->fps = 12;
	fmt->bps = 400000;

   	set_request_head(cli->send_head, 0, OPTIONS_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_login(struct client *cli)
{
	int ret;
	struct request *req = (struct request *)cli->recv_buf;
	DEBUG("recv_login");
	if(req->code == 200)
	{
		return send_options(cli);	
	}
	else
	{
		ret = ERROR;
		DEBUG("login error code %d msg %s", req->code, req->msg);
	}
	return ret;
}

static int send_login(struct client *cli)
{
	int client_major = 0, client_minor = 0;
    get_version(&client_major, &client_minor);
	
	cli->send_size = SZ_VERFORMAT;
	cli->send_buf = (unsigned char *)malloc(cli->send_size + 1);
	if(!cli->send_buf)
		return ERROR;	

	sprintf(cli->send_buf, VERSIONFORMAT, client_major, client_minor);
   	set_request_head(cli->send_head, 0, LOGIN_MSG, cli->send_size);
    return send_request(cli);
}

int process_client_msg(struct client *cli)
{
    int ret;
    DEBUG("read_msg_order(cli->recv_head) %d", read_msg_order(cli->recv_head));
    switch(read_msg_order(cli->recv_head))
    {   
        /* pipe msg */
        case EXIT_PIPE:
            ret = EXIT_PIPE;
            break;
        case LOGIN_MSG:
            ret = recv_login(cli);
            break;
        case OPTIONS_MSG:
            ret = recv_options(cli);
            break;
        case PLAY_MSG:
            ret = recv_play(cli);
            break;
        case CONTROL_MSG:
            ret = recv_control(cli);
            break;
        case DONE_MSG:
            ret = recv_done(cli);
            break;
        default:
            break;
    }
    ret = SUCCESS;
    return ret;
}


static void tcp_loop(int sockfd)
{
    int maxfd = 0;
    int nready, ret, i, maxi = 0;
    fd_set reset, allset;

    struct timeval tv;
    tv.tv_sec = 1; 
    tv.tv_usec = 0; 

    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);
    FD_SET(pipe_tcp[0], &allset);

    maxfd = maxfd > sockfd ? maxfd : sockfd;
    maxfd = maxfd > pipe_tcp[0] ? maxfd : pipe_tcp[0];

    char buf[DATA_SIZE] = {0}; 
    char *tmp = &buf[HEAD_LEN];

    struct client *current = &m_client;
	struct client pipe_cli = {0};
	pipe_cli.fd = pipe_tcp[0];
	
	DEBUG("sockfd %d m_client %d", sockfd, m_client.fd);
    for(;;)
    {    
        tv.tv_sec = 1; 
        reset = allset;
        ret = select(maxfd + 1, &reset, NULL, NULL, &tv);
        if(ret == -1)
        {    
            if(errno == EINTR)
                continue;
            else if(errno != EBADF)
            {    
                DEBUG("select %s", strerror(ret));
                break;  
            }    
        }    
        nready = ret; 
        if(FD_ISSET(sockfd, &reset))
        {
            if(current->has_read_head == 0)
            {
                if((ret = recv(current->fd, current->recv_head + current->pos, HEAD_LEN - current ->pos, 0)) <= 0)
                {
                    if(ret < 0)
                    {
                        if(errno == EINTR || errno == EAGAIN)
                            continue;
                    }
                    DEBUG("close fd %d", sockfd);
                    break;
                }
                current->pos += ret;
                if(current->pos != HEAD_LEN)
                    continue;
                if(read_msg_syn(current->recv_head) != DATA_SYN)
                {
                    current->pos = 0;
                    current->has_read_head = 0;
                    continue;
                }

                current->has_read_head = 1;
                current->recv_size = read_msg_size(current->recv_head);
                current->pos = 0;

                if(current->recv_size < 0 || current->recv_size > CLIENT_BUF)
                {
                    current->pos = 0;
                    current->has_read_head = 0;
                    continue;
                }
                else if(current->recv_size > 0)
                {
					if(current->recv_buf)
						free(current->recv_buf);
					
                    current->recv_buf = (unsigned char*)malloc(current->recv_size + 1);
                    if(!current->recv_buf)
                    {
                        DEBUG("current->data_buf malloc error : %s ", strerror(errno));
                        break;
                    }
                }
            }
            if(current->has_read_head == 1)
            {
                if(current->pos < current->recv_size)
                {
                    if((ret = recv(current->fd, current->recv_buf + current->pos, current->recv_size - current ->pos,0)) <= 0)
                    {
                        if(ret < 0)
                        {
                            if(errno == EINTR || errno == EAGAIN)
                                continue;
                        }
                        DEBUG("close fd %d", sockfd);
                        break;
                    }
                    current->pos += ret;
                }
                if(current->pos == current->recv_size)
                {
                    if(process_client_msg(current))
                    {
						DEBUG("process_msg error");
						break;
                    }
                    memset(current->recv_buf, 0, HEAD_LEN);
                    current->data_size = 0;
                    current->pos = 0;
                    if(current->recv_buf)
                        free(current->recv_buf);
                    current->recv_buf = NULL;
                    current->has_read_head = 0;
                }

                if(current->pos > current->recv_size)
                {
                    current->pos = 0;
                    current->has_read_head = 0;
                    continue;
                }
            }
            if(--nready <= 0)
                continue;
        }
    }
}

static void *thread_tcp(void *param)
{
    int ret;
    pthread_attr_t st_attr;
    struct sched_param sched;

    int sockfd = *(int *)param;

    //pthread_detach(pthread_self());
    ret = pthread_attr_init(&st_attr);
    if(ret)
    {   
        DEBUG("thread server tcp attr init warning ");
    }   
    ret = pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);
    if(ret)
    {   
        DEBUG("thread server tcp set SCHED_FIFO warning");
    }   
    sched.sched_priority = SCHED_PRIORITY_DECODE;
    ret = pthread_attr_setschedparam(&st_attr, &sched);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //线程可以被取消掉
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//立即退出
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);//立即退出  PTHREAD_CANCEL_DEFERRED 

    tcp_loop(sockfd);
    return (void *)0;
}

int init_client()
{
	int ret;
	server_s = create_tcp_client(server_ip, server_port);
	memset(&m_client, 0, sizeof(struct client));
	if(server_s == -1)
	{
		DEBUG("connect server ip %s port %d error", server_ip, server_port);
		return ERROR;
	}
	m_client.fd = server_s;

	ret = pthread_create(&pthread_tcp, NULL, thread_tcp, &server_s);
	if(SUCCESS != ret)
	{
		close_fd(server_s);
		return ERROR;
	}
	ret = send_login(&m_client);	
	if(SUCCESS != ret)
	{
		DEBUG("login error");
		close_fd(server_s);
		return ERROR;
	}
	
	return SUCCESS;
}
