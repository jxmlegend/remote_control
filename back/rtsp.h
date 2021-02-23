#ifndef __RTSP_H__
#define __RTSP_H__

#include "ffmpeg.h"

#define MAX_CONN 4

#define RTSP_EL "\r\n"
#define RTSP_VER "RTSP/1.0"
#define HDR_REQUIRE "Require"
#define HDR_ACCEPT "Accept"
#define PACKAGE "ezrtspd"
#define VERSION "1.0"
#define SDP_EL "\r\n"
#define HDR_TRANSPORT "Transport"
#define HDR_SESSION "Session"

#define DEFAULT_TTL 32

#if 0
typedef enum RTSP_OPT{
	UNKNOWN = 0,
	OPTIONS,
	DESCRIBE,
	SETUP,
	PLAY,
	TEARDOWN,
}RTSP_OPT;
#endif


struct rtsp_buffer
{
	//int payload_type;		// 96 h263/h264
	//int session_id;
	//unsigned int rtsp_
	
	//unsigned frame_rate_step;
	uint32_t frame_rate_step;
	uint32_t time_stamp;	
	uint16_t seq;
	uint32_t ssrc;		

	uint8_t in_buf[1024];
	uint8_t out_buf[1024];
};

extern struct client;

typedef struct rtsp_cli
{
	int fd;
	
    struct sockaddr_in recv_addr;
    struct sockaddr_in send_addr; 
	
	uint8_t chn;
	uint8_t is_running;
	uint8_t conn_status;
	uint8_t rtspd_status;

	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;

	uint32_t video_port;
	uint32_t audio_port;

	uint8_t *vids_buf;
	pthread_t pthread_video_decode;
	pthread_t pthread_audio_decode;

	video_format video_fmt;
    audio_format audio_fmt;


	uint8_t *frame_buf;
	uint32_t frame_pos;
	uint32_t frame_size;

	struct client *cli;
}rtsp_cli;



#endif
