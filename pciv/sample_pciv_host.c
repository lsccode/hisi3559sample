/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sample_pciv_host.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2009/09/22
  Description   : this sample of pciv in PCI host
  History       :
  1.Date        : 2009/09/22
    Author      : Hi35xxMPP
    Modification: Created file
  2.Date        : 2010/02/12
    Author      : Hi35xxMPP
    Modification:
  3.Date        : 2010/06/10
    Author      : Hi35xxMPP
    Modification:
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
#include <signal.h>
#include <sys/prctl.h>

#include "hi_comm_pciv.h"
#include "mpi_pciv.h"
#include "pciv_msg.h"
#include "pciv_trans.h"
#include "sample_pciv_comm.h"
#include "hi_debug.h"
#include "sys_ext.h"

#include "sample_comm.h"

#define ALIGN_BACK(x, a)        ((a) * (((x + a -1) / (a))))
typedef struct hiSAMPLE_PCIV_DISP_CTX_S
{
    VO_DEV  VoDev;
    HI_BOOL bValid;
    HI_U32  u32PicDiv;
    VO_CHN  VoChnStart;
    VO_CHN  VoChnEnd;
} SAMPLE_PCIV_DISP_CTX_S;

typedef struct hiSAMPLE_PCIV_BIND_SRC_INFO_S
{
   MPP_CHN_S SrcMod;
   MPP_CHN_S DestMod;
}SAMPLE_PCIV_BIND_SRC_INFO_S;

typedef struct hiSAMPLE_PCIV_UnBIND_SRC_INFO_S
{
   MPP_CHN_S SrcMod;
   MPP_CHN_S DestMod;
}SAMPLE_PCIV_UNBIND_SRC_INFO_S;

//extern VIDEO_NORM_E   gs_enViNorm;
extern VO_INTF_SYNC_E gs_enSDTvMode;

#define SAMPLE_PCIV_VDEC_SIZE PIC_1080P
#define SAMPLE_PCIV_VDEC_FILE "sample_cif_25fps.h264"
#define SAMPLE_STREAM_PATH "../source_file"
#define SAMPLE_1080P_H264_PATH "./3840x2160_8bit.h264"

/* max pciv chn count in one vo dev */
#define SAMPLE_PCIV_CNT_PER_VO      16
#define PCIV_FRMNUM_ONCEDMA 5
#define PCIE_BAR0_ADDRESS   0x30800000
//#define PCIE_BAR0_ADDRESS   (0x30800000+0x100000)

pthread_t   VdecHostThread[2*VDEC_MAX_CHN_NUM];

/* max pciv chn count in one pci dev */
#define SAMPLE_PCIV_CNT_PER_DEV     SAMPLE_PCIV_CNT_PER_VO * VO_MAX_DEV_NUM

static HI_U64 g_u64PfWinBase[PCIV_MAX_CHIPNUM]  = {0};
//VIDEO_NORM_E   g_enViNorm   = VIDEO_ENCODING_MODE_PAL;

static SAMPLE_PCIV_VENC_CTX_S g_stPcivVencCtx = {0};
static SAMPLE_PCIV_VDEC_CTX_S astPcivVdecCtx[VDEC_MAX_CHN_NUM] ={0};

#define STREAM_SEND_VDEC    1
#define STREAM_SAVE_FILE    1
#define PCIV_START_STREAM   1

static int test_idx = 0;
//static int vdec_idx = 0;
static int Add_osd  = 0;
static HI_BOOL bQuit = HI_FALSE;
static HI_U64 au64PhyAddr[4] ={0};



HI_S32 SamplePcivGetPicSizeHD(HI_S32 s32VoDiv, SIZE_S *pstPicSize)
{
    SIZE_S stScreenSize;

    stScreenSize.u32Width  = 3840;	//3840;
    stScreenSize.u32Height = 2160;	//2160;
	//stScreenSize.u32Height = 1080;

    switch (s32VoDiv)
    {
        case 1 :
            pstPicSize->u32Width  = stScreenSize.u32Width;
            pstPicSize->u32Height = stScreenSize.u32Height;
            break;
        case 4 :
            pstPicSize->u32Width  = stScreenSize.u32Width  / 2;
            pstPicSize->u32Height = stScreenSize.u32Height / 2;
            break;
        case 9 :
            pstPicSize->u32Width  = stScreenSize.u32Width  / 3;
            pstPicSize->u32Height = stScreenSize.u32Height / 3;
            break;
        case 16 :
            pstPicSize->u32Width  = stScreenSize.u32Width  / 4;
            pstPicSize->u32Height = stScreenSize.u32Height / 4;
            break;
        default:
            printf("not support this vo div %d \n", s32VoDiv);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SamplePcivGetPicSize(HI_S32 s32VoDiv, SIZE_S *pstPicSize)
{
    SIZE_S stScreenSize;

    stScreenSize.u32Width  = 3840;
    stScreenSize.u32Height = 2160;

    switch (s32VoDiv)
    {
        case 1 :
            pstPicSize->u32Width  = stScreenSize.u32Width;
            pstPicSize->u32Height = stScreenSize.u32Height;
            break;
        case 4 :
            pstPicSize->u32Width  = stScreenSize.u32Width  / 2;
            pstPicSize->u32Height = stScreenSize.u32Height / 2;
            break;
        case 9 :
            pstPicSize->u32Width  = stScreenSize.u32Width  / 3;
            pstPicSize->u32Height = stScreenSize.u32Height / 3;
            break;
        case 16 :
            pstPicSize->u32Width  = stScreenSize.u32Width  / 4;
            pstPicSize->u32Height = stScreenSize.u32Height / 4;
            break;
        default:
            printf("not support this vo div %d \n", s32VoDiv);
            return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SamplePcivGetPicAttr(PICVB_ATTR_S *pstPicVbAttr, PCIV_PIC_ATTR_S *pstPicAttr)
{
    HI_S32                s32Ret;
    SIZE_S                stDispSize;
    PIC_SIZE_E            enDispPicSize;
    HI_U32                u32MainStride;
    HI_U32                u32BitWidth;
    HI_U32                u32TailInBytes;

    enDispPicSize = pstPicVbAttr->enDispPicSize;

	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}

    pstPicAttr->u32Width     = stDispSize.u32Width;
    pstPicAttr->u32Height    = stDispSize.u32Height;

    pstPicAttr->u32Field       = VIDEO_FIELD_FRAME;
    pstPicAttr->enPixelFormat  = pstPicVbAttr->enPixelFormat;
    pstPicAttr->enDynamicRange = pstPicVbAttr->enDynamicRange;
    pstPicAttr->enVideoFormat  = pstPicVbAttr->enVideoFormat;
    pstPicAttr->enCompressMode = pstPicVbAttr->enCompressMode;

    if(DYNAMIC_RANGE_SDR8 == pstPicAttr->enDynamicRange)
    {
        u32BitWidth = 8;
    }
    else
    {
         if(VIDEO_FORMAT_LINEAR_DISCRETE == pstPicAttr->enVideoFormat)
         {
            u32BitWidth = 16;
         }
         else
         {
            u32BitWidth = 10;
         }
    }
    if (COMPRESS_MODE_NONE == pstPicVbAttr->enCompressMode)
    {
        u32MainStride = ALIGN_UP((pstPicAttr->u32Width * u32BitWidth + 7) >> 3, 16);
    }
    else
    {
        if (u32BitWidth == 8)
        {
            u32MainStride  = ALIGN_UP(pstPicAttr->u32Width, 16);
        }
        else
        {
            u32TailInBytes = DIV_UP(pstPicAttr->u32Width%SEG_CMP_LENGTH*u32BitWidth, 8);
            u32MainStride  = ALIGN_DOWN(pstPicAttr->u32Width, SEG_CMP_LENGTH) + ((u32TailInBytes > SEG_CMP_LENGTH) ? SEG_CMP_LENGTH : u32TailInBytes);
            u32MainStride  = ALIGN_UP(u32MainStride, 16);
        }
    }

    pstPicAttr->u32Stride[0] = u32MainStride;
    pstPicAttr->u32Stride[1] = u32MainStride;
    pstPicAttr->u32Stride[2] = u32MainStride;

    return HI_SUCCESS;
}


HI_S32 SamplePcivGetBlkSize(PCIV_PIC_ATTR_S *pstPicAttr, HI_U32 *pu32BlkSize)
{
    switch (pstPicAttr->enPixelFormat)
    {
        case PIXEL_FORMAT_YVU_SEMIPLANAR_420:
            *pu32BlkSize = pstPicAttr->u32Stride[0]*pstPicAttr->u32Height*3/2;
            break;
        case PIXEL_FORMAT_YVU_SEMIPLANAR_422:
            *pu32BlkSize = pstPicAttr->u32Stride[0]*pstPicAttr->u32Height*2;
            break;
        case PIXEL_FORMAT_VYUY_PACKAGE_422:
            *pu32BlkSize = pstPicAttr->u32Stride[0]*pstPicAttr->u32Height;
            break;
        default:
            return -1;
    }

    return HI_SUCCESS;
}


HI_S32 SamplePcivSendMsgCreatePciv(HI_S32 s32TargetId,PCIV_PCIVCMD_CREATE_S *pstMsgCreate)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S stMsg;

    stMsg.stMsgHead.u32Target  = s32TargetId;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_CREATE_PCIV;
    stMsg.stMsgHead.u32MsgLen  = sizeof(PCIV_PCIVCMD_CREATE_S);
    memcpy(stMsg.cMsgBody, pstMsgCreate, sizeof(PCIV_PCIVCMD_CREATE_S));

    printf("=======PCIV_SendMsg SAMPLE_PCIV_MSG_CREATE_PCIV==========\n");

    s32Ret = PCIV_SendMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}


HI_S32 SamplePcivSendMsgDestroy(HI_S32 s32TargetId,PCIV_PCIVCMD_DESTROY_S *pstMsgDestroy)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S stMsg;

    stMsg.stMsgHead.u32Target  = 1;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_DESTROY_PCIV;
    stMsg.stMsgHead.u32MsgLen  = sizeof(PCIV_PCIVCMD_DESTROY_S);
    memcpy(stMsg.cMsgBody, pstMsgDestroy, sizeof(PCIV_PCIVCMD_DESTROY_S));

    printf("=======PCIV_SendMsg SAMPLE_PCIV_MSG_DESTROY_PCIV==========\n");

    s32Ret = PCIV_SendMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}


/*****************************************************************************
* function : start vpss. VPSS chn with frame
*****************************************************************************/
HI_S32 SamplePciv_StartVpss(HI_S32 VpssGrp, HI_S32 VpssChn, SIZE_S *pstSize, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    HI_S32 s32Ret;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};

    /*** Set Vpss Grp Attr ***/
    if(NULL == pstVpssGrpAttr)
    {
        //pstVpssGrpAttr->bStitchBlendEn = HI_FALSE;
        pstVpssGrpAttr->stFrameRate.s32SrcFrameRate = -1;
        pstVpssGrpAttr->stFrameRate.s32DstFrameRate = -1;
    }
    else
    {
        memcpy(&stGrpAttr, pstVpssGrpAttr, sizeof(VPSS_GRP_ATTR_S));
    }

    /*** create vpss group ***/
    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }


    /*** enable vpss chn, with frame ***/
    /* Set Vpss Chn attr */
    stChnAttr.enChnMode = VPSS_CHN_MODE_USER;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    stChnAttr.enDynamicRange  = DYNAMIC_RANGE_SDR8;
    stChnAttr.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stChnAttr.u32Width = 960;
    stChnAttr.u32Height = 540;
    stChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stChnAttr.stFrameRate.s32DstFrameRate = -1;


    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    /*** start vpss group ***/
    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SamplePciv_StopVpss(HI_S32 s32VpssGrpCnt, HI_S32 s32ChnCnt)
{
    HI_S32 i = 0;
    HI_S32 j = 0;
    HI_S32 s32Ret;
    HI_S32 VpssGrp;
    HI_S32 VpssChn;
    for (i = 0; i < s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret =  HI_MPI_VPSS_StopGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            printf("stop vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }
        for (j = 0; j < s32ChnCnt; j++)
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);

            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
        }
        s32Ret =  HI_MPI_VPSS_DestroyGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            printf("destroy vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }

    }
    printf("destroy vpss ok!\n");

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostInitWinVb(HI_S32 s32RmtChip, HI_U32 u32BlkCount)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_WINVB_S stWinVbArgs;

    stWinVbArgs.stPciWinVbCfg.u32PoolCount   = 1;
    stWinVbArgs.stPciWinVbCfg.u32BlkCount[0] = u32BlkCount;
    stWinVbArgs.stPciWinVbCfg.u32BlkSize[0]  = SAMPLE_PCIV_VDEC_STREAM_BUF_LEN;

    memcpy(stMsg.cMsgBody, &stWinVbArgs, sizeof(stWinVbArgs));
    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_INIT_WIN_VB;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stWinVbArgs);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_INIT_WIN_VB==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}


HI_S32 SamplePciv_HostExitWinVb(HI_S32 s32RmtChip)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;

    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_EXIT_WIN_VB;
    stMsg.stMsgHead.u32MsgLen  = 0;

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_EXIT_WIN_VB==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStartVdecChn(HI_S32 s32RmtChip, HI_U32 u32VdecChnNum,VDEC_MODE_S *pstVdecMode)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_VDEC_S stVdecArgs;

    stVdecArgs.u32VdecChnNum = u32VdecChnNum;
    memcpy(&stVdecArgs.stVdecMode,pstVdecMode,sizeof(VDEC_MODE_S));
    memcpy(stMsg.cMsgBody, &stVdecArgs, sizeof(stVdecArgs));

    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_START_VDEC;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stVdecArgs);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_START_VDEC==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}


HI_S32 SamplePciv_HostStopVdecChn(HI_S32 s32RmtChip, HI_U32 u32VdecChnNum)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_VDEC_S stVdecArgs={0};

    stVdecArgs.u32VdecChnNum = u32VdecChnNum;

    memcpy(stMsg.cMsgBody, &stVdecArgs, sizeof(SAMPLE_PCIV_MSG_VDEC_S));
    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_STOP_VDEC;
    stMsg.stMsgHead.u32MsgLen  = sizeof(SAMPLE_PCIV_MSG_VDEC_S);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_STOP_VDEC==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

/* read stream data from file, write to local buffer, then send to pci target direct at once */
HI_VOID * SamplePciv_SendVdStrmThread(HI_VOID *p)
{
    HI_S32          s32Ret, s32ReadLen;
    HI_U32          u32FrmSeq = 0;
    HI_U8           *pu8VdecBuf = NULL;
    HI_VOID         *pCreator    = NULL;
    FILE* file = NULL;
    PCIV_STREAM_HEAD_S      stHeadTmp;
    PCIV_TRANS_LOCBUF_STAT_S stLocBufSta;
    SAMPLE_PCIV_VDEC_CTX_S *pstCtx = (SAMPLE_PCIV_VDEC_CTX_S*)p;

    pCreator = pstCtx->pTransHandle;
    printf("%s -> Sender:%p, chnid:%d\n", __FUNCTION__, pCreator, pstCtx->VdecChn);

    /*open the stream file*/
    file = fopen(pstCtx->aszFileName, "r");
    if (HI_NULL == file)
    {
        printf("open file %s err\n", pstCtx->aszFileName);
        exit(-1);
    }

    pu8VdecBuf = (HI_U8*)malloc(SAMPLE_PCIV_SEND_VDEC_LEN);
    HI_ASSERT(pu8VdecBuf);

    while(pstCtx->bThreadStart)
    {
        s32ReadLen = fread(pu8VdecBuf, 1, SAMPLE_PCIV_SEND_VDEC_LEN, file);
        if (s32ReadLen <= 0)
        {
            fseek(file, 0, SEEK_SET);/*read file again*/
            continue;
        }
        usleep(100000);
        /* you should insure buf len is enough */
        PCIV_Trans_QueryLocBuf(pCreator, &stLocBufSta);
        if (stLocBufSta.u32FreeLen < s32ReadLen + sizeof(PCIV_STREAM_HEAD_S))
        {
            printf("venc stream local buffer not enough, %d < %d\n",stLocBufSta.u32FreeLen,s32ReadLen);
            break;
        }
        /* fill stream header info */
        stHeadTmp.u32Magic         = PCIV_STREAM_MAGIC;
        stHeadTmp.enPayLoad        = PT_H264;
        stHeadTmp.s32ChnID         = pstCtx->VdecChn;
        stHeadTmp.u32StreamDataLen = s32ReadLen;
        stHeadTmp.u32Seq           = u32FrmSeq++;

        if(s32ReadLen%4)
        {
            stHeadTmp.u32DMADataLen = (s32ReadLen/4+1)*4;
        }
        else
        {
            stHeadTmp.u32DMADataLen = s32ReadLen;
        }

        /* write stream header */
        s32Ret = PCIV_Trans_WriteLocBuf(pCreator, (HI_U8*)&stHeadTmp, sizeof(stHeadTmp));
        HI_ASSERT((HI_SUCCESS == s32Ret));
        /* write stream data */
        s32Ret = PCIV_Trans_WriteLocBuf(pCreator, pu8VdecBuf, stHeadTmp.u32DMADataLen);
        HI_ASSERT((HI_SUCCESS == s32Ret));
        //printf("Func: %s, Line: %d, vdec chn: %d.#####################\n", __FUNCTION__, __LINE__, pstCtx->VdecChn);
        /* send local data to pci target */
        while (PCIV_Trans_SendData(pCreator) && pstCtx->bThreadStart)
        {
            usleep(10000);
        }
    }
    fclose(file);
    free(pu8VdecBuf);
    return NULL;
}
HI_S32 SamplePciv_StartVdecByChip(HI_S32 s32RmtChipId, HI_U32 u32VdecCnt,VDEC_MODE_S *pstVdecMode)
{
    HI_S32 s32Ret;
    s32Ret = SamplePciv_HostInitWinVb(s32RmtChipId, u32VdecCnt);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    s32Ret = SamplePciv_HostStartVdecChn(s32RmtChipId, u32VdecCnt,pstVdecMode);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_StopVdecByChip(HI_S32 s32RmtChipId, HI_U32 u32VdecCnt)
{
    HI_S32 s32Ret;

    s32Ret = SamplePciv_HostStopVdecChn(s32RmtChipId, u32VdecCnt);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    s32Ret = SamplePciv_HostExitWinVb(s32RmtChipId);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }



    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStartVdecStream(HI_S32 s32RmtChip, VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    PCIV_PCIVCMD_MALLOC_S stMallocCmd;
    PCIV_PCIVCMD_MALLOC_S *pstMallocEcho;
    PCIV_TRANS_ATTR_S stInitPara;
    SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = &astPcivVdecCtx[VdecChn];
    /* send msg to slave(PCI Device), for malloc Dest Addr of stream buffer */
    /* PCI transfer data from Host to Device, Dest Addr must in PCI WINDOW of PCI Device */
    stMallocCmd.u32BlkCount = 1;
    stMallocCmd.u32BlkSize = SAMPLE_PCIV_VDEC_STREAM_BUF_LEN;

    memcpy(stMsg.cMsgBody,&stMallocCmd,sizeof(PCIV_PCIVCMD_MALLOC_S));
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_MALLOC;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_PCIVCMD_MALLOC_S);

    printf("\n=========PCIV_SendMsg SAMPLE_PCIV_MSG_MALLOC==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    /* read msg, phyaddr will return */
    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    pstMallocEcho = (PCIV_PCIVCMD_MALLOC_S *)stMsg.cMsgBody;
    /* init vdec stream sender in local chip */
    stInitPara.s32RmtChip = s32RmtChip;
    stInitPara.s32ChnId = VdecChn;
    stInitPara.u32BufSize = pstMallocEcho->u32BlkSize;
    stInitPara.u64PhyAddr = pstMallocEcho->u64PhyAddr[0] + PCIE_BAR0_ADDRESS;
    stInitPara.s32MsgPortWrite = pstVdecCtx->s32MsgPortWrite;
    stInitPara.s32MsgPortRead = pstVdecCtx->s32MsgPortRead;

    s32Ret = PCIV_Trans_InitSender(&stInitPara, &pstVdecCtx->pTransHandle);
    HI_ASSERT(HI_SUCCESS == s32Ret);

    /* send msg to slave chip to init vdec stream transfer */
    stInitPara.s32RmtChip = 0;
    stInitPara.u64PhyAddr = pstMallocEcho->u64PhyAddr[0];
    memcpy(stMsg.cMsgBody, &stInitPara, sizeof(stInitPara));
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_INIT_STREAM_VDEC;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_TRANS_ATTR_S);
    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_INIT_STREAM_VDEC==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    /* after target inited stream receiver, local start sending stream thread */
    sprintf(pstVdecCtx->aszFileName, "%s", SAMPLE_1080P_H264_PATH);
    pstVdecCtx->VdecChn = VdecChn;
    pstVdecCtx->bThreadStart = HI_TRUE;
    s32Ret = pthread_create(&pstVdecCtx->pid, NULL, SamplePciv_SendVdStrmThread, pstVdecCtx);
    printf("init vdec:%d stream transfer ok ==================\n", VdecChn);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStopVdecStream(HI_S32 s32RmtChip, VDEC_CHN VdecChn)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    PCIV_PCIVCMD_FREE_S stFreeCmd;
    PCIV_TRANS_ATTR_S   stDeInitPara;
    PCIV_TRANS_SENDER_S *pstSender;

    SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = &astPcivVdecCtx[VdecChn];

    pstVdecCtx->bThreadStart = HI_FALSE;
    pthread_join(pstVdecCtx->pid,NULL);

    pstSender =  (PCIV_TRANS_SENDER_S*)pstVdecCtx->pTransHandle;
    stFreeCmd.u32BlkCount = 1;
    //stFreeCmd.u64PhyAddr[0] = pstSender->stRmtBuf.s32BaseAddr - PCIE_BAR0_ADDRESS;
    /* send msg to slave(PCI Device), for malloc Dest Addr of stream buffer */
    /* PCI transfer data from Host to Device, Dest Addr must in PCI WINDOW of PCI Device */

    memcpy(stMsg.cMsgBody,&stFreeCmd,sizeof(PCIV_PCIVCMD_FREE_S));
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_FREE;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_PCIVCMD_FREE_S);

    printf("\n=========PCIV_SendMsg SAMPLE_PCIV_MSG_FREE==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    /* read msg, phyaddr will return */
    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    /* Deinit vdec stream sender in local chip */
    s32Ret = PCIV_Trans_DeInitSender(pstSender);
    HI_ASSERT(HI_SUCCESS == s32Ret);

    /* send msg to slave chip to init vdec stream transfer */
    stDeInitPara.s32ChnId = VdecChn;
    memcpy(stMsg.cMsgBody,&stDeInitPara,sizeof(PCIV_TRANS_ATTR_S));

    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_EXIT_STREAM_VDEC;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_TRANS_ATTR_S);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_EXIT_STREAM_VDEC==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);
    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    /* after target Deinited stream receiver, local stop sending stream thread */

    printf("Deinit vdec:%d stream transfer ok ==================\n", VdecChn);
    return HI_SUCCESS;
}

HI_S32 SamplePciv_VpssCropPic(HI_U32 u32VpssCnt)
{
    HI_S32 i;
    HI_S32 s32Ret;
    HI_S32 s32Div;
    VPSS_CROP_INFO_S stCropInfo;

    s32Div = sqrt(u32VpssCnt);

    printf("Func: %s, u32VpssCnt: %d, s32Div: %d.\n", __FUNCTION__, u32VpssCnt, s32Div);

    stCropInfo.bEnable              = HI_TRUE;
    stCropInfo.enCropCoordinate     = VPSS_CROP_ABS_COOR;
    stCropInfo.stCropRect.u32Width  = 960;
    stCropInfo.stCropRect.u32Height = 544;
    stCropInfo.stCropRect.s32X      = 0;
    stCropInfo.stCropRect.s32Y      = 0;

    for (i = 0; i < u32VpssCnt; i++)
    {

        stCropInfo.stCropRect.s32X  = stCropInfo.stCropRect.u32Width * (i % s32Div);
        stCropInfo.stCropRect.s32Y  = stCropInfo.stCropRect.u32Height * (i / s32Div);
        s32Ret = HI_MPI_VPSS_SetGrpCrop(i, &stCropInfo);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivStopSlaveVirtualVo(HI_S32 s32RmtChip, VO_DEV VoDev, HI_U32 u32VoChnNum)
{
    HI_S32            s32Ret;
    SAMPLE_PCIV_MSG_S stMsg;
    SAMPLE_PCIV_MSG_VO_S stVoArgs;

    stVoArgs.VirtualVo       = VoDev;
    stVoArgs.stBInd.enModId  = HI_ID_VDEC;
    stVoArgs.stBInd.s32DevId = 0;
    stVoArgs.stBInd.s32ChnId = 0;
    stVoArgs.enPicSize       = PIC_1080P;
    stVoArgs.s32VoChnNum     = u32VoChnNum;


    memcpy(stMsg.cMsgBody, &stVoArgs, sizeof(stVoArgs));
    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_STOP_VO;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stVoArgs);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_STOP_VO==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 SamplePcivStartSlaveVirtualVo(HI_S32 s32RmtChip, VO_DEV VoDev, HI_U32 u32VoChnNum)
{
    HI_S32               s32Ret;
    SAMPLE_PCIV_MSG_S    stMsg;
    SAMPLE_PCIV_MSG_VO_S stVoArgs;

    stVoArgs.VirtualVo       = VoDev;
    stVoArgs.stBInd.enModId  = HI_ID_VDEC;
    stVoArgs.stBInd.s32DevId = 0;
    stVoArgs.stBInd.s32ChnId = 0;
    stVoArgs.enPicSize       = PIC_1080P;
    stVoArgs.enIntfSync      = VO_OUTPUT_1080P30;
    stVoArgs.s32VoChnNum     = u32VoChnNum;

    memcpy(stMsg.cMsgBody, &stVoArgs, sizeof(stVoArgs));
    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_START_VO;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stVoArgs);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_START_VO==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStartVencChn(HI_S32 s32RmtChip,VPSS_GRP VpssGrp, HI_BOOL bBindVdec)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_VPSS_S stVpssArgs = {0};

    memcpy(stMsg.cMsgBody, &stVpssArgs, sizeof(stVpssArgs));
    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_START_VPSS;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stVpssArgs);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_START_VPSS==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_StartVencByChip(HI_S32 s32RmtChipId, HI_U32 u32VencCnt, HI_BOOL bBindVdec)
{
    HI_S32 s32Ret, j;

    for (j=0; j<u32VencCnt; j++)
    {
        s32Ret  = SamplePciv_HostStartVencChn(s32RmtChipId, j, bBindVdec);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStartVpssChn(HI_S32 s32RmtChip , VPSS_MODE_S *pstVpssMode,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_VPSS_S stVpssArgs;

    memset(&stVpssArgs,0,sizeof(SAMPLE_PCIV_MSG_VPSS_S));
    memcpy(&stVpssArgs.stVpssMode,pstVpssMode,sizeof(VPSS_MODE_S));
    memcpy(&stVpssArgs.stPicVbAttr,pstPicVbAttr,sizeof(PICVB_ATTR_S));
    memcpy(stMsg.cMsgBody, &stVpssArgs, sizeof(stVpssArgs));
    stMsg.stMsgHead.u32Target  = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_START_VPSS;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stVpssArgs);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_START_VPSS==========\n");

    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStopVpssChn(HI_S32 s32RmtChip, VPSS_MODE_S *pstVpssMode)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_VPSS_S stVpssArgs  = {0};

    stVpssArgs.stVpssMode.s32VpssGroupCnt  = pstVpssMode->s32VpssGroupCnt;
    stVpssArgs.stVpssMode.s32VpssChnCnt  = pstVpssMode->s32VpssChnCnt;

    memcpy(stMsg.cMsgBody, &stVpssArgs, sizeof(stVpssArgs));
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_STOP_VPSS;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_PCIVCMD_MALLOC_S);
    printf("=======PCIV_SendMsg SAMPLE_PCIV_MSG_STOP_VPSS==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_StartVpssByChip(HI_S32 s32RmtChipId, VPSS_MODE_S *pstVpssMode,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32 s32Ret;

    s32Ret  = SamplePciv_HostStartVpssChn(s32RmtChipId, pstVpssMode,pstPicVbAttr);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_StopVpssByChip(HI_S32 s32RmtChipId, VPSS_MODE_S *pstVpssMode)
{
    HI_S32 S32Ret;

    S32Ret=SamplePciv_HostStopVpssChn(s32RmtChipId, pstVpssMode);
    if(HI_SUCCESS!=S32Ret)
    {
        printf("SamplePciv Stop slave chip Error!\n");
     return S32Ret;
    }
   return S32Ret;
}


HI_S32 SamplePciv_HostStartVi(HI_S32 s32RmtChip, HI_S32 s32ViCnt,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S      stMsg;
    SAMPLE_PCIV_MSG_INIT_VI_S stViArgs;

    memset(&stViArgs,0,sizeof(SAMPLE_PCIV_MSG_INIT_VI_S));
    stViArgs.s32ViChnCnt = s32ViCnt;
    memcpy(&stViArgs.stPicVbAttr,pstPicVbAttr,sizeof(PICVB_ATTR_S));
    memcpy(stMsg.cMsgBody, &stViArgs, sizeof(SAMPLE_PCIV_MSG_INIT_VI_S));

    /* send msg to slave chip to init vi */
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_INIT_VI;
    stMsg.stMsgHead.u32MsgLen = sizeof(SAMPLE_PCIV_MSG_INIT_VI_S);

    printf("\nSend (init vi) message  to slave!\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(0);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    printf("Slave init vi succeed!\n");
    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostExitVi(HI_S32 s32RmtChip, HI_S32 s32ViCnt)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S      stMsg;
    SAMPLE_PCIV_MSG_EXIT_VI_S stExitViArg;

    memset(&stExitViArg,0,sizeof(SAMPLE_PCIV_MSG_EXIT_VI_S));
    stExitViArg.s32ViChnCnt = s32ViCnt;
    memcpy(stMsg.cMsgBody, &stExitViArg, sizeof(SAMPLE_PCIV_MSG_EXIT_VI_S));

    /* send msg to slave chip to exit vi */
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_EXIT_VI;
    stMsg.stMsgHead.u32MsgLen = sizeof(SAMPLE_PCIV_MSG_EXIT_VI_S);
    memcpy(stMsg.cMsgBody, &stExitViArg, sizeof(SAMPLE_PCIV_MSG_EXIT_VI_S));
    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_EXIT_VI==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(0);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    printf("exit all vi ok \n");
    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostCaptureCtrlC(HI_S32 s32RmtChip)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S      stMsg;

    /* send msg to slave chip to deal with ctrl+c */
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_CAP_CTRL_C;
    stMsg.stMsgHead.u32MsgLen = 0;
    printf("\n CTRL+C Send message to slave!\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(0);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    printf("Slave deal with ctrl+c succeed!\n");
    return HI_SUCCESS;
}



HI_S32 SamplePciv_GetDefVoAttr(VO_DEV VoDev, VO_INTF_SYNC_E enIntfSync, VO_PUB_ATTR_S *pstPubAttr,
    VO_VIDEO_LAYER_ATTR_S *pstLayerAttr, HI_S32 s32SquareSort, VO_CHN_ATTR_S *astChnAttr)
{
    VO_INTF_TYPE_E enIntfType;
    HI_U32 u32Width;
	HI_U32 u32Height;
	HI_U32 j;

    switch (VoDev)
    {
        default:
        case 0: enIntfType = VO_INTF_VGA;  break;
        case 1: enIntfType = VO_INTF_HDMI; break;
        case 2: enIntfType = VO_INTF_CVBS; break;
        case 3: enIntfType = VO_INTF_CVBS; break;
    }

    printf("Func: %s, line: %d.....\n", __FUNCTION__, __LINE__);
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL      :  u32Width = 720;  u32Height = 576;   break;
        case VO_OUTPUT_NTSC     :  u32Width = 720;  u32Height = 480;   break;
        case VO_OUTPUT_1080P24  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_1080P25  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_1080P30  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_720P50   :  u32Width = 1280; u32Height = 720;   break;
        case VO_OUTPUT_720P60   :  u32Width = 1280; u32Height = 720;   break;
        case VO_OUTPUT_1080I50  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_1080I60  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_1080P50  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_1080P60  :  u32Width = 1920; u32Height = 1080;  break;
        case VO_OUTPUT_576P50   :  u32Width = 720;  u32Height = 576;   break;
        case VO_OUTPUT_480P60   :  u32Width = 720;  u32Height = 480;   break;
        case VO_OUTPUT_800x600_60: u32Width = 800;  u32Height = 600;   break;
        case VO_OUTPUT_1024x768_60:u32Width = 1024; u32Height = 768;   break;
        case VO_OUTPUT_1280x1024_60:u32Width =1280; u32Height = 1024;  break;
        case VO_OUTPUT_1366x768_60:u32Width = 1366; u32Height = 768;   break;
        case VO_OUTPUT_1440x900_60:u32Width = 1440; u32Height = 900;   break;
        case VO_OUTPUT_1280x800_60:u32Width = 1280; u32Height = 800;   break;

        default: return HI_FAILURE;
    }
    printf("Func: %s, line: %d.....\n", __FUNCTION__, __LINE__);
    if (NULL != pstPubAttr)
    {
        pstPubAttr->enIntfSync = enIntfSync;
        pstPubAttr->u32BgColor = 0;
        pstPubAttr->enIntfType = enIntfType;
    }
    printf("Func: %s, line: %d.....\n", __FUNCTION__, __LINE__);
    if (NULL != pstLayerAttr)
    {
        pstLayerAttr->stDispRect.s32X       = 0;
        pstLayerAttr->stDispRect.s32Y       = 0;
        pstLayerAttr->stDispRect.u32Width   = u32Width;
        pstLayerAttr->stDispRect.u32Height  = u32Height;
        pstLayerAttr->stImageSize.u32Width  = u32Width;
        pstLayerAttr->stImageSize.u32Height = u32Height;
        pstLayerAttr->u32DispFrmRt          = 25;
        pstLayerAttr->enPixFormat           = SAMPLE_PIXEL_FORMAT;
        pstLayerAttr->bDoubleFrame          = HI_FALSE;
        pstLayerAttr->bClusterMode          = HI_FALSE;
    }
    printf("Func: %s, line: %d.....\n", __FUNCTION__, __LINE__);
    if (NULL != astChnAttr)
    {
        for (j=0; j<(s32SquareSort * s32SquareSort); j++)
        {
            astChnAttr[j].stRect.s32X       = ALIGN_BACK((u32Width/s32SquareSort) * (j%s32SquareSort), 4);
            astChnAttr[j].stRect.s32Y       = ALIGN_BACK((u32Height/s32SquareSort) * (j/s32SquareSort), 4);
            astChnAttr[j].stRect.u32Width   = ALIGN_BACK(u32Width/s32SquareSort, 4);
            astChnAttr[j].stRect.u32Height  = ALIGN_BACK(u32Height/s32SquareSort, 4);
            astChnAttr[j].u32Priority       = 0;
            astChnAttr[j].bDeflicker        = HI_FALSE;
        }
    }
    printf("Func: %s, line: %d.....\n", __FUNCTION__, __LINE__);
    return HI_SUCCESS;
}
HI_S32 SamplePciv_StartVO(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr,
    VO_VIDEO_LAYER_ATTR_S *astLayerAttr, VO_CHN_ATTR_S *astChnAttr, HI_S32 s32ChnNum)
{
    HI_S32 i, s32Ret;

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoDev);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_Disable(VoDev);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstPubAttr);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_Enable(VoDev);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoDev, &astLayerAttr[0]);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoDev);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<s32ChnNum; i++)
    {
        s32Ret = HI_MPI_VO_SetChnAttr(VoDev, i, &astChnAttr[i]);
        PCIV_CHECK_ERR(s32Ret);

        s32Ret = HI_MPI_VO_EnableChn(VoDev, i);
        PCIV_CHECK_ERR(s32Ret);
    }

    return 0;
}
HI_S32 SamplePciv_StopVO(VO_DEV VoDev, HI_S32 s32ChnNum)
{
    HI_S32 i, s32Ret;

    for (i=0; i<s32ChnNum; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoDev, i);
        PCIV_CHECK_ERR(s32Ret);
    }

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoDev);
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = HI_MPI_VO_Disable(VoDev);
    PCIV_CHECK_ERR(s32Ret);

    return 0;
}

HI_S32 SamplePciv_HostStartPciv(PCIV_CHN PcivChn, PCIV_REMOTE_OBJ_S *pstRemoteObj,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32                s32Ret;
    PCIV_ATTR_S           stPcivAttr;
    PCIV_PCIVCMD_CREATE_S stMsgCreate;

    /* 1) config pic buffer info, count/size/addr */
    SamplePcivGetPicAttr(pstPicVbAttr, &stPcivAttr.stPicAttr);

    stPcivAttr.stRemoteObj.s32ChipId = pstRemoteObj->s32ChipId;
    stPcivAttr.stRemoteObj.pcivChn   = pstRemoteObj->pcivChn;
    stPcivAttr.u32Count = 5;
    stPcivAttr.u32BlkSize = COMMON_GetPicBufferSize(stPcivAttr.stPicAttr.u32Width, stPcivAttr.stPicAttr.u32Height,
					pstPicVbAttr->enPixelFormat, DATA_BITWIDTH_8,pstPicVbAttr->enCompressMode, 0);

    s32Ret = HI_MPI_PCIV_MallocChnBuffer(PcivChn, stPcivAttr.u32BlkSize, stPcivAttr.u32Count, stPcivAttr.u64PhyAddr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("pciv malloc err, size:%d, count:%d, s32Ret:0x%x\n", stPcivAttr.u32BlkSize, stPcivAttr.u32Count, s32Ret);
        return s32Ret;
    }

    /* 2) create pciv chn */
    s32Ret = HI_MPI_PCIV_Create(PcivChn, &stPcivAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("pciv chn %d create failed with:0x%x.\n", PcivChn, s32Ret);
        return s32Ret;
    }

    /* 3) start pciv chn (now vo will display pic from slave chip) */
    s32Ret = HI_MPI_PCIV_Start(PcivChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("pciv chn %d start err \n", PcivChn);
        return s32Ret;
    }

    printf("pciv chn%d start ok, remote(%d,%d); then send msg to slave chip !\n",
        PcivChn, stPcivAttr.stRemoteObj.s32ChipId,stPcivAttr.stRemoteObj.pcivChn);

    /* 4) send msg to slave chip to start picv chn ========================================*/
    stMsgCreate.pcivChn = pstRemoteObj->pcivChn;
    memcpy(&stMsgCreate.stDevAttr, &stPcivAttr, sizeof(stPcivAttr));

    /* reconfig remote obj for slave device */
    stMsgCreate.stDevAttr.stRemoteObj.s32ChipId = 0;
    stMsgCreate.stDevAttr.stRemoteObj.pcivChn   = PcivChn;
    stMsgCreate.bAddOsd                         = Add_osd;

    /* send msg */
    s32Ret = SamplePcivSendMsgCreatePciv(pstRemoteObj->s32ChipId, &stMsgCreate);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    printf("send msg to slave chip to start pciv chn %d ok! \n\n", PcivChn);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStopPciv(PCIV_CHN PcivChn)
{
    HI_S32 s32Ret;
    PCIV_ATTR_S  stPciAttr;
    PCIV_PCIVCMD_DESTROY_S stMsgDestroy;

    /* 1, stop */
    s32Ret = HI_MPI_PCIV_Stop(PcivChn);
    PCIV_CHECK_ERR(s32Ret);

    /* 2, free */
    s32Ret = HI_MPI_PCIV_GetAttr(PcivChn, &stPciAttr);
    s32Ret = HI_MPI_PCIV_FreeChnBuffer(PcivChn, stPciAttr.u32Count);
    PCIV_CHECK_ERR(s32Ret);

    /* 3, destroy */
    s32Ret = HI_MPI_PCIV_Destroy(PcivChn);
    PCIV_CHECK_ERR(s32Ret);

    printf("start send msg to slave chip to destroy pciv chn %d\n", PcivChn);
    stMsgDestroy.pcivChn = PcivChn;
	stMsgDestroy.bAddOsd = Add_osd;
    //memcpy(&stMsgDestroy.stBindObj[0], 0, sizeof(PCIV_BIND_OBJ_S));
    s32Ret = SamplePcivSendMsgDestroy(stPciAttr.stRemoteObj.s32ChipId, &stMsgDestroy);
    PCIV_CHECK_ERR(s32Ret);
    printf("destroy pciv chn %d ok \n", PcivChn);

    return HI_SUCCESS;
}



HI_S32 SamplePcivStartHostVdecChn(HI_U32 u32VdecChnNum, HI_BOOL bReadStream,HI_BOOL bReciveStream)
{
    HI_S32              s32Ret;
    HI_S32              i;
    SAMPLE_VDEC_ATTR    astSampleVdec[VDEC_MAX_CHN_NUM];
    VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];

    for(i=0; i<u32VdecChnNum; i++)
    {
	    astSampleVdec[i].enType   						  = PT_H264;
		astSampleVdec[i].u32Width 						  = 3840;
		astSampleVdec[i].u32Height						  = 2160;
		astSampleVdec[i].enMode   						  = VIDEO_MODE_FRAME;
		astSampleVdec[i].stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IPB;
		astSampleVdec[i].stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;
		astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum = 3;
		astSampleVdec[i].u32DisplayFrameNum				  = 2;
		astSampleVdec[i].u32FrameBufCnt = astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + astSampleVdec[i].u32DisplayFrameNum + 1;
        if(bReciveStream)
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

    if(HI_TRUE == bReadStream)
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
        SAMPLE_COMM_VDEC_StartSendStream(u32VdecChnNum, &stVdecSend[0], &VdecHostThread[0]);
    }
    return s32Ret;
}

HI_S32 SamplePcivStopHostVdecChn(HI_U32 u32VdecChnNum)
{
    return HI_SUCCESS;
}

void* SamplePcivVdReStreamThread(void* arg)
{
    SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = (SAMPLE_PCIV_VDEC_CTX_S*)arg;
    HI_VOID *pReceiver = pstVdecCtx->pTransHandle;
    PCIV_STREAM_HEAD_S *pStrmHead = NULL;
    HI_U8 *pu8Addr;
    HI_U32 u32Len;
    VDEC_STREAM_S stStream;
    HI_CHAR aszFileName[64] = {0};
    //HI_S32 s32WriteLen = 0;
    static FILE *pFile[VENC_MAX_CHN_NUM] = {NULL};

    while (pstVdecCtx->bThreadStart)
    {
        /* get data from pciv stream receiver */
        if (PCIV_Trans_GetData(pReceiver, &pu8Addr, &u32Len))
        {
            usleep(10000);
            continue;
        }
        pStrmHead = (PCIV_STREAM_HEAD_S *)pu8Addr;
        HI_ASSERT(PCIV_STREAM_MAGIC == pStrmHead->u32Magic);
    #if 0
        if (0 == pstVdecCtx->VdecChn)
        {
            printf("Func: %s, Line: %d, u32Len: 0x%x, u32StreamDataLen: 0x%x, u32DMADataLen: 0x%x, head: %d, chn: %d.\n",
                __FUNCTION__, __LINE__, u32Len, pStrmHead->u32StreamDataLen, pStrmHead->u32DMADataLen, sizeof(PCIV_STREAM_HEAD_S), pstVdecCtx->VdecChn);
        }
    #endif
        HI_ASSERT(u32Len >= pStrmHead->u32DMADataLen + sizeof(PCIV_STREAM_HEAD_S));

        /* send the data to video decoder */
        stStream.pu8Addr = pu8Addr + sizeof(PCIV_STREAM_HEAD_S);
        stStream.u64PTS = 0;
        stStream.u32Len = pStrmHead->u32StreamDataLen;
        //stStream.bEndOfFrame = HI_FALSE;
        stStream.bEndOfStream = HI_FALSE;
        //printf("Func: %s, Line: %d, u32Len: 0x%x, \n", __FUNCTION__, __LINE__, stStream.u32Len);
        if (0 == stStream.u32Len)
        {
            usleep(10000);
            continue;
        }

        /* save stream data to file */
        if (NULL == pFile[pstVdecCtx->VdecChn])
        {
            sprintf(aszFileName, "host_vdec_chn%d.h264", pstVdecCtx->VdecChn);
            pFile[pstVdecCtx->VdecChn] = fopen(aszFileName, "wb");
            HI_ASSERT(pFile[pstVdecCtx->VdecChn]);
        }
        //s32WriteLen = fwrite(pu8Addr + sizeof(PCIV_STREAM_HEAD_S), pStrmHead->u32StreamDataLen, 1, pFile[pstVdecCtx->VdecChn]);
		fwrite(pu8Addr + sizeof(PCIV_STREAM_HEAD_S), pStrmHead->u32StreamDataLen, 1, pFile[pstVdecCtx->VdecChn]);
        //HI_ASSERT(1 == s32WriteLen);

        while (HI_TRUE == pstVdecCtx->bThreadStart && (HI_MPI_VDEC_SendStream(pstVdecCtx->VdecChn, &stStream, HI_IO_NOBLOCK)))
        {
            usleep(10000);
        }
        //memset(pu8Addr, 0, u32Len);
        /* release data to pciv stream receiver */
        PCIV_Trans_ReleaseData(pReceiver, pu8Addr, u32Len);
    }
    //printf("func:%s,line:%d\n",__FUNCTION__,__LINE__);
    pstVdecCtx->bThreadStart = HI_FALSE;
    return NULL;
}

HI_S32 SamplePciv_HostStartSendStream(HI_S32 s32RmtChipId,HI_S32 StreamNum)
{
    HI_S32 s32Ret;
    HI_S32 i;
    HI_U64 au64PhyAddr[4];
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_INIT_STREAM_S stInitStreamArgs;
    HI_U32 u32BufLen = SAMPLE_PCIV_VENC_STREAM_BUF_LEN;
    PCIV_TRANS_ATTR_S stStreamArgs;

    /* alloc phy buf for DMA receive stream data */
    s32Ret = HI_MPI_PCIV_Malloc(u32BufLen, StreamNum, au64PhyAddr);
    HI_ASSERT(HI_SUCCESS == s32Ret);

    stInitStreamArgs.StreamNum = StreamNum;
    for(i=0;i<StreamNum;i++)
    {
      stInitStreamArgs.au64PhyAdress[i] = au64PhyAddr[i];
    }

    /* config venc stream transfer info */
    for(i=0;i<StreamNum;i++)
    {
        SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx = &astPcivVdecCtx[i];
        stStreamArgs.bReciver = HI_TRUE;
        stStreamArgs.s32ChnId = i;
        stStreamArgs.s32MsgPortRead = pstVdecCtx->s32MsgPortRead;
        stStreamArgs.s32MsgPortWrite = pstVdecCtx->s32MsgPortWrite;
        stStreamArgs.s32RmtChip = s32RmtChipId;
        stStreamArgs.u32BufSize = u32BufLen;
        stStreamArgs.u64PhyAddr = au64PhyAddr[i];
        s32Ret = PCIV_Trans_InitReceiver(&stStreamArgs, &pstVdecCtx->pTransHandle);
        PCIV_CHECK_ERR(s32Ret);

        pstVdecCtx->bThreadStart = HI_TRUE;
        pstVdecCtx->VdecChn = stStreamArgs.s32ChnId;

        /* create thread to get stream coming from host chip, and send stream to decoder */
        pthread_create(&pstVdecCtx->pid, NULL, SamplePcivVdReStreamThread, pstVdecCtx);

        printf("init vdec:%d stream receiver in slave chip ok!\n", stStreamArgs.s32ChnId);

    }

    memcpy(stMsg.cMsgBody, &stInitStreamArgs, sizeof(stStreamArgs));
    stMsg.stMsgHead.u32Target = s32RmtChipId;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_INIT_STREAM_SEND;
    stMsg.stMsgHead.u32MsgLen = sizeof(SAMPLE_PCIV_MSG_INIT_STREAM_S);
    printf("\nSend(start send stream)message to slave\n");
    s32Ret = PCIV_SendMsg(s32RmtChipId,PCIV_MSGPORT_COMM_CMD,&stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);
    while (PCIV_ReadMsg(s32RmtChipId, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(0);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    return s32Ret;

}

HI_S32 Samplepciv_BindSlaveSrcDest(HI_S32 s32TargetId,SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_BIND_S stBindArgs;

    memset(&stBindArgs,0,sizeof(SAMPLE_PCIV_MSG_BIND_S));
    stBindArgs.DestMod.enModId = stBindInfo.DestMod.enModId;
    stBindArgs.DestMod.s32DevId = stBindInfo.DestMod.s32DevId;
    stBindArgs.DestMod.s32ChnId = stBindInfo.DestMod.s32ChnId;
    stBindArgs.SrcMod.enModId = stBindInfo.SrcMod.enModId;
    stBindArgs.SrcMod.s32DevId = stBindInfo.SrcMod.s32DevId;
    stBindArgs.SrcMod.s32ChnId = stBindInfo.SrcMod.s32ChnId;
    memcpy(&stMsg.cMsgBody,&stBindArgs,sizeof(SAMPLE_PCIV_MSG_BIND_S));

    stMsg.stMsgHead.u32Target = s32TargetId;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_BIND_SRC_DEST;
    stMsg.stMsgHead.u32MsgLen = sizeof(SAMPLE_PCIV_MSG_BIND_S);

    printf("=======PCIV_SendMsg SAMPLE_PCIV_MSG_BIND_SRC_DEST==========\n");

    s32Ret = PCIV_SendMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    return HI_SUCCESS;
}

HI_S32 Samplepciv_UnBindSlaveSrcDest(HI_S32 s32TargetId,SAMPLE_PCIV_UNBIND_SRC_INFO_S stUnBindInfo)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_UNBIND_S stUnBindArgs;

    memset(&stUnBindArgs,0,sizeof(SAMPLE_PCIV_UNBIND_SRC_INFO_S));
    stUnBindArgs.DestMod.enModId  = stUnBindInfo.DestMod.enModId;
    stUnBindArgs.DestMod.s32DevId = stUnBindInfo.DestMod.s32DevId;
    stUnBindArgs.DestMod.s32ChnId = stUnBindInfo.DestMod.s32ChnId;
    stUnBindArgs.SrcMod.enModId   = stUnBindInfo.SrcMod.enModId;
    stUnBindArgs.SrcMod.s32DevId  = stUnBindInfo.SrcMod.s32DevId;
    stUnBindArgs.SrcMod.s32ChnId  = stUnBindInfo.SrcMod.s32ChnId;
    memcpy(&stMsg.cMsgBody,&stUnBindArgs,sizeof(SAMPLE_PCIV_UNBIND_SRC_INFO_S));

    stMsg.stMsgHead.u32Target = s32TargetId;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_UNBIND_SRC_DEST;
    stMsg.stMsgHead.u32MsgLen = sizeof(SAMPLE_PCIV_UNBIND_SRC_INFO_S);

    printf("=======PCIV_SendMsg SAMPLE_PCIV_MSG_BIND_SRC_DEST==========\n");

    s32Ret = PCIV_SendMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    return HI_SUCCESS;
}

/*
 * Start all pciv chn for one vo dev,
 * @u32RmtIdx:
 * @s32RmtChipId:
 * @u32DispIdx:
 * @pstDispCtx:
 */
HI_S32 SamplePciv_StartPciv(HI_S32 s32RmtChipId,HI_S32 pcivChnNum,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32 s32Ret, j;
    PCIV_CHN RmtPcivChn, LocPcivChn;
    PCIV_REMOTE_OBJ_S stRemoteObj;

    for (j=0; j<pcivChnNum; j++)
    {
        /* 1) local pciv chn and remote pciv chn */
        LocPcivChn = j ;
        RmtPcivChn = j ;
        HI_ASSERT(LocPcivChn < PCIV_MAX_CHN_NUM);

        /* 2) remote dev and chn */
        stRemoteObj.s32ChipId   = s32RmtChipId;
        stRemoteObj.pcivChn     = RmtPcivChn;

        /* 3)start local and remote pciv chn */
        s32Ret = SamplePciv_HostStartPciv(LocPcivChn, &stRemoteObj,pstPicVbAttr);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_StopPciv(HI_U32 u32RmtIdx, HI_S32 pcivChnNum)
{
    HI_S32 j;
    PCIV_CHN LocPcivChn;

    for (j=0; j<pcivChnNum; j++)
    {
        LocPcivChn = j ;
        HI_ASSERT(LocPcivChn < PCIV_MAX_CHN_NUM);

        SamplePciv_HostStopPciv(LocPcivChn);
    }

    return HI_SUCCESS;
}

HI_S32 SamplePciv_BindSlaveSrcDestByChip(HI_S32 s32RmtChipId,SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo)
{
     HI_S32 s32Ret;

    s32Ret = Samplepciv_BindSlaveSrcDest(s32RmtChipId,stBindInfo);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SamplePciv_UnBindSlaveSrcDestByChip(HI_S32 s32RmtChipId,SAMPLE_PCIV_UNBIND_SRC_INFO_S stBindInfo)
{
     HI_S32 s32Ret;

    s32Ret = Samplepciv_UnBindSlaveSrcDest(s32RmtChipId,stBindInfo);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SamplePciv_StartPcivByChip(HI_S32 s32RmtChipId,HI_S32 pcivChnNum,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32 s32Ret;

    s32Ret = SamplePciv_StartPciv(s32RmtChipId,pcivChnNum,pstPicVbAttr);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SamplePciv_StopPcivByChip(HI_S32 s32RmtChipId, HI_U32 pcivChnNum)
{
    HI_S32 s32Ret;

    s32Ret = SamplePciv_StopPciv(s32RmtChipId, pcivChnNum);
    if (s32Ret != HI_SUCCESS)
    {
       return s32Ret;
    }
    return HI_SUCCESS;
}


HI_S32 SamplePcivInitMsgPort(HI_S32 s32RmtChip)
{
    HI_S32 i, s32Ret;
	HI_S32 s32MsgPort;
	s32MsgPort = PCIV_MSG_BASE_PORT+1;


    SAMPLE_PCIV_MSG_S stMsg;
    PCIV_MSGPORT_INIT_S stMsgPort;

    /* all venc stream use one pci transfer port */
    g_stPcivVencCtx.s32MsgPortWrite = s32MsgPort++;
    g_stPcivVencCtx.s32MsgPortRead  = s32MsgPort++;
    s32Ret = PCIV_OpenMsgPort(s32RmtChip, g_stPcivVencCtx.s32MsgPortWrite);
    s32Ret += PCIV_OpenMsgPort(s32RmtChip, g_stPcivVencCtx.s32MsgPortRead);
    HI_ASSERT(HI_SUCCESS == s32Ret);

    /* each vdec stream use one pci transfer port */
    for (i=0; i<VDEC_MAX_CHN_NUM; i++)
    {
        astPcivVdecCtx[i].s32MsgPortWrite   = s32MsgPort++;
        astPcivVdecCtx[i].s32MsgPortRead    = s32MsgPort++;
        s32Ret = PCIV_OpenMsgPort(s32RmtChip, astPcivVdecCtx[i].s32MsgPortWrite);
        s32Ret += PCIV_OpenMsgPort(s32RmtChip, astPcivVdecCtx[i].s32MsgPortRead);
        HI_ASSERT(HI_SUCCESS == s32Ret);
    }


    /* send msg port to pci slave device --------------------------------------------------*/
    stMsgPort.s32VencMsgPortW = g_stPcivVencCtx.s32MsgPortWrite;
    stMsgPort.s32VencMsgPortR = g_stPcivVencCtx.s32MsgPortRead;
    for (i=0; i<VDEC_MAX_CHN_NUM; i++)
    {
        stMsgPort.s32VdecMsgPortW[i] = astPcivVdecCtx[i].s32MsgPortWrite;
        stMsgPort.s32VdecMsgPortR[i] = astPcivVdecCtx[i].s32MsgPortRead;
    }
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_INIT_MSG_PORG;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_MSGPORT_INIT_S);
    memcpy(stMsg.cMsgBody, &stMsgPort, sizeof(PCIV_MSGPORT_INIT_S));
    printf("\n============Prepare to open venc and vdec ports!=======\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);
    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        //usleep(10000);
        usleep(1000);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
    printf("======venc and vdec ports have open!");
    return HI_SUCCESS;
}

HI_VOID SamplePcivExitMsgPort(HI_S32 s32RmtChip)
{
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S stMsg;

    /* close all stream msg port in local */
    PCIV_CloseMsgPort(s32RmtChip, g_stPcivVencCtx.s32MsgPortWrite);
    PCIV_CloseMsgPort(s32RmtChip, g_stPcivVencCtx.s32MsgPortRead);
    for (i=0; i<VDEC_MAX_CHN_NUM; i++)
    {
        PCIV_CloseMsgPort(s32RmtChip, astPcivVdecCtx[i].s32MsgPortWrite);
        PCIV_CloseMsgPort(s32RmtChip, astPcivVdecCtx[i].s32MsgPortRead);
    }

    /* close all stream msg port in remote */
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_EXIT_MSG_PORG;
    stMsg.stMsgHead.u32MsgLen = 0;
    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_EXIT_MSG_PORG==========\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);
    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);
}


static HI_S32 SamplePcivInitMpp(HI_VOID)
{
    HI_S32 s32Ret;
    VB_CONFIG_S stVbConf;

    SAMPLE_COMM_SYS_Exit();

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt              = 256;

    stVbConf.astCommPool[0].u64BlkSize   = 1920*1080*2  ;
    stVbConf.astCommPool[0].u32BlkCnt    = 10;

    stVbConf.astCommPool[3].u64BlkSize   = 12441600  ;
    stVbConf.astCommPool[3].u32BlkCnt    = 10;

    stVbConf.astCommPool[1].u64BlkSize   = 1920*1080*3/2;
    stVbConf.astCommPool[1].u32BlkCnt    = 10;

    stVbConf.astCommPool[2].u64BlkSize   = 12582912;
    stVbConf.astCommPool[2].u32BlkCnt    = 2;

    stVbConf.astCommPool[4].u64BlkSize   = 82944000;
    stVbConf.astCommPool[4].u32BlkCnt    = 2;

    stVbConf.astCommPool[5].u64BlkSize   = 12493440;
    stVbConf.astCommPool[5].u32BlkCnt    = 6;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    PCIV_CHECK_ERR(s32Ret);

    #if 0
    memset(&stVdecVbConf, 0, sizeof(VB_CONFIG_S));

    stVdecVbConf.u32MaxPoolCnt               = 2;
    stVdecVbConf.astCommPool[0].u64BlkSize   = 1920*1080*2;
    stVdecVbConf.astCommPool[0].u32BlkCnt    = 20;

    stVdecVbConf.astCommPool[1].u64BlkSize   = 1920*1080*3/2;
    stVdecVbConf.astCommPool[1].u32BlkCnt    = 40;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVdecVbConf);
    PCIV_CHECK_ERR(s32Ret);
    #endif
    return HI_SUCCESS;
}

HI_S32 SamplePcivInitComm(HI_S32 s32RmtChipId)
{
    HI_S32 s32Ret;

    /* wait for pci device connect ... ...  */
    s32Ret = PCIV_WaitConnect(s32RmtChipId);
    PCIV_CHECK_ERR(s32Ret);
    /* open pci msg port for commom cmd */
    s32Ret = PCIV_OpenMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
    PCIV_CHECK_ERR(s32Ret);

    /* open pci msg port for all stream transfer(venc stream and vdec stream) */
    s32Ret = SamplePcivInitMsgPort(s32RmtChipId);
    PCIV_CHECK_ERR(s32Ret);
    return HI_SUCCESS;
}

int SamplePcivEnumChip(int *local_id,int remote_id[HISI_MAX_MAP_DEV-1], int *count)
{
    HI_S64 fd;
    int i;
    struct hi_mcc_handle_attr attr;

    fd = open("/dev/mcc_userdev", O_RDWR);
    if (fd<=0)
    {
        printf("open mcc dev fail\n");
        return -1;
    }

    /* HI_MCC_IOC_ATTR_INIT should be sent first ! */
    if (ioctl(fd, HI_MCC_IOC_ATTR_INIT, &attr))
    {
     printf("initialization for attr failed!\n");
     return -1;
    }

    *local_id = ioctl(fd, HI_MCC_IOC_GET_LOCAL_ID, &attr);
    printf("pci local id is %d \n", *local_id);

    if (ioctl(fd, HI_MCC_IOC_GET_REMOTE_ID, &attr))
    {
        printf("get pci remote id fail \n");
        return -1;
    }
    for (i=0; i<HISI_MAX_MAP_DEV-1; i++)
    {
        if (-1 == attr.remote_id[i]) break;
        *(remote_id++) = attr.remote_id[i];
        printf("get pci remote id : %d \n", attr.remote_id[i]);
    }

    *count = i;

    printf("===================close port %d!\n", attr.port);
    close(fd);
    return 0;
}

HI_S32 SamplePcivGetPfWin(HI_S32 s32ChipId)
{
    HI_S32 s32Ret;
    PCIV_BASEWINDOW_S stPciBaseWindow;
    s32Ret = HI_MPI_PCIV_GetBaseWindow(s32ChipId, &stPciBaseWindow);
    PCIV_CHECK_ERR(s32Ret);
    printf("pci device %d -> slot:%d, pf:0x%llu,np:0x%llu,cfg:0x%llu\n", s32ChipId, s32ChipId-1,
        stPciBaseWindow.u64PfWinBase,stPciBaseWindow.u64NpWinBase,stPciBaseWindow.u64CfgWinBase);
    g_u64PfWinBase[s32ChipId] = stPciBaseWindow.u64PfWinBase;
    return HI_SUCCESS;
}

int SamplePcivSwitchPic(HI_U32 u32RmtIdx, HI_S32 s32RemoteChip, VO_DEV VoDev)
{
    #if 0
    char ch;
    HI_S32 i, k=0, s32Ret, u32DispIdx, s32VoPicDiv;
    HI_U32 u32Width, u32Height;
#if TEST_OTHER_DIVISION
    HI_S32 as32VoPicDiv[] = {1, 4, 9, 16, 76, 100, 114, 132, 174, 200, 242, 300, 320, 640};
#else
    HI_S32 as32VoPicDiv[] = {1, 4, 9, 16};
#endif
    SAMPLE_PCIV_DISP_CTX_S *pstDispCtx = NULL;

    /* find display contex by vo dev */
    for (u32DispIdx=0; u32DispIdx<VO_MAX_DEV_NUM; u32DispIdx++)
    {
        if (VoDev == s_astPcivDisp[u32DispIdx].VoDev)
        {
            pstDispCtx = &s_astPcivDisp[u32DispIdx];
            break;
        }
    }
    if (NULL == pstDispCtx) return HI_FAILURE;

    s32VoPicDiv = pstDispCtx->u32PicDiv;

    while (1)
    {
        printf("\n >>>> Please input commond as follow : \n");
        printf("\t ENTER : switch display div , 1 -> 4-> 9 -> 16 -> 1 -> ... \n");
        printf("\t s : step play by one frame \n");
        printf("\t q : quit the pciv sample \n");
        printf("------------------------------------------------------------------\n");

        ch = getchar();
        if (ch == 'q')
            break;
        else if ((1 == vdec_idx) && ('s' == ch))
        {
            //s32Ret = HI_MPI_VO_ChnStep(VoDev, 0);
            //PCIV_CHECK_ERR(s32Ret);
        }
        else
        {
            /* 1, disable all vo chn */
            for (i=0; i<s32VoPicDiv; i++)
            {
                HI_MPI_VO_DisableChn(VoDev, i);
            }

            /* 2, stop all pciv chn */
            SamplePciv_StopPcivByVo(u32RmtIdx, s32RemoteChip, u32DispIdx, pstDispCtx);

            /* switch pic 1 -> 4-> 9 -> 16 -> 1 ->... */
            s32VoPicDiv = as32VoPicDiv[(k++)%(sizeof(as32VoPicDiv)/sizeof(HI_S32))];

            /* 3, restart pciv chn by new pattern */
            pstDispCtx->u32PicDiv  = s32VoPicDiv;
            pstDispCtx->VoChnEnd   = pstDispCtx->VoChnStart + s32VoPicDiv - 1;
            s32Ret = SamplePciv_StartPcivByVo(u32RmtIdx, s32RemoteChip, u32DispIdx, pstDispCtx);
            PCIV_CHECK_ERR(s32Ret);

            /* 4, recfg vo chn and enable them by new pattern */
            u32Width  = 720;
            u32Height = 576;

            s32Ret = SAMPLE_SetVoChnMScreen(VoDev, s32VoPicDiv, u32Width, u32Height);
            PCIV_CHECK_ERR(s32Ret);

            for (i=0; i<s32VoPicDiv; i++)
            {
                s32Ret = HI_MPI_VO_EnableChn(VoDev, i);
                PCIV_CHECK_ERR(s32Ret);
            }
        }
    }
    #endif
    return HI_SUCCESS;
}


HI_S32 SamplePcivBindHostChipVdecVpss(HI_U32 u32VdecChnNum, HI_U32 u32VpssGrpCnt)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = 0; i < u32VdecChnNum; i++)
    {
        stSrcChn.enModId = HI_ID_VDEC;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = i;

        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = i;
        stDestChn.s32ChnId = 0;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivBindHostChipVpssVo(HI_U32 u32VpssCnt, VO_DEV VoDev)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = 0; i < u32VpssCnt; i++)
    {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = i;
        stSrcChn.s32ChnId = 0;

        stDestChn.enModId = HI_ID_VO;
        stDestChn.s32DevId = VoDev;
        stDestChn.s32ChnId = i;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivBindHostChipVpssVenc(HI_U32 u32VpssCnt, HI_U32 u32VpssChn)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = 0; i < u32VpssCnt; i++)
    {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = i;
        stSrcChn.s32ChnId = u32VpssChn;

		stDestChn.enModId = HI_ID_VENC;
		stDestChn.s32DevId= 0;
        stDestChn.s32ChnId = i;


        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}


HI_S32 SamplePcivUnBindHostChipVpssVo(HI_U32 u32VpssCnt,  VO_DEV VoDev)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = 0; i < u32VpssCnt; i++)
    {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = i;
        stSrcChn.s32ChnId = 0;

        stDestChn.enModId = HI_ID_VO;
        stDestChn.s32DevId = VoDev;
        stDestChn.s32ChnId = i;
        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivUnBindHostChipVpssVenc(HI_U32 u32VpssCnt, HI_U32 u32VpssChn)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = 0; i < u32VpssCnt; i++)
    {
        stSrcChn.enModId = HI_ID_VPSS;
        stSrcChn.s32DevId = i;
        stSrcChn.s32ChnId = u32VpssChn;

		stDestChn.enModId = HI_ID_VENC;
		stDestChn.s32DevId= 0;
		stDestChn.s32ChnId = i;

        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 SamplehostPcivBindSrcDest(SAMPLE_PCIV_BIND_SRC_INFO_S *pstBindArgs)
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

HI_S32 SamplehostPcivUnBindSrcDest(SAMPLE_PCIV_UNBIND_SRC_INFO_S *pstBindArgs)
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

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SamplePcivBindHostChipPcivVpss(HI_U32 u32VpssCnt, HI_U32 u32VpssChn, HI_U32 u32VpssChnStart)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = u32VpssChnStart; i <= u32VpssCnt; i++)
    {
        stSrcChn.enModId = HI_ID_PCIV;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = 0;

        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = i;
        stDestChn.s32ChnId = u32VpssChn;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}


HI_S32 SamplePcivUnBindHostChipPcivVpss(HI_U32 u32VpssCnt)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn, stDestChn;

    for (i = 0; i < u32VpssCnt; i++)
    {
        stSrcChn.enModId = HI_ID_PCIV;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = i;

        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = i;
        stDestChn.s32ChnId = 0;
        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        PCIV_CHECK_ERR(s32Ret);
    }

    return HI_SUCCESS;
}


HI_S32 VDEC_SendEos(VDEC_CHN Vdchn)
{
    return HI_SUCCESS;
}

HI_S32 SamplePcivStartHostVpss(VPSS_MODE_S  *pstVpssMode,PICVB_ATTR_S *pstPicVbAttr)
{
    HI_S32 s32Ret;
    HI_S32 iLoop;
    HI_BOOL         abChnEnable[4] = {0,0,0,0};
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    SIZE_S          stSize;
    PIC_SIZE_E      enPicSize;
    HI_S32          s32VpssChnCnt;
    HI_S32          u32VpssGrpCnt;

    s32VpssChnCnt = pstVpssMode->s32VpssChnCnt;
    u32VpssGrpCnt = pstVpssMode->s32VpssGroupCnt;

    enPicSize = pstPicVbAttr->enDispPicSize;
    SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    for (iLoop = 0; iLoop < s32VpssChnCnt; iLoop++)
    {
        abChnEnable[iLoop] = HI_TRUE;
    }

    /* create VPSS group in master pciv*/

    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = pstPicVbAttr->enPixelFormat;
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
            stVpssChnAttr[iLoop].enChnMode                    = pstVpssMode->enChnMode;
            stVpssChnAttr[iLoop].enCompressMode               = pstPicVbAttr->enCompressMode;
            stVpssChnAttr[iLoop].enDynamicRange               = DYNAMIC_RANGE_SDR8;
            stVpssChnAttr[iLoop].enPixelFormat                = pstPicVbAttr->enPixelFormat;
            stVpssChnAttr[iLoop].stFrameRate.s32SrcFrameRate  = -1;
            stVpssChnAttr[iLoop].stFrameRate.s32DstFrameRate  = -1;
            stVpssChnAttr[iLoop].u32Depth                     = 0;
            stVpssChnAttr[iLoop].bMirror                      = HI_FALSE;
            stVpssChnAttr[iLoop].bFlip                        = HI_FALSE;
            stVpssChnAttr[iLoop].enVideoFormat                = pstPicVbAttr->enVideoFormat;
            stVpssChnAttr[iLoop].stAspectRatio.enMode         = ASPECT_RATIO_NONE;
        }

    }

    for (iLoop=0;iLoop<u32VpssGrpCnt;iLoop++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Start(iLoop, abChnEnable,&stVpssGrpAttr,stVpssChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}



HI_S32 SamplePcivStopHostVpss(VPSS_MODE_S  *pstVpssMode)
{
    HI_S32    s32Ret;
    HI_BOOL   abChnEnable[4] = {0,0,0,0};
    HI_S32    iLoop;
    HI_S32 u32VpssGroupCnt;
    HI_S32 s32VpssChnCnt;

    u32VpssGroupCnt = pstVpssMode->s32VpssGroupCnt;
    s32VpssChnCnt   = pstVpssMode->s32VpssChnCnt;

    for (iLoop = 0; iLoop < s32VpssChnCnt; iLoop++)
    {
        abChnEnable[iLoop] = HI_TRUE;
    }

    for (iLoop =0;iLoop < u32VpssGroupCnt;iLoop++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Stop(iLoop, abChnEnable);
        if (s32Ret != HI_SUCCESS)
        {
            return s32Ret;
        }
    }
    return HI_SUCCESS;
}

HI_S32 SamplePcivStartHostVoDev(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
    HI_S32 s32Ret;
    SAMPLE_COMM_VO_GetDefConfig(pstVoConfig);

    pstVoConfig->VoDev                                    = SAMPLE_VO_DEV_DHD0;
    pstVoConfig->enVoIntfType                             = VO_INTF_HDMI;
    pstVoConfig->u32DisBufLen                             = 3;

    s32Ret= SAMPLE_COMM_VO_StartVO(pstVoConfig);
    if(HI_SUCCESS!=s32Ret)
    {
       printf("SAMPLE_COMM_VO_Start failed! s32Ret:0x%x.\n", s32Ret);
       return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SamplePcivStopHostVoDev(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
    HI_S32 s32Ret;
    s32Ret = SAMPLE_COMM_VO_StopVO(pstVoConfig);
    if(HI_SUCCESS!=s32Ret)
    {
        printf("SAMPLE_COMM_VO_Stop failed! s32Ret:0x%x.\n", s32Ret);
    }
    return HI_SUCCESS;
}

HI_S32 SamplePcivStartHostVenc(HI_U32 u32VencChnNum, PAYLOAD_TYPE_E enType, PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode)
{
	HI_S32 i;
	HI_S32 s32Ret;
    VENC_GOP_ATTR_S stGopAttr;
    HI_S32 VencChn[2]={0,1};

	for (i=0; i< u32VencChnNum; i++)
	{
		s32Ret = SAMPLE_COMM_VENC_Start(i, enType, enSize,enRcMode, 0, &stGopAttr);
		if (HI_SUCCESS != s32Ret)
	    {
	        printf("SAMPLE_COMM_VENC_Start failed! s32Ret:0x%x.\n", s32Ret);
        	return s32Ret;
	    }
	}

	s32Ret = SAMPLE_COMM_VENC_StartGetStream(VencChn,u32VencChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VENC_StartGetStream failed! s32Ret:0x%x.\n", s32Ret);
		for (i=0; i< u32VencChnNum; i++)
		{
			SAMPLE_COMM_VENC_Stop(i);
		}

        return s32Ret;
    }

	return HI_SUCCESS;

}


HI_S32 SamplePcivStopHostVenc(HI_U32 u32VencChnNum)
{
	HI_S32 i;
	HI_S32 s32Ret;

	s32Ret = SAMPLE_COMM_VENC_StopGetStream();
    if (HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_COMM_VENC_StopGetStream failed! s32Ret:0x%x.\n", s32Ret);
		for (i=0; i< u32VencChnNum; i++)
		{
			SAMPLE_COMM_VENC_Stop(i);
		}

        return s32Ret;
    }

	for (i=0; i< u32VencChnNum; i++)
	{
		s32Ret = SAMPLE_COMM_VENC_Stop(i);
		if (HI_SUCCESS != s32Ret)
	    {
	        printf("SAMPLE_COMM_VENC_Start failed! s32Ret:0x%x.\n", s32Ret);
        	return s32Ret;
	    }
	}

	return HI_SUCCESS;
}


HI_VOID *SamplePciv_RevVeStrmThread(HI_VOID *arg)
{
    SAMPLE_PCIV_VENC_CTX_S *pstCtx = (SAMPLE_PCIV_VENC_CTX_S*)arg;
    HI_VOID *pReceiver = pstCtx->pTransHandle;
    PCIV_STREAM_HEAD_S *pStrmHead = NULL;
    HI_U8 *pu8Addr, *pu8AddrTmp;
    HI_U32 u32Len;
    HI_S32 s32VencChn;
#if STREAM_SAVE_FILE
    HI_CHAR aszFileName[64] = {0};
    HI_S32 s32WriteLen = 0;
    static FILE *pFile[VENC_MAX_CHN_NUM] = {NULL};
#endif
    prctl(PR_SET_NAME, (unsigned long)"hi_RevStrm", 0, 0, 0);

    while (pstCtx->bThreadStart)
    {
        /* get data from pciv stream receiver */
        if (PCIV_Trans_GetData(pReceiver, &pu8Addr, &u32Len))
        {
            //usleep(0);
            continue;
        }
        //usleep(100000);
        for (pu8AddrTmp = pu8Addr; pu8AddrTmp < (pu8Addr + u32Len); )
        {
            pStrmHead = (PCIV_STREAM_HEAD_S *)pu8AddrTmp;

            HI_ASSERT(PCIV_STREAM_MAGIC == pStrmHead->u32Magic);

            s32VencChn = pStrmHead->s32ChnID;
            HI_ASSERT(s32VencChn < VENC_MAX_CHN_NUM);

#if STREAM_SAVE_FILE
            /* save stream data to file */
            if (NULL == pFile[s32VencChn])
            {
            	if(PT_H265 == pstCtx->enType)
            	{
                	sprintf(aszFileName, "host_venc_chn%d.h265", s32VencChn);
            	}
				else if(PT_H264 == pstCtx->enType)
				{
					sprintf(aszFileName, "host_venc_chn%d.h264", s32VencChn);
				}
				else
				{
					sprintf(aszFileName, "host_venc_chn%d.stream", s32VencChn);
					printf("Unknown The Stream Type!!\n");
				}

                pFile[s32VencChn] = fopen(aszFileName, "wb");
                HI_ASSERT(pFile[s32VencChn]);
            }
            s32WriteLen = fwrite(pu8AddrTmp + sizeof(PCIV_STREAM_HEAD_S), pStrmHead->u32StreamDataLen, 1, pFile[s32VencChn]);
            HI_ASSERT(1 == s32WriteLen);
#endif

            pu8AddrTmp += (sizeof(PCIV_STREAM_HEAD_S) + pStrmHead->u32DMADataLen);
            HI_ASSERT(pu8AddrTmp <= (pu8Addr + u32Len));
        }

        /* release data to pciv stream receiver */

        PCIV_Trans_ReleaseData(pReceiver, pu8Addr, u32Len);
        pstCtx->u32Seq ++;
    }

    pstCtx->bThreadStart = HI_FALSE;
    return NULL;
}


HI_S32 SamplePciv_HostStartVenc(HI_S32 s32RmtChip, HI_U32 u32VechCnt,
    PIC_SIZE_E aenVencSize[2],PAYLOAD_TYPE_E aenType[2])
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S      stMsg;
    PCIV_VENCCMD_INIT_S stVencMsg;

    HI_U32 u32BufLen = SAMPLE_PCIV_VENC_STREAM_BUF_LEN;
    PCIV_TRANS_ATTR_S *pstStreamArgs;
    SAMPLE_PCIV_VENC_CTX_S *pstVencCtx = &g_stPcivVencCtx;

    if (!u32VechCnt) return HI_SUCCESS;

    /* config venc chn info */
    stVencMsg.u32GrpCnt = u32VechCnt;
    stVencMsg.aenSize[0] = aenVencSize[0];
    stVencMsg.aenSize[1] = aenVencSize[1];

    stVencMsg.aenType[0] = aenType[0];
    stVencMsg.aenType[1] = aenType[1];

    /* alloc phy buf for DMA receive stream data */
    s32Ret = HI_MPI_PCIV_Malloc(u32BufLen, 1, au64PhyAddr);
    HI_ASSERT(HI_SUCCESS == s32Ret);

    /* config venc stream transfer info */
    pstStreamArgs = &stVencMsg.stStreamArgs;
    pstStreamArgs->u32BufSize = u32BufLen;
    pstStreamArgs->u64PhyAddr = au64PhyAddr[0];
    pstStreamArgs->s32RmtChip = s32RmtChip;
    printf("\nMalloc local buf succed! Address is 0x%llu.\n", au64PhyAddr[0]);
    /* notes:msg port have open when init */
    pstStreamArgs->s32MsgPortWrite = pstVencCtx->s32MsgPortWrite;
    pstStreamArgs->s32MsgPortRead  = pstVencCtx->s32MsgPortRead;

    /* init stream recerver */
    s32Ret = PCIV_Trans_InitReceiver(pstStreamArgs, &pstVencCtx->pTransHandle);
    HI_ASSERT(HI_FAILURE != s32Ret);

    /* send msg to slave chip to create venc chn and init stream transfer */
    stMsg.stMsgHead.u32Target = s32RmtChip;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_INIT_ALL_VENC;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_VENCCMD_INIT_S);
    memcpy(stMsg.cMsgBody, &stVencMsg, sizeof(PCIV_VENCCMD_INIT_S));
    printf("\nSend (start venc) message to slave!\n");
    s32Ret = PCIV_SendMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);
    while (PCIV_ReadMsg(s32RmtChip, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(0);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    printf("Pciv venc init ok, remote:%d, grpcnt:%d =======\n", s32RmtChip, u32VechCnt);

    /* create thread to reveive venc stream from slave chip */
    pstVencCtx->bThreadStart = HI_TRUE;
	pstVencCtx->enType		 = aenType[0];
    s32Ret = pthread_create(&pstVencCtx->pid, NULL, SamplePciv_RevVeStrmThread, pstVencCtx);

    return HI_SUCCESS;
}

HI_S32 SamplePciv_HostStopVenc(HI_S32 s32TargetId, HI_U32 u32GrpCnt, HI_BOOL bHaveMinor)
{
    HI_S32 s32Ret;
    SAMPLE_PCIV_MSG_S   stMsg;
    PCIV_VENCCMD_EXIT_S stVencMsg;
    SAMPLE_PCIV_VENC_CTX_S *pstVencCtx = &g_stPcivVencCtx;

    if (!u32GrpCnt) return HI_SUCCESS;

    stVencMsg.u32GrpCnt = u32GrpCnt;
    stVencMsg.bHaveMinor = bHaveMinor;

    /* alloc phy buf for DMA receive stream data */
    s32Ret = HI_MPI_PCIV_Free(1, au64PhyAddr);
    HI_ASSERT(HI_SUCCESS == s32Ret);
    au64PhyAddr[0] = HI_NULL;

    /* send msg to slave chip to exit all venc chn */
    stMsg.stMsgHead.u32Target = s32TargetId;
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_MSG_EXIT_ALL_VENC;
    stMsg.stMsgHead.u32MsgLen = sizeof(PCIV_VENCCMD_INIT_S);
    memcpy(stMsg.cMsgBody, &stVencMsg, sizeof(PCIV_VENCCMD_EXIT_S));
    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_EXIT_ALL_VENC==========\n");
    s32Ret = PCIV_SendMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);
    while (PCIV_ReadMsg(s32TargetId, PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(0);
    }
    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    /* exit the thread of getting venc stream */
    if (pstVencCtx->bThreadStart)
    {
        pstVencCtx->bThreadStart = HI_FALSE;
        pthread_join(pstVencCtx->pid, 0);
    }

    /* deinit venc stream receiver */
    PCIV_Trans_DeInitReceiver(pstVencCtx->pTransHandle);

    printf("pciv venc exit ok !========= \n");
    return HI_SUCCESS;
}
HI_S32 SampleInitMediaSyS(HI_VOID)
{
    HI_S32 s32Ret;

    /* system init ----------------------------------------------------------------------- */
    /* Init mpp sys and video buffer */
    s32Ret = SamplePcivInitMpp();
    PCIV_CHECK_ERR(s32Ret);

    s32Ret = SAMPLE_COMM_SYS_MemConfig();
    PCIV_CHECK_ERR(s32Ret);
    return s32Ret;
}

HI_S32 SampleInitPcie(int *ps32PciLocalId,int as32PciRmtId[HISI_MAX_MAP_DEV-1], int *ps32AllPciRmtCnt)
{
    HI_S32 s32Ret;
    HI_S32 i;
    /* Get pci local id and all target id */
    s32Ret = SamplePcivEnumChip(ps32PciLocalId, as32PciRmtId, ps32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<*ps32AllPciRmtCnt; i++)
    {
        /* Wait for slave dev connect, and init message communication */
        s32Ret = SamplePcivInitComm(as32PciRmtId[i]);
        PCIV_CHECK_ERR(s32Ret);

        /* Get PCI PF Window info of pci device, used for DMA trans that host to slave */
        (HI_VOID)SamplePcivGetPfWin(as32PciRmtId[i]);
    }
    return s32Ret;
}

HI_S32 SampleHostPcivBindVpss(HI_S32 PcivChnNum)
{
    HI_S32     i;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    for (i=0; i< PcivChnNum;i++)
    {
        stSrcChn.enModId   = HI_ID_PCIV;
        stSrcChn.s32DevId  = 0;
        stSrcChn.s32ChnId  = i;

        stDestChn.enModId  = HI_ID_VPSS;
        stDestChn.s32DevId = i;
        stDestChn.s32ChnId = VPSS_CHN0;

        CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(PCIV-VPSS)");
    }

    return HI_SUCCESS;
}

HI_S32 SampleHostPcivUnBindVpss(HI_S32 PcivChnNum)
{
    HI_S32     i;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    for (i=0; i< PcivChnNum;i++)
    {
        stSrcChn.enModId   = HI_ID_PCIV;
        stSrcChn.s32DevId  = 0;
        stSrcChn.s32ChnId  = i;

        stDestChn.enModId  = HI_ID_VPSS;
        stDestChn.s32DevId = i;
        stDestChn.s32ChnId = VPSS_CHN0;

        CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind(PCIV-VPSS)");
    }

    return HI_SUCCESS;
}

HI_S32 SampleHostPcivBindVo(HI_S32 PcivChnNum)
{
    HI_S32   iLoop;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    for (iLoop=0; iLoop< PcivChnNum;iLoop++)
    {
        stSrcChn.enModId   = HI_ID_PCIV;
        stSrcChn.s32DevId  = 0;
        stSrcChn.s32ChnId  = iLoop;

        stDestChn.enModId  = HI_ID_VO;
        stDestChn.s32DevId = 0;
        stDestChn.s32ChnId = iLoop;

        CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(PCIV-VO)");
    }
    return HI_SUCCESS;
}

HI_S32 SampleHostPcivUnBindVo(HI_S32 PcivChnNum)
{
    HI_S32   iLoop;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    for (iLoop=0; iLoop< PcivChnNum;iLoop++)
    {
        stSrcChn.enModId   = HI_ID_PCIV;
        stSrcChn.s32DevId  = 0;
        stSrcChn.s32ChnId  = iLoop;

        stDestChn.enModId  = HI_ID_VO;
        stDestChn.s32DevId = 0;
        stDestChn.s32ChnId = iLoop;

        CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind(PCIV-VO)");
    }
    return HI_SUCCESS;

}

HI_S32 SampleViVpssSlaveSendPicToHostVpssVoDisplayMode(HI_VOID)
{
    HI_S32           s32Ret;
    HI_S32           i,j, s32RmtChipId;
    HI_S32           s32PciLocalId, as32PciRmtId[PCIV_MAX_CHIPNUM], s32AllPciRmtCnt;
    SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo;
    SAMPLE_PCIV_UNBIND_SRC_INFO_S stUnBindInfo;
    //Vi Param
	HI_S32           s32ViChnCnt;

    //VPSS Param
    VPSS_MODE_S      stVpssMode;
    //PCIV Param
    HI_S32           pcivChnNum;

    PICVB_ATTR_S     stPicVbAttr;

    //VO Param
    SAMPLE_VO_CONFIG_S stVoConfig;

	//Venc Param
    HI_U32           u32VencChnNum = 1;
    PIC_SIZE_E       aenVencSize[2] = {PIC_3840x2160, PIC_CIF};//only use aenVencSize[0]
    PAYLOAD_TYPE_E   aenType[2] = {PT_H264, PT_H264};//only use aenType[0]

    /*******Init MediaSys********/
    s32Ret = SampleInitMediaSyS();
    PCIV_CHECK_ERR(s32Ret);

    /*******Init PCIE*************/
    s32Ret = SampleInitPcie(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);

    stPicVbAttr.enCompressMode = COMPRESS_MODE_NONE;
    stPicVbAttr.enDispPicSize  = PIC_3840x2160;
    stPicVbAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stPicVbAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
    stPicVbAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    /* Init ------------------------------------------------------------------------------ */
    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /* init vi in slave chip */
        s32ViChnCnt =1;
        s32Ret = SamplePciv_HostStartVi(s32RmtChipId, s32ViChnCnt,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* start slave chip vpss */
        stVpssMode.s32VpssGroupCnt = 1;
        stVpssMode.s32VpssChnCnt   = 1;
        stVpssMode.enChnMode = VPSS_CHN_MODE_USER;
        s32Ret = SamplePciv_StartVpssByChip(s32RmtChipId, &stVpssMode,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* bind vi and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VI;
            stBindInfo.SrcMod.s32DevId = 0;
            stBindInfo.SrcMod.s32ChnId = j;

            stBindInfo.DestMod.enModId = HI_ID_VPSS;
            stBindInfo.DestMod.s32DevId = j;
            stBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start host and slave chip pciv */
        pcivChnNum =1;
        s32Ret = SamplePciv_StartPcivByChip(s32RmtChipId,pcivChnNum,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /*bind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stBindInfo.SrcMod.s32DevId = j;
            stBindInfo.SrcMod.s32ChnId = 0;

            stBindInfo.DestMod.enModId = HI_ID_PCIV;
            stBindInfo.DestMod.s32DevId = 0;
            stBindInfo.DestMod.s32ChnId = j;
            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /*At the Same Time To Venc*/
        s32Ret = SamplePciv_HostStartVenc(s32RmtChipId, u32VencChnNum, aenVencSize, aenType);
        PCIV_CHECK_ERR(s32Ret);

        /*bind vpss and venc in slae chip*/
        stBindInfo.SrcMod.enModId = HI_ID_VPSS;
        stBindInfo.SrcMod.s32DevId = 0;
        stBindInfo.SrcMod.s32ChnId = 0;

        stBindInfo.DestMod.enModId = HI_ID_VENC;
        stBindInfo.DestMod.s32DevId = 0;
        stBindInfo.DestMod.s32ChnId = 0;
        s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* start host chip vo dev  */
    stVoConfig.enIntfSync        = VO_OUTPUT_3840x2160_30;
    stVoConfig.enPicSize         = PIC_3840x2160;
    stVoConfig.enDstDynamicRange = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode          = VO_MODE_1MUX;
    s32Ret = SamplePcivStartHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /*bind host pciv and vo*/
    s32Ret = SampleHostPcivBindVo(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    printf("\n#################Press enter to exit########################\n");
    getchar();
    /* Exit ------------------------------------------------------------------------------ */

    /*unbind pciv and vo*/
    s32Ret = SampleHostPcivUnBindVo(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* stop host chip vo */
    s32Ret = SamplePcivStopHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /*Unbind vpss and venc in slae chip*/
        stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
        stUnBindInfo.SrcMod.s32DevId = 0;
        stUnBindInfo.SrcMod.s32ChnId = 0;

        stUnBindInfo.DestMod.enModId = HI_ID_VENC;
        stUnBindInfo.DestMod.s32DevId = 0;
        stUnBindInfo.DestMod.s32ChnId = 0;
        s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
        PCIV_CHECK_ERR(s32Ret);

		/* stop venc and stream transfer (venc is in slave and send stream to host) */
        s32Ret = SamplePciv_HostStopVenc(s32RmtChipId, u32VencChnNum, HI_TRUE);
        PCIV_CHECK_ERR(s32Ret);

        for (j= 0;j< 1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stUnBindInfo.SrcMod.s32DevId = j;
            stUnBindInfo.SrcMod.s32ChnId = 0;

            stUnBindInfo.DestMod.enModId = HI_ID_PCIV;
            stUnBindInfo.DestMod.s32DevId = 0;
            stUnBindInfo.DestMod.s32ChnId = j;
            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop host and slave chip pciv */
        s32Ret = SamplePciv_StopPcivByChip(s32RmtChipId, pcivChnNum);
        PCIV_CHECK_ERR(s32Ret);

        /*Unbind vi and vpss*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VI;
            stUnBindInfo.SrcMod.s32DevId = 0;
            stUnBindInfo.SrcMod.s32ChnId = j;

            stUnBindInfo.DestMod.enModId = HI_ID_VPSS;
            stUnBindInfo.DestMod.s32DevId = j;
            stUnBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop slave chip vpss */
        s32Ret = SamplePciv_StopVpssByChip(s32RmtChipId, &stVpssMode);
        PCIV_CHECK_ERR(s32Ret);

        /* stop slave chip vi*/
        s32Ret = SamplePciv_HostExitVi(s32RmtChipId,s32ViChnCnt);
        PCIV_CHECK_ERR(s32Ret);

        /* close all msg port */
        SamplePcivExitMsgPort(s32RmtChipId);

        s32Ret = PCIV_CloseMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* Exit whole mpp sys  */
    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}

HI_S32 SampleVdecVpssSlaveSendPicToHostVpssVoDisplayMode(HI_VOID)
{
    HI_S32    s32Ret;
    HI_S32    s32PciLocalId, as32PciRmtId[PCIV_MAX_CHIPNUM],s32AllPciRmtCnt;
    HI_S32    s32PciRmtChipCnt = 1;
    HI_S32    i,j, s32RmtChipId;
    HI_U32    u32VdecCnt = 1;
    SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo;
    SAMPLE_PCIV_UNBIND_SRC_INFO_S stUnBindInfo;
    VDEC_MODE_S stVdecMode;

    //pciv param
    HI_S32 pcivChnNum;

    //VPSS Param
    VPSS_MODE_S      stVpssMode;

    PICVB_ATTR_S     stPicVbAttr;

    //vo param
    SAMPLE_VO_CONFIG_S stVoConfig;

    /*******Init MediaSys********/
    s32Ret = SampleInitMediaSyS();
    PCIV_CHECK_ERR(s32Ret);

    /*******Init PCIE*************/
    s32Ret = SampleInitPcie(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);

    stPicVbAttr.enCompressMode = COMPRESS_MODE_NONE;
    stPicVbAttr.enDispPicSize  = PIC_3840x2160;
    stPicVbAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stPicVbAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
    stPicVbAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;

    /* Init ------------------------------------------------------------------------------ */
    test_idx =1;
    for (i=0; i<s32PciRmtChipCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /* start slave chip vdec and read stream file */
        stVdecMode.bReadStream   = HI_TRUE;
        stVdecMode.bReciveStream = HI_FALSE;
        stVdecMode.enPicSize     = PIC_3840x2160;
        stVdecMode.enPayLoadType = PT_H264;
        stVdecMode.enDisplayMode = VIDEO_DISPLAY_MODE_PREVIEW;
        s32Ret = SamplePciv_HostStartVdecChn(s32RmtChipId, u32VdecCnt, &stVdecMode);
        PCIV_CHECK_ERR(s32Ret);

        /* start slave chip vpss */
        stVpssMode.s32VpssGroupCnt = 1;
        stVpssMode.s32VpssChnCnt   = 1;
        stVpssMode.enChnMode = VPSS_CHN_MODE_AUTO;
        s32Ret = SamplePciv_StartVpssByChip(s32RmtChipId, &stVpssMode,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* bind vdec and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VDEC;
            stBindInfo.SrcMod.s32DevId = 0;
            stBindInfo.SrcMod.s32ChnId = j;

            stBindInfo.DestMod.enModId = HI_ID_VPSS;
            stBindInfo.DestMod.s32DevId = j;
            stBindInfo.DestMod.s32ChnId = 0;
            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start host and slave chip pciv */
        pcivChnNum =1;
        s32Ret = SamplePciv_StartPcivByChip(s32RmtChipId,pcivChnNum,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* bind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stBindInfo.SrcMod.s32DevId = j;
            stBindInfo.SrcMod.s32ChnId = 0;

            stBindInfo.DestMod.enModId = HI_ID_PCIV;
            stBindInfo.DestMod.s32DevId = 0;
            stBindInfo.DestMod.s32ChnId = j;

            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }
    }

    /* start host chip vpss group */
    s32Ret = SamplePcivStartHostVpss(&stVpssMode,&stPicVbAttr);
    PCIV_CHECK_ERR(s32Ret);

    /*bind host pciv and VPSS*/
    s32Ret = SampleHostPcivBindVpss(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* start host chip vo dev  */
    stVoConfig.enIntfSync                               = VO_OUTPUT_3840x2160_30;
    stVoConfig.enPicSize                                = PIC_3840x2160;
    stVoConfig.enDstDynamicRange                        = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode                                 = VO_MODE_1MUX;
    s32Ret = SamplePcivStartHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /**bind host vpss and vo**/
    for(i=0;i<1;i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Bind_VO(i,VPSS_CHN0,0,i);
        PCIV_CHECK_ERR(s32Ret);
    }

    printf("\n#################Press enter to exit########################\n");
    getchar();

    /* Exit ------------------------------------------------------------------------------ */

    /**Unbind host vpss and vo**/
    for(i=0;i<1;i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i,VPSS_CHN0,0,i);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* stop  host chip vo */
    s32Ret= SamplePcivStopHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /*Unbind host pciv and VPSS*/
    s32Ret = SampleHostPcivUnBindVpss(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* stop  host chip vpss */
    s32Ret=SamplePcivStopHostVpss(&stVpssMode);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<s32PciRmtChipCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /* unbind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stUnBindInfo.SrcMod.s32DevId = 0;
            stUnBindInfo.SrcMod.s32ChnId = j;

            stUnBindInfo.DestMod.enModId = HI_ID_PCIV;
            stUnBindInfo.DestMod.s32DevId = 0;
            stUnBindInfo.DestMod.s32ChnId = j;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop host and slave chip pciv */
        s32Ret = SamplePciv_StopPcivByChip(s32RmtChipId,pcivChnNum);
        PCIV_CHECK_ERR(s32Ret);

        /* unbind vi and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VDEC;
            stUnBindInfo.SrcMod.s32DevId = 0;
            stUnBindInfo.SrcMod.s32ChnId = j;

            stUnBindInfo.DestMod.enModId = HI_ID_VPSS;
            stUnBindInfo.DestMod.s32DevId = j;
            stUnBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop slave chip vpss */
        s32Ret = SamplePciv_StopVpssByChip(s32RmtChipId, &stVpssMode);
        PCIV_CHECK_ERR(s32Ret);

        /* stop slave chip vdec and read stream file */
        s32Ret = SamplePciv_HostStopVdecChn(s32RmtChipId, u32VdecCnt);
        PCIV_CHECK_ERR(s32Ret);

        /* close all msg port */
        SamplePcivExitMsgPort(s32RmtChipId);

        s32Ret = PCIV_CloseMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* Exit whole mpp sys  */
    SAMPLE_COMM_SYS_Exit();
    return HI_SUCCESS;
}

HI_S32 SampleHostSendStreamSlaveAndSlaveSendYUVToHostMode(HI_VOID)
{
    HI_S32    s32Ret;
    HI_S32    s32PciLocalId, as32PciRmtId[PCIV_MAX_CHIPNUM],s32AllPciRmtCnt;
    HI_S32    i,j, s32RmtChipId;
    HI_U32    u32VdecCnt = 1;
    SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo;
    SAMPLE_PCIV_UNBIND_SRC_INFO_S stUnBindInfo;
    //VPSS Param
    VPSS_MODE_S      stVpssMode;

    //PCIV Param
    HI_S32           pcivChnNum;

    PICVB_ATTR_S     stPicVbAttr;

    //vo param
    SAMPLE_VO_CONFIG_S stVoConfig;

    //VDEC Param
    VDEC_MODE_S      stVdecMode;

    /*******Init MediaSys********/
    s32Ret = SampleInitMediaSyS();
    PCIV_CHECK_ERR(s32Ret);

    /*******Init PCIE*************/
    s32Ret = SampleInitPcie(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);

    stPicVbAttr.enCompressMode = COMPRESS_MODE_NONE;
    stPicVbAttr.enDispPicSize  = PIC_3840x2160;
    stPicVbAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stPicVbAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
    stPicVbAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;

    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /*start slave chip vdec */
        stVdecMode.bReadStream   = HI_FALSE;
        stVdecMode.bReciveStream = HI_TRUE;
        stVdecMode.enPicSize     = PIC_3840x2160;
        stVdecMode.enPayLoadType = PT_H264;
        stVdecMode.enDisplayMode = VIDEO_DISPLAY_MODE_PLAYBACK;
        s32Ret = SamplePciv_StartVdecByChip(s32RmtChipId,u32VdecCnt,&stVdecMode);
        PCIV_CHECK_ERR(s32Ret);

        /* host start send stream to slave chip*/
        for (j = 0; j < u32VdecCnt; j++)
        {
            s32Ret = SamplePciv_HostStartVdecStream(s32RmtChipId, j);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start slave chip vpss */
        stVpssMode.s32VpssChnCnt = 1;
        stVpssMode.s32VpssGroupCnt = 1;
        stVpssMode.enChnMode = VPSS_CHN_MODE_AUTO;
        s32Ret = SamplePciv_StartVpssByChip(s32RmtChipId, &stVpssMode,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* bind vdec and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VDEC;
            stBindInfo.SrcMod.s32DevId = 0;
            stBindInfo.SrcMod.s32ChnId = j;

            stBindInfo.DestMod.enModId = HI_ID_VPSS;
            stBindInfo.DestMod.s32DevId = j;
            stBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start host and slave chip pciv */
        pcivChnNum =1;
        s32Ret = SamplePciv_StartPcivByChip(s32RmtChipId,pcivChnNum,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /*bind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stBindInfo.SrcMod.s32DevId = j;
            stBindInfo.SrcMod.s32ChnId = 0;

            stBindInfo.DestMod.enModId = HI_ID_PCIV;
            stBindInfo.DestMod.s32DevId = 0;
            stBindInfo.DestMod.s32ChnId = j;
            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }
    }
    /* start host chip vpss group */
    s32Ret = SamplePcivStartHostVpss(&stVpssMode,&stPicVbAttr);
    PCIV_CHECK_ERR(s32Ret);

    /*bind host pciv and VPSS*/
    s32Ret = SampleHostPcivBindVpss(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* start host chip vo dev  */
    stVoConfig.enIntfSync         = VO_OUTPUT_3840x2160_30;
    stVoConfig.enPicSize          = PIC_3840x2160;
    stVoConfig.enDstDynamicRange  = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode           = VO_MODE_1MUX;
    s32Ret = SamplePcivStartHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /**bind host vpss and vo**/
    for(i=0;i<1;i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Bind_VO(i,VPSS_CHN0,0,i);
        PCIV_CHECK_ERR(s32Ret);
    }

    printf("\n#################Press enter to exit########################\n");
    getchar();
    /* Exit ------------------------------------------------------------------------------ */

    /**Unbind host vpss and vo**/
    for(i=0;i<1;i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i,VPSS_CHN0,0,i);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* stop  host chip vo */
    s32Ret= SamplePcivStopHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /*Unbind host pciv and VPSS*/
    s32Ret = SampleHostPcivUnBindVpss(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* stop  host chip vpss */
    s32Ret=SamplePcivStopHostVpss(&stVpssMode);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];
        /* Unbind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stUnBindInfo.SrcMod.s32DevId = j;
            stUnBindInfo.SrcMod.s32ChnId = 0;

            stUnBindInfo.DestMod.enModId = HI_ID_PCIV;
            stUnBindInfo.DestMod.s32DevId = 0;
            stUnBindInfo.DestMod.s32ChnId = j;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop host and slave chip pciv */
        s32Ret = SamplePciv_StopPcivByChip(s32RmtChipId, pcivChnNum);
        PCIV_CHECK_ERR(s32Ret);

        /* Unbind vdec and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VDEC;
            stUnBindInfo.SrcMod.s32DevId = 0;
            stUnBindInfo.SrcMod.s32ChnId = j;

            stUnBindInfo.DestMod.enModId = HI_ID_VPSS;
            stUnBindInfo.DestMod.s32DevId = j;
            stUnBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop slave chip vpss */
        s32Ret = SamplePciv_StopVpssByChip(s32RmtChipId, &stVpssMode);
        PCIV_CHECK_ERR(s32Ret);

        /*stop slave chip vdec */
        s32Ret = SamplePciv_StopVdecByChip(s32RmtChipId,u32VdecCnt);
        PCIV_CHECK_ERR(s32Ret);

        /* host stop send stream to slave chip*/
        for (j = 0; j < u32VdecCnt; j++)
        {
            s32Ret = SamplePciv_HostStopVdecStream(s32RmtChipId, j);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* close all msg port */
        SamplePcivExitMsgPort(s32RmtChipId);

        s32Ret = PCIV_CloseMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* Exit whole mpp sys  */
    SAMPLE_COMM_SYS_Exit();
    return HI_SUCCESS;
}

HI_S32 SampleViVpssSlaveSendCompressPicToHostVpssVoDisplayMode(HI_VOID)
{
    HI_S32           s32Ret;
    HI_S32           i,j, s32RmtChipId;
    HI_S32           s32PciLocalId, as32PciRmtId[PCIV_MAX_CHIPNUM], s32AllPciRmtCnt;
    SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo;
    SAMPLE_PCIV_UNBIND_SRC_INFO_S stUnBindInfo;
    //Vi Param
	HI_S32           s32ViChnCnt;

    //VPSS Param
    VPSS_MODE_S      stVpssMode;
    //PCIV Param
    HI_S32           pcivChnNum;

    PICVB_ATTR_S     stPicVbAttr;

    //VO Param
    SAMPLE_VO_CONFIG_S stVoConfig;

	//Venc Param
    HI_U32           u32VencChnNum = 1;
    PIC_SIZE_E       aenVencSize[2] = {PIC_3840x2160, PIC_CIF};//only use aenVencSize[0]
    PAYLOAD_TYPE_E   aenType[2] = {PT_H264, PT_H264};//only use aenType[0]

    /*******Init MediaSys********/
    s32Ret = SampleInitMediaSyS();
    PCIV_CHECK_ERR(s32Ret);

    /*******Init PCIE*************/
    s32Ret = SampleInitPcie(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);

    stPicVbAttr.enCompressMode = COMPRESS_MODE_SEG;
    stPicVbAttr.enDispPicSize  = PIC_3840x2160;
    stPicVbAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stPicVbAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
    stPicVbAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;

    /* Init ------------------------------------------------------------------------------ */
    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /* init vi in slave chip */
        s32ViChnCnt =1;
        s32Ret = SamplePciv_HostStartVi(s32RmtChipId, s32ViChnCnt,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* start slave chip vpss */
        stVpssMode.s32VpssGroupCnt = 1;
        stVpssMode.s32VpssChnCnt   = 1;
        stVpssMode.enChnMode = VPSS_CHN_MODE_USER;
        s32Ret = SamplePciv_StartVpssByChip(s32RmtChipId, &stVpssMode,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* bind vi and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VI;
            stBindInfo.SrcMod.s32DevId = 0;
            stBindInfo.SrcMod.s32ChnId = j;

            stBindInfo.DestMod.enModId = HI_ID_VPSS;
            stBindInfo.DestMod.s32DevId = j;
            stBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start host and slave chip pciv */
        pcivChnNum =1;
        s32Ret = SamplePciv_StartPcivByChip(s32RmtChipId,pcivChnNum,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /*bind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stBindInfo.SrcMod.s32DevId = j;
            stBindInfo.SrcMod.s32ChnId = 0;

            stBindInfo.DestMod.enModId = HI_ID_PCIV;
            stBindInfo.DestMod.s32DevId = 0;
            stBindInfo.DestMod.s32ChnId = j;
            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /*At the Same Time To Venc*/
        s32Ret = SamplePciv_HostStartVenc(s32RmtChipId, u32VencChnNum, aenVencSize, aenType);
        PCIV_CHECK_ERR(s32Ret);

        /*bind vpss and venc in slae chip*/
        stBindInfo.SrcMod.enModId = HI_ID_VPSS;
        stBindInfo.SrcMod.s32DevId = 0;
        stBindInfo.SrcMod.s32ChnId = 0;

        stBindInfo.DestMod.enModId = HI_ID_VENC;
        stBindInfo.DestMod.s32DevId = 0;
        stBindInfo.DestMod.s32ChnId = 0;
        s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* start host chip vo dev  */
    stVoConfig.enIntfSync        = VO_OUTPUT_3840x2160_30;
    stVoConfig.enPicSize         = PIC_3840x2160;
    stVoConfig.enDstDynamicRange = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode          = VO_MODE_1MUX;
    s32Ret = SamplePcivStartHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /*bind host pciv and vo*/
    s32Ret = SampleHostPcivBindVo(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    printf("\n#################Press enter to exit########################\n");
    getchar();
    /* Exit ------------------------------------------------------------------------------ */

    /*unbind pciv and vo*/
    s32Ret = SampleHostPcivUnBindVo(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* stop host chip vo */
    s32Ret = SamplePcivStopHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /*Unbind vpss and venc in slae chip*/
        stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
        stUnBindInfo.SrcMod.s32DevId = 0;
        stUnBindInfo.SrcMod.s32ChnId = 0;

        stUnBindInfo.DestMod.enModId = HI_ID_VENC;
        stUnBindInfo.DestMod.s32DevId = 0;
        stUnBindInfo.DestMod.s32ChnId = 0;
        s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
        PCIV_CHECK_ERR(s32Ret);

		/* stop venc and stream transfer (venc is in slave and send stream to host) */
        s32Ret = SamplePciv_HostStopVenc(s32RmtChipId, u32VencChnNum, HI_TRUE);
        PCIV_CHECK_ERR(s32Ret);

        for (j= 0;j< 1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stUnBindInfo.SrcMod.s32DevId = j;
            stUnBindInfo.SrcMod.s32ChnId = 0;

            stUnBindInfo.DestMod.enModId = HI_ID_PCIV;
            stUnBindInfo.DestMod.s32DevId = 0;
            stUnBindInfo.DestMod.s32ChnId = j;
            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop host and slave chip pciv */
        s32Ret = SamplePciv_StopPcivByChip(s32RmtChipId, pcivChnNum);
        PCIV_CHECK_ERR(s32Ret);

        /*Unbind vi and vpss*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VI;
            stUnBindInfo.SrcMod.s32DevId = 0;
            stUnBindInfo.SrcMod.s32ChnId = j;

            stUnBindInfo.DestMod.enModId = HI_ID_VPSS;
            stUnBindInfo.DestMod.s32DevId = j;
            stUnBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop slave chip vpss */
        s32Ret = SamplePciv_StopVpssByChip(s32RmtChipId, &stVpssMode);
        PCIV_CHECK_ERR(s32Ret);

        /* stop slave chip vi*/
        s32Ret = SamplePciv_HostExitVi(s32RmtChipId,s32ViChnCnt);
        PCIV_CHECK_ERR(s32Ret);

        /* close all msg port */
        SamplePcivExitMsgPort(s32RmtChipId);

        s32Ret = PCIV_CloseMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* Exit whole mpp sys  */
    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}

HI_S32 SampleHostSendStreamSlaveAndSlaveSendYUVToHostCompressMode(HI_VOID)
{
    HI_S32    s32Ret;
    HI_S32    s32PciLocalId, as32PciRmtId[PCIV_MAX_CHIPNUM],s32AllPciRmtCnt;
    HI_S32    i,j, s32RmtChipId;
    HI_U32    u32VdecCnt = 1;
    SAMPLE_PCIV_BIND_SRC_INFO_S stBindInfo;
    SAMPLE_PCIV_UNBIND_SRC_INFO_S stUnBindInfo;
    //VPSS Param
    VPSS_MODE_S      stVpssMode;

    //PCIV Param
    HI_S32           pcivChnNum;

    PICVB_ATTR_S     stPicVbAttr;

    //vo param
    SAMPLE_VO_CONFIG_S stVoConfig;

    //VDEC Param
    VDEC_MODE_S      stVdecMode;

    /*******Init MediaSys********/
    s32Ret = SampleInitMediaSyS();
    PCIV_CHECK_ERR(s32Ret);

    /*******Init PCIE*************/
    s32Ret = SampleInitPcie(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);

    stPicVbAttr.enCompressMode = COMPRESS_MODE_SEG;
    stPicVbAttr.enDispPicSize  = PIC_3840x2160;
    stPicVbAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stPicVbAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;
    stPicVbAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;

    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /*start slave chip vdec */
        stVdecMode.bReadStream   = HI_FALSE;
        stVdecMode.bReciveStream = HI_TRUE;
        stVdecMode.enPicSize     = PIC_3840x2160;
        stVdecMode.enPayLoadType = PT_H264;
        stVdecMode.enDisplayMode = VIDEO_DISPLAY_MODE_PLAYBACK;
        s32Ret = SamplePciv_StartVdecByChip(s32RmtChipId,u32VdecCnt,&stVdecMode);
        PCIV_CHECK_ERR(s32Ret);

        /* host start send stream to slave chip*/
        for (j = 0; j < u32VdecCnt; j++)
        {
            s32Ret = SamplePciv_HostStartVdecStream(s32RmtChipId, j);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start slave chip vpss */
        stVpssMode.s32VpssChnCnt = 1;
        stVpssMode.s32VpssGroupCnt = 1;
        stVpssMode.enChnMode = VPSS_CHN_MODE_AUTO;
        s32Ret = SamplePciv_StartVpssByChip(s32RmtChipId, &stVpssMode,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /* bind vdec and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VDEC;
            stBindInfo.SrcMod.s32DevId = 0;
            stBindInfo.SrcMod.s32ChnId = j;

            stBindInfo.DestMod.enModId = HI_ID_VPSS;
            stBindInfo.DestMod.s32DevId = j;
            stBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* start host and slave chip pciv */
        pcivChnNum =1;
        s32Ret = SamplePciv_StartPcivByChip(s32RmtChipId,pcivChnNum,&stPicVbAttr);
        PCIV_CHECK_ERR(s32Ret);

        /*bind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stBindInfo.SrcMod.s32DevId = j;
            stBindInfo.SrcMod.s32ChnId = 0;

            stBindInfo.DestMod.enModId = HI_ID_PCIV;
            stBindInfo.DestMod.s32DevId = 0;
            stBindInfo.DestMod.s32ChnId = j;
            s32Ret = SamplePciv_BindSlaveSrcDestByChip(s32RmtChipId, stBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }
    }
    /* start host chip vpss group */
    s32Ret = SamplePcivStartHostVpss(&stVpssMode,&stPicVbAttr);
    PCIV_CHECK_ERR(s32Ret);

    /*bind host pciv and VPSS*/
    s32Ret = SampleHostPcivBindVpss(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* start host chip vo dev  */
    stVoConfig.enIntfSync         = VO_OUTPUT_3840x2160_30;
    stVoConfig.enPicSize          = PIC_3840x2160;
    stVoConfig.enDstDynamicRange  = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode           = VO_MODE_1MUX;
    s32Ret = SamplePcivStartHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /**bind host vpss and vo**/
    for(i=0;i<1;i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Bind_VO(i,VPSS_CHN0,0,i);
        PCIV_CHECK_ERR(s32Ret);
    }

    printf("\n#################Press enter to exit########################\n");
    getchar();
    /* Exit ------------------------------------------------------------------------------ */

    /**Unbind host vpss and vo**/
    for(i=0;i<1;i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i,VPSS_CHN0,0,i);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* stop  host chip vo */
    s32Ret= SamplePcivStopHostVoDev(&stVoConfig);
    PCIV_CHECK_ERR(s32Ret);

    /*Unbind host pciv and VPSS*/
    s32Ret = SampleHostPcivUnBindVpss(pcivChnNum);
    PCIV_CHECK_ERR(s32Ret);

    /* stop  host chip vpss */
    s32Ret=SamplePcivStopHostVpss(&stVpssMode);
    PCIV_CHECK_ERR(s32Ret);

    for (i=0; i<s32AllPciRmtCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];
        /* Unbind vpss and pciv in slave chip*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VPSS;
            stUnBindInfo.SrcMod.s32DevId = j;
            stUnBindInfo.SrcMod.s32ChnId = 0;

            stUnBindInfo.DestMod.enModId = HI_ID_PCIV;
            stUnBindInfo.DestMod.s32DevId = 0;
            stUnBindInfo.DestMod.s32ChnId = j;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop host and slave chip pciv */
        s32Ret = SamplePciv_StopPcivByChip(s32RmtChipId, pcivChnNum);
        PCIV_CHECK_ERR(s32Ret);

        /* Unbind vdec and vpss in slave chip*/
        for (j= 0;j<1;j++)
        {
            stUnBindInfo.SrcMod.enModId = HI_ID_VDEC;
            stUnBindInfo.SrcMod.s32DevId = 0;
            stUnBindInfo.SrcMod.s32ChnId = j;

            stUnBindInfo.DestMod.enModId = HI_ID_VPSS;
            stUnBindInfo.DestMod.s32DevId = j;
            stUnBindInfo.DestMod.s32ChnId = 0;

            s32Ret = SamplePciv_UnBindSlaveSrcDestByChip(s32RmtChipId, stUnBindInfo);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* stop slave chip vpss */
        s32Ret = SamplePciv_StopVpssByChip(s32RmtChipId, &stVpssMode);
        PCIV_CHECK_ERR(s32Ret);

        /*stop slave chip vdec */
        s32Ret = SamplePciv_StopVdecByChip(s32RmtChipId,u32VdecCnt);
        PCIV_CHECK_ERR(s32Ret);

        /* host stop send stream to slave chip*/
        for (j = 0; j < u32VdecCnt; j++)
        {
            s32Ret = SamplePciv_HostStopVdecStream(s32RmtChipId, j);
            PCIV_CHECK_ERR(s32Ret);
        }

        /* close all msg port */
        SamplePcivExitMsgPort(s32RmtChipId);

        s32Ret = PCIV_CloseMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* Exit whole mpp sys  */
    SAMPLE_COMM_SYS_Exit();
    return HI_SUCCESS;
}

HI_S32 SampleTestMode(HI_VOID)
{
    HI_S32    s32Ret = HI_SUCCESS;
    HI_S32    s32PciLocalId, as32PciRmtId[PCIV_MAX_CHIPNUM],s32AllPciRmtCnt;
    HI_S32    s32PciRmtChipCnt = 1;
    HI_S32    i, s32RmtChipId;

    VB_BLK vbInBlk;
    HI_CHAR *pMmzName 		= HI_NULL;
    HI_U32 u32BufSize;
    HI_U64 u64InPhyAddr;
    SAMPLE_PCIV_MSG_S  stMsg;
    SAMPLE_PCIV_MSG_TEST_S stPcivArgs = {0};
    HI_U8 *u8InVirtAddr;
    PIC_SIZE_E            enDispPicSize;
    SIZE_S                stDispSize;
    PICVB_ATTR_S     stPicVbAttr;

    /* system init ----------------------------------------------------------------------- */
    /* Init mpp sys and video buffer */
    s32Ret = HI_SUCCESS;
    s32Ret = SamplePcivInitMpp();
    PCIV_CHECK_ERR(s32Ret);
    s32Ret = SAMPLE_COMM_SYS_MemConfig();
    PCIV_CHECK_ERR(s32Ret);

    /* Get pci local id and all target id */
    s32Ret = SamplePcivEnumChip(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    PCIV_CHECK_ERR(s32Ret);
    for (i=0; i<s32PciRmtChipCnt; i++)
    {
        /* Wait for slave dev connect, and init message communication */
        s32Ret = SamplePcivInitComm(as32PciRmtId[i]);
        PCIV_CHECK_ERR(s32Ret);
        /* Get PCI PF Window info of pci device, used for DMA trans that host to slave */
        (HI_VOID)SamplePcivGetPfWin(as32PciRmtId[i]);
    }

    stPicVbAttr.enCompressMode = COMPRESS_MODE_NONE;
    stPicVbAttr.enDispPicSize  = PIC_3840x2160;
    stPicVbAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stPicVbAttr.enVideoFormat  = VIDEO_FORMAT_LINEAR;

    //sleep(10);
    enDispPicSize = stPicVbAttr.enDispPicSize;

	s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
	if(s32Ret != HI_SUCCESS)
	{
		HI_PRINT("sys get pic size fail for %#x!\n", s32Ret);
	}
    u32BufSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
				 stPicVbAttr.enPixelFormat, DATA_BITWIDTH_8,stPicVbAttr.enCompressMode, 0);
    vbInBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32BufSize, pMmzName);
    if (VB_INVALID_HANDLE == vbInBlk)
    {
        printf("Func:%s, Info:HI_MPI_VB_GetBlock(size:%d) fail\n", __FUNCTION__, u32BufSize);
        return HI_FAILURE;
    }
    u64InPhyAddr = HI_MPI_VB_Handle2PhysAddr(vbInBlk);
    if (0 == u64InPhyAddr)
    {
        printf("Func:%s, Info:HI_MPI_VB_Handle2PhysAddr fail\n", __FUNCTION__);
        HI_MPI_VB_ReleaseBlock(vbInBlk);
        return HI_FAILURE;
    }

    u8InVirtAddr = (HI_U8 *)HI_MPI_SYS_Mmap(u64InPhyAddr, u32BufSize);

    if (NULL == u8InVirtAddr)
    {
        printf("Func:%s, Info:HI_MPI_SYS_Mmap fail\n", __FUNCTION__);
        HI_MPI_VB_ReleaseBlock(vbInBlk);
        return HI_FAILURE;
    }
    printf("u64InPhyAddr:0x%llu,u32BufSize:%d\n",u64InPhyAddr,u32BufSize);
    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_GETPIC_FROM_HOST==========\n");
    memset(u8InVirtAddr, 0x61, u32BufSize);
    stPcivArgs.u64InPhyAddr = u64InPhyAddr;
    memcpy(stMsg.cMsgBody, &stPcivArgs, sizeof(stPcivArgs));
    stMsg.stMsgHead.u32Target  = as32PciRmtId[0];
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_START_MSG_TEST;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stPcivArgs);

    s32Ret = PCIV_SendMsg(as32PciRmtId[0], PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(as32PciRmtId[0], PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);


    /* Exit ------------------------------------------------------------------------------ */
    printf("\n#################Press enter to exit########################\n");
    getchar();

    s32Ret = HI_MPI_VB_ReleaseBlock(vbInBlk);
    PCIV_CHECK_ERR(s32Ret);

    printf("\n=======PCIV_SendMsg SAMPLE_PCIV_MSG_GETPIC_FROM_HOST==========\n");
    stMsg.stMsgHead.u32Target  = as32PciRmtId[0];
    stMsg.stMsgHead.u32MsgType = SAMPLE_PCIV_STOP_MSG_TEST;
    stMsg.stMsgHead.u32MsgLen  = sizeof(stPcivArgs);

    s32Ret = PCIV_SendMsg(as32PciRmtId[0], PCIV_MSGPORT_COMM_CMD, &stMsg);
    HI_ASSERT(HI_FAILURE != s32Ret);

    while (PCIV_ReadMsg(as32PciRmtId[0], PCIV_MSGPORT_COMM_CMD, &stMsg))
    {
        usleep(10000);
    }

    HI_ASSERT(stMsg.stMsgHead.u32MsgType == SAMPLE_PCIV_MSG_ECHO);
    HI_ASSERT(stMsg.stMsgHead.s32RetVal == HI_SUCCESS);

    for (i=0; i<s32PciRmtChipCnt; i++)
    {
        s32RmtChipId = as32PciRmtId[i];

        /* close all msg port */
        SamplePcivExitMsgPort(s32RmtChipId);

        s32Ret = PCIV_CloseMsgPort(s32RmtChipId, PCIV_MSGPORT_COMM_CMD);
        PCIV_CHECK_ERR(s32Ret);
    }

    /* Exit whole mpp sys  */
    SAMPLE_COMM_SYS_Exit();
    return HI_SUCCESS;
}

HI_VOID SAMPLE_PCIV_Usage(HI_VOID)
{
    printf("press sample command as follows!\n");
	printf("\t 1) (YUV)VI->VPSS->PCIV(slave)->PCIV(host)->VPSS->VO(HD)+ (STREAM)VI->PCIT(Slave)->PCIT(Host)->File\n");
    printf("\t 2) (YUV)VDEC->VPSS->PCIV(slave)->PCIV(host)->VPSS->VO(HD)\n");
    printf("\t 3) (YUV)VB->PCIV(slave)->PCIV(host)->DDR(HD)\n");
    printf("\t 4) file->PCIV(host)->PCIV(slave)->VDEC->PCIV(slave)->PCIV(host)->VPSS->VO\n");
    printf("\t 5) (COMPRESS)(YUV)VI->VPSS->PCIV(slave)->PCIV(host)->VPSS->VO(HD)+ (STREAM)VI->PCIT(Slave)->PCIT(Host)->File\n");
    printf("\t 6) (COMPRESS)file->PCIV(host)->PCIV(slave)->VDEC->PCIV(slave)->PCIV(host)->VPSS->VO\n");
    printf("sample command:");
    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
HI_VOID SAMPLE_PCIV_HandleSig(HI_S32 signo)
{
	int i;
	SAMPLE_PCIV_VDEC_CTX_S *pstVdecCtx;
	SAMPLE_PCIV_VENC_CTX_S *pstVencCtx;
	HI_S32 s32PciRmtChipCnt = 1;
    HI_S32 s32PciLocalId,s32RmtChipId, as32PciRmtId[PCIV_MAX_CHIPNUM], s32AllPciRmtCnt;

    if (SIGINT == signo || SIGTSTP == signo)
    {
        SamplePcivEnumChip(&s32PciLocalId, as32PciRmtId, &s32AllPciRmtCnt);
    	for(i=0;i<VDEC_MAX_CHN_NUM;i++)
    	{
    		pstVdecCtx = &astPcivVdecCtx[i];

			if (HI_TRUE == pstVdecCtx->bThreadStart)
	    	{
				pstVdecCtx->bThreadStart = HI_FALSE;
				pthread_join(pstVdecCtx->pid,NULL);
			}

    	}
		pstVencCtx = &g_stPcivVencCtx;
		if (pstVencCtx->bThreadStart)
		{
		    pstVencCtx->bThreadStart = HI_FALSE;
			pthread_join(pstVencCtx->pid,NULL);
		}
		for(i=0;i<s32PciRmtChipCnt;i++)
		{
			s32RmtChipId = as32PciRmtId[i];
			SamplePciv_HostCaptureCtrlC(s32RmtChipId);
		}

        if (HI_NULL!=au64PhyAddr[i])
        {
            HI_MPI_PCIV_Free(1, au64PhyAddr);
        }

        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}


/******************************************************************************
* function    : main()
* Description : region
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR ch;

    bQuit = HI_FALSE;

    signal(SIGINT, SAMPLE_PCIV_HandleSig);
    signal(SIGTERM, SAMPLE_PCIV_HandleSig);

    while (1)
    {
        SAMPLE_PCIV_Usage();
        ch = getchar();
		if (10 == ch)
        {
            continue;
        }
		getchar();
        switch (ch)
        {

			case '1':
            {
	            /* VI->VPSS(Scale)->PCIV(slave)->PCIV(host)->VPSS(Scale)->VO(HD)*/
                test_idx = 1;
                Add_osd  = 1;
                s32Ret = SampleViVpssSlaveSendPicToHostVpssVoDisplayMode();
                PCIV_CHECK_ERR(s32Ret);
                bQuit = HI_TRUE;
			    break;
	        }
            case '2':
            {
	            /* VDEC->VPSS(Scale)->PCIV(slave)->PCIV(host)->VPSS(Scale)->VO(HD)*/
                test_idx = 1;
                Add_osd  = 0;
                s32Ret = SampleVdecVpssSlaveSendPicToHostVpssVoDisplayMode();
                PCIV_CHECK_ERR(s32Ret);
                bQuit = HI_TRUE;
			    break;
	        }
            case '3':
            {
                /* YUV)VB->PCIV(slave)->PCIV(host)->DDR(HD) */
                test_idx = 1;
                Add_osd  = 0;
                s32Ret = SampleTestMode();
                PCIV_CHECK_ERR(s32Ret);
                bQuit = HI_TRUE;
			    break;

            }
            case '4':
            {
                /*file-> PCIT(HOST)->PCIT(SLAVE)->VDEC->VPSS->PCIV(SLAVE)->PCIV(HOST)->VPSS->VO*/
                test_idx = 1;
                Add_osd  = 0;
                s32Ret = SampleHostSendStreamSlaveAndSlaveSendYUVToHostMode();
                PCIV_CHECK_ERR(s32Ret);
                bQuit = HI_TRUE;
			    break;

            }
            case '5':
            {   /* (Compress)VI->VPSS(Scale)->PCIV(slave)->PCIV(host)->VPSS(Scale)->VO(HD)*/
                test_idx = 1;
                Add_osd  = 0;
                s32Ret = SampleViVpssSlaveSendCompressPicToHostVpssVoDisplayMode();
                PCIV_CHECK_ERR(s32Ret);
                bQuit = HI_TRUE;
			    break;
            }
            case '6':
            {
                /* (Compress)VDEC->VPSS(Scale)->PCIV(slave)->PCIV(host)->VPSS(Scale)->VO(HD)*/
                test_idx = 1;
                Add_osd  = 1;
                s32Ret = SampleHostSendStreamSlaveAndSlaveSendYUVToHostCompressMode();
                PCIV_CHECK_ERR(s32Ret);
                bQuit = HI_TRUE;
			    break;
            }
			case 'q':
            {
				/* Exit */
                bQuit = HI_TRUE;
			    break;

	        }
            default :
            {
                printf("input invaild! please try again.\n");
                break;
            }
        }

        if (bQuit)
        {
            break;
        }
    }

    return s32Ret;

}
