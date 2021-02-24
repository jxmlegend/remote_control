#include "base.h"
#include "client.h"
#include "rtsp.h"
#include "server.h"
#include "control.h"

static int server_s = -1;
static pthread_t pthread_tcp;

void *thread_ffmpeg_audio_encode(void *param);
void *thread_ffmpeg_video_encode(void *param);

struct client m_client;
struct rtsp_cli m_rtsp;

void simulate_keyboard(rfb_keyevent *key);
void simulate_mouse(rfb_pointevent *point);

void exit_client()
{
    void *tret = NULL;

    pthread_join(pthread_tcp, &tret);  //等待线程同步
    DEBUG("pthread_exit %d tcp", (int)tret);
}


int send_done(struct client *cli)
{   
    cli->status = READY;
	cli->send_size = 0;
    set_request_head(cli->send_head, 0, DONE_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_done(struct client *cli)
{   
    void *tret;
    if(m_rtsp.is_running)
    {
        pthread_cancel(m_rtsp.pthread_video_decode);
        pthread_join(m_rtsp.pthread_video_decode, &tret);
        //pthread_cancel(rtsp[i].pthread_audio_decode);
        //pthread_join(rtsp[i].pthread_audio_decode, &tret);
        m_rtsp.is_running = 0;
		close_fd(m_rtsp.video_fmt.fd);
		m_rtsp.video_fmt.fd = -1;
    }
	cli->status = READY;
	return send_done(cli);
}

int send_control(struct client *cli)
{   
    cli->send_size = sizeof(struct request);
    cli->send_buf = (unsigned char *)malloc(cli->send_size);
    if(!cli->send_buf)
        return ERROR;

    struct request *req = (struct request *)cli->send_buf;

    req->code = 200;
    cli->status = CONTROL;
    set_request_head(cli->send_head, 0, CONTROL_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_control(struct client *cli)
{   
	int ret;
    if(m_rtsp.is_running)
	{
		return ERROR;
	}
    rtp_format *fmt = (rtp_format *)cli->recv_buf;

	m_rtsp.video_fmt.width = fmt->video_fmt.width;
	m_rtsp.video_fmt.height = fmt->video_fmt.height;
	m_rtsp.video_fmt.fps = fmt->video_fmt.fps;
	m_rtsp.video_fmt.bps = fmt->video_fmt.bps;
	m_rtsp.video_fmt.draw_mouse = 0;
	m_rtsp.video_fmt.fd = create_udp_client(server_ip, fmt->video_port);

	vids_width = fmt->video_fmt.width;
	vids_height = fmt->video_fmt.height;
	
	if(m_rtsp.video_fmt.fd == INVALID_SOCKET)
	{
		DEBUG("udp socket connection ip %s port %d error", server_ip, fmt->video_port);
		return ERROR;
	}	
	//cli->audio_fmt.code = 0;
	ret = pthread_create(&m_rtsp.pthread_video_decode, NULL, thread_ffmpeg_video_encode, &m_rtsp.video_fmt);
	if(ret != SUCCESS)
	{
		DEBUG("create thread encode video error");
		return ERROR;
	}

	m_rtsp.is_running = 1;
	return send_control(cli);
}

int send_play(struct client *cli)
{   
    cli->send_size = sizeof(struct request);
    cli->send_buf = (unsigned char *)malloc(cli->send_size);
    if(!cli->send_buf)
        return ERROR;

    struct request *req = (struct request *)cli->send_buf;

    req->code = 200;
    cli->status = PLAY;
    set_request_head(cli->send_head, 0, PLAY_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_play(struct client *cli)
{   
	int ret;
    if(m_rtsp.is_running)
	{
		return ERROR;
	}
    rtp_format *fmt = (rtp_format *)cli->recv_buf;

	m_rtsp.video_fmt.width = fmt->video_fmt.width;
	m_rtsp.video_fmt.height = fmt->video_fmt.height;
	m_rtsp.video_fmt.fps = fmt->video_fmt.fps;
	m_rtsp.video_fmt.bps = fmt->video_fmt.bps;
	
	m_rtsp.video_fmt.draw_mouse = 0;
	m_rtsp.video_fmt.fd = create_udp_client(server_ip, fmt->video_port);
	DEBUG("fmt->video_port %d", fmt->video_port);
	if(m_rtsp.video_fmt.fd == INVALID_SOCKET)
	{
		DEBUG("udp socket connection ip %s port %d error", server_ip, fmt->video_port);
		return ERROR;
	}	
	//cli->audio_fmt.code = 0;
	ret = pthread_create(&m_rtsp.pthread_video_decode, NULL, thread_ffmpeg_video_encode, &m_rtsp.video_fmt);
	if(ret != SUCCESS)
	{
		DEBUG("create thread encode video error");
		return ERROR;
	}
	m_rtsp.is_running = 1;
	return send_play(cli);
}

static int recv_options(struct client *cli)
{
	struct request *req = (struct request *)cli->recv_buf;
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

    int ret;
    cli->send_size = sizeof(rtp_format) + 1;
    cli->send_buf = malloc(cli->send_size);
    if(!cli->send_buf)
        return ERROR;

    rtp_format *fmt = (rtp_format *)cli->send_buf;

    fmt->video_fmt.width = screen_width;
    fmt->video_fmt.height = screen_height;
    fmt->video_fmt.fps = 12;
    fmt->video_fmt.bps = 200000;

    fmt->video_port = -1;

    set_request_head(cli->send_head, 0, OPTIONS_MSG, cli->send_size);
    return send_request(cli);
}

static int recv_login(struct client *cli)
{
	int ret;
	struct request *req = (struct request *)cli->recv_buf;
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
	DEBUG("cli->send_buf %s", cli->send_buf);
   	set_request_head(cli->send_head, 0, LOGIN_MSG, cli->send_size);
    return send_request(cli);
}

int process_client_msg(struct client *cli)
{
    int ret;
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
	    case MOUSE_MSG:
			if(cli->status == CONTROL)
				simulate_mouse(cli->recv_buf);
			ret = SUCCESS;
			break;
		case KEYBD_MSG:
			if(cli->status == CONTROL)
				simulate_keyboard(cli->recv_buf);
			ret = SUCCESS;
			break;
        default:
    		ret = ERROR;
            break;
    }
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
    //FD_SET(pipe_tcp[0], &allset);

    maxfd = maxfd > sockfd ? maxfd : sockfd;
    maxfd = maxfd > pipe_tcp[0] ? maxfd : pipe_tcp[0];

    char buf[DATA_SIZE] = {0}; 
    char *tmp = &buf[HEAD_LEN];

    struct client *current = &m_client;
	struct client pipe_cli = {0};
	pipe_cli.fd = pipe_tcp[0];
	
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
                    memset(current->recv_head, 0, HEAD_LEN);
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
	
	memset(&m_rtsp, 0, sizeof(struct rtsp_cli));

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
