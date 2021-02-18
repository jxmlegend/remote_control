#ifndef __GLOBAL_H__
#define __GLOBAL_H__


extern int client_port;
extern int window_size;
extern int max_connections;

void *thread_event(void *param);


extern int pipe_tcp[2];
extern int pipe_udp[2];
extern int pipe_event[2];
extern int pipe_ui[2];


#endif //__GLOBAL_H__
