#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "sample_snap_api.h"

static HI_S32 MSG_SNAP_Trigger(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VI_PIPE ViPipe = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_SNAP_COMM_Trigger(ViPipe);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_SNAP_SetAttr(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VI_PIPE ViPipe = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_SNAP_COMM_SetAttr(ViPipe, (SNAP_ATTR_S*)pstMsg->pBody);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_SNAP_Enable(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VI_PIPE ViPipe = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_SNAP_COMM_Enable(ViPipe);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_SNAP_Disable(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VI_PIPE ViPipe = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_SNAP_COMM_Disable(ViPipe);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_SNAP_GetBnrRaw(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VI_PIPE ViPipe = pstMsg->as32PrivData[0];
    HI_S32 s32timeOut = pstMsg->as32PrivData[1];
    VIDEO_FRAME_INFO_S stVideoFrame;

    s32Ret = SAMPLE_SNAP_COMM_GetBNRRaw(ViPipe, &stVideoFrame, s32timeOut);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stVideoFrame, sizeof(VIDEO_FRAME_INFO_S));

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_SNAP_ReleaseBnrRaw(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VI_PIPE ViPipe = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_SNAP_COMM_ReleaseBNRRaw(ViPipe, (VIDEO_FRAME_INFO_S*)pstMsg->pBody);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}



HI_S32 MSG_SNAP_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_SNAP_CMD_TRIGGER:
            s32Ret = MSG_SNAP_Trigger(s32MsgId, pstMsg);
            break;

        case SAMPLE_SNAP_CMD_SET_ATTR:
            s32Ret = MSG_SNAP_SetAttr(s32MsgId, pstMsg);
            break;

        case SAMPLE_SNAP_CMD_ENABLE:
            s32Ret = MSG_SNAP_Enable(s32MsgId, pstMsg);
            break;

        case SAMPLE_SNAP_CMD_DISABLE:
            s32Ret = MSG_SNAP_Disable(s32MsgId, pstMsg);
            break;

        case SAMPLE_SNAP_CMD_GET_BNRRAW:
            s32Ret = MSG_SNAP_GetBnrRaw(s32MsgId, pstMsg);
            break;

        case SAMPLE_SNAP_CMD_RELEASE_BNRRAW:
            s32Ret = MSG_SNAP_ReleaseBnrRaw(s32MsgId, pstMsg);
            break;

        default:
            printf("snap error cmd %d\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}


