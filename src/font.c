

//“我”的编码是0xAFCA  HZK16点阵字库 起始位置 ((CA-161)*94+AF-161)*32
void DISP_PutHz_String_24x24(UINT32 uiX, UINT32 uiY, CHAR8 * pcStr,USHORT16 usColor)
{
  
  SLONG32 i,j,k,slDataPos,slOfs,slOffset;
  UCHAR8 ucData,ucDataPos,ucQh,ucWh;
  USHORT16 usDataSrc,usDataDest,*pusTemp,*pusBaseAddr;
  UCHAR8 ucHZK24[72];
  
  ucDataPos=0;
  pusBaseAddr=(uiX+uiY*DISP_WIDTH+stFbData.pusFbBuffer);
  pusTemp=pusBaseAddr;
  
  FILE *fp_hzk = fopen("/mnt/nfs/hzk24_sth.fon", "rb");
  
  while(1)
    { 
      slOfs=0;
      if('\0'==(*pcStr))
        break;
        
        if(' ' ==(*pcStr))
        {   
            pcStr++;    
            pusBaseAddr += 8;
        }

      
      ucQh=(*pcStr-0xa0);     //获得区码
      ucWh=(*(pcStr+1)-0xa0); //获得位码
      slOffset=((94*(ucQh -1) + (ucWh -1))*72);
      fseek(fp_hzk,slOffset,SEEK_SET);
      fread(ucHZK24,72,1,fp_hzk);
      
      ucDataPos=0;
      slDataPos>>=1;
      for(i=0;i<24;i++) {
        for(j=0;j<3;j++) {
          ucData=ucHZK24[ucDataPos++];
          for(k=0;k<8;k++){
            if(ucData&0x01)
              *pusTemp=usColor;
            ucData>>=1;
            pusTemp+=DISP_WIDTH;
          }
        }
        pusBaseAddr+=1;
        pusTemp=pusBaseAddr;
      }
      pcStr+=2;
    }
    fclose(fp_hzk);
}

void DISP_PutHz_String_24x24_Center(UINT32 uiY, CHAR8 * pcStr,USHORT16 usColor)
{
  UINT32 uiXPos;
  uiXPos=strlen(pcStr);
  uiXPos=(DISP_WIDTH-uiXPos*12);
  uiXPos>>=1;
  DISP_PutHz_String_24x24(uiXPos,uiY,pcStr,usColor);
}

void DISP_PutTB_24x24(UINT32 x, UINT32 y, UINT32 dat ,USHORT16 usColor)
{
  int i,j,k;
  int yOffset;
  unsigned short *pusBaseAddr,*pusTemp;
  UCHAR8 ucData,ucDataPos;

  ucDataPos=0;
  pusBaseAddr=(x+y*320+stFbData.pusFbBuffer);
  pusTemp=pusBaseAddr;

  for(i=0;i<24;i++) {
    for(j=0;j<3;j++) {
      ucData=tb_24x24[dat][ucDataPos++];
      for(k=0;k<8;k++){
        if(ucData&0x01)
          *pusTemp=usColor;
        ucData>>=1;
        pusTemp+=320;
      }
    }
    pusBaseAddr+=1;
    pusTemp=pusBaseAddr;
  }
}

