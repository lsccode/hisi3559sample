#include "hi_common.h"
#include "hi_comm_video.h"

#ifndef SAMPLE_PRT
#define SAMPLE_PRT(fmt...)   \
do {\
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif

HI_S32 SAMPLE_COMM_VENC_SavePicture(VENC_CHN VeChn, HI_U32 u32Cnt, const char* cFileName);


