#include "base.h"
#include <SDL2/SDL.h>

typedef struct packet_head
{
	uint8_t flag;					// 0xFF
	int serial;						//帧序号
	uint8_t code;
	uint8_t key_frame;				//关键帧
	uint8_t packet_num;	
	uint16_t data_size;
	uint64_t total_size;
}packet_head;

typedef struct packet
{
	int64_t pts;
	int64_t dts;
	int size;
	int pos;	
	int serial;	
	uint8_t *data;
}packet;

typedef struct packet_list
{
	packet *pkt;	
	struct packet_list *next;
	int serial;	
}packet_list;


typedef struct packet_queue
{
	packet_list *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int64_t duration;
	int serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
}packet_queue;

static int init_clock(int *queue_serial)
{
	*queue_serial = 0;
}

packet_queue video_queue;
packet flush_pkt;

static int packet_queue_put_private(packet_queue *q, packet *pkt)
{
	packet_list *pkt_list;
	
	pkt_list = (packet_list *)malloc(sizeof(packet_list));

	if(!pkt_list)
		return ERROR;
	
	pkt_list->pkt = pkt;
	pkt_list->next = NULL;
	
	if(pkt == &flush_pkt)
		q->serial++;
	pkt_list->serial = q->serial;

	if(!q->last_pkt)
		q->first_pkt = pkt_list;
	else
		q->last_pkt->next = pkt_list;

	q->last_pkt = pkt_list;
	q->nb_packets++;
	q->size += pkt_list->pkt->size + sizeof(*pkt_list);

	//SDL_CondSignal(q->cond);	
	return 0;
}

static int packet_queue_put(packet_queue *q, packet *pkt)
{
	int ret;
	SDL_LockMutex(q->mutex);
	ret = packet_queue_put_private(q, pkt);
	SDL_UnlockMutex(q->mutex);
	
	if(pkt != &flush_pkt && ret < 0)
		;//packet_unref(pkt);
	return ret;	
}

/* 空白的packet 占位 */
static int packet_queue_put_nullpacket(packet_queue *q, int serial) 
{
	packet *pkt;
	pkt = (packet *)malloc(sizeof(packet));
	memset(pkt, 0, sizeof(*pkt));
	pkt->data = NULL;
	pkt->size = 0;
	pkt->serial = serial;
	
	return packet_queue_put(q, pkt);
}

static int packet_queue_init(packet_queue *q)
{
	memset(q, 0, sizeof(packet_queue));
	q->mutex = SDL_CreateMutex();
	if(!q->mutex)
	{
		DEBUG("SDL_CreateMutex() error :%s", SDL_GetError());
		return ERROR;
	}
	q->cond = SDL_CreateCond();
	if(!q->cond)
	{
		DEBUG("SDL_CreateCond() error :%s", SDL_GetError());
		return ERROR;
	}
	return 0;
}

static void packet_queue_flush(packet_queue *q)
{

}

static void packet_queue_destory(packet_queue *q)
{

}

static int packet_queue_get(packet_queue *q, packet **pkt, int block)
{
	int ret;
	packet_list *pkt_list;
	
	SDL_LockMutex(q->mutex);
	for(;;)
	{
		pkt_list = q->first_pkt;
		if(pkt_list)
		{
			q->first_pkt = pkt_list->next;
			if(!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets --;
			q->size -= pkt_list->pkt->size + sizeof(*pkt_list);
			*pkt = pkt_list->pkt;
			free(pkt_list);
			ret = 1;
			break;
		}
		else if(!block)
		{
			ret = 0;
			break;
		}
		else
		{
			SDL_CondWait(q->cond, q->mutex);
		}	
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

static int packet_queue_get_serial(packet_queue *q, packet **pkt, int serial)
{
	int ret;
	packet_list *pkt_list = q->first_pkt;
	SDL_LockMutex(q->mutex);
	for(;;)
	{
		if(pkt_list)
		{
			*pkt = pkt_list->pkt;
			if(serial == (*pkt)->serial)
			{
				break;
			}
			pkt_list = pkt_list->next;
		}
		else
		{
			*pkt = NULL;
			break;
		}	
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}



#if 0
int main(int argc, char *argv[])
{
#if 0
	packet_queue_init(&video_queue);
	packet_queue_put_nullpacket(&video_queue, 1);
	
	packet *pkt = NULL, *pkt_1;
	packet_queue_get_serial(&video_queue, &pkt, 1, 1);;
	
	DEBUG("pkt->serial %d", pkt->serial);
	pkt->size = 10;

	DEBUG("data %d", sizeof(pkt->data));
	pkt->data = malloc(1024 * 1400);
	
	DEBUG("data %d", sizeof(pkt->data));
	
	packet_queue_get(&video_queue, &pkt_1, 0);
	DEBUG("pkt->size %d", pkt_1->size);
	DEBUG("pkt->size %d %s", pkt_1->size, pkt_1->data);
#endif

#if 0
	int serial = 0;
	//init_clock();
	packet_queue *q;
	q = &video_queue;

	packet pkt3, *pkt, *pkt1, *pkt_1;
	pkt1 = &pkt3;

	pkt1->size = 1;
	pkt1->data = malloc(10);
	strcpy(pkt1->data, "Hello");
	pkt1->serial = 1;

	packet_queue_put(&video_queue, pkt1);

	packet pkt2;
	
	pkt1 = &pkt2;
	pkt1->size = 2;
	pkt1->serial = 2;

	pkt1->data = malloc(10);
	strcpy(pkt1->data, "World");

	packet_queue_put(&video_queue, pkt1);

	packet_queue_get_serial(&video_queue, &pkt, 2, NULL);
	DEBUG("pkt->size %d %s %d", pkt->size, pkt->data, pkt->serial);
	DEBUG("q->nb %d q->size %d q->serial %d", q->nb_packets, q->size, q->serial);

	packet_queue_get(&video_queue, &pkt_1, 0);
	DEBUG("pkt->size %d", pkt_1->size);
	DEBUG("pkt->size %d %s", pkt_1->size, pkt_1->data);

	packet_queue_get(&video_queue, &pkt_1, 0);
	DEBUG("pkt->size %d", pkt_1->size);
	DEBUG("pkt->size %d %s", pkt_1->size, pkt_1->data);

	DEBUG("q->nb %d q->size %d q->serial %d", q->nb_packets, q->size, q->serial);
#endif
}
#endif
