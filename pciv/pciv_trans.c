/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : pciv_trans.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/12/09
  Description   : data transfer of pci application
  History       :
  1.Date        : 2009/12/09
    Author      : Hi3520MPP
    Modification: Created file
    Modification: 2009-12 Created
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "hi_debug.h"
#include "hi_comm_pciv.h"
#include "hi_comm_vb.h"
#include "pciv_trans.h"
#include "pciv_msg.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_pciv.h"
#include "sample_pciv_comm.h"

#define PCIV_TRANS_MSG_WRITE_DONE   18
#define PCIV_TRANS_MSG_READ_DONE    19

HI_VOID * PCIV_Trans_SenderThread(HI_VOID *p);


/*
** If the buffer tail is not enough then go to the head.
** And the tail will not be used
*/
HI_S32 Trans_GetWriteOffset(PCIV_TRANS_SENDER_S *pstSender,
        HI_S32 s32Size, HI_S32 *pOffset)
{
    HI_BOOL FLAG = HI_FALSE;
    PCIV_TRANS_RMTBUF_S *pBuf = NULL;
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 s32ReadPos;

    *pOffset = -1;

    pBuf = &pstSender->stRmtBuf;

    /* The Circule Buffer must be twice as much as the data lenghth that the Writepos
     and Readpos can be set rightly as the rule below*/
    if (s32Size > (pBuf->s32Length/2))
    {
        //printf("Func: %s, line: %d...Trans_GetWriteOffset failure!\n", __FUNCTION__, __LINE__);
        //printf("#### ReadPos is 0x%x, WritePos is 0x%x, stream length is 0x%x, buffer length 0x%x.\n",
        //    pBuf->s32ReadPos, pBuf->s32WritePos, s32Size, pBuf->s32Length);
        FLAG = HI_TRUE;
        return HI_FAILURE;
    }
    if (FLAG)
    {
        //printf("s32Size:%d,pBuf->s32Length/2:%d\n",s32Size,(pBuf->s32Length/2));
        FLAG = HI_FALSE;
    }

    /* get the s32ReadPos*/
    s32ReadPos = pBuf->s32ReadPos;

    if (pBuf->s32WritePos >= s32ReadPos)
    {
        if ((pBuf->s32WritePos + s32Size) <= (pBuf->s32Length))/* the idle is not enough */
        {
            *pOffset = pBuf->s32WritePos;
            s32Ret = HI_SUCCESS;
            //printf("s32ReadPos:%d < s32WritePos:%d + s32Size:%d < s32Length:%d!\n",s32ReadPos,pBuf->s32WritePos,s32Size,pBuf->s32Length);
        }
        else if (s32Size < s32ReadPos)/* the idle is not enough ,if the s32ReadPos is greater than size ,read from start */
        {
            *pOffset = 0;
            s32Ret = HI_SUCCESS;
            //printf("s32Size:%d < s32ReadPos:%d  < s32WritePos:%d\n",s32Size,s32ReadPos,pBuf->s32WritePos);
        }
        else
        {
            //printf("s32Size:%d > s32ReadPos:%d,s32WritePos:%d failed\n",s32Size,s32ReadPos,pBuf->s32WritePos);
        }

    }
    else/* s32ReadPos is greater than the s32WritePos */
    {
        if ((pBuf->s32WritePos + s32Size) < (s32ReadPos))/* the data written is not longger the s32ReadPos */
        {
            *pOffset = pBuf->s32WritePos;
            s32Ret = HI_SUCCESS;
            //printf("(s32WritePos:%d + s32Size:%d) < (s32ReadPos:%d)\n",pBuf->s32WritePos,s32Size,s32ReadPos);
        }
        else
        {
            //printf("(s32WritePos:%d + s32Size:%d) > (s32ReadPos:%d) failed\n",pBuf->s32WritePos,s32Size,s32ReadPos);
        }
    }

    return s32Ret;
}

HI_S32 Trans_UpdWritePos(PCIV_TRANS_SENDER_S *pstSender,
        HI_S32 s32Size, HI_S32 s32Offset)
{
    pstSender->stRmtBuf.s32WritePos = s32Offset + s32Size;
    return HI_SUCCESS;
}

HI_S32 Trans_UpdReadPos(PCIV_TRANS_SENDER_S *pstSender, HI_S32 s32Offset)
{
    pstSender->stRmtBuf.s32ReadPos = s32Offset;
    return HI_SUCCESS;
}
/*------------------------------------------------------------------------------------------*/

HI_S32 PCIV_Trans_InitSender(PCIV_TRANS_ATTR_S *pstAttr, HI_VOID **ppSender)
{
    HI_U64 u64PhyAddr;
    HI_U8 *u8VirtAddr;
    VB_BLK vbBlk;
    PCIV_TRANS_SENDER_S *pstSender = NULL;
    HI_ASSERT(NULL != pstAttr);
    HI_CHAR *pMmzName = HI_NULL;
    pstSender = (PCIV_TRANS_SENDER_S*)malloc(sizeof(PCIV_TRANS_SENDER_S));
    HI_ASSERT(pstSender);
    memset(pstSender, 0, sizeof(PCIV_TRANS_SENDER_S));

    /* Init buffer info for Dest */
    pstSender->stRmtBuf.s64BaseAddr     = pstAttr->u64PhyAddr;
    pstSender->stRmtBuf.s32Length       = pstAttr->u32BufSize;
    pstSender->stRmtBuf.s32ReadPos      = 0;
    pstSender->stRmtBuf.s32WritePos     = 0;

    /* Malloc memory for local tmp buffer */
    vbBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, pstAttr->u32BufSize, pMmzName);
    if (VB_INVALID_HANDLE == vbBlk)
    {
        printf("Func:%s, Info:HI_MPI_VB_GetBlock(size:%d) fail\n", __FUNCTION__,pstAttr->u32BufSize);
		free(pstSender);
        return HI_FAILURE;
    }

    u64PhyAddr = HI_MPI_VB_Handle2PhysAddr(vbBlk);
    if (0 == u64PhyAddr)
    {
        printf("Func:%s, Info:HI_MPI_VB_Handle2PhysAddr fail\n", __FUNCTION__);
        HI_MPI_VB_ReleaseBlock(vbBlk);
		free(pstSender);
        return HI_FAILURE;
    }

    u8VirtAddr = (HI_U8 *)HI_MPI_SYS_Mmap(u64PhyAddr, pstAttr->u32BufSize);
    if (NULL == u8VirtAddr)
    {
        printf("Func:%s, Info:HI_MPI_SYS_Mmap fail\n", __FUNCTION__);
        HI_MPI_VB_ReleaseBlock(vbBlk);
		free(pstSender);
        return HI_FAILURE;
    }

    pstSender->stLocBuf.u64PhyAddr  = u64PhyAddr;
    pstSender->stLocBuf.pBaseAddr   = u8VirtAddr;
    pstSender->stLocBuf.vbBlk       = vbBlk;
    pstSender->stLocBuf.u32CurLen   = 0;
    pstSender->stLocBuf.u32BufLen   = pstAttr->u32BufSize;
    pstSender->s32MsgPortWrite      = pstAttr->s32MsgPortWrite;
    pstSender->s32MsgPortRead       = pstAttr->s32MsgPortRead;
    pstSender->s32RmtChip           = pstAttr->s32RmtChip;
    pstSender->bInit                = HI_TRUE;
    printf("%s ok, handle:0x%lx, remote:%d, dma_buf:0x%llx,len:%d; tmp_buf:(0x%llu,%p), msgport:(%d,%d)\n",
        __FUNCTION__, (long)pstSender,pstSender->s32RmtChip, pstSender->stRmtBuf.s64BaseAddr, pstSender->stRmtBuf.s32Length,
        pstSender->stLocBuf.u64PhyAddr, pstSender->stLocBuf.pBaseAddr, pstSender->s32MsgPortWrite, pstSender->s32MsgPortRead);

    /* create thread to process ReadDone message from receiver */
    pstSender->bThreadStart = HI_TRUE;
    pthread_create(&pstSender->pid, NULL, PCIV_Trans_SenderThread, pstSender);

    *ppSender = pstSender;
    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_InitReceiver(PCIV_TRANS_ATTR_S *pstAttr, HI_VOID **ppReceiver)
{
    PCIV_TRANS_RECEIVER_S *pRev = NULL;
    HI_VOID *pVirAddr = HI_NULL;
    HI_ASSERT(NULL != pstAttr);

    pRev = (PCIV_TRANS_RECEIVER_S*)malloc(sizeof(PCIV_TRANS_RECEIVER_S));
    HI_ASSERT(pRev);
    memset(pRev, 0, sizeof(PCIV_TRANS_RECEIVER_S));

    /* This address is used in user mode, so we need to map it */
    pVirAddr = (HI_VOID *)HI_MPI_SYS_Mmap(pstAttr->u64PhyAddr, pstAttr->u32BufSize);
    if (NULL == pVirAddr)
    {
    	free(pRev);
        return HI_FAILURE;
    }

    *(HI_U32*)(HI_UL)pVirAddr      = 0x777777;
    //*(HI_U32*)pVirAddr      = 0x777777;
	pRev->pu8BufBaseAddr    = (HI_U8*)pVirAddr;
    pRev->u32BufLen         = pstAttr->u32BufSize;
    pRev->s32RmtChip        = pstAttr->s32RmtChip;
    pRev->s32MsgPortWirte   = pstAttr->s32MsgPortWrite;
    pRev->s32MsgPortRead    = pstAttr->s32MsgPortRead;
    pRev->bInit             = HI_TRUE;

    printf("%s ok, remote:%d, addr:(0x%llx,%p), buflen:%d, msgport:(%d,%d)\n",
        __FUNCTION__, pRev->s32RmtChip, pstAttr->u64PhyAddr, pRev->pu8BufBaseAddr,
        pRev->u32BufLen, pRev->s32MsgPortWirte, pRev->s32MsgPortRead);
    *ppReceiver = (HI_VOID*)pRev;
    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_DeInitSender(HI_VOID *pSender)
{
    PCIV_TRANS_SENDER_S *pstSender = (PCIV_TRANS_SENDER_S*)pSender;

    if (!pstSender->bInit)
    {
        return HI_SUCCESS;
    }

    if (pstSender->bThreadStart)
    {
        pstSender->bThreadStart = HI_FALSE;
        pthread_join(pstSender->pid, 0);
    }

    /* Release temp buffer */
    (HI_VOID)HI_MPI_SYS_Munmap(pstSender->stLocBuf.pBaseAddr, pstSender->stLocBuf.u32BufLen);
    pstSender->stLocBuf.pBaseAddr = NULL;

    (HI_VOID)HI_MPI_VB_ReleaseBlock(pstSender->stLocBuf.vbBlk);
    pstSender->bInit = HI_FALSE;
    free(pstSender);
    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_DeInitReceiver(HI_VOID *pReceiver)
{
    PCIV_TRANS_RECEIVER_S *pRev = (PCIV_TRANS_RECEIVER_S*)pReceiver;

    if (!pRev->bInit)
    {
        return HI_SUCCESS;
    }

    (HI_VOID)HI_MPI_SYS_Munmap(pRev->pu8BufBaseAddr, pRev->u32BufLen);

    pRev->bInit = HI_FALSE;
    free(pRev);
    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_QueryLocBuf(HI_VOID *pSender, PCIV_TRANS_LOCBUF_STAT_S *pstStatus)
{
    PCIV_TRANS_SENDER_S *pstSender = (PCIV_TRANS_SENDER_S*)pSender;

    HI_ASSERT(pstSender->stLocBuf.u32CurLen <= pstSender->stLocBuf.u32BufLen);

    pstStatus->u32FreeLen = pstSender->stLocBuf.u32BufLen - pstSender->stLocBuf.u32CurLen;

    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_WriteLocBuf(HI_VOID *pSender, HI_U8 *pu8Addr, HI_U32 u32Len)
{
    PCIV_TRANS_SENDER_S *pstSender = (PCIV_TRANS_SENDER_S*)pSender;

    if ((pstSender->stLocBuf.u32CurLen + u32Len) > pstSender->stLocBuf.u32BufLen)
    {
        printf("local buf full !!, curpos:%d, len:%d, buflen:%d\n",
            pstSender->stLocBuf.u32CurLen, u32Len, pstSender->stLocBuf.u32BufLen);
        return HI_FAILURE;
    }

    memcpy(pstSender->stLocBuf.pBaseAddr + pstSender->stLocBuf.u32CurLen, pu8Addr, u32Len);
    pstSender->stLocBuf.u32CurLen += u32Len;

    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_SendData(HI_VOID *pSender)
{
    HI_S32 s32Ret;
    HI_S32 s32WriteOff = 0;
    SAMPLE_PCIV_MSG_S   stMsgSend;
    PCIV_TRANS_NOTIFY_S *pStreamInfo;
    PCIV_DMA_TASK_S  stTask;
    PCIV_DMA_BLOCK_S stDmaBlk[PCIV_MAX_DMABLK];
    PCIV_TRANS_SENDER_S *pstSender = (PCIV_TRANS_SENDER_S*)pSender;

#if 1
    /* get writen offset (if there is not free space to write,return failure) */
    if (Trans_GetWriteOffset(pstSender, pstSender->stLocBuf.u32CurLen, &s32WriteOff))
    {
        //printf("Func: %s, line: %d...Trans_GetWriteOffset failure!\n", __FUNCTION__, __LINE__);
        return HI_FAILURE;
    }
#endif

    //printf("u32DstAddr: 0x%x.\n", pstSender->stRmtBuf.s32BaseAddr);
    /* Call the driver to send on frame. */
    stDmaBlk[0].u32BlkSize = pstSender->stLocBuf.u32CurLen;
    stDmaBlk[0].u64SrcAddr = pstSender->stLocBuf.u64PhyAddr;
    stDmaBlk[0].u64DstAddr = pstSender->stRmtBuf.s64BaseAddr + s32WriteOff;
    stTask.pBlock          = &stDmaBlk[0];
    stTask.u32Count        = 1;
    stTask.bRead           = HI_FALSE;


    s32Ret = HI_MPI_PCIV_DmaTask(&stTask);
    while (HI_ERR_PCIV_BUSY == s32Ret)
    {
        usleep(10000);
        printf("---- PCI DMA Wait ----\n");
        s32Ret = HI_MPI_PCIV_DmaTask(&stTask);
    }
    if (HI_SUCCESS != s32Ret)
    {
        printf("Func:%s -> dma task fail,s32ret= 0x%x\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    /* update write pos after data have write */
    Trans_UpdWritePos(pstSender, pstSender->stLocBuf.u32CurLen, s32WriteOff);

    /* Send WriteDone message to notify the receiver */
    stMsgSend.stMsgHead.u32MsgType = PCIV_TRANS_MSG_WRITE_DONE;
    stMsgSend.stMsgHead.u32MsgLen = sizeof(PCIV_TRANS_NOTIFY_S);
    stMsgSend.stMsgHead.u32Target = pstSender->s32RmtChip;
    pStreamInfo = (PCIV_TRANS_NOTIFY_S *)stMsgSend.cMsgBody;
    pStreamInfo->s32Start   = s32WriteOff;
    pStreamInfo->s32End     = s32WriteOff + pstSender->stLocBuf.u32CurLen;
    pStreamInfo->u32Seq     = pstSender->u32MsgSeqSend ++;
    s32Ret = PCIV_SendMsg(stMsgSend.stMsgHead.u32Target, pstSender->s32MsgPortWrite, &stMsgSend);
    HI_ASSERT((HI_FAILURE != s32Ret));

    if (pstSender->u32MsgSeqSend % 2001 == 0)
    {
        printf("PCIV_Trans_SendData -> w:%d, r:%d, offset:%d, len:%d \n",
            pstSender->stRmtBuf.s32WritePos, pstSender->stRmtBuf.s32ReadPos,
            s32WriteOff, pstSender->stLocBuf.u32CurLen);
    }

    pstSender->stLocBuf.u32CurLen =  0;
    return HI_SUCCESS;
}

HI_VOID * PCIV_Trans_SenderThread(HI_VOID *p)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S stMsgRev;
    PCIV_TRANS_NOTIFY_S *pStreamInfo;
    PCIV_TRANS_SENDER_S *pstSender = (PCIV_TRANS_SENDER_S*)p;
    HI_S32 s32RmtChip = pstSender->s32RmtChip;
    HI_S32 s32MsgPortRead = pstSender->s32MsgPortRead;

    prctl(PR_SET_NAME, "hi_pciv_Send", 0,0,0);

    while (pstSender->bThreadStart)
    {
        memset(&stMsgRev, 0, sizeof(stMsgRev));

        /* read ReadDone Message from stream Receiver */
        s32Ret = PCIV_ReadMsg(s32RmtChip, s32MsgPortRead, &stMsgRev);
        if (s32Ret != HI_SUCCESS)
        {
            usleep(10000);
            continue;
        }
        HI_ASSERT(PCIV_TRANS_MSG_READ_DONE == stMsgRev.stMsgHead.u32MsgType);

        /* get stream info in msg body  */
        pStreamInfo = (PCIV_TRANS_NOTIFY_S *)&stMsgRev.cMsgBody;

        /* Check message sequence number */
        if (pStreamInfo->u32Seq - pstSender->u32MsgSeqFree > 1)
        {
            printf("%d,%d, start:%d,end:%d \n",
                pStreamInfo->u32Seq, pstSender->u32MsgSeqFree,
                pStreamInfo->s32Start, pStreamInfo->s32End);
        }
        HI_ASSERT(pStreamInfo->u32Seq - pstSender->u32MsgSeqFree <= 1);
        pstSender->u32MsgSeqFree = pStreamInfo->u32Seq;

        /* The greater the gap between sequence number of send message and release message ,with greater delay the receiver get the data*/
        if (pstSender->u32MsgSeqSend - pstSender->u32MsgSeqFree > 50)
        {
            printf("Warnning: send:%d,free:%d \n", pstSender->u32MsgSeqSend, pstSender->u32MsgSeqFree);
        }

        /* If ReadDone message is recived then update the read position of remote buffer */
        s32Ret = Trans_UpdReadPos(pstSender, pStreamInfo->s32End);
        HI_ASSERT(HI_SUCCESS == s32Ret);
    }

    return NULL;
}

HI_S32 PCIV_Trans_GetData(HI_VOID *pReceiver, HI_U8 **pu8Addr, HI_U32 *pu32Len)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S stMsgRev = {{0}};
    PCIV_TRANS_NOTIFY_S *pStreamInfo;
    PCIV_TRANS_RECEIVER_S *pstReceiver = (PCIV_TRANS_RECEIVER_S*)pReceiver;

    HI_ASSERT(pstReceiver->bInit);
    /* Receive WriteDone message from Sender */
    s32Ret = PCIV_ReadMsg(pstReceiver->s32RmtChip, pstReceiver->s32MsgPortWirte, &stMsgRev);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
    HI_ASSERT(PCIV_TRANS_MSG_WRITE_DONE == stMsgRev.stMsgHead.u32MsgType);

    /* Get stream info in msg body  */
    pStreamInfo = (PCIV_TRANS_NOTIFY_S *)&stMsgRev.cMsgBody;

    //HI_ASSERT( (pStreamInfo->s32End > pStreamInfo->s32Start)
    //    && (pStreamInfo->s32End <= pstReceiver->u32BufLen));
    if(!(pStreamInfo->s32End > pStreamInfo->s32Start)
        && (pStreamInfo->s32End <= pstReceiver->u32BufLen))
    {
        printf("func:%s,line:%d\n",__FUNCTION__,__LINE__);
        return -1;
    }

    *pu8Addr = pstReceiver->pu8BufBaseAddr + pStreamInfo->s32Start;
    *pu32Len = pStreamInfo->s32End - pStreamInfo->s32Start;

    if(!(pStreamInfo->u32Seq - pstReceiver->u32MsgSeqSend <= 1))
    {
        //printf("func:%s,line:%d\n",__FUNCTION__,__LINE__);
        return -1;
    }

    if (pStreamInfo->u32Seq % 5001 == 0)
    {
        //printf("PCIV_Trans_GetData -> addr:%p, streamlen:%d \n",
            //*pu8Addr, *pu32Len);
    }
    /* test and check the sequece num of the message */
    if (pStreamInfo->u32Seq - pstReceiver->u32MsgSeqSend > 1)
    {
        printf("%d,%d, start:%d,end:%d \n",pStreamInfo->u32Seq, pstReceiver->u32MsgSeqSend,
                pStreamInfo->s32Start, pStreamInfo->s32End);
    }
    HI_ASSERT(pStreamInfo->u32Seq - pstReceiver->u32MsgSeqSend <= 1);
    pstReceiver->u32MsgSeqSend = pStreamInfo->u32Seq;
    return HI_SUCCESS;
}

HI_S32 PCIV_Trans_ReleaseData(HI_VOID *pReceiver, HI_U8 *pu8Addr, HI_U32 u32Len)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S stMsgRev;
    PCIV_TRANS_NOTIFY_S *pStreamInfo;
    PCIV_TRANS_RECEIVER_S *pstReceiver = (PCIV_TRANS_RECEIVER_S*)pReceiver;

    HI_ASSERT(pstReceiver->bInit);

    /* Send msg to sender, we have used the data over */
    pStreamInfo = (PCIV_TRANS_NOTIFY_S*)stMsgRev.cMsgBody;
    pStreamInfo->s32Start   = pu8Addr - pstReceiver->pu8BufBaseAddr;
    pStreamInfo->s32End     = pStreamInfo->s32Start + u32Len;
    pStreamInfo->u32Seq     = pstReceiver->u32MsgSeqFree ++;

    stMsgRev.stMsgHead.u32MsgType = PCIV_TRANS_MSG_READ_DONE;
    stMsgRev.stMsgHead.u32MsgLen = sizeof(PCIV_TRANS_NOTIFY_S);
    stMsgRev.stMsgHead.u32Target = pstReceiver->s32RmtChip;
    s32Ret = PCIV_SendMsg(pstReceiver->s32RmtChip, pstReceiver->s32MsgPortRead, &stMsgRev);
    HI_ASSERT(s32Ret != HI_FAILURE);

    return HI_SUCCESS;
}

