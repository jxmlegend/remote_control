#include "base.h"
#include "ffmpeg.h"
#include "rtsp.h"

struct rtsp_cli *rtsp = NULL;
QUEUE *vids_queue = NULL;

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

        close_fd(rtsp[i].video_fd);
    }
    free(rtsp);
    rtsp = NULL;
}

void rtspd_chn_stop(struct rtsp_cli *chn)
{
	void *tret;
	if(!chn)
		return ;

	if(chn->is_running)
	{
        pthread_cancel(chn->pthread_video_decode);
        pthread_join(chn->pthread_video_decode, &tret);
        //pthread_cancel(chn->pthread_audio_decode);
        //pthread_join(chn->pthread_audio_decode, &tret);
        chn->is_running = 0;
        //chn->cli = NULL;
	}
    chn->conn_status = 0;
}

#if 0
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
}
#endif


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
            rtsp[chn].video_fmt.chn = chn;
            rtsp[chn].video_fmt.rect.x = i * vids_width;
            rtsp[chn].video_fmt.rect.y = j * vids_height;
            rtsp[chn].video_fmt.rect.w = vids_width;
            rtsp[chn].video_fmt.rect.h = vids_height;
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




