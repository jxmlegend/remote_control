#ifndef __GLOBAL_H__
#define __GLOBAL_H__

extern time_t current_time;
extern int client_port;
extern int window_size;
extern int max_connections;

void *thread_sdl(void *param);


extern int pipe_tcp[2];
extern int pipe_udp[2];
extern int pipe_event[2];
extern int pipe_ui[2];

extern int screen_width;
extern int screen_height;
extern int vids_width;
extern int vids_height;

extern int max_conn;



#endif //__GLOBAL_H__
