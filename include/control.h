#ifndef __CONFIG_H__
#define __CONFIG_H__

/* control */
#define  MOUSE_LEFT_DOWN    1<<1
#define  MOUSE_LEFT_UP      1<<2

#define  MOUSE_RIGHT_DOWN   1<<3
#define  MOUSE_RIGHT_UP     1<<4

#define MOUSE_WHEEL_DOWN    1<<5
#define MOUSE_WHEEL_UP      1<<6


enum control_msg_type{
    MOUSE = 0x03,
    KEYBOARD,
    COPY_TEXT,
    COPY_FILE
};

typedef struct _rfb_filemsg
{
    char flags;
    short datasize;
    char path[128];
}rfb_filemsg;

typedef struct _rfb_textmsg
{
    short pad1;
    short pad2;
    int length;
}rfb_textmsg;

typedef struct _rfb_key_event
{
    char down;
    int key;            //SDL keycode
    int scan_code;
    unsigned short mod;         //组合键
}rfb_keyevent;

typedef struct _rfb_pointer_evnet
{
    short mask;
    short x;
    short y;
    short wheel;
}rfb_pointevent;

#endif


