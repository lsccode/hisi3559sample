#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "sample_vpss_api.h"
#include "hi_comm_video.h"

static HI_S32 MSG_VPSS_GetChnFrame(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VPSS_GRP VpssGrp = pstMsg->as32PrivData[0];
    VPSS_CHN VpssChn = pstMsg->as32PrivData[1];
    HI_S32 s32timeOut = pstMsg->as32PrivData[2];
    VIDEO_FRAME_INFO_S stVideoFrame;

    s32Ret = SAMPLE_VPSS_COMM_GetChnFrame(VpssGrp, VpssChn, &stVideoFrame, s32timeOut);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_VPSS_ReleaseChnFrame(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VPSS_GRP VpssGrp = pstMsg->as32PrivData[0];
    VPSS_CHN VpssChn = pstMsg->as32PrivData[1];

    s32Ret = SAMPLE_VPSS_COMM_ReleaseChnFrame(VpssGrp, VpssChn, (VIDEO_FRAME_INFO_S*)pstMsg->pBody);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VPSS_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_VPSS_CMD_GET_CHN_FRAME:
            s32Ret = MSG_VPSS_GetChnFrame(s32MsgId, pstMsg);
            break;
        case SAMPLE_VPSS_CMD_RELEASE_CHN_FRAME:
            s32Ret = MSG_VPSS_ReleaseChnFrame(s32MsgId, pstMsg);
            break;
        default:
            printf("vpss error cmd %d\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}


