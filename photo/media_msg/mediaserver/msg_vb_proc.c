#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "sample_vb_api.h"
#include "hi_comm_video.h"

static HI_S32 MSG_VB_GetBlock(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VB_POOL Pool = pstMsg->as32PrivData[0];
    HI_U64 u64BlkSize = pstMsg->as32PrivData[1];
    VB_BLK VbBlk;

    VbBlk = SAMPLE_VB_COMM_GetBlock(Pool, u64BlkSize, pstMsg->pBody);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &VbBlk, sizeof(VB_BLK));

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_VB_ReleaseBlock(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VB_BLK VbBlk;

    memcpy(&VbBlk, pstMsg->pBody, sizeof(VB_BLK));
    s32Ret = SAMPLE_VB_COMM_ReleaseBlock(VbBlk);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

static HI_S32 MSG_VB_Handle2PhysAddr(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *respMsg;
    VB_BLK VbBlk;
    HI_U64 u64PhyAddr;

    memcpy(&VbBlk, pstMsg->pBody, sizeof(VB_BLK));
    u64PhyAddr = SAMPLE_VB_COMM_Handle2PhysAddr(VbBlk);

    respMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &u64PhyAddr, sizeof(HI_U64));

    HI_IPCMSG_SendAsync(s32MsgId, respMsg, NULL);
    HI_IPCMSG_DestroyMessage(respMsg);

    return HI_SUCCESS;
}

HI_S32 MSG_VB_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_VB_CMD_GET_BLOCK:
            s32Ret = MSG_VB_GetBlock(s32MsgId, pstMsg);
            break;

        case SAMPLE_VB_CMD_RELEASE_BLOCK:
            s32Ret = MSG_VB_ReleaseBlock(s32MsgId, pstMsg);
            break;

        case SAMPLE_VB_CMD_HANDLE2PHYSADDR:
            s32Ret = MSG_VB_Handle2PhysAddr(s32MsgId, pstMsg);
            break;

        default:
            printf("vb error cmd %d\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}


