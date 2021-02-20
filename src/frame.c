#include <SDL2/SDL.h>
#include "base.h"
#include "frame.h"


/* 获取一个可写的Frame写入 */
static Frame *frame_queue_peek_writable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while(f->size >= f->max_size)               //没有可写入的
    {
        SDL_CondWait(f->cond, f->mutex);        //信号阻塞等待
    }

    SDL_UnlockMutex(f->mutex);

    return &f->queue[f->windex];
}


static void frame_queue_push(FrameQueue *f)
{
    if(++f->windex == f->max_size)
        f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

static void frame_queue_unref_item(Frame *vp)
{
    /* 重置AVFrame */
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

int frame_queue_init(FrameQueue *f, int max_size)
{
    int i = 0;;
    memset(f, 0, sizeof(FrameQueue));
    if (!(f->mutex = SDL_CreateMutex()))
    {
        DEBUG("SDL_CreateMutex(): %s", SDL_GetError());
        return -1;
    }

    if(!(f->cond = SDL_CreateCond()))
    {
        DEBUG("SDL_CreateMutex(): %s", SDL_GetError());
        return -1;
    }
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    for(i = 0; i < f->max_size; i++)
    {
        if (!(f->queue[i].frame = av_frame_alloc()))
        {
            return -1;
        }
    }
    return 0;
}

void frame_queue_destroy(FrameQueue *f)
{
    int i = 0;
    for(i = 0; i < f->max_size; i++)
    {
        Frame *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

static void frame_queue_signal(FrameQueue *f)
{


}

/* 出当前的。因为f->rindex + f->rindex_shown可能会超过max_size,所以用了取余 */
Frame *frame_queue_peek(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

/* 用于在显示之前。判断队列中，是否还存在下一个，队列阻塞会导致*/
static Frame *frame_queue_peek_next(FrameQueue *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

/* 不阻塞读取数据 */
static Frame *frame_queue_peek_last(FrameQueue *f)
{
    return &f->queue[f->rindex];
}

/* 获取一个可读的Frame读入 */
static Frame *frame_queue_peek_readable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while(f->size - f->rindex_shown <= 0)       //没有可读入的
    {
        SDL_CondWait(f->cond, f->mutex);        //信号阻塞等待
    }

    SDL_UnlockMutex(f->mutex);

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

void frame_queue_pop(FrameQueue *f)
{
    if(!f->rindex_shown)
    {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if(++f->rindex == f->max_size)
        f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);

}

/* 返回还未显示的数量 */
int frame_queue_nb_remaining(FrameQueue *f)
{
    return f->size - f->rindex_shown;
}
