#ifndef __FRAME_H__
#define __FRAME_H__

#include <libavformat/avformat.h>

#define FRAME_QUEUE_SIZE 16

typedef struct Frame
{
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;
    double duration;
    int64_t pos;
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
}Frame;

typedef struct FrameQueue
{
    Frame queue[FRAME_QUEUE_SIZE];

    int rindex;
    int windex;

    int size;
    int max_size;
    //int keep_last;

    int rindex_shown;
    SDL_mutex *mutex;
    SDL_cond *cond;
} FrameQueue;


#endif
