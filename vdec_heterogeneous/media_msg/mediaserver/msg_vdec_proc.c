#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "hi_datafifo.h"
#include "sample_vdec_api.h"

HI_S32 MSG_VDEC_DatafifoInit(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_S32 VdecChn = pstMsg->as32PrivData[0];
    HI_U64 u64PhyAddr = *(HI_U64 *)(pstMsg->pBody);

    s32Ret = SAMPLE_VDEC_DatafifoInit(VdecChn, &u64PhyAddr);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);


    return HI_SUCCESS;
}



HI_S32 MSG_VDEC_DatafifoDeinit(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_S32 VdecChn = pstMsg->as32PrivData[0];

    SAMPLE_VDEC_DatafifoDeinit(VdecChn);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, HI_SUCCESS, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}


static HI_S32 MSG_VDEC_StartVideo(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_S32 s32ChnNum = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_VDEC_StartVideo(s32ChnNum);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_VDEC_StopVideo(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    HI_S32 s32ChnNum = pstMsg->as32PrivData[0];

    s32Ret = SAMPLE_VDEC_StopVideo(s32ChnNum);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}




HI_S32 MSG_VDEC_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_VDEC_CMD_DATAFIFO_INIT:
            s32Ret = MSG_VDEC_DatafifoInit(s32MsgId, pstMsg);
            break;

        case SAMPLE_VDEC_CMD_DATAFIFO_DEINIT:
            s32Ret = MSG_VDEC_DatafifoDeinit(s32MsgId, pstMsg);
            break;

        case SAMPLE_VDEC_CMD_START_VIDEO:
            s32Ret = MSG_VDEC_StartVideo(s32MsgId, pstMsg);
            break;

        case SAMPLE_VDEC_CMD_STOP_VIDEO:
            s32Ret = MSG_VDEC_StopVideo(s32MsgId, pstMsg);
            break;

        default:
            printf("venc error cmd %d\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}


