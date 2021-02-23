#include <unistd.h>
#include <linux/fb.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
 
#include <asm/types.h>        
#include <linux/videodev2.h>
 
#define CAMERA_DEVICE "/dev/video0"
#define FB_DEVICE "/dev/fb0"
 
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define BUFFER_COUNT 4
 
const int rvarrxyp[]={
-180,-179,-177,-176,-175,-173,-172,-170,-169,-167,-166,-165,-163,-162,-160,-159,-158,-156,-155,-153,-152,-151,
-149,-148,-146,-145,-144,-142,-141,-139,-138,-137,-135,-134,-132,-131,-129,-128,-127,-125,-124,-122,-121,-120,
-118,-117,-115,-114,-113,-111,-110,-108,-107,-106,-104,-103,-101,-100,-99,-97,-96,-94,-93,-91,-90,-89,-87,-86,
-84,-83,-82,-80,-79,-77,-76,-75,-73,-72,-70,-69,-68,-66,-65,-63,-62,-61,-59,-58,-56,-55,-53,-52,-51,-49,-48,
-46,-45,-44,-42,-41,-39,-38,-37,-35,-34,-32,-31,-30,-28,-27,-25,-24,-23,-21,-20,-18,-17,-15,-14,-13,-11,-10,
-8,-7,-6,-4,-3,-1,0,1,3,4,6,7,8,10,11,13,14,15,17,18,20,21,23,24,25,27,28,30,31,32,34,35,37,38,39,41,42,44,45,
46,48,49,51,52,53,55,56,58,59,61,62,63,65,66,68,69,70,72,73,75,76,77,79,80,82,83,84,86,87,89,90,91,93,94,96,
97,99,100,101,103,104,106,107,108,110,111,113,114,115,117,118,120,121,122,124,125,127,128,129,131,132,134,135,
137,138,139,141,142,144,145,146,148,149,151,152,153,155,156,158,159,160,162,163,165,166,167,169,170,172,173,175,
176,177,179};
 
const int guarrxyp[]={
-44,-44,-44,-43,-43,-42,-42,-42,-41,-41,-41,-40,-40,-40,-39,-39,-39,-38,-38,-38,-37,-37,-37,-36,-36,-36,-35,-35,
-35,-34,-34,-34,-33,-33,-32,-32,-32,-31,-31,-31,-30,-30,-30,-29,-29,-29,-28,-28,-28,-27,-27,-27,-26,-26,-26,-25,
-25,-25,-24,-24,-23,-23,-23,-22,-22,-22,-21,-21,-21,-20,-20,-20,-19,-19,-19,-18,-18,-18,-17,-17,-17,-16,-16,-16,
-15,-15,-15,-14,-14,-13,-13,-13,-12,-12,-12,-11,-11,-11,-10,-10,-10,-9,-9,-9,-8,-8,-8,-7,-7,-7,-6,-6,-6,-5,-5,-4,
-4,-4,-3,-3,-3,-2,-2,-2,-1,-1,-1,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,11,12,12,
12,13,13,13,14,14,15,15,15,16,16,16,17,17,17,18,18,18,19,19,19,20,20,20,21,21,21,22,22,22,23,23,23,24,24,25,25,25,
26,26,26,27,27,27,28,28,28,29,29,29,30,30,30,31,31,31,32,32,32,33,33,34,34,34,35,35,35,36,36,36,37,37,37,38,38,38,
39,39,39,40,40,40,41,41,41,42,42,42,43,43,44,44};
 
const int gvarrxyp[]={
-92,-91,-90,-90,-89,-88,-87,-87,-86,-85,-85,-84,-83,-82,-82,-81,-80,-80,-79,-78,-77,-77,-76,-75,-75,-74,-73,-72,
-72,-71,-70,-70,-69,-68,-67,-67,-66,-65,-65,-64,-63,-62,-62,-61,-60,-60,-59,-58,-57,-57,-56,-55,-54,-54,-53,-52,-52,
-51,-50,-49,-49,-48,-47,-47,-46,-45,-44,-44,-43,-42,-42,-41,-40,-39,-39,-38,-37,-37,-36,-35,-34,-34,-33,-32,-32,-31,
-30,-29,-29,-28,-27,-27,-26,-25,-24,-24,-23,-22,-22,-21,-20,-19,-19,-18,-17,-16,-16,-15,-14,-14,-13,-12,-11,-11,-10,
-9,-9,-8,-7,-6,-6,-5,-4,-4,-3,-2,-1,-1,0,1,1,2,3,4,4,5,6,6,7,8,9,9,10,11,11,12,13,14,14,15,16,16,17,18,19,19,20,21,
22,22,23,24,24,25,26,27,27,28,29,29,30,31,32,32,33,34,34,35,36,37,37,38,39,39,40,41,42,42,43,44,44,45,46,47,47,48,49,
49,50,51,52,52,53,54,54,55,56,57,57,58,59,60,60,61,62,62,63,64,65,65,66,67,67,68,69,70,70,71,72,72,73,74,75,75,76,77,
77,78,79,80,80,81,82,82,83,84,85,85,86,87,87,88,89,90,90,91};
 
const int buarrxyp[]={
-228,-226,-224,-222,-221,-219,-217,-215,-213,-212,-210,-208,-206,-205,-203,-201,-199,-197,-196,-194,-192,-190,-189,
-187,-185,-183,-181,-180,-178,-176,-174,-173,-171,-169,-167,-165,-164,-162,-160,-158,-157,-155,-153,-151,-149,-148,
-146,-144,-142,-141,-139,-137,-135,-133,-132,-130,-128,-126,-125,-123,-121,-119,-117,-116,-114,-112,-110,-109,-107,
-105,-103,-101,-100,-98,-96,-94,-93,-91,-89,-87,-85,-84,-82,-80,-78,-76,-75,-73,-71,-69,-68,-66,-64,-62,-60,-59,-57,
-55,-53,-52,-50,-48,-46,-44,-43,-41,-39,-37,-36,-34,-32,-30,-28,-27,-25,-23,-21,-20,-18,-16,-14,-12,-11,-9,-7,-5,-4,
-2,0,2,4,5,7,9,11,12,14,16,18,20,21,23,25,27,28,30,32,34,36,37,39,41,43,44,46,48,50,52,53,55,57,59,60,62,64,66,68,69,
71,73,75,76,78,80,82,84,85,87,89,91,93,94,96,98,100,101,103,105,107,109,110,112,114,116,117,119,121,123,125,126,128,
130,132,133,135,137,139,141,142,144,146,148,149,151,153,155,157,158,160,162,164,165,167,169,171,173,174,176,178,180,
181,183,185,187,189,190,192,194,196,197,199,201,203,205,206,208,210,212,213,215,217,219,221,222,224,226};
 
typedef struct VideoBuffer {
    void   *start;
    size_t  length;
} VideoBuffer;
 
VideoBuffer framebuf[BUFFER_COUNT];
 
//查表法，把YUV422转换为RGB32
void process(unsigned long  *fb_buf, unsigned char *v_buf)
{
    int r,g,b;
    int y0,u,y1,v;
    y0=(int)*v_buf;
    v=(int)*(v_buf+1);
    y1=(int)*(v_buf+2);
    u=(int)*(v_buf+3);
 
    r=y0+rvarrxyp[v];
    g=y0-guarrxyp[u]-gvarrxyp[v];
    b=y0+buarrxyp[u];
    if (r>255) r=255;
    if (r<0) r=0;
    if (g>255) g=255;
    if (g<0) g=0;
    if (b>255) b=255;
    if (b<0) b=0;
    *fb_buf=(b<<16)+(g<<8)+r;
 
    r=y1+rvarrxyp[v];
    g=y1-guarrxyp[u]-gvarrxyp[v];
    b=y1+buarrxyp[u];
    if (r>255) r=255;
    if (r<0) r=0;
    if (g>255) g=255;
    if (g<0) g=0;
    if (b>255) b=255;
    if (b<0) b=0;
    *(fb_buf+1)=(b<<16)+(g<<8)+r;
}
 
int main()
{
    int i, ret;  
 
    // 打开摄像头设备
    int fd;
    fd = open(CAMERA_DEVICE, O_RDWR, 0);
    if (fd < 0) {
        printf("Open %s failed\n", CAMERA_DEVICE);
        return -1;
    }
 
    // 获取驱动信息
    struct v4l2_capability cap;
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
        return ret;
    }
 
    printf("Capability Informations:\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %u.%u.%u\n", (cap.version>>16)&0XFF, (cap.version>>8)&0XFF,cap.version&0XFF);
    printf(" capabilities: %08X\n", cap.capabilities);
 
    // 设置视频格式
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = VIDEO_WIDTH;
    fmt.fmt.pix.height      = VIDEO_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        printf("VIDIOC_S_FMT failed (%d)\n", ret);
        return ret;
    }
 
    //获取视频格式
    ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
    if (ret < 0) {
        printf("VIDIOC_G_FMT failed (%d)\n", ret);
        return ret;
    }
    // Print Stream Format
    printf("Stream Format Informations:\n");
    printf(" type: %d\n", fmt.type);
    printf(" width: %d\n", fmt.fmt.pix.width);
    printf(" height: %d\n", fmt.fmt.pix.height);
    char fmtstr[8];
    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
    printf(" pixelformat: %s\n", fmtstr);
    printf(" field: %d\n", fmt.fmt.pix.field);
    printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
    printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
    printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
    printf(" priv: %d\n", fmt.fmt.pix.priv);
    printf(" raw_date: %s\n", fmt.fmt.raw_data);
 
    //请求分配内存
    struct v4l2_requestbuffers reqbuf;
    
    reqbuf.count = BUFFER_COUNT;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    
    ret = ioctl(fd , VIDIOC_REQBUFS, &reqbuf);
    if(ret < 0) {
        printf("VIDIOC_REQBUFS failed (%d)\n", ret);
        return ret;
    }
 
    VideoBuffer*  buffers = calloc( reqbuf.count, sizeof(*buffers) );
    struct v4l2_buffer buf;
 
    for (i = 0; i < reqbuf.count; i++) 
    {
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        //获取内存空间
        ret = ioctl(fd , VIDIOC_QUERYBUF, &buf);
        if(ret < 0) {
            printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
            return ret;
        }
 
        //映射到用户空间
        framebuf[i].length = buf.length;
        framebuf[i].start = (char *) mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (framebuf[i].start == MAP_FAILED) {
            printf("mmap (%d) failed: %s\n", i, strerror(errno));
            return -1;
        }
    
        //内存入队列
        ret = ioctl(fd , VIDIOC_QBUF, &buf);
        if (ret < 0) {
            printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
            return -1;
        }
 
        printf("Frame buffer %d: address=0x%x, length=%d\n", i, (unsigned int)framebuf[i].start, framebuf[i].length);
    }
 
    // 启动视频流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        printf("VIDIOC_STREAMON failed (%d)\n", ret);
        return ret;
    }
 
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    int x,y;
    int fd_fb;
    long int screen_size = 0;
    unsigned long  *fbp32 = NULL;
    //打开LCD设备
    fd_fb = open(FB_DEVICE, O_RDWR);
    if (!fd_fb)
    {
        printf("Error: cannot open framebuffer device.\n");
        return(1);
    }
    if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &finfo))
    {
        printf("Error reading fixed information.\n");
        return(2);
    }
    //获取LCD属性信息
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo))
    {
        printf("Error 2 reading variable information.\n");
        return(3);
    }
 
    //一屏的字节数
    screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    printf("LCD:  %dx%d, %dbpp, screen_size = %ld\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, screen_size );
 
    //映射framebuffer到用户空间
    fbp32 = (unsigned long *)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
 
    if ((int)fbp32 == -1)
    {
        printf("Error: failed to map framebuffer device to memory.\n");
        return(4);
    }
 
    printf("start camera testing...\n");
    //开始视频显示
    while(1)
    {
        //内存空间出队列
        ret = ioctl(fd, VIDIOC_DQBUF, &buf);
        if (ret < 0) 
         {
            printf("VIDIOC_DQBUF failed (%d)\n", ret);
            return ret;
         }
 
       if(vinfo.bits_per_pixel == 32)
         {
           for(y = 0; y < fmt.fmt.pix.height;  y++)
              {
               for(x = 0; x < fmt.fmt.pix.width; x++)
                   {
                   //YUV422转换为RGB32
                   process(fbp32+y * vinfo.xres + x, framebuf[buf.index].start+y * fmt.fmt.pix.width*2+x*2);
                   x++;
                   }
               }
 
          }
 
        // 内存重新入队列
        ret = ioctl(fd, VIDIOC_QBUF, &buf);
        if (ret < 0) 
          {
            printf("VIDIOC_QBUF failed (%d)\n", ret);
            return ret;
          }
 
    }//while(1)
 
    //释放资源
    for (i=0; i< 4; i++) 
    {
        munmap(framebuf[i].start, framebuf[i].length);
    }
    munmap(fbp32, screen_size);
    //关闭设备
    close(fd);
    close(fd_fb);
    return 0;
}


