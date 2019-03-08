#include "sample_comm.h"
#include "sample_venc_api.h"

VENC_PACK_S astJpegPack[SAMPLE_JPEG_PACK_NUM];

HI_S32 SAMPLE_VENC_COMM_SendFrame(VENC_CHN VeChn, VIDEO_FRAME_INFO_S *pstFrame ,HI_S32 s32MilliSec)
{
    return HI_MPI_VENC_SendFrame(VeChn, pstFrame, s32MilliSec);
}

HI_S32 SAMPLE_VENC_COMM_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, HI_S32 s32MilliSec)
{
    pstStream->pstPack = astJpegPack;
    pstStream->u32PackCount = SAMPLE_JPEG_PACK_NUM;
    return HI_MPI_VENC_GetStream(VeChn, pstStream, s32MilliSec);
}

HI_S32 SAMPLE_VENC_COMM_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
    pstStream->pstPack = astJpegPack;
    pstStream->u32PackCount = SAMPLE_JPEG_PACK_NUM;
    return HI_MPI_VENC_ReleaseStream(VeChn, pstStream);
}

HI_S32 SAMPLE_VENC_COMM_GetStreamBufInfo(VENC_CHN VeChn, VENC_STREAM_BUF_INFO_S *pstStreamBufInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret =  HI_MPI_VENC_GetStreamBufInfo(VeChn, pstStreamBufInfo);

    return s32Ret;
}

HI_S32 SAMPLE_VENC_COMM_GetJpegPack(VENC_CHN VeChn, VENC_PACK_S pstJpegPack[])
{
    memcpy(pstJpegPack, astJpegPack, sizeof(VENC_PACK_S)*SAMPLE_JPEG_PACK_NUM);
    return HI_SUCCESS;
}

