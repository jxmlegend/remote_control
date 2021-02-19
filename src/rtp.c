#include "base.h"
#include "rtp.h"

typedef enum {
    NALU_TYPE_SLICE    = 1,
    NALU_TYPE_DPA      = 2,
    NALU_TYPE_DPB      = 3,
    NALU_TYPE_DPC      = 4,
    NALU_TYPE_IDR      = 5,
    NALU_TYPE_SEI      = 6,
    NALU_TYPE_SPS      = 7,
    NALU_TYPE_PPS      = 8,
    NALU_TYPE_AUD      = 9,
    NALU_TYPE_EOSEQ    = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL     = 12,
} NaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIRITY_LOW         = 1,
    NALU_PRIORITY_HIGH       = 2,
    NALU_PRIORITY_HIGHEST    = 3
} NaluPriority;

typedef struct
{
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    unsigned max_size;            //! Nal Unit Buffer size
    int forbidden_bit;            //! should be always FALSE
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    int nal_unit_type;            //! NALU_TYPE_xxxx    
    char *buf;                    //! contains the first byte followed by the EBSP
} NALU_t;

FILE *h264bitstream = NULL;                //!< the bit stream file

int info2=0, info3=0;

static int FindStartCode2 (unsigned char *Buf){
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //0x000001?
    else return 1;
}

static int FindStartCode3 (unsigned char *Buf){
    if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//0x00000001?
    else return 1;
}



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

static int find_start_code2(uint8_t *buf)
{
#if 0
	if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
		return 1;
	else
		return 0;	
#endif
    if(buf[0]!=0 || buf[1]!=0 || buf[2] !=1) return 0; //0x000001?
    else return 1;
}

static int find_start_code3(uint8_t *buf)
{
#if 0
	if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
		return 1;
	else
		return 0;	
#endif

    if(buf[0]!=0 || buf[1]!=0 || buf[2] !=0 || buf[3] !=1) return 0;//0x00000001?
    else return 1;
}

uint32_t abstr_nalu_indic(uint8_t *buf, uint32_t len, uint32_t *be_found)
{
    uint8_t *p_tmp;
    int32_t offset;
    int32_t frame_size;
    
    *be_found = 0;
    offset = 0;
    frame_size = 4;
    p_tmp = buf + 4;
    
    while(frame_size < len - 4)
    {   
        if(p_tmp[2])
            offset = 4;
        else if(p_tmp[1])
            offset = 2;
        else if(p_tmp[0])
            offset = 1;
        else
        {   
            if(p_tmp[3] != 1)
            {   
                if(p_tmp[3])
                    offset = 4;
                else
                    offset = 1;
            }   
            else
            {   
                *be_found = 1;
                break;
            }   
        }   
        frame_size += offset;
        p_tmp += offset;
    }   
    if(!*be_found)
        frame_size = len;

    return frame_size;
}

uint32_t timestamp;
uint16_t seq;

void build_rtp_header(RTP_header *r,  int dt)
{
    r->version = 2;
    r->padding = 0;
    r->extension = 0;
    r->csrc_len = 0;
    r->marker = 0;
    r->payload = 96;
    r->seq_no = htons(seq);
    timestamp += 3600;
    r->timestamp = htonl(timestamp);
    r->ssrc = htonl(0);

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
uint8_t nalu_buf[1448];

int build_rtp_nalu(uint8_t *inbuffer, uint32_t frame_size, int sockfd)
{
	/* 计算时间戳 */
    RTP_header rtp_header;
    int32_t time_delay;
    int32_t data_left;

    uint8_t nalu_header;
    uint8_t fu_indic;
    uint8_t fu_header;
    uint8_t *p_nalu_data;
    uint8_t *nalu_buffer;

    int32_t fu_start = 1;
    int32_t fu_end = 0;

    if(!inbuffer)
        return -1;

    nalu_buffer = nalu_buf;

    build_rtp_header(&rtp_header, 0);

    data_left = frame_size;
    p_nalu_data = inbuffer;

    //Single RTP Packet
    if(data_left <= SINGLE_NALU_DATA_MAX)
    {
        rtp_header.seq_no = htons(seq++);
        rtp_header.marker = 1;
        memcpy(nalu_buffer, &rtp_header, sizeof(rtp_header));
        memcpy(nalu_buffer + RTP_HEADER_SIZE, p_nalu_data, data_left);
		//DEBUG("send %d", data_left + RTP_HEADER_SIZE);
		send(sockfd, nalu_buf, data_left + RTP_HEADER_SIZE, 0);
        //usleep(DE_TIME);
        return 0;
    }
    //FU-A RTP Packet.
    nalu_header = inbuffer[0];
    fu_indic = (nalu_header & 0xE0)|28;
    data_left   -= NALU_HEAD_SIZE;
    p_nalu_data += NALU_HEAD_SIZE;

    while(data_left > 0)
    {
        int32_t proc_size = MIN(data_left, SLICE_NALU_DATA_MAX);
        int32_t rtp_size = proc_size    +
                    RTP_HEADER_SIZE +
                    FU_A_HEAD_SIZE  +
                    FU_A_INDI_SIZE ;

        fu_end = (proc_size == data_left);
        fu_header = nalu_header&0x1F;

        if(fu_start)
            fu_header |= 0x80;
        else if(fu_end)
            fu_header |= 0x40;


        rtp_header.seq_no = htons(seq ++);
        memcpy(nalu_buffer, &rtp_header, sizeof(rtp_header));
        memcpy(nalu_buffer + 14, p_nalu_data, proc_size);
        nalu_buffer[12] = fu_indic;
        nalu_buffer[13] = fu_header;
        //udp_write(rtp_size, cur_conn_num);
		//DEBUG("send %d", rtp_size);
		send(sockfd, nalu_buf, rtp_size, 0);
        if(fu_end)
            usleep(36000);

        data_left -= proc_size;
        p_nalu_data += proc_size;
        fu_start = 0;
    }

    return 0;
}


FILE *fp = NULL;

int GetAnnexbNALU (NALU_t *nalu){
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL) 
        printf ("GetAnnexbNALU: Could not allocate Buf memory\n");

    nalu->startcodeprefix_len=3;

    if (3 != fread (Buf, 1, 3, h264bitstream)){
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2 (Buf);
    if(info2 != 1) {
        if(1 != fread(Buf+3, 1, 1, h264bitstream)){
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3 (Buf);
        if (info3 != 1){ 
            free(Buf);
            return -1;
        }
        else {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else{
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;

    while (!StartCodeFound){
        if (feof (h264bitstream)){
            nalu->len = (pos-1)-nalu->startcodeprefix_len;
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);     
            //printf("Buf[0] = 0x%x\n", Buf[0]);
            nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
            nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
            nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
            free(Buf);
            return pos-1;
        }
        Buf[pos++] = fgetc (h264bitstream);
        info3 = FindStartCode3(&Buf[pos-4]);
        if(info3 != 1)
            info2 = FindStartCode2(&Buf[pos-3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }

    // Here, we have found another start code (and read length of startcode bytes more than we should
    // have.  Hence, go back in the file
    rewind = (info3 == 1)? -4 : -3;

    if (0 != fseek (h264bitstream, rewind, SEEK_CUR)){
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }

    // Here the Start code, the complete NALU, and the next start code is in the Buf.  
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

    nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
    memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//
    //printf("nalu->buf[0] = 0x%x\n", nalu->buf[0]);
    nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
    nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
    nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
    free(Buf);

    return (pos+rewind);
}

int rtp_send_from_file(int sockfd)
{
    NALU_t *n;
    int buffersize=100000;

    //FILE *myout=fopen("output_log.txt","wb+");
    FILE *myout=stdout;

    h264bitstream=fopen("./1.h264", "rb+");
    if (h264bitstream==NULL){
        printf("Open file error\n");
        return 0;
    }

    n = (NALU_t*)calloc (1, sizeof (NALU_t));
    if (n == NULL){
        printf("Alloc NALU Error\n");
        return 0;
    }

    n->max_size=buffersize;
    n->buf = (char*)calloc (buffersize, sizeof (char));
    if (n->buf == NULL){
        free (n);
        printf ("AllocNALU: n->buf");
        return 0;
    }

    int data_offset=0;
    int nal_num=0;
    printf("-----+-------- NALU Table ------+---------+\n");
    printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
    printf("-----+---------+--------+-------+---------+\n");

    while(!feof(h264bitstream)) 
    {
        int data_lenth;
        data_lenth = GetAnnexbNALU(n);
        char type_str[20]={0};
        switch(n->nal_unit_type){
            case NALU_TYPE_SLICE:sprintf(type_str,"SLICE");break;
            case NALU_TYPE_DPA:sprintf(type_str,"DPA");break;
            case NALU_TYPE_DPB:sprintf(type_str,"DPB");break;
            case NALU_TYPE_DPC:sprintf(type_str,"DPC");break;
            case NALU_TYPE_IDR:sprintf(type_str,"IDR");break;
            case NALU_TYPE_SEI:sprintf(type_str,"SEI");break;
            case NALU_TYPE_SPS:sprintf(type_str,"SPS");break;
            case NALU_TYPE_PPS:sprintf(type_str,"PPS");break;
            case NALU_TYPE_AUD:sprintf(type_str,"AUD");break;
            case NALU_TYPE_EOSEQ:sprintf(type_str,"EOSEQ");break;
            case NALU_TYPE_EOSTREAM:sprintf(type_str,"EOSTREAM");break;
            case NALU_TYPE_FILL:sprintf(type_str,"FILL");break;
        }
        char idc_str[20]={0};
        switch(n->nal_reference_idc>>5){
            case NALU_PRIORITY_DISPOSABLE:sprintf(idc_str,"DISPOS");break;
            case NALU_PRIRITY_LOW:sprintf(idc_str,"LOW");break;
            case NALU_PRIORITY_HIGH:sprintf(idc_str,"HIGH");break;
            case NALU_PRIORITY_HIGHEST:sprintf(idc_str,"HIGHEST");break;
        }
        fprintf(myout,"%5d| %8d| %7s| %6s| %8d|\n",nal_num,data_offset,idc_str,type_str,n->len);
		
		build_rtp_nalu(n->buf, n->len, sockfd);
        data_offset=data_offset+data_lenth;
        nal_num++;
    }
    //Free
    if (n){
        if (n->buf){
            free(n->buf);
            n->buf=NULL;
        }
        free (n);
    }
    return 0;
}

int rtp_send_from_stream()
{
	
}

int rtp_send_packet(const char * ip, int port, int flag)
{
	int sockfd = -1;
	DEBUG("rtp_send_packet ip %s port %d", ip, port);
	if(flag) //tcp
	{

	}
	else	//udp
	{
		//sockfd = create_udp_client(ip, port);
	}
	sockfd = create_udp_client("192.169.27.196", port);
	//sockfd = create_udp_client("192.169.27."
	//send(sockfd, "123", strlen("123"), 0);

	/* file */
	rtp_send_from_file(sockfd);
	/* stream */
	//rtp_send_from_stream();
}


