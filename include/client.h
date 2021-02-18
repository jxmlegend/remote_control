#ifndef __CLIENT_H__
#define __CLIENT_H__

//#include "rbtree.h"

#define IPADDR_LEN 36

struct client
{
    unsigned int status;

    /* red black node */
    //struct rb_node rb_node;

    /* tcp sock fd */
    int fd; 
    char ip[IPADDR_LEN];
	//int ip;
    int port;
        
    unsigned char head_buf[HEAD_LEN + 1];
    /** has read msg head or not ,0 :not 1: yes**/
    int has_read_head ;

    unsigned char *data_buf;

	unsigned char rtsp_buf[DATA_SIZE];
	uint32_t rtsp_cseq;

    /** current data position **/
    int pos;
    /** curreant data size **/
    int data_size;
    /** max alloc size **/
    int max_size;

    time_t last_time;
};




#endif //__CLIENT_H__ 

