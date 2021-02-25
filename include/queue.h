#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stdio.h>
#include <string.h>

#define MAX_QUEUESIZE 128


typedef struct 
{
	uint32_t uiSize;
	uint8_t *pBuf;
	uint8_t ucType;
}QUEUE_INDEX;


typedef struct 
{
  uint32_t uiFront;
  uint32_t uiRear;
  uint32_t uiMaxBufSize;
  uint32_t uiBufOffset;
  uint8_t *pBuf;
  sem_t sem;

  QUEUE_INDEX stIndex[MAX_QUEUESIZE];
}QUEUE;

void init_queue(QUEUE *pQueue,unsigned char *pucBuf,unsigned int uiMaxBufSize);
QUEUE_INDEX * de_queue(QUEUE *pQueue);
unsigned char de_queuePos(QUEUE *pQueue);
unsigned char en_queue(QUEUE *pQueue,unsigned char *ucpData,unsigned int uiSize,unsigned char ucType);
unsigned char empty_queue(QUEUE *pQueue);
unsigned char full_queue(QUEUE *pQueue);

#endif // QUEUE_H

