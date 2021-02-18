#include "base.h"
#include "client.h"

static int server_s = -1;
static pthread_t pthread_event, pthread_tcp;
int total_connections = 0;
struct client **clients = NULL;

void exit_server()
{

}

int convert_mode(int mode)
{

}

void close_client()
{

}

int process_server_msg(struct client *cli)
{
	int ret;
	
	switch(read_msg_order(cli->head_buf))
	{
		/* pipe msg */

		//case 

		//case :

		default:
			break;	
	}
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
    
		for(i = 0; i <= maxi; i++)
		{
            if(clients[i] == NULL || (sockfd = clients[i]->fd) < 0)     
                continue;
            if(FD_ISSET(sockfd, &reset))
            {    
                if(clients[i]->has_read_head == 0)
                {    
                    if((ret = recv(sockfd, (void *)clients[i]->head_buf + clients[i]->pos, 
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
     
                    if(read_msg_syn(clients[i]->head_buf) != DATA_SYN)
                    {    
                        DEBUG(" %02X client send SYN flag error close client index: %d ip: %s port %d", 
                            read_msg_syn(clients[i]->head_buf), i, clients[i]->ip, 
                            clients[i]->port);

                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }    
     
                    clients[i]->has_read_head = 1; 
                    clients[i]->data_size = read_msg_size(clients[i]->head_buf);
                    clients[i]->pos = 0; 
                    if(clients[i]->data_size >= 0 && clients[i]->data_size < CLIENT_BUF)
                    {    
                        clients[i]->data_buf = (unsigned char*)malloc(clients[i]->data_size + 1);
                        if(!clients[i]->data_buf)
                        {    
                            DEBUG("malloc data buf error: %s close client index: %d ip: %s port %d",
                                    strerror(errno), i, clients[i]->ip, clients[i]->port);

                            FD_CLR(clients[i]->fd, &allset);
                            close_client(clients[i]);
                            clients[i] = NULL;
                            total_connections--;
                            continue;
                        }
                        memset(clients[i]->data_buf, 0, clients[i]->data_size);
                    }
                    else
                    {
                        DEBUG("client send size: %d error close client index: %d ip: %s port %d",
                                clients[i]->data_size, i, clients[i]->ip, clients[i]->port);

                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }
                }

                if(clients[i]->has_read_head == 1)
                {
                    if(clients[i]->pos < clients[i]->data_size)
                    {
                        if((ret = recv(sockfd,clients[i]->data_buf+clients[i]->pos,
                                        clients[i]->data_size - clients[i]->pos,0)) <= 0)
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
                    if(clients[i]->pos == clients[i]->data_size)
                    {
                        if(process_server_msg(clients[i]))
                        {
                            DEBUG("process msg error client index: %d ip: %s port %d",i, clients[i]->ip, clients[i]->port);
                            FD_CLR(clients[i]->fd, &allset);
                            close_client(clients[i]);
                            clients[i] = NULL;
                            total_connections--;
                            continue;
                        }
                        memset(clients[i]->head_buf, 0, HEAD_LEN);
                        clients[i]->data_size = HEAD_LEN;
                        clients[i]->pos = 0;
                        if(clients[i]->data_buf)
                            free(clients[i]->data_buf);
                        clients[i]->data_buf = NULL;
                        clients[i]->has_read_head = 0;
                    }
                    if(clients[i]->pos > clients[i]->data_size)
                    {
                        DEBUG("loss msg data client index: %d ip: %s port %d", i, clients[i]->ip, clients[i]->port);
                        FD_CLR(clients[i]->fd, &allset);
                        close_client(clients[i]);
                        clients[i] = NULL;
                        total_connections--;
                        continue;
                    }
                }
                if(--nready <= 0)
                    break;
			}

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

int init_server()
{
	int ret;
#if 0
	init_logs();

	ret = load_wsa();
    if(SUCCESS != ret)
    {   
        DEBUG("load wsa error");
		return ERROR;
    }

	ret = init_pipe();
	if(SUCCESS != ret)
	{

	}
#endif
	
	ret = init_SDL();
}

int start_server()
{
	int ret;
	
    if(window_size <= 0 || window_size > 5)
    {   
        DEBUG("window size: %d error", window_size);
        return ERROR;
    }   

    clients = (struct client **)malloc(sizeof(struct client *) * max_connections);
    if(!clients)
    {   
        DEBUG("clients malloc error %s max_connections %d", strerror(errno), max_connections);
        return ERROR;
    }   
	server_s = create_tcp_server(NULL, client_port);
	if(server_s == -1)
	{
		free(clients);
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

	/* event */	
	ret = pthread_create(&pthread_event, NULL, thread_event, NULL);
	if(SUCCESS != ret)
	{
		pthread_cancel(pthread_tcp);				
		free(clients);
		close_fd(server_s);
		return ERROR;
	}
	return SUCCESS;	
}

int stop_server()
{

}
