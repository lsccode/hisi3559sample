/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sample_pciv_slave.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/09/22
  Description   : this sample of pciv in PCI device
  History       :
  1.Date        : 2009/09/22
    Author      : Hi3520MPP
    Modification: Created file
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <math.h>
#include <sys/prctl.h>

#include "hi_debug.h"
#include "hi_comm_pciv.h"

#include "mpi_pciv.h"
#include "pciv_msg.h"
#include "pciv_trans.h"
#include "sample_pciv_comm.h"
#include "sample_comm.h"
#include "loadbmp.h"
#include "mpi_venc.h"


#define PCIV_FRMNUM_ONCEDMA 5
#define SLAVE_SAVE_STREAM	1
#define SAMPLE_STREAM_PATH "./"
//#define SAMPLE_1080P_H264_PATH "./3840x2160_8bit.h264"

typedef struct hiSAMPLE_PCIV_CTX_S
{
    VDEC_CHN VdChn;
    pthread_t pid;
    HI_BOOL bThreadStart;
    HI_CHAR aszFileName[64];
} SAMPLE_PCIV_CTX_S;

static SAMPLE_PCIV_VENC_CTX_S g_stSamplePcivVenc = {0};
static SAMPLE_PCIV_VDEC_CTX_S g_astSamplePcivVdec[VDEC_MAX_CHN_NUM];	//= {{0}};

static HI_S32 g_s32PciLocalId  = -1;
static HI_U64 g_u64PfAhbBase   = 0;
pthread_t   VdecSlaveThread[2*VDEC_MAX_CHN_NUM];
//static HI_BOOL bExit[VDEC_MAX_CHN_NUM] = {HI_FALSE};
HI_U32 gs_u32ViFrmRate = 0;
PIC_SIZE_E g_enPicSize = PIC_4096x2160;
//VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;

SAMPLE_VO_CONFIG_S g_stVoConfig =
{
    .u32DisBufLen = 3,
};
SAMPLE_VI_CONFIG_S g_stViConfig;

static VB_BLK vbInBlk;

VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];

static HI_U64 au64PhyAddr[PCIV_MAX_BUF_NUM] = {0};

static HI_VOID TraceStreamInfo(VENC_CHN vencChn, VENC_STREAM_S *pstVStream)
{
    if ((pstVStream->u32Seq % SAMPLE_PCIV_SEQ_DEBUG) == 0 )
    {
        printf("Send VeStream -> VeChn[%2d], PackCnt[%d], Seq:%d, DataLen[%6d], hours:%d\n",
            vencChn, pstVStream->u32PackCount, pstVStream->u32Seq,
            pstVStream->pstPack[0].u32Len,
            pstVStream->u32Seq/(25*60*60));
    }
}

HI_S32 SamplePcivEchoMsg(HI_S32 s32RetVal, HI_S32 s32EchoMsgLen, SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;

    pMsg->stMsgHead.u32Target  = 0; /* To host */
    pMsg->stMsgHead.s32RetVal  = s32RetVal;
    pMsg->stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_ECHO;
    pMsg->stMsgHead.u32MsgLen  = s32EchoMsgLen + sizeof(SAMPLE_PCIV_MSGHEAD_S);
    s32Ret = PCIV_SendMsg(0, PCIV_MSGPORT_COMM_CMD, pMsg);
    HI_ASSERT(s32Ret != HI_FAILURE);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveInitWinVb(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_WINVB_S *pstWinVbArgs = (SAMPLE_PCIV_MSG_WINVB_S*)pMsg->cMsgBody;

    /* create buffer pool in PCI Window */
    s32Ret = HI_MPI_PCIV_WinVbDestroy();
	PCIV_CHECK_ERR(s32Ret);
    s32Ret = HI_MPI_PCIV_WinVbCreate(&pstWinVbArgs->stPciWinVbCfg);
    PCIV_CHECK_ERR(s32Ret);
    return s32Ret;
}

HI_S32 SamplePciv_SlaveExitWinVb()
{
    HI_S32 s32Ret;

    /* distroy buffer pool in PCI Window */
    s32Ret = HI_MPI_PCIV_WinVbDestroy();
	PCIV_CHECK_ERR(s32Ret);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveMalloc(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret = HI_SUCCESS, i;
    //HI_U64 au64PhyAddr[PCIV_MAX_BUF_NUM] = {0};
    PCIV_PCIVCMD_MALLOC_S *pstMallocArgs = (PCIV_PCIVCMD_MALLOC_S *)pMsg->cMsgBody;

    /* in slave chip, this func will alloc a buffer from Window MMZ */
    s32Ret = HI_MPI_PCIV_Malloc(pstMallocArgs->u32BlkSize, pstMallocArgs->u32BlkCount, au64PhyAddr);
    HI_ASSERT(!s32Ret);

    /* Attation: return the offset from PCI shm_phys_addr */
    g_u64PfAhbBase = 0xfa000000;
    for(i=0; i<pstMallocArgs->u32BlkCount; i++)
    {
        pstMallocArgs->u64PhyAddr[i] = au64PhyAddr[i] - g_u64PfAhbBase;
        printf("func:%s, phyaddr:0x%llu = 0x%llu - 0x%llu \n",
            __FUNCTION__, pstMallocArgs->u64PhyAddr[i], au64PhyAddr[i], g_u64PfAhbBase);
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveFree(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //HI_S32 i;
    //HI_U64 au64PhyAddr[PCIV_MAX_BUF_NUM] = {0};
    PCIV_PCIVCMD_FREE_S *pstMallocArgs = (PCIV_PCIVCMD_FREE_S *)pMsg->cMsgBody;

    //for (i = 0; i < pstMallocArgs->u32BlkCount; i++)
    //{
    //    //au64PhyAddr[i] = (HI_U64)pstMallocArgs->u64PhyAddr + (HI_U64)g_u64PfAhbBase;
    //    //printf("func:%s, phyaddr:0x%llu = 0x%llu + 0x%llu \n",
    //    //    __FUNCTION__, au64PhyAddr[i], (HI_U64)pstMallocArgs->u64PhyAddr, (HI_U64)g_u64PfAhbBase);
    //}

    s32Ret = HI_MPI_PCIV_Free(pstMallocArgs->u32BlkCount, au64PhyAddr);
    HI_ASSERT(!s32Ret);

    return s32Ret;
}
void* SamplePcivVdStreamThread(void* arg)
{
    SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = (SAMPLE_PCIV_VDEC_CTX_S*)arg;
    HI_VOID *pReceiver = pstVdecCtx->pTransHandle;
    PCIV_STREAM_HEAD_S *pStrmHead = NULL;
    HI_U8 *pu8Addr;
    HI_U32 u32Len;
    VDEC_STREAM_S stStream;

    while (pstVdecCtx->bThreadStart)
    {

        /* get data from pciv stream receiver */
        if (PCIV_Trans_GetData(pReceiver, &pu8Addr, &u32Len))
        {
            //usleep(10000);
            continue;
        }
        pStrmHead = (PCIV_STREAM_HEAD_S *)pu8Addr;
        HI_ASSERT(PCIV_STREAM_MAGIC == pStrmHead->u32Magic);

        HI_ASSERT(u32Len >= pStrmHead->u32DMADataLen + sizeof(PCIV_STREAM_HEAD_S));
        /* send the data to video decoder */
        stStream.pu8Addr = pu8Addr + sizeof(PCIV_STREAM_HEAD_S);
        stStream.u64PTS = 0;
        stStream.u32Len = pStrmHead->u32StreamDataLen;
        stStream.bEndOfStream = HI_FALSE;


        if (0 == stStream.u32Len)
        {
            usleep(10000);
            continue;
        }

        /* save stream data to file */
        //if (NULL == pFile[pstVdecCtx->VdecChn])
        //{
        //    sprintf(aszFileName, "slave_vdec_chn%d.h264", pstVdecCtx->VdecChn);
        //    pFile[pstVdecCtx->VdecChn] = fopen(aszFileName, "wb");
        //    HI_ASSERT(pFile[pstVdecCtx->VdecChn]);
        //}
        //s32WriteLen = fwrite(pu8Addr + sizeof(PCIV_STREAM_HEAD_S), pStrmHead->u32StreamDataLen, 1, pFile[pstVdecCtx->VdecChn]);
		//fwrite(pu8Addr + sizeof(PCIV_STREAM_HEAD_S), pStrmHead->u32StreamDataLen, 1, pFile[pstVdecCtx->VdecChn]);
        //HI_ASSERT(1 == s32WriteLen);

        while (HI_TRUE == pstVdecCtx->bThreadStart && (HI_SUCCESS != HI_MPI_VDEC_SendStream(pstVdecCtx->VdecChn, &stStream, HI_IO_NOBLOCK)))
        {
            usleep(10000);
        }
        //memset(pu8Addr, 0, u32Len);
        /* release data to pciv stream receiver */

        PCIV_Trans_ReleaseData(pReceiver, pu8Addr, u32Len);
    }

    pstVdecCtx->bThreadStart = HI_FALSE;
    return NULL;
}

/* get stream from venc chn, write to local buffer, then send to pci target,
    we send several stream frame one time, to improve PCI DMA efficiency  */
HI_VOID * SamplePciv_SendVencThread(HI_VOID *p)
{
    HI_S32           s32Ret, i, k;
    HI_S32           s32StreamCnt;
    HI_BOOL          bBufFull;
    VENC_CHN         vencChn = 0, VeChnBufFul = 0;
    HI_S32           s32Size, DMADataLen;
    VENC_STREAM_S    stVStream;
    //VENC_PACK_S      astPack[128];
    VENC_PACK_S      astPack[64];
	HI_VOID         *pCreator    = NULL;
    PCIV_STREAM_HEAD_S      stHeadTmp;
    PCIV_TRANS_LOCBUF_STAT_S stLocBufSta;
    SAMPLE_PCIV_VENC_CTX_S *pstCtx = (SAMPLE_PCIV_VENC_CTX_S*)p;

    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];

#if SLAVE_SAVE_STREAM
    //HI_CHAR aszFileName[64] = {0};
    //static FILE *pFile[VENC_MAX_CHN_NUM] = {NULL};
#endif

    for (i=0; i<pstCtx->s32VencCnt; i++)
    {
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        HI_ASSERT(VencFd[i] > 0);
        if (maxfd <= VencFd[i]) maxfd = VencFd[i];
    }

    pCreator     = pstCtx->pTransHandle;
    s32StreamCnt = 0;
    bBufFull     = HI_FALSE;
    stVStream.pstPack = astPack;
    printf("%s -> Sender:%p, chncnt:%d\n", __FUNCTION__, pCreator, pstCtx->s32VencCnt);
    prctl(PR_SET_NAME, (unsigned long)"hi_SendStrm", 0, 0, 0);

    while (pstCtx->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i=0; i<pstCtx->s32VencCnt; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 10;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (!s32Ret) {printf("timeout\n"); continue;} //conditionl is not all
        HI_ASSERT(s32Ret >= 0);

        for (i=0; i<pstCtx->s32VencCnt; i++)
        {
            vencChn = i;

            /* if buf is not full last time, get venc stream from venc chn by noblock method*/
            if (HI_FALSE == bBufFull)
            {
                VENC_CHN_STATUS_S stVencStat;
                if (!FD_ISSET(VencFd[vencChn], &read_fds))
                    continue;
                memset(&stVStream, 0, sizeof(stVStream));
                s32Ret = HI_MPI_VENC_QueryStatus(vencChn, &stVencStat);
                HI_ASSERT(HI_SUCCESS == s32Ret);

                stVStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stVencStat.u32CurPacks);
                HI_ASSERT(stVStream.pstPack);
                stVStream.u32PackCount = stVencStat.u32CurPacks;
                //s32Ret = HI_MPI_VENC_GetStream(vencChn, &stVStream, HI_IO_BLOCK);
                s32Ret = HI_MPI_VENC_GetStream(vencChn,&stVStream,-1);
                HI_ASSERT(HI_SUCCESS == s32Ret);

                s32StreamCnt++;
                TraceStreamInfo(vencChn, &stVStream);/* only for DEBUG */
            }
            else/* else, use stVStream of last time */
            {
                bBufFull = HI_FALSE;
                vencChn = VeChnBufFul;
            }
            /* Caclute the data size, and check there is enough space in local buffer */
            s32Size = 0;
            for (k=0; k<stVStream.u32PackCount; k++)
            {
            	s32Size += stVStream.pstPack[k].u32Len;
            }
            if (0 == (s32Size%4))
            {
                DMADataLen = s32Size;
            }
            else
            {
                DMADataLen = s32Size + (4 - (s32Size%4));
            }

            PCIV_Trans_QueryLocBuf(pCreator, &stLocBufSta);
            if (stLocBufSta.u32FreeLen < DMADataLen + sizeof(PCIV_STREAM_HEAD_S))
            {
                printf("venc stream local buffer not enough,chn:%d, %d < %d+%ld \n",
                    vencChn, stLocBufSta.u32FreeLen, DMADataLen, sizeof(PCIV_STREAM_HEAD_S));
                bBufFull = HI_TRUE;
                VeChnBufFul = vencChn;
                break;
            }

            /* fill stream header info */
            stHeadTmp.u32Magic  = PCIV_STREAM_MAGIC;
            stHeadTmp.enPayLoad = PT_H264;
            stHeadTmp.s32ChnID   = vencChn;
            stHeadTmp.u32StreamDataLen = s32Size;
            stHeadTmp.u32DMADataLen = DMADataLen;
            stHeadTmp.u32Seq    = stVStream.u32Seq;
            stHeadTmp.u64PTS    = stVStream.pstPack[0].u64PTS;
            //stHeadTmp.bFieldEnd = stVStream.pstPack[0].bFieldEnd;
            stHeadTmp.bFrameEnd = stVStream.pstPack[0].bFrameEnd;
            stHeadTmp.enDataType= stVStream.pstPack[0].DataType;

            /* write stream header */
            s32Ret = PCIV_Trans_WriteLocBuf(pCreator, (HI_U8*)&stHeadTmp, sizeof(stHeadTmp));
            HI_ASSERT((HI_SUCCESS == s32Ret));

            /* write stream data */
            for (k=0; k<stVStream.u32PackCount; k++)
            {

                s32Ret = PCIV_Trans_WriteLocBuf(pCreator,
                		stVStream.pstPack[k].pu8Addr, stVStream.pstPack[k].u32Len);
				HI_ASSERT((HI_SUCCESS == s32Ret));

            }

            PCIV_TRANS_SENDER_S *pstSender = (PCIV_TRANS_SENDER_S*)pCreator;
            if (0 != pstSender->stLocBuf.u32CurLen%4)
            {
                pstSender->stLocBuf.u32CurLen += (4 - (pstSender->stLocBuf.u32CurLen%4));
            }
            s32Ret = HI_MPI_VENC_ReleaseStream(vencChn, &stVStream);
            HI_ASSERT((HI_SUCCESS == s32Ret));

            free(stVStream.pstPack);
            stVStream.pstPack = NULL;
        }

        /* while writed sufficient stream frame or buffer is full, send local data to pci target */
        if ((s32StreamCnt >= PCIV_FRMNUM_ONCEDMA)|| (bBufFull == HI_TRUE))
        {
            int times = 0;
            while (PCIV_Trans_SendData(pCreator) && pstCtx->bThreadStart)
            {
                usleep(1);
                /* The more failure times ,with greater delay the receiver get the data*/
                if (++times >=1) printf("PCIV_Trans_SendData, times:%d\n", times);
            }
            s32StreamCnt = 0;/* reset stream count after send stream to remote chip successed */
        }
        //usleep(0);
    }

    pstCtx->bThreadStart = HI_FALSE;
    return NULL;
}

HI_S32 SamplePciv_SlaveStartVdecStream(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    PCIV_TRANS_ATTR_S *pstTransAttr = (PCIV_TRANS_ATTR_S*)pMsg->cMsgBody;
    SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = &g_astSamplePcivVdec[pstTransAttr->s32ChnId];

    /* msg port should have open when SamplePciv_SlaveInitPort() */
    HI_ASSERT(pstTransAttr->s32MsgPortWrite == pstVdecCtx->s32MsgPortWrite);
    HI_ASSERT(pstTransAttr->s32MsgPortRead  == pstVdecCtx->s32MsgPortRead);

    /* init vdec stream receiver */
    pstTransAttr->u64PhyAddr += g_u64PfAhbBase;/* NOTE:phyaddr in msg is a offset */
    s32Ret = PCIV_Trans_InitReceiver(pstTransAttr, &pstVdecCtx->pTransHandle);
    PCIV_CHECK_ERR(s32Ret);

    pstVdecCtx->bThreadStart = HI_TRUE;
    pstVdecCtx->VdecChn = pstTransAttr->s32ChnId;
    /* create thread to get stream coming from host chip, and send stream to decoder */
    pthread_create(&pstVdecCtx->pid, NULL, SamplePcivVdStreamThread, pstVdecCtx);

    printf("init vdec:%d stream receiver in slave chip ok!\n", pstTransAttr->s32ChnId);
    return HI_SUCCESS;
}
HI_S32 SamplePciv_SlaveStopVdecStream(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    PCIV_TRANS_ATTR_S *pstDeInitCmd = (PCIV_TRANS_ATTR_S*)pMsg->cMsgBody;
    SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = &g_astSamplePcivVdec[pstDeInitCmd->s32ChnId];

    /* exit thread*/
    if (HI_TRUE == pstVdecCtx->bThreadStart)
    {
        pstVdecCtx->bThreadStart = HI_FALSE;
        pthread_join(pstVdecCtx->pid, 0);
    }

    /* eixt vdec stream receiver */
    s32Ret = PCIV_Trans_DeInitReceiver(pstVdecCtx->pTransHandle);
    PCIV_CHECK_ERR(s32Ret);

    printf("exit vdec:%d stream receiver in slave chip ok!\n", pstVdecCtx->VdecChn);
    return HI_SUCCESS;
}

HI_S32 SamplePcivLoadRgnBmp(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if(GetBmpInfo(filename,&bmpFileHeader,&bmpInfo) < 0)
    {
		printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;

    pstBitmap->pData = malloc(2*(bmpInfo.bmiHeader.biWidth)*(bmpInfo.bmiHeader.biHeight));

    if(NULL == pstBitmap->pData)
    {
        printf("malloc osd memroy err!\n");
        return HI_FAILURE;
    }
    CreateSurfaceByBitMap(filename,&Surface,(HI_U8*)(pstBitmap->pData));

    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    pstBitmap->enPixelFormat = PIXEL_FORMAT_ARGB_1555;

    int i,j;
    HI_U16 *pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;

    if (bFil)
    {
        for (i=0; i<pstBitmap->u32Height; i++)
        {
            for (j=0; j<pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }
                pu16Temp++;
            }
        }

    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivChnCreateRegion(PCIV_CHN PcivChn)
{
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;
    BITMAP_S stBitmap;

    /* creat region*/
    stRgnAttr.enType = OVERLAYEX_RGN;
    stRgnAttr.unAttr.stOverlayEx.enPixelFmt = PIXEL_FORMAT_ARGB_1555;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Width  = 128;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = 128;
    stRgnAttr.unAttr.stOverlayEx.u32BgColor = 0xfc;

    s32Ret = HI_MPI_RGN_Create(PcivChn, &stRgnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("region of pciv chn %d create fail. value=0x%x.", PcivChn, s32Ret);
        return s32Ret;
    }

    /*attach region to chn*/
    stChn.enModId = HI_ID_PCIV;
    stChn.s32DevId = 0;
    stChn.s32ChnId = PcivChn;

    stChnAttr.bShow = HI_TRUE;
    stChnAttr.enType = OVERLAYEX_RGN;
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 128;
    stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 128;
    stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 128;
    stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 128;
    stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = 0;

    /* load bitmap*/
    s32Ret = SamplePcivLoadRgnBmp("mm2.bmp", &stBitmap, HI_FALSE, 0);
	if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    s32Ret = HI_MPI_RGN_SetBitMap(PcivChn, &stBitmap);
    if (s32Ret != HI_SUCCESS)
    {
        printf("region set bitmap to  pciv chn %d fail. value=0x%x.", PcivChn, s32Ret);
		free(stBitmap.pData);
        return s32Ret;
    }
    free(stBitmap.pData);

    s32Ret = HI_MPI_RGN_AttachToChn(PcivChn, &stChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("region attach to  pciv chn %d fail. value=0x%x.", PcivChn, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivChnDestroyRegion(PCIV_CHN PcivChn)
{
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    stChn.enModId = HI_ID_PCIV;
    stChn.s32DevId = 0;
    stChn.s32ChnId = PcivChn;
    /* detach region from chn */
    s32Ret = HI_MPI_RGN_DetachFromChn(PcivChn, &stChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("region attach to  pciv chn %d fail. value=0x%x.", PcivChn, s32Ret);
        return s32Ret;
    }

    /* destroy region */
    s32Ret = HI_MPI_RGN_Destroy(PcivChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("destroy  pciv chn %d region fail. value=0x%x.", PcivChn, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivStartVenc(PCIV_VENCCMD_INIT_S *pstMsgCreate)
{
    HI_S32                  s32Ret;
    VENC_GOP_ATTR_S         stGopAttr;
    PCIV_TRANS_ATTR_S       *pstStreamArgs = &pstMsgCreate->stStreamArgs;
    SAMPLE_PCIV_VENC_CTX_S  *pstVencCtx = &g_stSamplePcivVenc;

    /* create group and venc chn */
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = SAMPLE_COMM_VENC_Start(0, pstMsgCreate->aenType[0], pstMsgCreate->aenSize[0], SAMPLE_RC_CBR,0,&stGopAttr);
    PCIV_CHECK_ERR(s32Ret);

    /* msg port should have open when SamplePciv_SlaveInitPort() */
    HI_ASSERT(pstStreamArgs->s32MsgPortWrite == pstVencCtx->s32MsgPortWrite);
    HI_ASSERT(pstStreamArgs->s32MsgPortRead == pstVencCtx->s32MsgPortRead);

    /* init stream creater  */
    pstStreamArgs->s32RmtChip = 0;
    s32Ret = PCIV_Trans_InitSender(pstStreamArgs, &pstVencCtx->pTransHandle);
    PCIV_CHECK_ERR(s32Ret);

    pstVencCtx->bThreadStart = HI_TRUE;
	pstVencCtx->s32VencCnt = pstMsgCreate->u32GrpCnt;
    s32Ret = pthread_create(&pstVencCtx->pid, NULL, SamplePciv_SendVencThread, pstVencCtx);
    return s32Ret;
}

HI_S32 SamplePciv_SlaveStartVenc(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    PCIV_VENCCMD_INIT_S *pstMsgCreate = (PCIV_VENCCMD_INIT_S *)pMsg->cMsgBody;

    s32Ret = SamplePcivStartVenc(pstMsgCreate);
    return s32Ret;
}

HI_S32 SamplePcivStopVenc(PCIV_VENCCMD_INIT_S *pstMsgCreate)
{
    HI_S32 i,s32Ret;
    SAMPLE_PCIV_VENC_CTX_S *pstVencCtx = &g_stSamplePcivVenc;
    MPP_CHN_S stBindSrc;
    MPP_CHN_S stBindDest;

    /* exit the thread of sending stream */
    if (pstVencCtx->bThreadStart)
    {
        pstVencCtx->bThreadStart = HI_FALSE;
        pthread_join(pstVencCtx->pid, 0);
    }

    /* destroy all venc chn */
    s32Ret = SAMPLE_COMM_VENC_Stop(0);
    PCIV_CHECK_ERR(s32Ret);

    /* Unbind */
    for (i=0; i<pstMsgCreate->u32GrpCnt; i++)
    {

        stBindSrc.enModId = HI_ID_VI;
        stBindSrc.s32DevId = i;
        stBindSrc.s32ChnId = 4*i;

        stBindDest.enModId = HI_ID_VENC;
        stBindDest.s32DevId = 0;
        stBindDest.s32ChnId = i;

        s32Ret = HI_MPI_SYS_UnBind(&stBindSrc, &stBindDest);
        if (s32Ret != HI_SUCCESS)
        {
            printf("vi bind venc err 0x%x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    /* exit the transfer sender */
    PCIV_Trans_DeInitSender(pstVencCtx->pTransHandle);

    printf("venc exit ok, grp cnt :%d===========================\n",pstMsgCreate->u32GrpCnt);
    return HI_SUCCESS;


}

HI_S32 SamplePciv_SlaveStopVenc(SAMPLE_PCIV_MSG_S *pMsg)	//
{
    HI_S32 s32Ret;
    PCIV_VENCCMD_INIT_S *pstMsgCreate = (PCIV_VENCCMD_INIT_S *)pMsg->cMsgBody;

    s32Ret = SamplePcivStopVenc(pstMsgCreate);

    return s32Ret;
}

HI_S32 SamplePcivStartPciv(PCIV_PCIVCMD_CREATE_S *pstMsgCreate)
{
    HI_S32 s32Ret;
    PCIV_CHN PcivChn = pstMsgCreate->pcivChn;
    PCIV_ATTR_S *pstPicvAttr = &pstMsgCreate->stDevAttr;

    printf("PcivChn:%d PCIV_ADD_OSD is %d.\n",PcivChn,pstMsgCreate->bAddOsd);

    /* 1) create pciv chn */
    s32Ret = HI_MPI_PCIV_Create(PcivChn, pstPicvAttr);
    PCIV_CHECK_ERR(s32Ret);

    /* 2) create region for pciv chn */
    if (1 == pstMsgCreate->bAddOsd)
    {
        s32Ret = SamplePcivChnCreateRegion(PcivChn);
        if (s32Ret != HI_SUCCESS)
        {
            printf("pciv chn %d SamplePcivChnCreateRegion err, value = 0x%x. \n", PcivChn, s32Ret);
            return s32Ret;
        }
    }

    /* 3) start pciv chn */
    s32Ret = HI_MPI_PCIV_Start(PcivChn);
    PCIV_CHECK_ERR(s32Ret);

    printf("slave start pciv chn %d ok, remote chn:%d=========\n",
        PcivChn,pstPicvAttr->stRemoteObj.pcivChn);
    return s32Ret;
}

HI_S32 SamplePciv_SlaveStartPciv(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    PCIV_PCIVCMD_CREATE_S *pstMsgCreate = (PCIV_PCIVCMD_CREATE_S *)pMsg->cMsgBody;

    s32Ret = SamplePcivStartPciv(pstMsgCreate);

    return s32Ret;
}

HI_S32 SamplePcivStopPciv(PCIV_PCIVCMD_DESTROY_S *pstMsgDestroy)
{
    HI_S32 s32Ret;
    PCIV_CHN pcivChn = pstMsgDestroy->pcivChn;

    s32Ret = HI_MPI_PCIV_Stop(pcivChn);
    PCIV_CHECK_ERR(s32Ret);

    if (1 == pstMsgDestroy->bAddOsd)
    {
        s32Ret = SamplePcivChnDestroyRegion(pcivChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    s32Ret = HI_MPI_PCIV_Destroy(pcivChn);
    PCIV_CHECK_ERR(s32Ret);

    printf("pciv chn %d destroy ok \n", pcivChn);
    return s32Ret;
}

HI_S32 SamplePciv_SlaveStopPicv(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    PCIV_PCIVCMD_DESTROY_S *pstMsgDestroy = (PCIV_PCIVCMD_DESTROY_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivStopPciv(pstMsgDestroy);

    return s32Ret;
}

HI_S32 SamplePcivStopVdec(SAMPLE_PCIV_MSG_VDEC_S *pstVdecArgs)
{
    HI_S32              i;
    HI_S32              u32VdecChnNum;
    SAMPLE_VDEC_ATTR    astSampleVdec[VDEC_MAX_CHN_NUM];
    //VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];

    u32VdecChnNum =     pstVdecArgs->u32VdecChnNum;

   for(i=0; i<u32VdecChnNum; i++)
    {
		snprintf(stVdecSend[i].cFileName, sizeof(stVdecSend[i].cFileName), "3840x2160_10bit.h265");
        snprintf(stVdecSend[i].cFilePath, sizeof(stVdecSend[i].cFilePath), "%s", SAMPLE_STREAM_PATH);
        stVdecSend[i].enType          = astSampleVdec[i].enType;
        stVdecSend[i].s32StreamMode   = astSampleVdec[i].enMode;
        stVdecSend[i].s32ChnId        = i;
        stVdecSend[i].s32IntervalTime = 1000;
        stVdecSend[i].u64PtsInit      = 0;
        stVdecSend[i].u64PtsIncrease  = 0;
        stVdecSend[i].eThreadCtrl     = THREAD_CTRL_STOP;
        stVdecSend[i].bCircleSend     = HI_TRUE;
        stVdecSend[i].s32MilliSec     = 0;
        stVdecSend[i].s32MinBufSize   = (astSampleVdec[i].u32Width * astSampleVdec[i].u32Height * 3)>>1;
    }

    SAMPLE_COMM_VDEC_Stop(u32VdecChnNum);

    SAMPLE_COMM_VDEC_StopSendStream(u32VdecChnNum, &stVdecSend[0], &VdecSlaveThread[0]);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveStopVdec(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 S32Ret;
    SAMPLE_PCIV_MSG_VDEC_S *pstVdecArgs = (SAMPLE_PCIV_MSG_VDEC_S*)pMsg->cMsgBody;
    S32Ret = SamplePcivStopVdec(pstVdecArgs);

    return S32Ret;
}

HI_S32 SamplePcivStartVdec(SAMPLE_PCIV_MSG_VDEC_S *pstVdecArgs)
{

    HI_S32              s32Ret;
    HI_S32              i;
    HI_U32              u32VdecChnNum;
    SAMPLE_VDEC_ATTR    astSampleVdec[VDEC_MAX_CHN_NUM];
    //VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];
    SIZE_S              stSize;
    PIC_SIZE_E          enPicSize;

    enPicSize = pstVdecArgs->stVdecMode.enPicSize;
    SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    u32VdecChnNum =     pstVdecArgs->u32VdecChnNum;

    for(i=0; i<u32VdecChnNum; i++)
    {
	    astSampleVdec[i].enType   						  = pstVdecArgs->stVdecMode.enPayLoadType;
		astSampleVdec[i].u32Width 						  = stSize.u32Width;
		astSampleVdec[i].u32Height						  = stSize.u32Height;
		astSampleVdec[i].enMode   						  = VIDEO_MODE_FRAME;
		astSampleVdec[i].stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IPB;
		astSampleVdec[i].stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;
		astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum = 3;
		astSampleVdec[i].u32DisplayFrameNum				  = 2;
		astSampleVdec[i].u32FrameBufCnt = astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + astSampleVdec[i].u32DisplayFrameNum + 1;
        if(pstVdecArgs->stVdecMode.bReciveStream)
        {
            astSampleVdec[i].enMode   					  = VIDEO_MODE_STREAM;
        }
    }
	s32Ret = SAMPLE_COMM_VDEC_InitVBPool(u32VdecChnNum, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        HI_PRINT("SAMPLE COMM VDEC InitVBPool fail for %#x!\n", s32Ret);
    }
    /*Start VDEC*/
    s32Ret = SAMPLE_COMM_VDEC_Start(u32VdecChnNum, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        HI_PRINT("start VDEC fail for %#x!\n", s32Ret);

    }

    for (i = 0; i < u32VdecChnNum; i++)
    {
        s32Ret = HI_MPI_VDEC_SetDisplayMode(i, pstVdecArgs->stVdecMode.enDisplayMode);
        if (s32Ret != HI_SUCCESS)
        {
            HI_PRINT("start VDEC fail for %#x!\n", s32Ret);

        }

    }

    if(HI_TRUE == pstVdecArgs->stVdecMode.bReadStream)
    {
        for(i=0; i<u32VdecChnNum; i++)
        {
    		snprintf(stVdecSend[i].cFileName, sizeof(stVdecSend[i].cFileName), "3840x2160_8bit.h264");
            snprintf(stVdecSend[i].cFilePath, sizeof(stVdecSend[i].cFilePath), "%s", SAMPLE_STREAM_PATH);
            stVdecSend[i].enType          = astSampleVdec[i].enType;
            stVdecSend[i].s32StreamMode   = astSampleVdec[i].enMode;
            stVdecSend[i].s32ChnId        = i;
            stVdecSend[i].s32IntervalTime = 1000;
            stVdecSend[i].u64PtsInit      = 0;
            stVdecSend[i].u64PtsIncrease  = 0;
            stVdecSend[i].eThreadCtrl     = THREAD_CTRL_START;
            stVdecSend[i].bCircleSend     = HI_TRUE;
            stVdecSend[i].s32MilliSec     = 0;
            stVdecSend[i].s32MinBufSize   = (astSampleVdec[i].u32Width * astSampleVdec[i].u32Height * 3)>>1;

        }

        SAMPLE_COMM_VDEC_StartSendStream(u32VdecChnNum, &stVdecSend[0], &VdecSlaveThread[0]);
    }
    return s32Ret;
}

HI_S32 SamplePciv_SlaveStartVdec(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_VDEC_S *pstVdecArgs = (SAMPLE_PCIV_MSG_VDEC_S*)pMsg->cMsgBody;
    s32Ret = SamplePcivStartVdec(pstVdecArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
    return HI_SUCCESS;
}


HI_S32 SamplePcivStartVpss(SAMPLE_PCIV_MSG_VPSS_S *pstVpssArgs)
{
    HI_S32 s32Ret;
	SIZE_S stSize;
    HI_S32 iLoop;
    HI_BOOL         abChnEnable[4] = {0,0,0,0};
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    PIC_SIZE_E      enPicSize;

    enPicSize = pstVpssArgs->stPicVbAttr.enDispPicSize;
    SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    HI_U32 vpssGrpCnt = pstVpssArgs->stVpssMode.s32VpssGroupCnt;
    HI_U32 vpssChnCnt = pstVpssArgs->stVpssMode.s32VpssChnCnt;

    for (iLoop=0;iLoop<vpssChnCnt;iLoop++)
    {
        abChnEnable[iLoop] = HI_TRUE;
    }
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = pstVpssArgs->stPicVbAttr.enPixelFormat;
    stVpssGrpAttr.u32MaxW        = stSize.u32Width;
    stVpssGrpAttr.u32MaxH        = stSize.u32Height;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;

    for(iLoop=0; iLoop<VPSS_MAX_PHY_CHN_NUM; iLoop++)
    {
        if(HI_TRUE == abChnEnable[iLoop])
        {
            stVpssChnAttr[iLoop].u32Width                     = stSize.u32Width;
            stVpssChnAttr[iLoop].u32Height                    = stSize.u32Height;
            stVpssChnAttr[iLoop].enChnMode                    = pstVpssArgs->stVpssMode.enChnMode;
            stVpssChnAttr[iLoop].enCompressMode               = pstVpssArgs->stPicVbAttr.enCompressMode;
            stVpssChnAttr[iLoop].enDynamicRange               = DYNAMIC_RANGE_SDR8;
            stVpssChnAttr[iLoop].enPixelFormat                = pstVpssArgs->stPicVbAttr.enPixelFormat;
            stVpssChnAttr[iLoop].stFrameRate.s32SrcFrameRate  = -1;
            stVpssChnAttr[iLoop].stFrameRate.s32DstFrameRate  = -1;
            stVpssChnAttr[iLoop].u32Depth                     = 0;
            stVpssChnAttr[iLoop].bMirror                      = HI_FALSE;
            stVpssChnAttr[iLoop].bFlip                        = HI_FALSE;
            stVpssChnAttr[iLoop].enVideoFormat                = pstVpssArgs->stPicVbAttr.enVideoFormat;
            stVpssChnAttr[iLoop].stAspectRatio.enMode         = ASPECT_RATIO_NONE;
        }

    }
    for (iLoop =0;iLoop< vpssGrpCnt;iLoop++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Start(iLoop, abChnEnable, &stVpssGrpAttr, stVpssChnAttr);

        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}
HI_S32 SamplePcivStopVpss(SAMPLE_PCIV_MSG_VPSS_S *pstVpssArgs)
{
    HI_S32 iLoop, s32Ret;
    HI_S32 vpssGrpCnt;
    HI_S32 vpssChnCnt;

    HI_BOOL         abChnEnable[4] = {0,0,0,0};

    vpssGrpCnt = pstVpssArgs->stVpssMode.s32VpssGroupCnt;
    vpssChnCnt = pstVpssArgs->stVpssMode.s32VpssChnCnt;

    for (iLoop=0;iLoop<vpssChnCnt;iLoop++)
    {
        abChnEnable[iLoop] = HI_TRUE;
    }

    for (iLoop = 0; iLoop < vpssGrpCnt; iLoop++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Stop(iLoop, abChnEnable);
        if (HI_SUCCESS != s32Ret)
        {
            printf("VPSS stop error, value= 0x%x.\n", s32Ret);
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveStartVpss(SAMPLE_PCIV_MSG_S *pMsg)
{

    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_VPSS_S *pstVpssArgs = (SAMPLE_PCIV_MSG_VPSS_S*)pMsg->cMsgBody;
    s32Ret = SamplePcivStartVpss(pstVpssArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}
HI_S32 SamplePciv_SlaveStopVpss(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_VPSS_S *pstVpssArgs = (SAMPLE_PCIV_MSG_VPSS_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivStopVpss(pstVpssArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivInitVi(SAMPLE_PCIV_MSG_INIT_VI_S *pstViArgs)
{
    HI_S32             s32Ret;
    VI_DEV             ViDev               = 0;
    VI_PIPE            ViPipe              = 0;
    VI_CHN             ViChn               = 0;
    HI_S32             s32SnsId            = 0;
    PIC_SIZE_E         enPicSize           = PIC_3840x2160;

    /************************************************
    step1:  Get all sensors information
    *************************************************/
    SAMPLE_COMM_VI_GetSensorInfo(&g_stViConfig);

    g_stViConfig.s32WorkingViNum                                  = 1;

    g_stViConfig.as32WorkingViId[0]                               = s32SnsId;

    g_stViConfig.astViInfo[s32SnsId].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(g_stViConfig.astViInfo[s32SnsId].stSnsInfo.enSnsType, s32SnsId);
    g_stViConfig.astViInfo[s32SnsId].stSnsInfo.s32BusId           = 0;

    g_stViConfig.astViInfo[s32SnsId].stDevInfo.ViDev              = ViDev;
    g_stViConfig.astViInfo[s32SnsId].stDevInfo.enWDRMode          = WDR_MODE_NONE;

    g_stViConfig.astViInfo[s32SnsId].stPipeInfo.enMastPipeMode    = VI_ONLINE_VPSS_ONLINE;
    g_stViConfig.astViInfo[s32SnsId].stPipeInfo.aPipe[0]          = ViPipe;
    g_stViConfig.astViInfo[s32SnsId].stPipeInfo.aPipe[1]          = -1;
    g_stViConfig.astViInfo[s32SnsId].stPipeInfo.aPipe[2]          = -1;
    g_stViConfig.astViInfo[s32SnsId].stPipeInfo.aPipe[3]          = -1;

    g_stViConfig.astViInfo[s32SnsId].stChnInfo.ViChn              = ViChn;
    g_stViConfig.astViInfo[s32SnsId].stChnInfo.enPixFormat        = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    g_stViConfig.astViInfo[s32SnsId].stChnInfo.enDynamicRange     = DYNAMIC_RANGE_SDR8;
    g_stViConfig.astViInfo[s32SnsId].stChnInfo.enVideoFormat      = VIDEO_FORMAT_LINEAR;
    g_stViConfig.astViInfo[s32SnsId].stChnInfo.enCompressMode     = COMPRESS_MODE_NONE;

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(g_stViConfig.astViInfo[s32SnsId].stSnsInfo.enSnsType, &enPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VI_GetSizeBySensor failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(&g_stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VI_SetParam failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SamplePcivStopVi(SAMPLE_PCIV_MSG_EXIT_VI_S *pstViArgs)
{
     HI_S32 s32Ret;
     s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViConfig);
     if (HI_SUCCESS != s32Ret)
     {
        printf("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
     }
     return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveInitVi(SAMPLE_PCIV_MSG_S *pMsg)
{
	HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_INIT_VI_S *pstViArgs = (SAMPLE_PCIV_MSG_INIT_VI_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivInitVi(pstViArgs);
	if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveStopVi(SAMPLE_PCIV_MSG_S *pMsg)
{
	HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_EXIT_VI_S *pstViArgs = (SAMPLE_PCIV_MSG_EXIT_VI_S*)pMsg->cMsgBody;

	s32Ret = SamplePcivStopVi(pstViArgs);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    printf("stop all vi ok!\n");
    return HI_SUCCESS;
}

HI_S32 SamplePcivInitPort(PCIV_MSGPORT_INIT_S *pstMsgPort)
{
    HI_S32 s32Ret, i;

    g_stSamplePcivVenc.s32MsgPortWrite = pstMsgPort->s32VencMsgPortW;
    g_stSamplePcivVenc.s32MsgPortRead  = pstMsgPort->s32VencMsgPortR;
    s32Ret  = PCIV_OpenMsgPort(0, g_stSamplePcivVenc.s32MsgPortWrite);
    s32Ret |= PCIV_OpenMsgPort(0, g_stSamplePcivVenc.s32MsgPortRead);
    HI_ASSERT(HI_SUCCESS == s32Ret);


    for (i=0; i<VDEC_MAX_CHN_NUM; i++)
    {

        g_astSamplePcivVdec[i].s32MsgPortWrite  = pstMsgPort->s32VdecMsgPortW[i];
        g_astSamplePcivVdec[i].s32MsgPortRead   = pstMsgPort->s32VdecMsgPortR[i];
        s32Ret  = PCIV_OpenMsgPort(0, g_astSamplePcivVdec[i].s32MsgPortWrite);
        s32Ret |= PCIV_OpenMsgPort(0, g_astSamplePcivVdec[i].s32MsgPortRead);
        HI_ASSERT(HI_SUCCESS == s32Ret);
    }

    return s32Ret;
}

HI_S32 SamplePciv_SlaveInitPort(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    PCIV_MSGPORT_INIT_S *pstMsgPort = (PCIV_MSGPORT_INIT_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivInitPort(pstMsgPort);

    return s32Ret;
}

HI_S32 SamplePcivExitPort(HI_VOID)
{
    HI_S32 i;
    PCIV_CloseMsgPort(0, g_stSamplePcivVenc.s32MsgPortWrite);
    PCIV_CloseMsgPort(0, g_stSamplePcivVenc.s32MsgPortRead);
    for (i=0; i<VDEC_MAX_CHN_NUM; i++)
    {
        PCIV_CloseMsgPort(0, g_astSamplePcivVdec[i].s32MsgPortWrite);
        PCIV_CloseMsgPort(0, g_astSamplePcivVdec[i].s32MsgPortRead);
    }
	printf("Slave chip close port ok!\n");

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveExitPort(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;

    s32Ret = SamplePcivExitPort();

    return s32Ret;
}

HI_S32 SamplePcivBindSrcDest(SAMPLE_PCIV_MSG_BIND_S *pstBindArgs)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = pstBindArgs->SrcMod.enModId;
    stSrcChn.s32DevId = pstBindArgs->SrcMod.s32DevId;
    stSrcChn.s32ChnId = pstBindArgs->SrcMod.s32ChnId;

    stDestChn.enModId = pstBindArgs->DestMod.enModId;
    stDestChn.s32DevId = pstBindArgs->DestMod.s32DevId;
    stDestChn.s32ChnId = pstBindArgs->DestMod.s32ChnId;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivUnBindSrcDest(SAMPLE_PCIV_MSG_UNBIND_S *pstUnBindArgs)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId   = pstUnBindArgs->SrcMod.enModId;
    stSrcChn.s32DevId  = pstUnBindArgs->SrcMod.s32DevId;
    stSrcChn.s32ChnId  = pstUnBindArgs->SrcMod.s32ChnId;

    stDestChn.enModId  = pstUnBindArgs->DestMod.enModId;
    stDestChn.s32DevId = pstUnBindArgs->DestMod.s32DevId;
    stDestChn.s32ChnId = pstUnBindArgs->DestMod.s32ChnId;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveBindSrcDest(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_BIND_S *pstBindArgs = (SAMPLE_PCIV_MSG_BIND_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivBindSrcDest(pstBindArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveUnBindSrcDest(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_UNBIND_S *pstBindArgs = (SAMPLE_PCIV_MSG_UNBIND_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivUnBindSrcDest(pstBindArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

#if 0
HI_S32 SamplePcivStartVirtualVo(SAMPLE_PCIV_MSG_VO_S *pstVoArgs)
{
    HI_S32 i;
    HI_S32 s32Ret;
    HI_S32 s32DispNum;
    VO_LAYER VoLayer;
    VO_PUB_ATTR_S stPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CHN_ATTR_S astChnAttr[16];
    VO_DEV VirtualVo;
    HI_S32 u32VoChnNum;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    VO_INTF_SYNC_E enIntfSync;

    VirtualVo   = pstVoArgs->VirtualVo;
    u32VoChnNum = pstVoArgs->s32VoChnNum;
    enIntfSync  = pstVoArgs->enIntfSync;

    s32DispNum = SamplePcivGetVoDisplayNum(u32VoChnNum);
    if(s32DispNum < 0)
    {
        printf("SAMPLE_RGN_GetVoDisplayNum failed! u32VoChnNum: %d.\n", u32VoChnNum);
        return HI_FAILURE;
    }
    printf("Func: %s, line: %d, u32VoChnNum: %d, s32DispNum: %d\n", __FUNCTION__, __LINE__, u32VoChnNum, s32DispNum);

    s32Ret = SamplePcivGetVoAttr(VirtualVo, enIntfSync, &stPubAttr, &stLayerAttr, s32DispNum, astChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_GetVoAttr failed!\n");
        return HI_FAILURE;
    }

    VoLayer = SamplePcivGetVoLayer(VirtualVo);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VirtualVo);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Disable(VirtualVo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Disable failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_SetPubAttr(VirtualVo, &stPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_SetPubAttr failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_Enable(VirtualVo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Enable failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }
    //printf("VO dev:%d enable ok \n", VoDev);

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_SetVideoLayerAttr failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_EnableVideoLayer failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    //printf("VO video layer:%d enable ok \n", VoDev);

    for (i = 0; i < u32VoChnNum; i++)
    {
        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &astChnAttr[i]);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_SetChnAttr failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_EnableChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stSrcChn.enModId   = pstVoArgs->stBInd.enModId;
        stSrcChn.s32DevId  = pstVoArgs->stBInd.s32DevId;
        stSrcChn.s32ChnId  = i;

        stDestChn.enModId  = HI_ID_VO;
        stDestChn.s32DevId = VoLayer;
        stDestChn.s32ChnId = i;

        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
        //printf("VO chn:%d enable ok \n", i);
    }

    //printf("VO: %d enable ok!\n", VoDev);

    return HI_SUCCESS;
}
#endif



HI_S32 SamplePcivStopVirtualVo(SAMPLE_PCIV_MSG_VO_S *pstVoArgs)
{
    #if 0
    HI_S32 i, s32Ret;
    VO_DEV VirtualVo;
    HI_S32 s32ChnNum;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    VO_LAYER VoLayer;

    VirtualVo = pstVoArgs->VirtualVo;
    s32ChnNum = pstVoArgs->s32VoChnNum;


    VoLayer = SamplePcivGetVoLayer(VirtualVo);
    if(VoLayer < 0)
    {
        printf("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VirtualVo);
        return HI_FAILURE;
    }
    for (i=0; i<s32ChnNum; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
        PCIV_CHECK_ERR(s32Ret);

        stSrcChn.enModId   = pstVoArgs->stBInd.enModId;
        stSrcChn.s32DevId  = pstVoArgs->stBInd.s32DevId;
        stSrcChn.s32ChnId  = i;

        stDestChn.enModId  = HI_ID_VO;
        stDestChn.s32DevId = VoLayer;
        stDestChn.s32ChnId = i;

        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);

    }

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_Disable(VirtualVo);
    PCIV_CHECK_ERR(s32Ret);
    #endif
    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveStartVo(SAMPLE_PCIV_MSG_S *pMsg)
{
    //HI_S32 s32Ret;
    //SAMPLE_PCIV_MSG_VO_S *pstVoArgs = (SAMPLE_PCIV_MSG_VO_S*)pMsg->cMsgBody;
    #if 0
    s32Ret = SamplePcivStartVirtualVo(pstVoArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
    #endif
    return HI_SUCCESS;
}

HI_S32 SamplePciv_SlaveStopVo(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_VO_S *pstVoArgs = (SAMPLE_PCIV_MSG_VO_S*)pMsg->cMsgBody;

    s32Ret = SamplePcivStopVirtualVo(pstVoArgs);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}



HI_S32 SamplePciv_SlaveGetHostCtrlC(SAMPLE_PCIV_MSG_S *pMsg)
{

    return 0;
}

HI_S32 SAMPLE_COMM_PCIV_GetPIC(HI_U64 u64InPhyAddr)
{
    PCIV_DMA_TASK_S sTask;
    PCIV_DMA_BLOCK_S   stDMABlock[1];
    HI_U64 u64OutPhyAddr;
    HI_U32 u32BufSize;

    HI_S32 s32Ret;
    HI_CHAR *pMmzName = HI_NULL;;
    PIC_SIZE_E            enDispPicSize;
    SIZE_S                stDispSize;

    enDispPicSize = PIC_3840x2160;

	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}
    u32BufSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
												PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8,COMPRESS_MODE_SEG, 0);
    vbInBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32BufSize, pMmzName);
    if (VB_INVALID_HANDLE == vbInBlk)
    {
        printf("Func:%s, Info:HI_MPI_VB_GetBlock(size:%d) fail\n", __FUNCTION__, u32BufSize);
        return HI_FAILURE;
    }

    u64OutPhyAddr = HI_MPI_VB_Handle2PhysAddr(vbInBlk);
    if (0 == u64OutPhyAddr)
    {
        printf("Func:%s, Info:HI_MPI_VB_Handle2PhysAddr fail\n", __FUNCTION__);
        HI_MPI_VB_ReleaseBlock(vbInBlk);
        return HI_FAILURE;
    }

    stDMABlock[0].u32BlkSize = u32BufSize;
    stDMABlock[0].u64SrcAddr = u64InPhyAddr;
    stDMABlock[0].u64DstAddr = u64OutPhyAddr;

    sTask.bRead = HI_TRUE;
    sTask.pBlock = stDMABlock;
    sTask.u32Count = 1;

    s32Ret=HI_MPI_PCIV_DmaTask(&sTask);
    while (HI_ERR_PCIV_BUSY == s32Ret)
    {
        usleep(10000);
        printf("---- PCI DMA Wait ----\n");
        s32Ret = HI_MPI_PCIV_DmaTask(&sTask);
    }
    if (HI_SUCCESS != s32Ret)
    {
        printf("Func:%s -> dma task fail,s32ret= 0x%x\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    printf("u64InPhyAddr:0x%llx u64OutPhyAddr:0x%llx,u32BufSize:%d\n",u64InPhyAddr,u64OutPhyAddr,u32BufSize);
    return s32Ret;
}
HI_S32 SAMPLE_COMM_PCIV_RELEASEPIC(HI_VOID)
{
     HI_S32 s32Ret;
     s32Ret = HI_MPI_VB_ReleaseBlock(vbInBlk);
     return s32Ret;
}
HI_S32 SamplePciv_SlaveStartGetHostPIC(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;

    SAMPLE_PCIV_MSG_TEST_S *pstPcivArgs = (SAMPLE_PCIV_MSG_TEST_S*)pMsg->cMsgBody;

    s32Ret = SAMPLE_COMM_PCIV_GetPIC(pstPcivArgs->u64InPhyAddr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_PCIV_GetPIC failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    printf("get pic from host ok!\n");
    return HI_SUCCESS;

}

HI_S32 SamplePciv_SlaveStopGetHostPIC(SAMPLE_PCIV_MSG_S *pMsg)
{
    HI_S32 s32Ret;

    s32Ret = SAMPLE_COMM_PCIV_RELEASEPIC();
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_PCIV_GetPIC failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    printf("get pic from host ok!\n");
    return HI_SUCCESS;

}

int SamplePcivGetLocalId(int *local_id)
{
    int fd;
    struct hi_mcc_handle_attr attr;

    fd = open("/dev/mcc_userdev", O_RDWR);
    if (fd < 0)
    {
        printf("open mcc dev fail\n");
        return -1;
    }

    *local_id = ioctl(fd, HI_MCC_IOC_GET_LOCAL_ID, &attr);
    printf("pci local id is %d \n", *local_id);

    attr.target_id = 0;
    attr.port      = 0;
    attr.priority  = 0;
    ioctl(fd, HI_MCC_IOC_CONNECT, &attr);
    printf("===================close port %d!\n",attr.port);
    close(fd);
    return 0;
}

HI_S32 SamplePcivInitMediaSys(HI_VOID)
{
    HI_S32 s32Ret;
    VB_CONFIG_S 	    stVbConf = {0};
    PIC_SIZE_E          enPicSize           = PIC_3840x2160;
    PIC_SIZE_E          enDispPicSize;
    HI_U32              u32BlkSize;
    SIZE_S              stDispSize;
    SIZE_S              stSize;

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(g_stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %d!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %d!\n", s32Ret);
        return s32Ret;
    }

    memset(&stVbConf,0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt              = 8;

    u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_10, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt   = 10;

    u32BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, 32);
    stVbConf.astCommPool[1].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    enDispPicSize = PIC_3840x2160;
	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}

    stVbConf.astCommPool[2].u32BlkCnt  = 20;
    stVbConf.astCommPool[2].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
												PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8,COMPRESS_MODE_NONE, 0);
    enDispPicSize = PIC_1080P;
	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}

    stVbConf.astCommPool[3].u32BlkCnt  = 20;
    stVbConf.astCommPool[3].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
												PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8,COMPRESS_MODE_NONE, 0);
    enDispPicSize = PIC_3840x2160;
	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}

    stVbConf.astCommPool[4].u32BlkCnt  = 20;
    stVbConf.astCommPool[4].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
												PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8,COMPRESS_MODE_SEG, 0);
    enDispPicSize = PIC_1080P;
	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}

    stVbConf.astCommPool[5].u32BlkCnt  = 20;
    stVbConf.astCommPool[5].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
												PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8,COMPRESS_MODE_SEG, 0);
    stVbConf.astCommPool[6].u32BlkCnt = 2;
    stVbConf.astCommPool[6].u64BlkSize = SAMPLE_PCIV_VENC_STREAM_BUF_LEN;

    stVbConf.astCommPool[7].u32BlkCnt = 2;
    stVbConf.astCommPool[7].u64BlkSize = SAMPLE_PCIV_VDEC_STREAM_BUF_LEN;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_MemConfig();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system MemConfig failed with %d!\n", s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}
int main(int argc, char *argv[])
{
    HI_S32 				s32Ret;
	HI_U32 				u32MsgType = 0;
	HI_S32 				s32EchoMsgLen = 0;
    PCIV_BASEWINDOW_S 	stPciBaseWindow;
    SAMPLE_PCIV_MSG_S 	stMsg;

	memset(&stPciBaseWindow,0,sizeof(PCIV_BASEWINDOW_S));

	SamplePcivGetLocalId(&g_s32PciLocalId);

    /* wait for pci host ... */
    s32Ret = PCIV_WaitConnect(0);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
	HI_PRINT("g_s32PciLocalId=%d\n",g_s32PciLocalId);

    /* open pci msg port for commom cmd */
    s32Ret = PCIV_OpenMsgPort(0, PCIV_MSGPORT_COMM_CMD);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    s32Ret = SamplePcivInitMediaSys();
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    /* get PF Window info of this pci device */
    stPciBaseWindow.s32ChipId = 0;
    s32Ret = HI_MPI_PCIV_GetBaseWindow(0, &stPciBaseWindow);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    g_u64PfAhbBase = stPciBaseWindow.u64PfAHBAddr;
    printf("PF AHB BASE Addr:0x%llu\n", stPciBaseWindow.u64PfAHBAddr);

    while (1)
    {
        s32EchoMsgLen = 0;
        s32Ret = PCIV_ReadMsg(0, PCIV_MSGPORT_COMM_CMD, &stMsg);
        if (s32Ret != HI_SUCCESS)
        {

            usleep(10000);
            continue;
        }
        printf("\nreceive msg, MsgType:(%d,%s) \n",
            stMsg.stMsgHead.u32MsgType, PCIV_MSG_PRINT_TYPE(stMsg.stMsgHead.u32MsgType));

		u32MsgType = stMsg.stMsgHead.u32MsgType;

        switch(stMsg.stMsgHead.u32MsgType)
        {
            case SAMPLE_PCIV_MSG_INIT_MSG_PORG:
            {
                s32Ret = SamplePciv_SlaveInitPort(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_EXIT_MSG_PORG:
            {
                s32Ret = SamplePciv_SlaveExitPort(&stMsg);
                break;
            }
			case SAMPLE_PCIV_MSG_INIT_VI:
            {
                s32Ret = SamplePciv_SlaveInitVi(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_EXIT_VI:
            {
                s32Ret = SamplePciv_SlaveStopVi(&stMsg);
                break;
            }

            case SAMPLE_PCIV_MSG_START_VDEC:
            {
                s32Ret = SamplePciv_SlaveStartVdec(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_STOP_VDEC:
            {
                s32Ret = SamplePciv_SlaveStopVdec(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_INIT_STREAM_VDEC:
            {
                s32Ret = SamplePciv_SlaveStartVdecStream(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_EXIT_STREAM_VDEC:
            {
                s32Ret = SamplePciv_SlaveStopVdecStream(&stMsg);
                break;
            }
			case SAMPLE_PCIV_MSG_INIT_ALL_VENC:
			{
				s32Ret = SamplePciv_SlaveStartVenc(&stMsg);;
                break;
			}
			 case SAMPLE_PCIV_MSG_EXIT_ALL_VENC:
            {
                s32Ret = SamplePciv_SlaveStopVenc(&stMsg);
                break;
            }

            case SAMPLE_PCIV_MSG_START_VPSS:
            {
                s32Ret = SamplePciv_SlaveStartVpss(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_STOP_VPSS:
            {
                s32Ret = SamplePciv_SlaveStopVpss(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_START_VO:
            {
                s32Ret = SamplePciv_SlaveStartVo(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_STOP_VO:
            {
                s32Ret = SamplePciv_SlaveStopVo(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_CREATE_PCIV:
            {
                s32Ret = SamplePciv_SlaveStartPciv(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_DESTROY_PCIV:
            {
                s32Ret = SamplePciv_SlaveStopPicv(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_INIT_WIN_VB:
            {
                s32Ret = SamplePciv_SlaveInitWinVb(&stMsg);
                break;
            }
			case SAMPLE_PCIV_MSG_EXIT_WIN_VB:
            {
                s32Ret = SamplePciv_SlaveExitWinVb();
                break;
            }
            case SAMPLE_PCIV_MSG_MALLOC:
            {
                s32Ret = SamplePciv_SlaveMalloc(&stMsg);
                s32EchoMsgLen = sizeof(PCIV_PCIVCMD_MALLOC_S);
                break;
            }
            case SAMPLE_PCIV_MSG_FREE:
            {
                s32Ret = SamplePciv_SlaveFree(&stMsg);
                break;
            }

			case SAMPLE_PCIV_MSG_CAP_CTRL_C:
            {
                s32Ret = SamplePciv_SlaveGetHostCtrlC(&stMsg);
                break;
            }
            case SAMPLE_PCIV_START_MSG_TEST:
            {
                s32Ret = SamplePciv_SlaveStartGetHostPIC(&stMsg);
                break;
            }
            case SAMPLE_PCIV_STOP_MSG_TEST:
            {
                s32Ret = SamplePciv_SlaveStopGetHostPIC(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_BIND_SRC_DEST:
            {
                s32Ret = SamplePciv_SlaveBindSrcDest(&stMsg);
                break;
            }
            case SAMPLE_PCIV_MSG_UNBIND_SRC_DEST:
            {
                s32Ret = SamplePciv_SlaveUnBindSrcDest(&stMsg);
                break;
            }
            default:
            {
                printf("invalid msg, type:%d \n", stMsg.stMsgHead.u32MsgType);
                s32Ret = HI_FAILURE;
                break;
            }
        }
        /* echo msg to host */
        SamplePcivEchoMsg(s32Ret, s32EchoMsgLen, &stMsg);

		if ((SAMPLE_PCIV_MSG_EXIT_MSG_PORG == u32MsgType)||(SAMPLE_PCIV_MSG_CAP_CTRL_C == u32MsgType))
		{
			break;
		}
    }

    /* exit */
    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}


