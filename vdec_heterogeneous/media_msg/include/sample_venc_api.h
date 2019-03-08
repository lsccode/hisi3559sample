#ifndef __SAMPLE_VENC_API__
#define __SAMPLE_VENC_API__

#include "hi_comm_video.h"
#include "hi_comm_venc.h"

#define SAMPLE_JPEG_PACK_NUM 2

HI_S32 SAMPLE_VENC_COMM_SendFrame(VENC_CHN VeChn, VIDEO_FRAME_INFO_S *pstFrame ,HI_S32 s32MilliSec);
HI_S32 SAMPLE_VENC_COMM_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, HI_S32 s32MilliSec);
HI_S32 SAMPLE_VENC_COMM_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream);
HI_S32 SAMPLE_VENC_COMM_GetStreamBufInfo(VENC_CHN VeChn, VENC_STREAM_BUF_INFO_S *pstStreamBufInfo);
HI_S32 SAMPLE_VENC_COMM_GetJpegPack(VENC_CHN VeChn, VENC_PACK_S pstJpegPack[]);



#endif

