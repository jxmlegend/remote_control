#ifndef __MMAP_CACHE_H__
#define __MMAP_CACHE_H__


#define MMAP_LIST_SIZE 256
#define MMAP_LIST_MASK 255
#define MMAP_LIST_USE_MAX 128
#define MMAP_LIST_NEXT(i) (((i)+1)&MMAP_LIST_MASK)
#define MMAP_LIST_HASH(dev,ino,size) ((ino)&MMAP_LIST_MASK)

#define MAX_FILE_MMAP 100 * 1024 /* 100K */


struct mmap_entry
{
	dev_t dev;
	ino_t ino;
	char *mmap;
	int use_count;
	size_t len;
};



#endif //__MMAP_CACHE_H__
