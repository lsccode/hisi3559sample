#include <stdio.h>
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_venc.h"

extern HI_S32 g_MCmsgId;

HI_S32 SAMPLE_VENC_COMM_SendFrame(VENC_CHN VeChn, VIDEO_FRAME_INFO_S *pstFrame ,HI_S32 s32MilliSec)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VENC, SAMPLE_VENC_CMD_SEND_FRAME, pstFrame, sizeof(VIDEO_FRAME_INFO_S));
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VeChn;
    pReq->as32PrivData[1] = s32MilliSec;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}

HI_S32 SAMPLE_VENC_COMM_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, HI_S32 s32MilliSec)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VENC, SAMPLE_VENC_CMD_GET_STREAM, NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VeChn;
    pReq->as32PrivData[1] = s32MilliSec;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    memcpy(pstStream, pResp->pBody, pResp->u32BodyLen);

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}


HI_S32 SAMPLE_VENC_COMM_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VENC, SAMPLE_VENC_CMD_RELEASE_STREAM, pstStream, sizeof(VENC_STREAM_S));
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VeChn;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}


HI_S32 SAMPLE_VENC_COMM_GetStreamBufInfo(VENC_CHN VeChn, VENC_STREAM_BUF_INFO_S *pstStreamBufInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VENC, SAMPLE_VENC_CMD_GET_STREAM_BUFINFO, NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VeChn;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    memcpy(pstStreamBufInfo, pResp->pBody, pResp->u32BodyLen);

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}


HI_S32 SAMPLE_VENC_COMM_GetJpegPack(VENC_CHN VeChn, VENC_PACK_S pstJpegPack[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VENC, SAMPLE_VENC_CMD_GET_JPEG_PACKS, NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VeChn;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    memcpy(pstJpegPack, pResp->pBody, pResp->u32BodyLen);

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}


