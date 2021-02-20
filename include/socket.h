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
	EXIT_PIPE = 2,

	/* socket */
	LOGIN_MSG,
	OPTIONS_MSG,
	PLAY_MSG,
	CONTROL_MSG,
	DONE_MSG,
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

#endif
