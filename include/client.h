#ifndef __CLIENT_H__
#define __CLIENT_H__

//#include "rbtree.h"
#include "ffmpeg.h"

#define IPADDR_LEN 36

struct client
{
    unsigned int status;

    /* red black node */
    //struct rb_node rb_node;

    /* tcp sock fd */
    int fd; 
    char ip[IPADDR_LEN];
    int port;
        
    /** has read msg head or not ,0 :not 1: yes**/
    int has_read_head;

    unsigned char send_head[HEAD_LEN + 1]; 
    unsigned char recv_head[HEAD_LEN + 1]; 

    unsigned char *send_buf;
    unsigned char *recv_buf;
	
//	rtp_format fmt;

    unsigned int send_size;
    unsigned int recv_size;

	int chn;	

    /** current data position **/
    int pos;
    /** curreant data size **/
    int data_size;
    /** max alloc size **/
    int max_size;

    time_t last_time;
};


#endif //__CLIENT_H__ 

