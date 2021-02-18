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


typedef enum RTSP_OPT{
	UNKNOWN = 0,
	OPTIONS,
	DESCRIBE,
	SETUP,
	PLAY,
	TEARDOWN,
}RTSP_OPT;


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




#endif
