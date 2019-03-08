#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_dsp.h"

#ifndef SAMPLE_PRT
#define SAMPLE_PRT(fmt...)   \
do {\
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif

HI_S32 SAMPLE_COMM_PHOTO_LoadDspCoreBinary(SVP_DSP_ID_E enCoreId);
HI_S32 SAMPLE_COMM_PHOTO_UnloadDspCoreBinary(SVP_DSP_ID_E enCoreId);

HI_S32 SAMPLE_COMM_PHOTO_SaveYUVFrame(FILE* pfd, VIDEO_FRAME_S* pVBuf);
HI_S32 SAMPLE_COMM_PHOTO_GetIsoByVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrame, HI_U32* pu32Iso);



