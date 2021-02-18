#include "base.h"
#include "rtp.h"

uint64_t get_random_seq()
{
	uint64_t seed;
	srand((unsigned)time(NULL));
	seed = 1 + (uint32_t)(rand() % (0XFFFF));
	
	return seed;
}

uint64_t get_random_timestamp()
{
	uint64_t seed;
	srand((unsigned)time(NULL));
	seed = 1 + (uint32_t)(rand() % (0XFFFF));
	
	return seed;
}

uint64_t get_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	return ((uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec);
}

int abstr_nalu_indic()
{

//	return frame_size;
}


void build_rtp_header(RTP_header *r, int cur_conn_num, int dt)
{
	r->version = 2;
    r->padding = 0;
    r->extension = 0;
    r->csrc_len = 0;
    r->marker = 0;
    r->payload = dt; 
#if 0
    r->seq_no = htons(rtsp[cur_conn_num]->cmd_port.seq);
    rtsp[cur_conn_num]->cmd_port.timestamp += rtsp[cur_conn_num]->cmd_port.frame_rate_step;
    r->timestamp = htonl(rtsp[cur_conn_num]->cmd_port.timestamp);
    r->ssrc = htonl(rtsp[cur_conn_num]->cmd_port.ssrc);
#endif
}

/********************************************************************************
 * RTP Packet:
 * 1. NALU length small than 1460-sizeof(RTP header):
 *	(RTP Header) + (NALU without Start Code)
 * 2. NALU length larger the MTU
 *  (RTP Header) + (FU Indicator) + (FU Header) + (NALU Slice)
 *  			 + (FU Indicator) + (FU Header) + (NALU Slice)
 *				 + ....
 *
 *	inbuffer -- NALU: 00 00 00 01    1 Byte       XX XX XX
 *				    |Start Code |  |NALU Header| |NALU Data|
 *				    
 *  NALU Slice : Cut NALU Data into Slice.
 *  
 *	NALU Header : F|NRI|TYPE
 *				  F: 1 bit
 *				  NRI: 2 bit
 *				  Type: 5 bit
 *
 *	FU Indicator : Like NALU Header, Type in FU-A(28)
 *
 *  FU Header : S|E|R|Type
 *  			S : 1bit , Start, First Slice should set
 *  			E : 1bit , End, Last Slice should set
 *  			R : 1bit , Reserved
 *  			Type: 5bit, Same with NALU Header's Type
 * ***********************************************************************/
int build_rtp_nalu(uint8_t *buf, int frame_size)
{

	/* 计算时间戳 */

	RTP_header rtp_header;

	//rtp_header 
	
	//build_rtp_head(&rtp_header, 1, H264);



#if 0
	int time_delay;
	
	uint8_t nalu;
	uint8_t fu_indic;
	uint8_t fu_header;
	uint8_t *nalu_data;
	uint8_t *nalu_buf;
	

	if(data_left <= )
	{
		//rtp_header.seq_no = htons(seq++);
		rtp_header.marker = 1;
		//memcpy(nalu, &rtp_header,);
		//memcpy(nalu, data);
		//send();
		return 0;
	}
	
	/* FU-A RTP Packet */

	fu_indic = (nalu_header & 0xE0) | 28;

	int rtp_size = 0;
	while()
	{
		//int pcor
		rtp_size = proc_size + RTP_HEADER_SIZE + FU_A_HEAD_SIZE + FU_A_INDI_SIZE;
	
		fu_end = ();
		fu_header = nalu_header & 0x1F;
		
		if(fu_start)
			fu_header |= 0x80;
		else if(fu_end)
			fu_header |= 0x40;
	
		rtp_header.seq_no = htons(rtsp[cur_conn_num]->cmd_port.seq ++);	
		
		memcpy(nalu_buffer, &rtp_header, sizeof(rtp_header));
		memcpy(nalu_buffer + 14, p_nalu_data, proc_size);
		nalu_buffer[12] = fu_indic;
        nalu_buffer[13] = fu_header;
	
		send();

	
	        data_left -= proc_size; 
        p_nalu_data += proc_size;
        fu_start = 0;

	}
#endif
}

int rtp_send_from_file()
{
	FILE *fp;
	uint64_t file_size;
	int is_running = 1;
	
	fp = fopen("./1.h264", "rb");
	if(fp == NULL)
	{
		DEBUG("open h264 file error");
		return ERROR;
	}

	//file_size = get_file_size(fp);
	if(file_size <= 4)
	{
		DEBUG("h264 file size error %llu ", file_size);
		fclose(fp);
		return ERROR;
	}	
	
	while(is_running)
	{
		//bytes_left = fread(fp, 1, READ_LEN
	}	
}

int rtp_send_from_stream()
{
	
}

int rtp_send_packet()
{
	/* file */
	rtp_send_from_file();
	/* stream */
	rtp_send_from_stream();
}


