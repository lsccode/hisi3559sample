#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "sample_photo_api.h"

static HI_S32 MSG_PHOTO_Start_Video(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = SAMPLE_PHOTO_COMM_Start_Video();

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_PHOTO_Stop_Video(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;

    s32Ret = SAMPLE_PHOTO_COMM_Stop_Video();

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_PHOTO_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_PHOTO_CMD_START_VIDEO:
            s32Ret = MSG_PHOTO_Start_Video(s32MsgId, pstMsg);
            break;
        case SAMPLE_PHOTO_CMD_STOP_VIDEO:
            s32Ret = MSG_PHOTO_Stop_Video(s32MsgId, pstMsg);
            break;
        default:
            printf("photo error cmd %d\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}


