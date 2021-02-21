#include "base.h"
#include "client.h"
#include "server.h"

static int server_s = -1;
static pthread_t pthread_sdl, pthread_tcp;
int total_connections = 0;
struct client **clients = NULL;

void exit_server()
{
	void *tret = NULL;

    pthread_join(pthread_sdl, &tret); //等待线程同步
    DEBUG("pthread_exit %d display", (int)tret);
    pthread_join(pthread_tcp, &tret);  //等待线程同步
    DEBUG("pthread_exit %d tcp", (int)tret);
}


static int recv_play(struct client *cli)
{   

}

static int send_play(struct client *cli)
{

}

static int send_options(struct client *cli)
{
	int free_chn = rtspd_freechn();
	if(free_chn == -1)
	{

	}
}

static int recv_options(struct client *cli)
{
	if(cli->status != OPTIONS)
		return ERROR;
	
	rtp_format *fmt = (rtp_format *)cli->recv_buf;
	memcpy(&cli->fmt, fmt, sizeof(rtp_format));
	return send_options(cli);	
}

static int send_login(struct client *cli)
{
	cli->send_size = sizeof(struct request) + 1;
	cli->send_buf = (unsigned char *)malloc(cli->send_size);

	if(!cli->send_buf)
		return ERROR;

	struct request *req = (struct request *)cli->send_buf;
	
	req->code = 200;
	cli->status = OPTIONS;
	set_request_head(cli->send_head, 0, LOGIN_MSG, cli->send_size);
	return send_request(cli);
}

static int recv_login(struct client *cli)
{
   	int server_major = 0, server_minor = 0;
    int client_major = 0, client_minor = 0;
	
	get_version(&server_major, &server_minor);
	sscanf(cli->recv_buf, VERSIONFORMAT, &client_major, &client_minor);
	if(server_major == client_major && server_minor == client_minor)
	{
		return send_login(cli);
	}
	return ERROR;
}

int process_server_msg(struct client *cli)
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

static void tcp_loop(int listenfd)
{
    int maxfd = 0, connfd, sockfd;
    int nready, ret, i, maxi = 0; 

    fd_set reset, allset;

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    
    struct timeval tv;
    tv.tv_sec = 1; 
    tv.tv_usec = 0; 
    
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    FD_SET(pipe_tcp[0], &allset);   

    maxfd = maxfd > listenfd ? maxfd : listenfd;
    maxfd = maxfd > pipe_tcp[0] ? maxfd : pipe_tcp[0];

	struct client pipe_cli = {0};
	clients[0] = &pipe_cli;	
	pipe_cli.fd = pipe_tcp[0];
	struct client *cli = NULL;
	time_t last_time = current_time;

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
                DEBUG("select %d %s ", errno, strerror(errno));
                return;
            }
        }
        nready = ret;

	   	(void)time(&current_time);
        if(current_time - last_time >= TIME_OUT)
        {   
            last_time = current_time;
        } 
	
        /* new connect */
        if(FD_ISSET(listenfd, &reset))
        {    
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
            if(connfd < 0) 
                continue;
            cli = (struct client *)malloc(sizeof(struct client));
            if(!cli)
            {    
                DEBUG("new connect and malloc struct client error :%s", strerror(errno));
                continue;
            }    
            memset(cli, 0, sizeof(struct client));
            cli->fd = connfd;
            cli->recv_size = HEAD_LEN;
#ifdef _WIN32
			ret = 1;
    		if(ioctlsocket(connfd, FIONBIO, (u_long *)&ret) == SOCKET_ERROR)
    		{   
        		DEBUG("fcntl F_SETFL fail");
    		} 
            memcpy(cli->ip,inet_ntoa(cliaddr.sin_addr), sizeof(cli->ip));
#else
            ret = fcntl(connfd, F_GETFL, 0);
            if(ret < 0) 
            {    
                DEBUG("fcntl connfd: %d  F_GETFL error :%s", connfd, strerror(errno));
                close_fd(connfd);
                free(cli);
                continue;
            }    

            if(fcntl(connfd, F_SETFL, ret | O_NONBLOCK) < 0) 
            {    
                DEBUG("fcntl connfd: %d F_SETFL O_NONBLOCK error :%s", connfd, strerror(errno));
                close_fd(connfd);
                free(cli);
                continue;
            }    
            /* recode client ip */
            if(inet_ntop(AF_INET, &cliaddr.sin_addr, cli->ip, sizeof(cli->ip)) == NULL)
            {    
                DEBUG("connfd: %d inet_ntop error ",connfd, strerror(errno));
                close_fd(connfd);
                free(cli);
                continue;
            }    
#endif
            FD_SET(connfd, &allset);
            for(i = 0; i < max_connections; i++)
            {
                if(clients[i] == NULL)
				{
                    clients[i] = cli;
                	break;
				}
            }
			cli->session_id = i;
            total_connections ++;
            if(i >= maxi)
                maxi = i;
            if(connfd > maxfd)
                maxfd = connfd;

            DEBUG("client index: %d total_connections: %d maxi: %d connfd ip: %s",i, total_connections, maxi, cli->ip);
            if(--nready <= 0)
                continue;
        }
    
		for(i = 0; i <= maxi; i++)
		{
            if(clients[i] == NULL || (sockfd = clients[i]->fd) < 0)     
                continue;
            if(FD_ISSET(sockfd, &reset))
            {    
                if(clients[i]->has_read_head == 0)
                {    
                    if((ret = recv(sockfd, (void *)clients[i]->recv_head + clients[i]->pos, 
                                        HEAD_LEN-clients[i]->pos, 0)) <= 0)     
                    {    
                        if(ret < 0) 
                        {    
                            if(errno == EINTR || errno == EAGAIN)
                                continue;
                        }    
                        DEBUG("client close index: %d ip: %s port %d", i,
                                    clients[i]->ip, clients[i]->port);

                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }    
                    clients[i]->pos += ret; 
                    if(clients[i]->pos != HEAD_LEN)
                        continue;
     
                    if(read_msg_syn(clients[i]->recv_head) != DATA_SYN)
                    {    
                        DEBUG(" %02X client send SYN flag error close client index: %d ip: %s port %d", 
                            read_msg_syn(clients[i]->recv_head), i, clients[i]->ip, 
                            clients[i]->port);
                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }    
                    clients[i]->has_read_head = 1; 
                    clients[i]->recv_size = read_msg_size(clients[i]->recv_head);
                    clients[i]->pos = 0; 
                    if(clients[i]->recv_size >= 0 && clients[i]->recv_size < CLIENT_BUF)
                    {    
                        clients[i]->recv_buf = (unsigned char*)malloc(clients[i]->recv_size + 1);
                        if(!clients[i]->recv_buf)
                        {    
                            DEBUG("malloc data buf error: %s close client index: %d ip: %s port %d",
                                    strerror(errno), i, clients[i]->ip, clients[i]->port);
                            FD_CLR(clients[i]->fd, &allset);
                            close_client(clients[i]);
                            clients[i] = NULL;
                            total_connections--;
                            continue;
                        }
                    }
                    else
                    {
                        DEBUG("client send size: %d error close client index: %d ip: %s port %d",
                                clients[i]->recv_size, i, clients[i]->ip, clients[i]->port);
                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }
                }
                if(clients[i]->has_read_head == 1)
                {
                    if(clients[i]->pos < clients[i]->recv_size)
                    {
                        if((ret = recv(sockfd,clients[i]->recv_buf+clients[i]->pos,
                                        clients[i]->recv_size - clients[i]->pos,0)) <= 0)
                        {
                            if(ret < 0)
                            {
                                if(errno == EINTR || errno == EAGAIN)
                                    continue;
                                DEBUG("client close index: %d ip: %s port %d",i, clients[i]->ip, clients[i]->port);
                                FD_CLR(clients[i]->fd, &allset);
                                close_client(clients[i]);
                                clients[i] = NULL;
                                total_connections--;
                                continue;
                            }
                        }
                        clients[i]->pos += ret;
                    }
                    if(clients[i]->pos == clients[i]->recv_size)
                    {
						ret = process_server_msg(clients[i]);
                        if(ret != SUCCESS)
                        {
							if(ret == EXIT_PIPE)
								goto run_out;

                            DEBUG("process msg error client index: %d ip: %s port %d",i, clients[i]->ip, clients[i]->port);
                            FD_CLR(clients[i]->fd, &allset);
                            close_client(clients[i]);
                            clients[i] = NULL;
                            total_connections--;
                            continue;
                        }
                        memset(clients[i]->recv_head, 0, HEAD_LEN);
                        clients[i]->recv_size = HEAD_LEN;
                        clients[i]->pos = 0;
                        if(clients[i]->recv_buf)
                            free(clients[i]->recv_buf);
                        clients[i]->recv_buf = NULL;
                        clients[i]->has_read_head = 0;
                    }
                    if(clients[i]->pos > clients[i]->recv_size)
                    {
                        DEBUG("loss msg data client index: %d ip: %s port %d", i, clients[i]->ip, clients[i]->port);
                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }
                }
#if RTSP
				if((ret = recv(sockfd, (void *)cli->rtsp_buf, sizeof(cli->rtsp_buf), 0)) <= 0)     
                {    
                    if(ret < 0) 
                    {    
                        if(errno == EINTR || errno == EAGAIN)
                            continue;
                    }    
                    DEBUG("client close index: %d ip: %s port %d", i,
                                clients[i]->ip, clients[i]->port);
                    FD_CLR(clients[i]->fd, &allset);
                    close_client(clients[i]);
                    clients[i] = NULL;
                    total_connections--;
                    continue;
                }    
				cli->recv_size = ret;
				//DEBUG("%s", cli->rtsp_buf);
 				if(rtsp_cmd_match(cli) < 0)
				{
				    FD_CLR(clients[i]->fd, &allset);
                    close_client(clients[i]);
                    clients[i] = NULL;
                    total_connections--;
				}
#endif
                if(--nready <= 0)
                    break;
			}
		}
	}

run_out:
	DEBUG("tcp loop end");
	return ;
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

int init_server()
{
	int ret;
    if(window_size <= 0 || window_size > 5)
    {   
        DEBUG("window size: %d error", window_size);
        return ERROR;
    }   

	server_s = create_tcp_server(NULL, client_port);
	if(server_s == -1)
	{
		free(clients);
		return ERROR;
	}

    clients = (struct client **)malloc(sizeof(struct client *) * max_connections);
    if(!clients)
    {   
        DEBUG("clients malloc error %s max_connections %d", strerror(errno), max_connections);
        return ERROR;
    }   
	
	/* socket */	
	ret = pthread_create(&pthread_tcp, NULL, thread_tcp, &server_s);
	if(SUCCESS != ret)
	{
		free(clients);
		close_fd(server_s);
		return ERROR;
	}
#if 0
	/* event */	
	ret = pthread_create(&pthread_sdl, NULL, thread_sdl, NULL);
	if(SUCCESS != ret)
	{
		pthread_cancel(pthread_tcp);				
		free(clients);
		close_fd(server_s);
		return ERROR;
	}
#endif
	return SUCCESS;	
}

