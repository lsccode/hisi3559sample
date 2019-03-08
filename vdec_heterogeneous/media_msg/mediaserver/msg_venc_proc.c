#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "sample_venc_api.h"


HI_S32 MSG_VENC_SendFrame(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VENC_CHN VeChn = pstMsg->as32PrivData[0];
    HI_S32 s32MilliSec = pstMsg->as32PrivData[1];

    s32Ret = SAMPLE_VENC_COMM_SendFrame(VeChn, (VIDEO_FRAME_INFO_S*)pstMsg->pBody, s32MilliSec);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VENC_GetStream(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VENC_CHN VeChn = pstMsg->as32PrivData[0];
    HI_S32 s32MilliSec = pstMsg->as32PrivData[1];
    VENC_STREAM_S stStream;

    s32Ret = SAMPLE_VENC_COMM_GetStream(VeChn, &stStream, s32MilliSec);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stStream, sizeof(VENC_STREAM_S));

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VENC_ReleaseStream(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VENC_CHN VeChn = pstMsg->as32PrivData[0];
    HI_S32 s32MilliSec = pstMsg->as32PrivData[1];

    s32Ret = SAMPLE_VENC_COMM_ReleaseStream(VeChn, (VENC_STREAM_S*)pstMsg->pBody);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VENC_GetStreamBufInfo(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VENC_CHN VeChn = pstMsg->as32PrivData[0];
    VENC_STREAM_BUF_INFO_S stStreamBufInfo;

    s32Ret = SAMPLE_VENC_COMM_GetStreamBufInfo(VeChn, &stStreamBufInfo);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stStreamBufInfo, sizeof(VENC_STREAM_BUF_INFO_S));

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VENC_GetJpegPacks(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VENC_CHN VeChn = pstMsg->as32PrivData[0];
    VENC_PACK_S stJpegPack[SAMPLE_JPEG_PACK_NUM];

    s32Ret = SAMPLE_VENC_COMM_GetJpegPack(VeChn, stJpegPack);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, stJpegPack, sizeof(VENC_PACK_S)*SAMPLE_JPEG_PACK_NUM);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VENC_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_VENC_CMD_SEND_FRAME:
            s32Ret = MSG_VENC_SendFrame(s32MsgId, pstMsg);
            break;

        case SAMPLE_VENC_CMD_GET_STREAM:
            s32Ret = MSG_VENC_GetStream(s32MsgId, pstMsg);
            break;

        case SAMPLE_VENC_CMD_RELEASE_STREAM:
            s32Ret = MSG_VENC_ReleaseStream(s32MsgId, pstMsg);
            break;

        case SAMPLE_VENC_CMD_GET_STREAM_BUFINFO:
            s32Ret = MSG_VENC_GetStreamBufInfo(s32MsgId, pstMsg);
            break;

        case SAMPLE_VENC_CMD_GET_JPEG_PACKS:
            s32Ret = MSG_VENC_GetJpegPacks(s32MsgId, pstMsg);
            break;

        default:
            printf("venc error cmd %d\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}


