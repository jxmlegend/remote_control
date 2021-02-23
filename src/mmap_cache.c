#include "base.h"
#include "mmap_cache.h"


#ifndef __WIN32
int mmap_list_entries_used = 0;
int mmap_list_total_requests = 0;
int mmap_list_hash_bounces = 00;

/* define local table variable */
static struct mmap_entry mmap_list[MMAP_LIST_SIZE];

struct mmap_entry *find_mmap(int data_fd, struct stat *s)
{
	char *m;
	int i, start;
	mmap_list_total_requests ++;
	i = start = MMAP_LIST_HASH(s->st_dev, s->st_ino, s->st_size);
	for(; mmap_list[i].use_count;)
	{
		if(mmap_list[i].dev == s->st_dev &&
			mmap_list[i].ino == s->st_ino &&
			mmap_list[i].len == s->st_size )
		{

			DEBUG("Old mmap_list entry %d use_count now %d hash was %d",
				i, mmap_list[i].use_count, start);	
		
			return mmap_list + i;
		}
	
		mmap_list_hash_bounces ++;
		i = MMAP_LIST_NEXT(i);
	
		/* Shouldn't happen, because of size limit enforcement below */
		if(i == start)
			return NULL;
	}

    /* didn't find an entry that matches our dev/inode/size.
       There might be an entry that matches later in the table,
       but that _should_ be rare.  The worst case is that we
       needlessly mmap() a file that is already mmap'd, but we
       did that all the time before this code was written,
       so it shouldn't be _too_ bad.
     */

    /* Enforce a size limit here */
	if(mmap_list_entries_used > MMAP_LIST_USE_MAX)
		return NULL;
	
	m = mmap(0, s->st_size, PROT_READ, MAP_FILE|MAP_PRIVATE, data_fd, 0);
	
	if((int)m == -1)
	{
		DEBUG("mmap fd %d error", data_fd);
		return NULL;
	}	

	//DEBUG("New mmap_list entry %d hash was %d", i, h);
	
	mmap_list_entries_used ++;
	mmap_list[i].dev = s->st_dev;
	mmap_list[i].ino = s->st_ino;
	mmap_list[i].len = s->st_size;
	mmap_list[i].mmap = m;
	mmap_list[i].use_count = 1;
	
	return mmap_list + i;
}

void release_mmap(struct mmap_entry *e)
{
	if(!e)
		return;
	
	if(!e->use_count)
	{
		DEBUG("mmap_lsit(%p)->use_count already zero", e);
		return;
	}

	if(!--(e->use_count))
	{
		munmap(e->mmap, e->len);
		mmap_list_entries_used --;
	}
}

struct mmap_entry *find_name_mmap(char *fname)
{
	int data_fd;
	struct stat st;
	struct mmap_entry *e;
	
	data_fd = open(fname, O_RDONLY);
	if(data_fd == -1)
	{
		DEBUG("open file %s error", fname);
		return NULL;
	}	
	
	fstat(data_fd, &st);
	if(S_ISDIR(st.st_mode))
	{
		DEBUG("%s is a directory", fname);
		return NULL;
	}
	e = find_mmap(data_fd, &st);	
	close(data_fd);
	return e;	
}
#endif

//#define MMAP_TEST
#ifdef MMAP_TEST
int main(int argc, char *argv)
{
	struct mmap_entry *entry = find_name_mmap("err.log");
	struct mmap_entry *entry_1 = find_name_mmap("err.log");
	
	DEBUG("entry->mmap %s", entry->mmap);	
	DEBUG("entry->mmap %d", entry_1->len);	

	

	release_mmap(entry);
	release_mmap(entry_1);
}
#endif
