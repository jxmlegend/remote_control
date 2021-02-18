#include "base.h"

sigjmp_buf env;
int handle_sighus;

//void sigsegv(int);


void init_sem()
{
	sem_t *sem;

	/* 0 O_CREAT, O_CREAT|O_EXCL */
	sem = sem_open("test", O_CREAT, 0644, 0);

	/* sem value == 0 阻塞 */
	sem_wait(sem);
	
	sem_trywait(sem);

	/* sem ++ */
	sem_post(sem);

	int val = 0;
	sem_getvalue(sem, &val);

	sem_close(sem);
	sem_unlink("test");

	DEBUG("val %d", val);
}



