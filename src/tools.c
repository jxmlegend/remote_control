#include "base.h"


#if 0
void test()
{

  struct statfs diskInfo;

  if(0==statfs(TF_PATH, &diskInfo)) {  //查询内存卡剩余容量
    freeDisk = (unsigned long long)(diskInfo.f_bfree) * (unsigned long long)(diskInfo.f_bsize);
    mbFreedisk = (freeDisk >> 20);
    mbFreedisk*=10;
    mbFreedisk/=1024;

    totalDisk = (unsigned long long)(diskInfo.f_blocks) * (unsigned long long)(diskInfo.f_bsize);
    mbTotalsize = totalDisk >> 20;
    mbTotalsize*=10;
    mbTotalsize/=1024;
  }

}
#endif


uint64_t get_file_size(FILE *fp)
{
	uint64_t file_size;
	
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	
	fseek(fp, 0L, SEEK_SET);
	return file_size;
}



