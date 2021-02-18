#include "base.h"

/* pipe */
int pipe_tcp[2] = {0};
int pipe_udp[2] = {0};
int pipe_event[2] = {0};
int pipe_ui[2] = {0};

int init_pipe()
{
#ifdef _WIN32

#else
    /* create pipe to give main thread infomation */
    if(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pipe_tcp) < 0)
    {   
        DEBUG("create server pipe err %s", strerror(errno));
        return ERROR;
    }

    fcntl(pipe_tcp[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_tcp[1], F_SETFL, O_NONBLOCK);

    /* create pipe to give main thread infomation */
    if(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pipe_udp) < 0)
    {
        DEBUG("create client pipe err %s", strerror(errno));
        return ERROR;
    }

    fcntl(pipe_udp[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_udp[1], F_SETFL, O_NONBLOCK);

    /* create pipe to give main thread infomation */
    if(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, pipe_event) < 0)
    {
        DEBUG("create client pipe err %s", strerror(errno));
        return ERROR;
    }

    fcntl(pipe_event[0], F_SETFL, O_NONBLOCK);
    fcntl(pipe_event[1], F_SETFL, O_NONBLOCK);
#endif

}


int close_pipe()
{
    close_fd(pipe_event[0]);
    close_fd(pipe_event[1]);
    close_fd(pipe_tcp[0]);
    close_fd(pipe_tcp[1]);
    close_fd(pipe_udp[0]);
    close_fd(pipe_udp[1]);
}

int send_pipe(char *buf, short cmd, int size, int type)
{
    int ret; 
    set_request_head(buf, 0x0, cmd, size);
    switch(type)
    {    
        case PIPE_UI:
            ret = write(pipe_ui[1], buf, size + HEAD_LEN);
            break;
        case PIPE_TCP:
            ret = write(pipe_tcp[1], buf, size + HEAD_LEN);
            break;
        case PIPE_EVENT:
            ret = write(pipe_event[1], buf, size + HEAD_LEN);
            break;
    }    
    return ret;
}


