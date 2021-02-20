#include "base.h"

sigjmp_buf env;
int handle_sighus;

//void sigsegv(int);

static void sig_quit_listen(int e)
{
    char s = 'S';
	char buf[HEAD_LEN];	
	
	send_pipe(buf, EXIT_PIPE, 0, PIPE_TCP);
    DEBUG("recv sig stop programe !!");
}


void init_signals()
{
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
    //signal(SIGINT, SIG_IGN);

    struct sigaction act;
    act.sa_handler = sig_quit_listen;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, 0); 
#endif
}


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


