#ifndef __RTSP_H__
#define __RTSP_H__

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

extern struct video_format;
extern struct audio_format;

typedef struct rtsp_cli
{
    int video_fd;
	int audio_fd;
	
	uint32_t video_port;
	uint32_t audio_port;

    struct sockaddr_in recv_addr;
    struct sockaddr_in send_addr;

    uint8_t chn;
    uint8_t is_running;
    uint8_t conn_status;
    uint8_t rtspd_status;

    uint8_t *vids_buf;
    pthread_t pthread_video_decode;
    pthread_t pthread_audio_decode;

    struct video_format video_fmt;
    struct audio_format audio_fmt;

    uint8_t *frame_buf;
    uint32_t frame_pos;
    uint32_t frame_size;

    unsigned char rtsp_buf[DATA_SIZE];
    unsigned char sdp_buf[DATA_SIZE];
    unsigned char host_name[128];
    unsigned char file_name[128];
    uint32_t rtsp_cseq;
    int32_t payload_type;
    int32_t session_id;
    uint32_t ssrc;
    uint16_t rtp_port;
    uint16_t rtcp_port;
    uint8_t nalu_buffer[1448];
    int is_tcp;
	
    struct client *cli;
}rtsp_cli;

typedef struct rtp_format
{
    struct video_format video_fmt;
    struct audio_format audio_fmt;
	
	uint32_t video_port;
	uint32_t audio_port;
	uint32_t control_port;
	
	uint32_t model;
}rtp_format;


#endif
