#include "base.h"
#include <stdarg.h>

static FILE *fp_err = NULL;
static FILE *fp_log = NULL;

extern time_t current_time;
static char c_dir[MAX_FILENAMELEN];

void init_logs()
{
    struct tm *t;
    struct stat file_state;
    int ret;

    (void)time(&current_time);
    t = localtime(&current_time);
    /* 检测文件夹是否存在 */
    ret = stat(LOG_DIR, &file_state);
    if (ret < 0)
    {
        if (errno == ENOENT)
        {
#ifdef _WIN32
            ret = mkdir(LOG_DIR);
#else
            ret = mkdir(LOG_DIR, 0755);
#endif
            if (ret < 0)
            {
                return ERROR;
            }
        }
    }
    sprintf(c_dir, "%s/%s-%4.4d%2.2d%2.2d.log", LOG_DIR, program_name, (1900 + t->tm_year), (1 + t->tm_mon), t->tm_mday);

    ret = stat(c_dir, &file_state);
    if (ret < 0)
    {
        if (errno == ENOENT)
        {
            fp_log = fopen(c_dir, "wb+");
        }
    }
    else
    {
        fp_log = fopen(c_dir, "ab+");
    }

    if (!fp_log)
    {
        DEBUG("fopen log error ");
    }

    fp_err = fopen(LOG_ERR_FILE, "ab");
    if (!fp_err)
    {
        DEBUG("fopen err_log error");
    }
}

void close_logs()
{
    if (fp_err)
        fclose(fp_err);

    if (fp_log)
        fclose(fp_log);

    fp_err = NULL;
    fp_log = NULL;
}

void log_msg(const char *fmt, ...)
{
    char buf[MAX_BUFLEN] = {0};
    char *ptr = buf;
    va_list ap;

    if (!fp_log)
        return;

    va_start(ap, fmt);
    vsprintf(ptr, fmt, ap);
    va_end(ap);

    fprintf(fp_log, "%s:%s", get_commonlog_time(), buf);
    fflush(fp_log);
}

void err_msg(const char *fmt, ...)
{
    char buf[MAX_BUFLEN] = {0};
    char *ptr = buf;
    va_list ap;

    if (!fp_err)
        return;

    va_start(ap, fmt);
    vsprintf(ptr, fmt, ap);
    va_end(ap);

    fprintf(fp_err, "%s:%s", get_commonlog_time(), buf);
    fflush(fp_err);
}

int upload_logs(char **buf, int *buf_len)
{
   	fseek(fp_log, 0, SEEK_END);
	*buf_len = ftell(fp_log);
	DEBUG("log file len %d", *buf_len);
    fseek(fp_log, 0, SEEK_SET);

	*buf = (char *)malloc(*buf_len + 1);
	if(!(*buf))
	{
		DEBUG("log malloc size: %d error: %s", *buf_len, strerror(errno));
		return ERROR;
	}	
	return fread(*buf, 1, *buf_len, fp_log);
}
