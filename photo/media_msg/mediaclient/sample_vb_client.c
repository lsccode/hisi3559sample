#include <stdio.h>
#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "hi_comm_video.h"
#include "hi_comm_vb.h"

extern HI_S32 g_MCmsgId;

VB_BLK SAMPLE_VB_COMM_GetBlock(VB_POOL Pool, HI_U64 u64BlkSize, HI_CHAR *pcMmzName)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    VB_BLK VbBlk;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VB, SAMPLE_VB_CMD_GET_BLOCK, NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = Pool;
    pReq->as32PrivData[1] = u64BlkSize;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }

    memcpy(&VbBlk, pResp->pBody, pResp->u32BodyLen);

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return VbBlk;
}


HI_S32 SAMPLE_VB_COMM_ReleaseBlock(VB_BLK Block)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VB, SAMPLE_VB_CMD_RELEASE_BLOCK, &Block, sizeof(VB_BLK));
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }

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

HI_U64 SAMPLE_VB_COMM_Handle2PhysAddr(VB_BLK Block)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    HI_U64 u64PhyAddr;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VB, SAMPLE_VB_CMD_HANDLE2PHYSADDR, &Block, sizeof(VB_BLK));
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    memcpy(&u64PhyAddr, pResp->pBody, pResp->u32BodyLen);

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return u64PhyAddr;
}


