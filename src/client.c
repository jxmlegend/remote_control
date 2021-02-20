#include "base.h"
#include "client.h"
#include "server.h"

static int server_s = -1;
static pthread_t pthread_tcp;

void *thread_ffmpeg_audio_encode(void *param);
void *thread_ffmpeg_video_encode(void *param);

struct client m_client;

static void exit_client()
{

}

static int recv_done(struct client *cli)
{   
    if(cli->status == CONTROL)
    {   
        //sem --;
    }
}

static int send_done(struct client *cli)
{   
    cli->status = READY;
}

static int send_control(struct client *cli)
{   
    cli->status = PLAY;
}

static int recv_control(struct client *cli)
{   
    if(cli->status != READY)
        return ERROR;
}

static int send_play(struct client *cli)
{   
    cli->status = PLAY;
}

static int recv_play(struct client *cli)
{   
    if(cli->status != READY)
        return ERROR;
}

static int send_options(struct client *cli)
{   
    cli->status = READY;
    // -> video 
    // -> control 
}

static int recv_options(struct client *cli)
{
    if(cli->status != OPTIONS)
        return ERROR;

    cli->status = READY;
}

static int send_login(struct client *cli)
{
    cli->status = OPTIONS;
}

static int recv_login(struct client *cli)
{
    int ret;
    int server_major = 0, server_minor = 0;
    int client_major = 0, client_minor = 0;

    get_version(&server_major, &server_minor);
    //sscanf(cli->data_buf, VERSIONFORMAT, &client_major, &client_minor);
    if(server_major == client_major && server_minor == client_minor)
    {
        ret = send_login(cli);
    }
    else
    {
        ret = ERROR;
#if 0
        DEBUG("version server"VERSIONFORMAT" client"VERSIONFORMAT "error", server_major, server_minor, 
                client_major, client_minor);
#endif
    }
    return ret;
}


int process_client_msg(struct client *cli)
{
    int ret;
    DEBUG("read_msg_order(cli->head_buf) %d", read_msg_order(cli->head_buf));
    switch(read_msg_order(cli->head_buf))
    {   
        /* pipe msg */
        case EXIT_PIPE:
            ret = EXIT_PIPE;
            break;
        case LOGIN_MSG:
            ret = recv_login(cli);
            break;
        case OPTIONS_MSG:
            ret = recv_login(cli);
            break;
        case PLAY_MSG:
            ret = recv_login(cli);
            break;
        case CONTROL_MSG:
            ret = recv_login(cli);
            break;
        case DONE_MSG:
            ret = recv_login(cli);
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
                if((ret = recv(current->fd, current->head_buf+current->pos,HEAD_LEN - current ->pos, 0)) <= 0)
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
                if(read_msg_syn(current->head_buf) != DATA_SYN)
                {
                    current->pos = 0;
                    current->has_read_head = 0;
                    continue;
                }

                current->has_read_head = 1;
                current->data_size = read_msg_size(current->head_buf);
                current->pos = 0;

                if(current->data_size < 0 || current->data_size > CLIENT_BUF)
                {
                    current->pos = 0;
                    current->has_read_head = 0;
                    continue;
                }
                else if(current->data_size > 0)
                {
                    current->data_buf = (unsigned char*)malloc(current->data_size + 1);
                    if(!current->data_buf)
                    {
                        DEBUG("current->data_buf malloc error : %s ", strerror(errno));
                        break;
                    }
                    memset(current->data_buf, 0, current->data_size + 1);
                }
            }
            if(current->has_read_head == 1)
            {
                if(current->pos < current->data_size)
                {
                    if((ret = recv(current->fd, current->data_buf + current->pos, current->data_size - current ->pos,0)) <= 0)
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
                if(current->pos == current->data_size)
                {
                    if(process_client_msg(current))
                    {
						//close_fd();
                    }
                    memset(current->head_buf, 0, HEAD_LEN);
                    current->data_size = 0;
                    current->pos = 0;
                    if(current->data_buf)
                        free(current->data_buf);
                    current->data_buf = NULL;
                    current->has_read_head = 0;
                }

                if(current->pos > current->data_size)
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

	server_s = create_tcp_client("", 9999);
	if(server_s == -1)
	{
		DEBUG("");
		return ERROR;
	}

	ret = pthread_create(pthread_tcp, NULL, thread_tcp, &server_s);
	if(SUCCESS != ret)
	{
		close_fd(server_s);
		return ERROR;
	}
	ret = send_login(&m_client);	
	if(SUCCESS != ret)
	{
		close_fd(server_s);
		return ERROR;
	}

	return SUCCESS;
}
