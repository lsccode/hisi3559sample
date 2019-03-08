#include <stdio.h>
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "hi_comm_video.h"

extern HI_S32 g_MCmsgId;

HI_S32 SAMPLE_VPSS_COMM_GetChnFrame(HI_S32 VpssGrp, HI_S32 VpssChn, VIDEO_FRAME_INFO_S* pstVideoFrame, HI_S32 s32MilliSecs)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VPSS, SAMPLE_VPSS_CMD_GET_CHN_FRAME, NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VpssGrp;
    pReq->as32PrivData[1] = VpssChn;
    pReq->as32PrivData[2] = s32MilliSecs;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    memcpy(pstVideoFrame, pResp->pBody, pResp->u32BodyLen);

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return s32Ret;
}


HI_S32 SAMPLE_VPSS_COMM_ReleaseChnFrame(HI_S32 VpssGrp, HI_S32 VpssChn, VIDEO_FRAME_INFO_S* pstVideoFrame)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VPSS, SAMPLE_VPSS_CMD_RELEASE_CHN_FRAME, pstVideoFrame, sizeof(VIDEO_FRAME_INFO_S));
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = VpssGrp;
    pReq->as32PrivData[1] = VpssChn;
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


