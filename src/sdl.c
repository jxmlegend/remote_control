#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#ifndef _WIN32
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>      /* BitmapOpenFailed, etc.    */
    #include <X11/cursorfont.h> /* pre-defined crusor shapes */
    #include <linux/input.h>

    Display *dpy = NULL;
#endif

#include "base.h"
#include "frame.h"
#include "control.h"
#include "client.h"

int screen_width  = 0;
int screen_height = 0;
int vids_width = 0;
int vids_height = 0;
int max_conn = 0;

static int default_width  = 1600;
static int default_height = 900;
static int audio_disable = 1;
static int video_disable = 0;
static int display_disable = 0;
static int borderless = 0;

//static int exit_on_keydown;
//static int exit_on_mousedown;

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_RendererInfo renderer_info = {0};
//static SDL_AudioDeviceID audio_dev;
static SDL_Texture *texture = NULL;
static SDL_Texture *full_texture = NULL;
static SDL_Rect full_rect;
static SDL_Rect rect;
static SDL_Texture *ttf_texture = NULL;
static TTF_Font *font = NULL;

static int area_id = -1;

SDL_mutex *renderer_mutex;
//pthread_mutex_t renderer_mutex = PTHREAD_MUTEX_INITIALIZER;
SDL_cond *cond;
extern struct client *control_cli;
int control_flag = 0;

FrameQueue frame_queue;

#define EVENT_QUIT    (SDL_USEREVENT + 2)

static int do_exit()
{

}

int destory_SDL()
{
    if(font)
        TTF_CloseFont(font);

    //if(window)
     //   SDL_DestroyWindow(window);

    if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
    {    
        DEBUG("SDL_WasInit");
        SDL_Quit();
        TTF_Quit();
    }
	close_X11();
}

int init_X11()
{
#ifndef _WIN32
    if((dpy = XOpenDisplay(0)) == NULL)
    {   
        DEBUG("XOpenDisplay error");
        return ERROR;
    }
#endif
    return SUCCESS;
}

void close_X11()
{
#ifndef _WIN32
    if(dpy)
        XCloseDisplay(dpy);
    dpy = NULL;
#endif
}

int get_screen_size(int *temp_w, int *temp_h)
{
#ifdef _WIN32
    //SDL_GetWindowSize(window, temp_w, temp_h);
    *temp_w = GetSystemMetrics (SM_CXSCREEN) ;  // wide
    *temp_h = GetSystemMetrics (SM_CYSCREEN) ;  // high
    return SUCCESS;
#else
    int id;
    Window root;

    if(!dpy)
        return ERROR;

    id = DefaultScreen(dpy);
    if(!(root = XRootWindow(dpy, id)))
    {
        DEBUG("get XRootWindow id: %d error", id);
        if(dpy)
            XCloseDisplay(dpy);
        return ERROR;
    }

    *temp_w = DisplayWidth(dpy, id);
    *temp_h = DisplayHeight(dpy, id);

    if(!(*temp_w) || !(*temp_h))
        return ERROR;
    else
        return SUCCESS;
#endif
}

/* 获取鼠标点击区域 */
static int get_area(int x, int y)
{
    return ((x/vids_width) + (y/vids_height) * window_size);
}

void sdl_text_show(char *buf, SDL_Rect *rect)
{   
    SDL_Color color = {255, 255, 255};
    /*
     * Solid  渲染的最快，但效果最差，文字不平滑，是单色文字，不带边框。
  　 * Shaded 比Solid渲染的慢，但显示效果好于Solid，带阴影。
　　 * Blend 渲染最慢，但显示效果最好。
     */
    SDL_Surface *surface = TTF_RenderUTF8_Solid(font, buf, color);

	SDL_LockMutex(renderer_mutex);
    //if(ttf_texture)
    //   SDL_DestroyTexture(ttf_texture);
    //ttf_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect ttf_rect;
    ttf_rect.x = rect->x + (vids_width / 2) - (surface->w / 2);
    ttf_rect.y = rect->y + vids_height - (surface->h * 1.2);
    ttf_rect.w = surface->w;
    ttf_rect.h = surface->h;
    
    SDL_BlitSurface(surface, NULL, window, &ttf_rect);
    //SDL_RenderCopy(renderer, ttf_texture, NULL, &ttf_rect);
    //SDL_RenderPresent(renderer);
	SDL_FreeSurface(surface);
	SDL_UnlockMutex(renderer_mutex);
}

void sdl_text_clear()
{
	clear_texture();
}

/* 被动刷新 */
void refresh_texture(double *remaining_time)
{
    Frame *vp = NULL;
    if(frame_queue_nb_remaining(&frame_queue) > 0)
    {
        vp = frame_queue_peek(&frame_queue);

        if(!vp)
            return;

    	if(!texture || !renderer)
        	return;

        SDL_UpdateTexture(texture, NULL, vp->frame->data[0], vp->frame->linesize[0]);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_RenderPresent(renderer);

        frame_queue_pop(&frame_queue);
    }
}

/* 主动刷新 */
void update_texture(AVFrame *frame_yuv, SDL_Rect *rect)
{
    if(!texture || !renderer || !full_texture || !frame_yuv)
        return;
	SDL_LockMutex(renderer_mutex);
    if(!rect)                               //全屏更新
    {    
        SDL_UpdateYUVTexture(full_texture, NULL,
        frame_yuv->data[0], frame_yuv->linesize[0],
        frame_yuv->data[1], frame_yuv->linesize[1],
        frame_yuv->data[2], frame_yuv->linesize[2]);

        SDL_RenderCopy(renderer, full_texture, NULL, &full_rect);
    }    
    else                                    //局部更新
    {    
        SDL_UpdateYUVTexture(texture, NULL,
            frame_yuv->data[0], frame_yuv->linesize[0],
            frame_yuv->data[1], frame_yuv->linesize[1],
            frame_yuv->data[2], frame_yuv->linesize[2]);
        SDL_RenderCopy(renderer, texture, NULL, rect);
    }    
    SDL_RenderPresent(renderer);
	SDL_UnlockMutex(renderer_mutex);
}

void clear_texture()
{
	SDL_LockMutex(renderer_mutex);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
	SDL_UnlockMutex(renderer_mutex);
}

static void refresh_loop_wait_event(SDL_Event *event)
{
	double remaining_time = 0.1;
	SDL_PumpEvents();
	while(!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
	{
		if(remaining_time > 0.0)
			usleep((int64_t)(remaining_time * 1000000.0));
		refresh_texture(&remaining_time);
		SDL_PumpEvents();
	}
}

int signal_sdl_event(int event_type, void *data)
{
    SDL_Event event;
    event.type = event_type;
    event.user.data1 = data;
    SDL_PushEvent(&event);
}

void sdl_window_quit()
{
	signal_sdl_event(EVENT_QUIT, NULL);
}

int close_window()
{

    if(texture)
        SDL_DestroyTexture(texture);
    if(full_texture)
        SDL_DestroyTexture(full_texture);

  	if(renderer)
        SDL_DestroyRenderer(renderer);


    if(window)
        SDL_DestroyWindow(window);

		//SDL_HideWindow(window);

	texture = NULL;
	full_texture = NULL;
	renderer = NULL;
	window = NULL;

	return SUCCESS;
}

static void send_control(char *data, int len, int cmd)
{
	if(!control_cli)
		return;		

    char *buf = (char *)malloc(len + HEAD_LEN + 1);
    if(!buf)
        return;

    char *tmp = &buf[HEAD_LEN];
    set_request_head(buf, 0, cmd, len);
    memcpy(tmp, data, len);

	send(control_cli->fd, buf, len + HEAD_LEN, 0);
		
    free(buf);
}

#define PLAY 4
#define CONTROL 5

void sdl_loop()
{
	SDL_Event event;
	//mouse_event mouse;
	//keybd_event keybd;

    rfb_pointevent point = {0};
    rfb_keyevent key = {0};
    short old_x = 0;
    short old_y = 0;

	int flags = 1;
	time_t last_time = current_time;
#if 0
	for(;;)
	{
		refresh_loop_wait_event(&event);
		switch(event.type)
		{
			case SDL_MOUSEWHEEL:
				mouse.wheel = event.wheel.y;
            	mouse.x = old_x;
            	mouse.y = old_y;
            	send_control((char *)&mouse, sizeof(mouse_event), MOUSE);
            	mouse.wheel = 0;
				break;	
			case SDL_MOUSEBUTTONDOWN:
				if(server_flag)
				{
					(void) time(&current_time);
					if((current_time - last_time) > 1)
					{
						area_id = -1;
						last_time = current_time;	
					}	
					if(area_id == get_area(event.motion.x, event.motion.y))
					{	
						convert_model(area_id, CONTROL);
					}
					area_id = get_area(event.motion.x, event.motion.y);
				}
				if(SDL_BUTTON_LEFT == event.button.button)
            	{
                	mouse.mask |= (1<<1);
            	}
            	if(SDL_BUTTON_RIGHT == event.button.button)
            	{
                	mouse.mask |= (1<<3);
            	}
				break;
			case SDL_MOUSEBUTTONUP:
            	if(SDL_BUTTON_LEFT == event.button.button)
            	{
                	point.mask |= (1<<2);
            	}
            	if(SDL_BUTTON_RIGHT == event.button.button)
            	{
                	point.mask |= (1<<4);
            	}
			case SDL_MOUSEMOTION:
				if(mouse.mask != 0)
				{
					mouse.x = event.motion.x;
					mouse.x = event.motion.x;
					mouse.x = event.motion.x;
				}	
				break;

			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_ESCAPE)
            	{
					DEBUG("convert_model 11111Z");
					convert_model(0, PLAY);
            	}
				keybd.key = event.key.keysym.sym;
				keybd.mod = event.key.keysym.mod;
				send_control((char *)&keybd, sizeof(keybd_event), KEYBOARD);
				break;
			case SDL_KEYUP:
				keybd.key = event.key.keysym.sym;
				keybd.mod = event.key.keysym.mod;
				keybd.down = 0;
				send_control((char *)&keybd, sizeof(keybd_event), KEYBOARD);
				break;
			case SDL_WINDOWEVENT:
				break;	
			case EVENT_QUIT:
				DEBUG("EVENT_QUIT ");
				goto run_out;
			case SDL_QUIT:
				stop_server();
				DEBUG("SDL_QUIT");
				goto run_out;
			default:
				break;
		}	
	}
#endif

	for(;;)
	{
        refresh_loop_wait_event(&event);
        /* 鼠标滚轮 */
        if(event.type == SDL_MOUSEWHEEL)
        {
            point.wheel = event.wheel.y;
            point.x = old_x;
            point.y = old_y;
            send_control((char *)&point, sizeof(rfb_pointevent), MOUSE_MSG);
            point.wheel = 0;
        }
        /* 鼠标按下 */
        if(event.type == SDL_MOUSEBUTTONDOWN)
        {
			if(!control_flag)
			{
            	(void) time(&current_time);
	            if((current_time - last_time) > 1)
	            {
	                area_id = -1;
	                last_time = current_time;
	            }
	
	            if(area_id == get_area(event.motion.x, event.motion.y))
	            {
#if 0
					if(rtsp[area_id].cli && rtsp[area_id].is_running)
					{
						//rtsp[area_id].cli->status = CONTROL;
						//send_pipe();	
					}
#endif
				}
	            area_id = get_area(event.motion.x, event.motion.y);
	            DEBUG("area_id %d", area_id);
			}
            
            if(SDL_BUTTON_LEFT == event.button.button)
            {
                point.mask |= (1<<1);
            }
            if(SDL_BUTTON_RIGHT == event.button.button)
            {
                point.mask |= (1<<3);
            }
        }
        if(event.type == SDL_MOUSEBUTTONUP)
        {
            if(SDL_BUTTON_LEFT == event.button.button)
            {
                point.mask |= (1<<2);
            }
            if(SDL_BUTTON_RIGHT == event.button.button)
            {
                point.mask |= (1<<4);
            }
        }
        if(point.mask != 0 || event.type == SDL_MOUSEMOTION)
        {
            point.x = event.motion.x;
            point.y = event.motion.y;
            old_x = point.x;
            old_y = point.y;
            send_control((char *)&point, sizeof(rfb_pointevent), MOUSE_MSG);
            point.mask = 0;
        }
        if(SDL_KEYDOWN == event.type)
        {
            key.key = event.key.keysym.sym; //*(SDL_GetKeyName(event.key.keysym.sym));
            key.mod = event.key.keysym.mod; //*(SDL_GetKeyName(event.key.keysym.sym));
            key.down = 1;
            send_control((char *)&key, sizeof(rfb_keyevent), KEYBD_MSG);
            if(event.key.keysym.sym == SDLK_ESCAPE)
            {
				//convert_model(area_id, PLAY);
				//DEBUG("send_pipe ");	
    			char buf[HEAD_LEN];
    			send_pipe(buf, EXIT_PIPE, 0, PIPE_TCP);
            }
        }
        if(SDL_KEYUP == event.type)
        {
            key.key = event.key.keysym.sym;
            key.mod = event.key.keysym.mod;
            key.down = 0;
            send_control((char *)&key, sizeof(rfb_keyevent), KEYBD_MSG);
        }

		if(EVENT_QUIT == event.type)
				goto run_out;
		if(SDL_QUIT == event.type)
				goto run_out;
	}
run_out:
	destory_thread_tcp();	
	free_rtspd();		
}

void get_window_size(int *width, int *height)
{
	get_screen_size(width, height);
	if(window_flag)
	{
        *width = *width / 3 *2;
        *height = *height / 3 * 2;
	}
}


int create_window()
{
	int flags = SDL_WINDOW_SHOWN;
	if(borderless)
		flags |= SDL_WINDOW_BORDERLESS;
	else
		flags |= SDL_WINDOW_RESIZABLE;

#ifdef _WIN32
   	if(hwnd && !window)
    	window = SDL_CreateWindowFrom(hwnd);
#endif
    if(!window)
       	window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                      	default_width, default_height, flags);

   	SDL_GetDisplayBounds(0, &full_rect);

    if(!window_flag)
    {
        screen_width = full_rect.w;
        screen_height = full_rect.h;
        full_rect.x = 0;
        full_rect.y = 0;
        //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        SDL_SetWindowPosition(window, 0, 0);
    }
    else
    {
        SDL_SetWindowPosition(window, full_rect.w / 6, full_rect.h /6);

        screen_width = full_rect.w / 3 *2;
        screen_height = full_rect.h / 3 * 2;

        full_rect.w = screen_width;
        full_rect.h = screen_height;
        full_rect.x = 0;
        full_rect.y = 0;
    }

    vids_width = screen_width / window_size;
    vids_height = screen_height / window_size;

	SDL_SetWindowSize(window, screen_width, screen_height);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    if(window)
    {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if(!renderer)
        {
            DEBUG("Failed to initialize a hardware accelerated renderer: %s", SDL_GetError());
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE); // SDL_RENDERER_SOFTWARE
        }
        if(renderer)
        {
            if (!SDL_GetRendererInfo(renderer, &renderer_info))
                DEBUG("Initialized %s renderer.", renderer_info.name);
        }
    }
    if(!window || !renderer || !renderer_info.num_texture_formats)
    {
        DEBUG("Failed to create window or renderer: %s", SDL_GetError());
		close_window();
		return ERROR;
    }

    /* 局部画板 */
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, vids_width, vids_height);

    /* 全局画板 */
    full_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    if(!full_texture || !texture)
    {
        DEBUG("create full texture err");
		return ERROR;
    }

	SDL_ShowWindow(window);
	clear_texture();
	sdl_loop();
	return SUCCESS;
}




int init_SDL()
{
	int ret;
	int flags;

	init_X11();
	get_screen_size(&screen_width, &screen_height);

	renderer_mutex = SDL_CreateMutex();
	if(!renderer_mutex)
	{
		DEBUG("SDL_CreateMutex() %s error", SDL_GetError());
		return ERROR;
	}

	cond = SDL_CreateCond();
	if(!cond)
	{
		DEBUG("SDL_CreateCond() %s error", SDL_GetError());
		return ERROR;
	}

    flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
    if(audio_disable)
        flags &= ~SDL_INIT_AUDIO;
    else
    {
        /* Try to work around an occasional ALSA buffer underflow issue when the
         * period size is NPOT due to ALSA resampling by forcing the buffer size. */
        if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"))
            SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE","1", 1);
    }

    if(display_disable)
        flags &= ~SDL_INIT_VIDEO;

    if(SDL_Init(flags))
    {    
        DEBUG("Could not initialize SDL - %s", SDL_GetError());
		return ERROR;
    }    
    if(TTF_Init() == -1)
    {    
        DEBUG("SDL_ttf Init error: %s", SDL_GetError());
		return ERROR;
    }    
    font = TTF_OpenFont(TTF_DIR, 16);
    if(!font)
    {    
        DEBUG("SDL_ttf open ttf dir: %s error", TTF_DIR);
    }    
    SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
    SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	return SUCCESS;
}



void *thread_sdl(void *param)
{
    int ret;
    pthread_attr_t st_attr;
    struct sched_param sched;

    //pthread_detach(pthread_self());
    ret = pthread_attr_init(&st_attr);
    if(ret)
    {   
        DEBUG("thread sdl attr init warning ");
    }   
    ret = pthread_attr_setschedpolicy(&st_attr, SCHED_FIFO);
    if(ret)
    {   
        DEBUG("thread sdl set SCHED_FIFO warning");
    }   
    sched.sched_priority = SCHED_PRIORITY_DECODE;
    ret = pthread_attr_setschedparam(&st_attr, &sched);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //线程可以被取消掉
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//立即退出
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);//立即退出  PTHREAD_CANCEL_DEFERRED 

	ret = create_window();
	close_window();	
	return (void *)&ret;
}

