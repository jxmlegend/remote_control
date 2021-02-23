#ifndef __RTP_H__
#define __RTP_H__


/*
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|            contributing source (CSRC) identifiers             |
|                             ....                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

#define RTP_SIZE_MAX    1460
#define RTP_HEADER_SIZE 12
#define NALU_INDIC_SIZE 4
#define NALU_HEAD_SIZE  1
#define FU_A_INDI_SIZE  1
#define FU_A_HEAD_SIZE  1

/* SINGLE_NALU_DATA_MAX = RTP_SIZE_MAX - RTP_HEADER_SIZE*/
#define SINGLE_NALU_DATA_MAX  1448
    
/* SLICE_NALU_DATA_MAX = RTP_SIZE_MAX - RTP_HEADER_SIZE - FU_A_INDI_SIZE
       - FU_A_HEAD_SIZE */
#define SLICE_NALU_DATA_MAX   1446
    
#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )
    
#define READ_LEN 150000
#define SLICE_SIZE 1448
#define SLICE_FSIZE 1435
#define DE_TIME 3600

typedef struct _RTP_header {
    /* byte 0 */
    #ifdef WORDS_BIGENDIAN
        uint8_t version:2;
        uint8_t padding:1;
        uint8_t extension:1;
        uint8_t csrc_len:4;
    #else
        uint8_t csrc_len:4;
        uint8_t extension:1;
        uint8_t padding:1;
        uint8_t version:2;
    #endif
    /* byte 1 */
    #if WORDS_BIGENDIAN
        uint8_t marker:1;
        uint8_t payload:7;
    #else
        uint8_t payload:7;
        uint8_t marker:1;
    #endif
    /* bytes 2, 3 */
    uint16_t seq_no;
    /* bytes 4-7 */
    uint32_t timestamp;
    /* bytes 8-11 */
    uint32_t ssrc;                   /* stream number is used here. */
} RTP_header;


typedef struct _NALU_header{
	uint8_t TYPE:5;
	uint8_t NRI:2;
	uint8_t F:1;
}NALU_header;

typedef struct _FU_indicator{
	uint8_t TYPE:5;
	uint8_t NRI:2;
	uint8_t F:1;
}FU_indicator;		

typedef struct _FU_header{
	uint8_t TYPE:5;
	uint8_t R:1;
	uint8_t E:1;
	uint8_t S:1;
}FU_header;

typedef struct _nalu_t
{
	//int 
	uint32_t len;
}nalu_t;

#endif //__RTP_H__

