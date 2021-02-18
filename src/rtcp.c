#include "base.h"
#include "rtcp.h"

int rtcp_send_packet(struct rtcp_pkt *pkt)
{
    RTCP_header hdr;
    RTCP_header_SDES hdr_sdes;
    uint32_t pkt_size = 0, hdr_s = 0, hdr_sdes_s, local_ssrc, name_s;

    if(!pkt)
        return -1; 
    
    pkt->comm.version = 2;
    pkt->comm.padding = 0;
    pkt->comm.count = 1;
    pkt->comm.pt = SDES;
    hdr_s = sizeof(hdr);
    //name_s = strlen(rtsp[0]->host_name);
    
    pkt_size = sizeof(rtcp_pkt) + name_s;
    pkt->comm.length = htons((pkt_size >> 2) - 1); 
    //local_ssrc = random32(0);
    pkt->sdec.ssrc = htonl(local_ssrc);
    pkt->sdec.attr_name = CNAME;
    pkt->sdec.len = name_s;
    //memcpy(pkt->sdec.name, rtsp[0]->host_name, name_s);
    
    return pkt_size;
}






