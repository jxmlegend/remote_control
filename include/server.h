#ifndef __SERVER_H__
#define __SERVER_H__


typedef enum client_state
{
    NORMAL = 0,
    LOGIN,
    OPTIONS,
    READY,
    PLAY,
    CONTROL,
    DONE,
    DEAD,
}client_state;


#endif //__SERVER_H__
