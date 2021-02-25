#ifndef _WIN32
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>      /* BitmapOpenFailed, etc.    */
    #include <X11/cursorfont.h> /* pre-defined crusor shapes */
    #include <linux/input.h>
#endif
#include <SDL2/SDL.h>

#include "base.h"
#include "control.h"
#include "keycode.h"

#ifdef _WIN32
void simulate_mouse(rfb_pointevent *point)
{
    DWORD flags = MOUSEEVENTF_ABSOLUTE;
    DWORD wheel_movement = 0;
    flags |= MOUSEEVENTF_MOVE;

    if(point->mask & MOUSE_LEFT_DOWN)
    {
        flags |= MOUSEEVENTF_LEFTDOWN;
    }
    if(point->mask & MOUSE_LEFT_UP)
    {
        flags |= MOUSEEVENTF_LEFTUP;
    }
    if(point->mask & MOUSE_RIGHT_DOWN)
    {
        flags |= MOUSEEVENTF_RIGHTDOWN;
    }
    if(point->mask & MOUSE_RIGHT_UP)
    {
        flags |= MOUSEEVENTF_RIGHTUP;
    }
    if(point->wheel)
    {
        flags |= MOUSEEVENTF_WHEEL;
        if(point->wheel > 0)
            wheel_movement = (DWORD)+120;
        else
            wheel_movement = (DWORD)-120;
    }

    /* 计算相对位置 */
    unsigned long x = (point->x * 65535) / (vids_width )  * (screen_width / screen_width);
    unsigned long y = (point->y * 65535) / (vids_height ) * (screen_height /screen_height);
    //DEBUG("x %ld y %ld ", x, y);

    mouse_event(flags, (DWORD)x, (DWORD)y, wheel_movement, 0);
	
}

void simulate_keyboard(rfb_keyevent *key)
{
	struct keycode *code = get_keycode(key->key); 

	if(code)
	{
		if(key->down)
        	keybd_event(key->key, key->scan_code, 0,0);
    	else
        	keybd_event(key->key, key->scan_code, KEYEVENTF_KEYUP,0);
	}
}
#else

extern Display *dpy;

void simulate_mouse(rfb_pointevent *point)
{
	unsigned long x = point->x / (float)vids_width * screen_width;
    unsigned long y = point->y / (float)vids_height * screen_height;

	if(!dpy)
		return;

    XTestFakeMotionEvent(dpy, 0, x, y, 0L);
    int button = 0;
    int down = 0;

    if(point->mask & MOUSE_LEFT_DOWN)
    {   
        button = 1;
        down = 1;
    }   
    if(point->mask & MOUSE_LEFT_UP)
    {   
        button = 1;
        down = 0;
    }   
    if(point->mask & MOUSE_RIGHT_DOWN)
    {   
        button = 3;
        down = 1;
    }   
    if(point->mask & MOUSE_RIGHT_UP)
    {   
        button = 3;
        down = 0;
    }   
    if(point->mask)
        XTestFakeButtonEvent(dpy, button, down, 0L);

    XFlush(dpy);
}

void simulate_keyboard(rfb_keyevent *key)
{
	int ret;

	if(!dpy)
		return;
		
    struct keycode *code = get_keycode(key->key); 

	if(code)
	{
		XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, code->value), key->down, 0L);
    	XFlush(dpy);    
	}
}
#endif

int init_simulate()
{
	int i, ret;
	for (i = 0; keybd_key[i].value != -1; i++)
	{
		ret = set_keycode(keybd_key[i].key, &keybd_key[i]);
		if(ret != SUCCESS)
		{
			DEBUG("ret %d hash map put error i %d keybd_key[i].sdl_key %d keybd_key[i].simulate_key %d", ret ,i, 
					keybd_key[i].key,keybd_key[i].value);
			return ERROR;
		}
	}
	return SUCCESS;
}
