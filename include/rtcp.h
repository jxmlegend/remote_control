#ifndef __RTCP_H__
#define __RTCP_H__

typedef enum{
    SR = 200,
    RR = 201,
    SDES = 202,
    BYE = 203,
    APP= 204 
}rtcp_pkt_type;


typedef enum{
    CNAME = 1,
    NAME,
    EMAIL,
    PHONE,
    LOC,
    TOOL,
    NOTE,
    PRIV
} rtcp_info;

typedef struct _RTCP_header {
    #ifdef WORDS_BIGENDIAN
        uint32_t version:2;
        uint32_t padding:1;
        uint32_t count:5;    //SC oppure RC

    #else
        uint32_t count:5;
        uint32_t padding:1;
        uint32_t version:2;
    
    #endif
    uint32_t pt:8;
    uint32_t length:16;
}RTCP_header;

typedef struct _RTCP_header_SR {
    uint32_t ssrc;
    uint32_t ntp_timestampH;
    uint32_t ntp_timestampL;
    uint32_t rtp_timestamp;
    uint32_t pkt_count;
    uint32_t octet_count;
}RTCP_header_SR;

typedef struct _RTCP_header_RR {
    uint32_t ssrc;
}RTCP_header_RR;

typedef struct _RTCP_header_SR_report_block{
    uint64_t ssrc;
    uint8_t fract_lost;
    uint8_t pck_lost[3];
    uint32_t h_seq_no;
    uint32_t jitter;
    uint32_t last_SR;
    uint32_t delay_last_SR;
}RTCP_header_SR_report_block;

typedef struct _RTCP_header_SDES {
    uint32_t ssrc;
    uint8_t attr_name;
    uint8_t len;
    char name[];
}RTCP_header_SDES;

typedef struct _RTCP_header_BYE{
    uint32_t ssrc;
    uint8_t length;
}RTCP_header_BYE;

struct rtcp_pkt{
    RTCP_header comm;
    RTCP_header_SDES sdec;
}rtcp_pkt;





#endif //__RTCP_H__

