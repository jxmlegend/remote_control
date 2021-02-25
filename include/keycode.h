#ifndef __KEYCODE_H__
#define __KEYCODE_H__

#include "rbtree.h"

struct keycode{
        //struct rb_node rb_node;    
    	uint32_t key;
		uint32_t value;
}; 


extern struct keycode keybd_key[];
#endif //__KEYCODE_H__



