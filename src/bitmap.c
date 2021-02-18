

//转换RGB888为RGB565
unsigned short RGB888to565(unsigned char red, unsigned char green, unsigned char blue)
{
    unsigned short B = (blue >> 3) & 0x001F;
    unsigned short G = ((green >> 2) << 5) & 0x07E0;
    unsigned short R = ((red >> 3) << 11) & 0xF800;

    return (unsigned short) (R | G |B );
}

int save_bitmap()
{

}

int cursor_bitmap_convert()
{
    int i, j;
    char *psrc = src;
    char *pdst = dst;
    char *p = psrc;
        
    pdst += (width * height * 3); 

    /* 由于bmp存储是从后面往前面, 所有需要倒序转换 */
    for(i = 0; i < height; i++)
    {   
        p = psrc + (i + 1) *width *3; 
        for(j = 0; j<width ; j++)
        {   
            pdst -= 3;
            p -= 3;
            pdst[0] = p[0];
            pdst[1] = p[1];
            pdst[2] = p[2];
        }   
    }   
    return 0;
}

void show_bitmap()
{


}

//二次线性插值缩放算法
unsigned char *do_stretch_linear(int w_dest, int h_dest, int bit_depth, unsigned char *src, int w_src, int h_src)
{
    int sw = w_src -1 ,sh = h_src - 1, dw = w_dest - 1, dh = h_dest - 1;
    int B, N, x , y, i, j, k;
    int nPixelSize = bit_depth/8;

    unsigned char *pLinePrev, *pLineNext;

    unsigned char Dests[w_dest * h_dest * bit_depth / 8];
    unsigned char *pDest = &Dests[0];
    unsigned char *tmp;
    unsigned char *pA, *pB, *pC, *pD;


    for(i = 0; i<= dh; ++i)
    {
        tmp = pDest + i*w_dest*nPixelSize;
        y = i*sh/dh;
        N = dh - i *sh%dh;
        pLinePrev = src + (y++) *w_src*nPixelSize;
        pLineNext = (N == dh) ? pLinePrev: src+y*w_src*nPixelSize;
        for(j = 0; j < dw; ++j)
        {
             x = j*sw/dw*nPixelSize;
            B = dw-j*sw%dw;
            pA = pLinePrev+x;
            pB = pA+nPixelSize;
            pC = pLineNext + x;
            pD = pC + nPixelSize;
            if(B == dw)
            {
                pB=pA;
                pD=pC;
            }


            for(k=0;k<nPixelSize;++k)
            {
                *tmp++ = ( unsigned char )( int )(
                    ( B * N * ( *pA++ - *pB - *pC + *pD ) + dw * N * *pB++
                    + dh * B * *pC++ + ( dw * dh - dh * B - dw * N ) * *pD++
                    + dw * dh / 2 ) / ( dw * dh ) );
            }
        }
    }

    return pDest;
}


/* jpeg 转RGB */
void Disp_Dec_Pics(UCHAR8 *ucPath)
{
    FILE *fp_pic;

    struct jpeg_decompress_struct jinfo;
    struct jpeg_error_mgr jerr;

    UCHAR8 *temp, *buffer, *buf_pos;


    if((fp_pic = fopen(ucPath, "rb")) == NULL)
    {
    }
     //绑定标志错误处理函数
    jinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&jinfo);

    //导入要解压的JPEG文件
    jpeg_stdio_src(&jinfo, fp_pic);

    //读取jpeg文件头
    (void)jpeg_read_header(&jinfo, TRUE);

    //设置解压参数, 将原图长宽缩小为1/2

    jinfo.scale_num=1;
    jinfo.scale_denom=1;

    //jinfo.image_width = 320;    // 为图的宽和高，单位为像素 
    //jinfo.image_height = 240;

    //计算每行需要的空间大小，比如RGB图像就是宽度×3，灰度图就是宽度×1
    //row_stride = jinfo.output_width * jinfo.output_components;

    //开始解压jpeg文件，解压后将分配给scanline缓冲区
    (void)jpeg_start_decompress(&jinfo);


    temp = (unsigned char *) malloc(jinfo.output_width
                                    * jinfo.output_components);

    buffer = (unsigned char *) malloc(jinfo.output_width * jinfo.output_height
                                            * jinfo.output_components );
    buf_pos = buffer;

    DEBUG("width %d height %d", jinfo.output_width, jinfo.output_height);

    while(jinfo.output_scanline < jinfo.output_height)
    {
        jpeg_read_scanlines(&jinfo, &temp, 1);
        memcpy(buf_pos, temp, jinfo.output_width * jinfo.output_components);
        buf_pos += jinfo.output_width * jinfo.output_components;
   }

    //memcpy(stDisp_State.ucBufTemp ,buffer, DISP_WIDTH * DISP_HEIGH * 3);
    memcpy(stDisp_State.ucBufTemp ,buffer, jinfo.output_width * jinfo.output_height * 3);

    Disp_Show_Pics(buffer, jinfo.output_width, jinfo.output_height,
                        0, 0, 0);

    UCHAR8 ucSize[100] = {0};
    UINT32 uiXPos = 200, uiYPos = 216;
    sprintf(ucSize, "%dx%d", jinfo.output_width, jinfo.output_height);

    Disp_PutString(uiXPos,uiYPos, ucSize, DISP_BOTB_COLOR);

    free(temp);
    free(buffer);
    fclose(fp_pic);
}


bool EncodeThread::Jpeg2Rgb_2(BYTE *data, int inLen, int *width, int *height, BYTE **outData, int *outLen)
{
    //qDebug()<<"Jpeg2Rgb_211";
    struct jpeg_decompress_struct cinfo = { 0 };
    struct jpeg_error_mgr jerr = { 0 };
    JSAMPARRAY buffer;
    int row_stride;

    //qDebug()<<"Jpeg2Rgb_212";
    cinfo.err = jpeg_std_error(&jerr);
    jerr.emit_message = my_emit_message;
    jerr.error_exit = my_emit_exit;
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, inLen);

    //qDebug()<<"Jpeg2Rgb_213";
    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    //qDebug()<<"Jpeg2Rgb_214";
    *outLen = row_stride * cinfo.output_height;
    *outData = new BYTE[*outLen];
    memset(*outData, 0, *outLen);
    BYTE *ptr = *outData;

    //qDebug()<<"Jpeg2Rgb_215";

    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(ptr, buffer[0], row_stride);
        ptr += row_stride;
    }

    //qDebug()<<"Jpeg2Rgb_216";
    if (NULL !=width)
        *width = cinfo.output_width;
    if (NULL != height)
        *height = cinfo.output_height;
    (void)jpeg_finish_decompress(&cinfo);

    //qDebug()<<"Jpeg2Rgb_217";
    jpeg_destroy_decompress(&cinfo);
    return true;
}

bool EncodeThread::Rgb2Jpeg_2(int width,int height,int quality,BYTE*data,BYTE **outData,int * outLen)
{
    //qDebug()<<"Rgb2Jpeg_211";
    struct jpeg_compress_struct cinfo = { 0 };
    struct jpeg_error_mgr jerr = {0};

    JSAMPROW row_pointer[1];
    int row_stride;
    int image_width = width;
    int image_height = height;
    //JSAMPLE* image_buffer = (JSAMPLE*)data;
    unsigned char*image_buffer = data;
    int imageRowSize = width *3;

    //qDebug()<<"Rgb2Jpeg_212"<<" data[0]:"<<data[0];
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    //qDebug()<<"Rgb2Jpeg_213";
    //内存中转换
    jpeg_mem_dest(&cinfo,outData,(unsigned long*)outLen);
    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo,quality,TRUE);
    jpeg_start_compress(&cinfo,TRUE);
    row_stride = imageRowSize;

    //qDebug()<<"Rgb2Jpeg_214 cinfo.image_height:"<<cinfo.image_height;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        //qDebug()<<"cinfo.next_scanline:"<<cinfo.next_scanline;
        row_pointer[0] = & image_buffer[cinfo.next_scanline*row_stride];
        //qDebug()<<"11cinfo.next_scanline:"<<cinfo.next_scanline;
        (void)jpeg_write_scanlines(&cinfo,row_pointer,1);
    }

//    for (int i = 0;i<image_height;i++)
//    {
//        qDebug()<<"i:"<<i<<" image_height:"<<image_height;
//        row_pointer[0] = data + i*image_width * 3;
//        jpeg_write_scanlines(&cinfo,row_pointer,1);
//    }

    //qDebug()<<"Rgb2Jpeg_215";
    (void)jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);

    //qDebug()<<"Rgb2Jpeg_216";
    return true;
}

bool MainWindow::RgbToBgr(char *Rgb, int width, int height, char *Bgr)
{
    char *pTemp, *ptr;
    pTemp = Rgb;
    ptr = Bgr;

    //height
    for (int j = height -1; j>=0; j--)
    {
        //width
        for (int i = 0; i< width; i++)
        {
            unsigned char r = pTemp[width * j * 3 + i * 3];
            unsigned char g = pTemp[width * j * 3 + i * 3 + 1];
            unsigned char b = pTemp[width * j * 3 + i * 3 + 2];

            *(ptr++) = b;
            *(ptr++) = g;
            *(ptr++) = r;
        }
    }
    return true;
}

