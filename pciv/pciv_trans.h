
#include "hi_common.h"
#include "hi_type.h"
#include "hi_comm_vb.h"

#ifndef __PCI_TRANS_H__
#define __PCI_TRANS_H__


typedef struct hiPCIV_TRANS_ATTR_S
{
    HI_S32              s32RmtChip;
    HI_BOOL             bReciver;           /* receiver or sender ? */
    HI_U32              u32BufSize;
    HI_U64              u64PhyAddr;
    HI_S32              s32MsgPortWrite;
    HI_S32              s32MsgPortRead;
    HI_S32              s32ChnId;
} PCIV_TRANS_ATTR_S;

/* buffer information of remote receiver */
typedef struct hiPCIV_TRANS_RMTBUF_S
{
    HI_S64              s64BaseAddr;        /* physic address of remote buffer */
    HI_S32              s32Length;          /* buffer length */
    HI_S32              s32ReadPos;         /* read position */
    HI_S32              s32WritePos;        /* write position */
} PCIV_TRANS_RMTBUF_S;

/* buffer information of local sender */
typedef struct hiPCIV_TRANS_LOCBUF_S
{
    VB_BLK              vbBlk;              /* VB of buffer */
    HI_U8               *pBaseAddr;         /* virtual address */
    HI_U64              u64PhyAddr;         /* physic address */
    HI_U32              u32BufLen;          /* buffer length */
    HI_U32              u32CurLen;          /* current data length in buffer */
} PCIV_TRANS_LOCBUF_S;


/* The notify message when a new frame is writed */
typedef struct hiPCIV_TRANS_NOTIFY_S
{
    HI_U32              s32Start;           /* Write or read start position of this frame */
    HI_U32              s32End;             /* Write or read end position of this frame */
    HI_U32              u32Seq;
} PCIV_TRANS_NOTIFY_S;


typedef struct hiPCIV_TRANS_RECEIVER_S
{
    HI_BOOL             bInit;
    HI_S32              s32RmtChip;
    HI_U8               *pu8BufBaseAddr;    /* virtual address of receiver buffer */
    HI_U32              u32BufLen;          /* length of receiver buffer */
    HI_S32              s32MsgPortWirte;    /* message port for WriteDone */
    HI_S32              s32MsgPortRead;     /* message port for ReadDone */
    HI_U32              u32MsgSeqSend;
    HI_U32              u32MsgSeqFree;
} PCIV_TRANS_RECEIVER_S;

typedef struct hiPCIV_TRANS_SENDER_S
{
    HI_BOOL             bInit;
    HI_S32              s32RmtChip;
    PCIV_TRANS_LOCBUF_S stLocBuf;           /* buffer info of local source */
    PCIV_TRANS_RMTBUF_S stRmtBuf;           /* buffer info of remote receiver */
    HI_S32              s32MsgPortWrite;    /* message port for WriteDone */
    HI_S32              s32MsgPortRead;     /* message port for ReadDone */
    pthread_t           pid;
    HI_BOOL             bThreadStart;
    HI_U32              u32MsgSeqSend;
    HI_U32              u32MsgSeqFree;
} PCIV_TRANS_SENDER_S;


typedef struct hiPCIV_TRANS_LOCBUF_STAT_S
{
    HI_U32              u32FreeLen;         /* free length of local buffer to write */

} PCIV_TRANS_LOCBUF_STAT_S;


HI_VOID PCIV_Trans_Init(HI_VOID);

HI_S32 PCIV_Trans_InitSender(PCIV_TRANS_ATTR_S *pstAttr, HI_VOID **ppSender);
HI_S32 PCIV_Trans_DeInitSender(HI_VOID *pSender);
HI_S32 PCIV_Trans_QueryLocBuf(HI_VOID *pSender, PCIV_TRANS_LOCBUF_STAT_S *pstStatus);
HI_S32 PCIV_Trans_WriteLocBuf(HI_VOID *pSender, HI_U8 *pu8Addr, HI_U32 u32Len);
HI_S32 PCIV_Trans_SendData(HI_VOID *pSender);


HI_S32 PCIV_Trans_InitReceiver(PCIV_TRANS_ATTR_S *pstAttr, HI_VOID **ppReceiver);
HI_S32 PCIV_Trans_DeInitReceiver(HI_VOID *pReceiver);
HI_S32 PCIV_Trans_GetData(HI_VOID *pReceiver, HI_U8 **pu8Addr, HI_U32 *pu32Len);
HI_S32 PCIV_Trans_ReleaseData(HI_VOID *pReceiver, HI_U8 *pu8Addr, HI_U32 u32Len);



#endif
