#include "base.h"
#include "ffmpeg.h"

extern int screen_width, screen_height;

void ffmpeg_video_decode(video_format *fmt)
{


}

void clean_video_encode(void *arg)
{
    DEBUG("clean_encode param memory"); 
    output_stream *out = (output_stream *)arg;
    if(!out)
    {   
        DEBUG("out param is NULL !!!!!!!!!!!!!!!");
        return;
    }
    
    DEBUG("clean_encode param memory");
    if(out->img_convert_ctx)
        sws_freeContext(out->img_convert_ctx);
    DEBUG("clean_encode param memory");
    if(out->out_buffer)
        av_free(out->out_buffer);
    DEBUG("clean_encode param memory");
    if(out->frame_yuv)
        av_free(out->frame_yuv);
    DEBUG("clean_encode param memory");
    if(out->frame)
        av_free(out->frame);
    DEBUG("clean_encode param memory");
    if(out->h264codec_ctx)
        avcodec_close(out->h264codec_ctx);
    DEBUG("clean_encode param memory");
    if(out->codec_ctx)
        avcodec_close(out->codec_ctx);
    DEBUG("clean_encode param memory");
    if(out->format_ctx)
        avformat_close_input(&(out->format_ctx));
    DEBUG("clean_encode param memory");
    if(out->packet)
        av_free_packet(out->packet);
    DEBUG("clean_encode param memory");
    
    out->img_convert_ctx = NULL;
    out->out_buffer = NULL;
    out->frame = NULL;
    out->frame_yuv = NULL;
    out->codec_ctx = NULL;
    out->h264codec_ctx = NULL;
    out->format_ctx = NULL;
    out->packet = NULL;
    
    out = NULL;
    arg = NULL;
    DEBUG("clean encode thread end");
}

void ffmpeg_video_encode(video_format *fmt)
{
    int  i, videoindex, ret, got_picture;
    uint8_t *out_buffer = NULL;

    AVFormatContext *format_ctx = NULL;
    AVCodecContext  *codec_ctx = NULL;
    AVCodec         *codec = NULL;
    AVCodecContext  *h264codec_ctx = NULL;
    AVCodec         *h264codec = NULL;
    AVFrame *frame = NULL,*frame_yuv = NULL;
    AVPacket *packet = NULL;
    struct SwsContext *img_convert_ctx = NULL;

    output_stream out;

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    format_ctx = avformat_alloc_context();

    AVDictionary* options = NULL;

    char opt_buf[12] = {0};
#ifdef _WIN32
    /* 截屏 fps = framerate * 2 */
    sprintf(opt_buf, "%d", fmt->fps);
    av_dict_set(&options,"framerate", opt_buf, 0);
	if(fmt->draw_mouse)
    	av_dict_set(&options,"draw_mouse","1", 0);               //鼠标
	else
    	av_dict_set(&options,"draw_mouse","0", 0);               //鼠标
    AVInputFormat *ifmt = av_find_input_format("gdigrab");
    if(avformat_open_input(&format_ctx,"desktop",ifmt, &options)!=0)
    {
        DEBUG("Couldn't open input stream. ");
    }
#else
    sprintf(opt_buf, "%d", fmt->fps);
    av_dict_set(&options,"framerate", opt_buf, 0);
	if(fmt->draw_mouse)
    	av_dict_set(&options,"draw_mouse","1",0);               //鼠标
	else
    	av_dict_set(&options,"draw_mouse","0",0);               //鼠标

    sprintf(opt_buf, "%dx%d", screen_width, screen_height);
    av_dict_set(&options,"video_size", opt_buf, 0);
    AVInputFormat *ifmt=av_find_input_format("x11grab");
    if(avformat_open_input(&format_ctx,":0.0+0,0",ifmt, &options) != 0)
    {
        DEBUG("Couldn't open input stream. ");
        goto run_out;
    }
#endif
    if(avformat_find_stream_info(format_ctx,NULL)<0)
    {   
        DEBUG("Couldn't find stream information.");
        goto run_out;
    }
    videoindex=-1;
    for(i=0; i<format_ctx->nb_streams; i++)
    {   
        if(format_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {   
            videoindex=i;
        }
    }
    if(videoindex==-1)
    {   
        DEBUG("Couldn't find a video stream.");
        goto run_out;
    }
    /* 根据视频中的流打开选择解码器 */
    codec_ctx=format_ctx->streams[videoindex]->codec;
    codec=avcodec_find_decoder(codec_ctx->codec_id);

    codec_ctx->thread_count = 4;
    codec_ctx->thread_type = 1;

    if(codec == NULL)
    {   
        DEBUG("Couldn't find a video stream.");
        goto run_out;
    }
    //打开解码器
    if(avcodec_open2(codec_ctx, codec,NULL)<0)
    {   
        DEBUG("Could not open codec.");
        goto run_out;
    }

    frame = av_frame_alloc();
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;

    frame_yuv=av_frame_alloc();
    frame_yuv->width = fmt->width;
    frame_yuv->height = fmt->height;
    frame_yuv->format = AV_PIX_FMT_YUV420P;

    out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, fmt->width, fmt->height));
    avpicture_fill((AVPicture *)frame_yuv, out_buffer, AV_PIX_FMT_YUV420P, fmt->width, fmt->height);
    packet=(AVPacket *)av_malloc(sizeof(AVPacket));

#ifdef ARM
	img_convert_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt + 3, fmt->width, fmt->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
#else
	img_convert_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, fmt->width, fmt->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
#endif
    /* 查找h264编码器 */
    h264codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!h264codec)
    {
        DEBUG("---------h264 codec not found----");
        goto run_out;
    }

    h264codec_ctx = avcodec_alloc_context3(h264codec);
    h264codec_ctx->codec_id = AV_CODEC_ID_H264;
    h264codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    h264codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    h264codec_ctx->width = codec_ctx->width > fmt->width ? codec_ctx->width : fmt->width;
    h264codec_ctx->height = codec_ctx->height > fmt->height ? codec_ctx->height : fmt->height;
    h264codec_ctx->time_base.num = 1;
    h264codec_ctx->time_base.den = 12;//帧率(既一秒钟多少张图片)
    h264codec_ctx->bit_rate = fmt->bps;//bps; //比特率(调节这个大小可以改变编码后视频的质量)
    h264codec_ctx->framerate = (AVRational){12, 1};
    h264codec_ctx->gop_size= 24;
    h264codec_ctx->qmin = 10;
    h264codec_ctx->qmax = 51;
    h264codec_ctx->max_b_frames = 0;
    h264codec_ctx->thread_count = 4;
    h264codec_ctx->thread_type = 1;

    if (h264codec_ctx->flags & AVFMT_GLOBALHEADER)
        h264codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* 设置编码质量 */
    /* "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo" */
    AVDictionary *param = 0;
    av_dict_set(&param, "preset", "ultrafast", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);  //实现实时编码

    if (avcodec_open2(h264codec_ctx, h264codec,&param) < 0)
    {
        DEBUG("Failed to open video encoder");
        goto run_out;
    }

    out.format_ctx = format_ctx;
    out.codec_ctx = codec_ctx;
    out.codec = codec;
    out.h264codec_ctx = h264codec_ctx;
    out.h264codec = h264codec;
    out.frame = frame;
    out.frame_yuv = frame_yuv;
    out.packet = packet;
    out.img_convert_ctx = img_convert_ctx;
    out.out_buffer = out_buffer;

    pthread_cleanup_push(clean_video_encode,(void *)&out);
    for(;;)
    {
        /* 读取截屏中的数据--->packet  */
        if(av_read_frame(format_ctx, packet) < 0)
        {
            DEBUG("av_read_frame no data");
            goto run_out;
        }

        if(packet->stream_index==videoindex)
        {
            ret = avcodec_decode_video2(codec_ctx, frame, &got_picture, packet);
            if(got_picture)
            {
                sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, frame_yuv->data, frame_yuv->linesize);
                ret = avcodec_encode_video2(h264codec_ctx, packet,frame_yuv, &got_picture);
                if(ret < 0)
                {
                     DEBUG("Failed to encode!");
                }
                //h264_send_data((char *)packet->data, packet->size, cli_display.h264_udp);
            }
        }
        av_free_packet(packet);
    }
    pthread_cleanup_pop(0);


run_out:
    if(img_convert_ctx)
        sws_freeContext(img_convert_ctx);
    if(out_buffer)
        av_free(out_buffer);
    if(frame_yuv)
        av_free(frame_yuv);
    if(frame)
        av_free(frame);
    if(codec_ctx)
        avcodec_close(codec_ctx);
    if(h264codec_ctx)
        avcodec_close(h264codec_ctx);
    if(format_ctx)
        avformat_close_input(&format_ctx);
    if(packet)
        av_free_packet(packet);

    img_convert_ctx = NULL;
    out_buffer = NULL;
    frame_yuv = NULL;
    frame = NULL;
    codec_ctx = NULL;
    h264codec_ctx = NULL;
    format_ctx = NULL;  
    codec = NULL;
    h264codec = NULL;
    packet = NULL;
}


ffmpeg_audio_decode()
{

}

void ffmpeg_audio_encode(audio_format *fmt)
{

}


void *thread_ffmpeg_audio_encode(void *param)
{
    int ret;
    pthread_attr_t st_attr;
    struct sched_param sched;

    //rfb_display *vid = (rfb_display *)param;

    //pthread_detach(pthread_self());
    ret = pthread_attr_init(&st_attr);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode attr init warning ");
    }   
    ret = pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode set SCHED_FIFO warning");
    }   
    sched.sched_priority = SCHED_PRIORITY_ENCODE;
    ret = pthread_attr_setschedparam(&st_attr, &sched);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //线程可以被取消掉
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//立即退出
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);//立即退出  PTHREAD_CANCEL_DEFERRED 

    //ffmpeg_decode(vid);
    return (void *)0;
}


void *thread_ffmpeg_video_encode(void *param)
{
    int ret;
    pthread_attr_t st_attr;
    struct sched_param sched;

	video_format *fmt = (video_format *)param;

    //pthread_detach(pthread_self());
    ret = pthread_attr_init(&st_attr);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode attr init warning ");
    }   
    ret = pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode set SCHED_FIFO warning");
    }   
    sched.sched_priority = SCHED_PRIORITY_ENCODE;
    ret = pthread_attr_setschedparam(&st_attr, &sched);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //线程可以被取消掉
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//立即退出
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);//立即退出  PTHREAD_CANCEL_DEFERRED 

	ffmpeg_video_encode(fmt);
    return (void *)ret;
}

void *thread_ffmpeg_audio_decode(void *param)
{
    int ret;
    pthread_attr_t st_attr;
    struct sched_param sched;

    //rfb_display *vid = (rfb_display *)param;

    //pthread_detach(pthread_self());
    ret = pthread_attr_init(&st_attr);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode attr init warning ");
    }   
    ret = pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode set SCHED_FIFO warning");
    }   
    sched.sched_priority = SCHED_PRIORITY_DECODE;
    ret = pthread_attr_setschedparam(&st_attr, &sched);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //线程可以被取消掉
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//立即退出
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);//立即退出  PTHREAD_CANCEL_DEFERRED 

    //ffmpeg_decode(vid);
    return (void *)0;
}

void *thread_ffmpeg_video_decode(void *param)
{
    int ret;
    pthread_attr_t st_attr;
    struct sched_param sched;

    //rfb_display *vid = (rfb_display *)param;

    //pthread_detach(pthread_self());
    ret = pthread_attr_init(&st_attr);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode attr init warning ");
    }   
    ret = pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);
    if(ret)
    {   
        DEBUG("thread ffmpeg audio encode set SCHED_FIFO warning");
    }   
    sched.sched_priority = SCHED_PRIORITY_DECODE;
    ret = pthread_attr_setschedparam(&st_attr, &sched);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //线程可以被取消掉
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//立即退出
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);//立即退出  PTHREAD_CANCEL_DEFERRED 

    //ffmpeg_decode(vid);
    return (void *)0;
}
