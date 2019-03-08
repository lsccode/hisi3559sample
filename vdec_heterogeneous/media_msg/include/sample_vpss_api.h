#ifndef __SAMPLE_VPSS_API__
#define __SAMPLE_VPSS_API__

#include "hi_comm_video.h"


HI_S32 SAMPLE_VPSS_COMM_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S* pstVideoFrame, HI_S32 s32MilliSecs);

HI_S32 SAMPLE_VPSS_COMM_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S* pstVideoFrame);


#endif

