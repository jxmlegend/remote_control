int convert_mode(int model)
{
    int i, ret;

#if 0
    for(i = 0; i < window_size ; i++)
    {
        if(display[i].cli)
        {
            send_done(display[i].cli);
        }
    }
#endif
}

void close_client(struct client *cli)
{
    close_fd(cli->fd);
}

static int recv_done(struct client *cli)
{   
    if(cli->status == CONTROL)
    {   
        //sem --;
    }   
       
    if(cli->status == PLAY)
    {   


    }   
}

static int send_done(struct client *cli)
{
    cli->status = READY;
}

static int recv_control(struct client *cli)
{
#if 0
    struct request *req = (struct request *)cli->recv_buf;
    if(req->code == 200)
    {   
        cli->status = READY;
        return SUCCESS;
    }   
    else
    {   
        DEBUG("login error code %d msg %s", req->code, req->msg);
        return ERROR;
    }
#endif
}

static int send_control(struct client *cli)
{
#if 0
    if(cli->status != READY)
        return ERROR;
    cli->status = PLAY;
#endif
}
static int send_play(struct client *cli)
{
#if 0
    cli->send_size = sizeof(rtp_format) + 1;
    cli->send_buf = malloc(cli->send_size);
    if(!cli->send_buf)
        return ERROR;

    rtp_format *fmt = (rtp_format *)cli->send_buf;
            
    //fmt->width = video_width;
    //fmt->height = video_width;
    fmt->video_code = H264;
    fmt->video_port = 23000;
    fmt->audio_code = PCM;
    fmt->audio_port = 24000;
    fmt->control_flag = 0;
    fmt->control_port = 0;
    fmt->fps = 12; 
    fmt->bps = 400000;

    set_request_head(cli->send_head, 0, PLAY_MSG, cli->send_size);
    return send_request(cli);
#endif
}

static int recv_play(struct client *cli)
{   
#if 0
    struct request *req = (struct request *)cli->recv_buf;
    DEBUG("play code %d", req->code);
    if(req->code == 200)
    {   
        cli->status = PLAY;
        //pthread_create();
        return SUCCESS;
    }   
    else
    {   
        DEBUG("play error code %d msg %s", req->code, req->msg);
        return ERROR;
    }
#endif
}



static int send_options(struct client *cli)
{
#if 0
    int ret;
    cli->send_size = sizeof(struct request) + 1;
    cli->send_buf = (unsigned char *)malloc(cli->send_size);

    if(!cli->send_buf)
        return ERROR;

    struct request *req = (struct request *)cli->send_buf;
    
    cli->status = READY;
    req->code = get_free_chn(cli);
    set_request_head(cli->send_head, 0, OPTIONS_MSG, cli->send_size);
    ret = send_request(cli);
    if(ret == SUCCESS)
    {
        return send_play(cli);
    }
    else
    {
        return ERROR;       
    }   
#endif

    
    

}

