#include <limits.h>
#include <ctype.h>
#ifndef _WIN32
#include <sys/un.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <linux/netlink.h>

#include <sys/socket.h>
#include <asm/types.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <time.h>


#define TIMEZONE_OFFSET(foo) foo->tm_gmtoff
#endif
//#include <linux/if.h>  
#include <assert.h>

#include "base.h"

const char month_tab[48] =
    "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";
const char day_tab[] = "Sun,Mon,Tue,Wed,Thu,Fri,Sat,";

int use_localtime = 0; //夏时令

char network_ip[10][128] = {0};
int interface_num = 0;

extern time_t current_time;

/*
 * Name: get_commonlog_time
 *
 * Description: Returns the current time in common log format in a static
 * char buffer.
 *
 * commonlog time is exactly 25 characters long
 * because this is only used in logging, we add " [" before and "] " after
 * making 29 characters
 * "[27/Feb/1998:20:20:04 +0000] "
 *
 * Constrast with rfc822 time:
 * "Sun, 06 Nov 1994 08:49:37 GMT"
 *
 * Altered 10 Jan 2000 by Jon Nelson ala Drew Streib for non UTC logging
 *
 */
char *get_commonlog_time(void)
{
    struct tm *t;
    char *p;
    unsigned int a;
    static char buf[30];
    int time_offset;

    (void)time(&current_time);

    if (use_localtime)
    {
        t = localtime(&current_time);
#ifndef _WIN32
        time_offset = TIMEZONE_OFFSET(t);
#endif
    }
    else
    {
        t = gmtime(&current_time);
        time_offset = 0;
    }

    p = buf + 29;
    *p-- = '\0';
    *p-- = ' ';
    *p-- = ']';
#ifndef _WIN32
    a = abs(time_offset / 60);
    *p-- = '0' + a % 10;
    a /= 10;
    *p-- = '0' + a % 6;
    a /= 6;
    *p-- = '0' + a % 10;
    *p-- = '0' + a / 10;
    *p-- = (time_offset >= 0) ? '+' : '-';
#endif
    *p-- = ' ';

    a = t->tm_sec;
    *p-- = '0' + a % 10;
    *p-- = '0' + a / 10;
    *p-- = ':';
    a = t->tm_min;
    *p-- = '0' + a % 10;
    *p-- = '0' + a / 10;
    *p-- = ':';
    a = t->tm_hour;
    *p-- = '0' + a % 10;
    *p-- = '0' + a / 10;
    *p-- = ':';
    a = 1900 + t->tm_year;
    while (a)
    {
        *p-- = '0' + a % 10;
        a /= 10;
    }
    /* p points to an unused spot */
    *p-- = '/';
    p -= 2;
    memcpy(p--, month_tab + 4 * (t->tm_mon), 3);
    *p-- = '/';
    a = t->tm_mday;
    *p-- = '0' + a % 10;
    *p-- = '0' + a / 10;
    *p = '[';
    return p; /* should be same as returning buf */
}

/* Return the UNIX time in microsends */
long long ustime(void)
{   
    struct timeval tv;
    long long ust;
    
    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec) * 1000000;
    ust += tv.tv_usec;
    return ust;
}

long long mstime(void)
{   
	return ustime() / 1000;
}



#if 0
void test()
{

  struct statfs diskInfo;

  if(0==statfs(TF_PATH, &diskInfo)) {  //查询内存卡剩余容量
    freeDisk = (unsigned long long)(diskInfo.f_bfree) * (unsigned long long)(diskInfo.f_bsize);
    mbFreedisk = (freeDisk >> 20);
    mbFreedisk*=10;
    mbFreedisk/=1024;

    totalDisk = (unsigned long long)(diskInfo.f_blocks) * (unsigned long long)(diskInfo.f_bsize);
    mbTotalsize = totalDisk >> 20;
    mbTotalsize*=10;
    mbTotalsize/=1024;
  }

}
#endif


uint64_t get_file_size(FILE *fp)
{
	uint64_t file_size;
	
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	
	fseek(fp, 0L, SEEK_SET);
	return file_size;
}

void exec_cmd(const char *cmd, char *result)
{
    char buf[MAX_BUFLEN];
    char tmp[MAX_BUFLEN] = {0};
    FILE *fp;
    strcpy(tmp, cmd);
    memset(result, 0, MAX_BUFLEN);
    if((fp = popen(tmp, "r")) != NULL)
    {   
        while(fgets(buf, MAX_BUFLEN, fp) != NULL)
        {   
            strcat(result, buf);
            if(strlen(result) > MAX_BUFLEN)
                break;
        }   
        pclose(fp); 
    }   
    else
    {   
        FAIL("popen cmd: %s", tmp);
        return ERROR;
    }   
}


