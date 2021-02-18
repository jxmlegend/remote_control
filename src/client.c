#include "base.h"
#include "client.h"

static int server_s = -1;
static pthread_t pthread_tcp;

void *thread_ffmpeg_audio_encode(void *param);
void *thread_ffmpeg_video_encode(void *param);

static void exit_client()
{

}

static void *thread_tcp(void *param)
{
	int ret;
	pthread_t pthread_video, pthread_audio;

	ret = pthread_create(pthread_video, NULL, thread_ffmpeg_audio_encode, NULL);
	if(SUCCESS != ret)
	{
		return ERROR;
	}

	ret = pthread_create(pthread_audio, NULL, thread_ffmpeg_video_encode, NULL);
	if(SUCCESS != ret)
	{
		return ERROR;
	}
}

int init_client()
{
	int ret;

	server_s = create_tcp_client("", 9999);
	if(server_s == -1)
	{
		DEBUG("");
		return ERROR;
	}

	ret = pthread_create(pthread_tcp, NULL, thread_tcp, &server_s);
	if(SUCCESS != ret)
	{
		close_fd(server_s);
		return ERROR;
	}

	return SUCCESS;
}
