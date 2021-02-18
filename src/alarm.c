#include "base.h"
static int die_if_timeout = 0;

void sig_alarm(int sig)
{
	DEBUG("alarm handler time out");
}

void set_alarm(int timeout)
{
	if(timeout)
	{
		alarm(timeout);
		die_if_timeout = 0;
	}
}


//#define ALARM_TEST

#ifdef ALARM_TEST
int main(int argc, char *argv[])
{
	signal(SIGALRM, sig_alarm);
		
	set_alarm(1);
	sleep(3);

	DEBUG("end");	
	return 0;
}
#endif

