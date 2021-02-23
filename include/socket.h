#ifndef __SOCKET_H__
#define __SOCKET_H__


#define INVALID_SOCKET -1


typedef struct req_head
{
    unsigned char syn;
    unsigned char encrypt_flag;
    unsigned short cmd;
    unsigned int data_size;
}req_head;


typedef enum PIPE_TYPE {
    PIPE_TCP = 0,
    PIPE_UDP,
    PIPE_UI,
    PIPE_EVENT,
}PIPE_TYPE;


enum MSG_TYPE{
	/* pipe */
	CONVERT_MODE_PIPE = 1,
	CLOSE_ALL_CLIENT_PIPE,
	EXIT_PIPE,

	/* socket */
	LOGIN_MSG,
	OPTIONS_MSG,
	PLAY_MSG,
	CONTROL_MSG,
	DONE_MSG,
	
	/* control */
	MOUSE_MSG,
	KEYBD_MSG,
	FILE_MSG,
};


struct request
{
	int code;
	union
	{
		unsigned char msg[36];
		unsigned int type;
	}
};

#if 0
typedef struct rtp_format
{
    unsigned int width;
    unsigned int height;

    unsigned int video_code;
    unsigned int video_port;
	
	unsigned int audio_code;
	unsigned int audio_port;

    unsigned int control_port;
    //unsigned char control_flag;            // 0 stop 1 play  2 control
	unsigned char model;
    unsigned char fps;
	uint16_t quality;

    unsigned int bps;
}rtp_format;
#endif

#define H264 1
#define PCM 2

#endif
