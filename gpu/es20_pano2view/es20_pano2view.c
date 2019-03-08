/******************************************************************************

  Copyright (C), 2015-2018, Hisilicon Tech. Co., Ltd.

******************************************************************************
  File Name        : es20_pano2view.c
  Version            : Initial Draft
  Author            : Hisilicon Device Chipset Key Technologies GPU group
  Created        : 2018/01/22
  Description        :
  History            :
  1.Date            : 2018/01/22
    Author        :
    Modification    : Create file

******************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <sys/syscall.h>

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "hifb.h"
#include "es20_common.h"

#include "pano2view.h"
#include "sample_comm.h"

#define INPUT_WIDTH    6144
#define INPUT_HEIGHT   3072
#define OUTPUT_WIDTH   1920
#define OUTPUT_HEIGHT  1080

#define SAMPLE_STREAM_NAME  "PanoramicVideo_6144x3072_8bit.h265"
#define SAMPLE_STREAM_PATH  "../res/"

HI_VOID * SAMPLE_GPU_ImageProcessing(HI_VOID *pArgs)
{
    HI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stVFrame1, stVFrame2;
    VDEC_THREAD_PARAM_S *pstVdecThreadParam =(VDEC_THREAD_PARAM_S *)pArgs;
    HI_PANO2VIEW_FRAME_S  stFrameUsrIn;
    HI_PANO2VIEW_FRAME_S  stFrameUsrOut;
    HI_U64 hBufferHandleIn = 0;
    HI_U64 hBufferHandleOut = 0;
    HI_PANO2VIEW_VIEW_S   stRyfaUsr;
    VB_BLK  Block;
    HI_U64 u64BlkSize;
    VB_POOL PoolId;

    u64BlkSize = COMMON_GetPicBufferSize(OUTPUT_WIDTH, OUTPUT_HEIGHT, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                            DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);
    u64BlkSize = ALIGN_UP(u64BlkSize, 4096);

    PoolId = HI_MPI_VB_CreatePool(u64BlkSize, 5, HI_NULL);
    if(VB_INVALID_POOLID == PoolId)
    {
        printf("<%s> Line %d: HI_MPI_VB_CreatePool fail!\n", __func__, __LINE__);
        return (HI_VOID *)HI_FAILURE;
    }

    s32Ret = HI_PANO2VIEW_Init( );
    if(HI_SUCCESS != s32Ret)
    {
        HI_MPI_VB_DestroyPool(PoolId);
        printf("<%s> Line %d: HI_PANO2VIEW_Init failed, s32Ret=%d\n", __func__, __LINE__, s32Ret);
        return (HI_VOID *)HI_FAILURE;
    }

    stRyfaUsr.fFovy = 90;
    stRyfaUsr.fYaw  =  0;
    stRyfaUsr.fRoll = 0;

    while (1)
    {
        if (pstVdecThreadParam->eThreadCtrl == THREAD_CTRL_STOP)
        {
            break;
        }
        else if (pstVdecThreadParam->eThreadCtrl == THREAD_CTRL_PAUSE)
        {
            sleep(1);
            continue;
        }

        Block =  HI_MPI_VB_GetBlock(PoolId, u64BlkSize, HI_NULL);
        if(VB_INVALID_HANDLE == Block)
        {
            usleep(10);
            continue;
        }

        s32Ret=HI_MPI_VPSS_GetChnFrame(pstVdecThreadParam->s32ChnId, VPSS_CHN0, &stVFrame1, pstVdecThreadParam->s32MilliSec);
        if(s32Ret==HI_SUCCESS)
        {
            memset(&stFrameUsrIn, 0, sizeof(stFrameUsrOut));
            stFrameUsrIn.eFormat = HI_PIXEL_FORMAT_YUV420SP_NV12;
            stFrameUsrIn.u32Width       = stVFrame1.stVFrame.u32Width;
            stFrameUsrIn.u32Height      = stVFrame1.stVFrame.u32Height;
            stFrameUsrIn.u32YStride     = stVFrame1.stVFrame.u32Stride[0];
            stFrameUsrIn.u32CbCrStride  = stVFrame1.stVFrame.u32Stride[0]/2;
            stFrameUsrIn.u64YPhyAddr    = stVFrame1.stVFrame.u64PhyAddr[0];
            stFrameUsrIn.u64CbCrPhyAddr = stVFrame1.stVFrame.u64PhyAddr[1];
            stFrameUsrIn.eFrameType = HI_PANO2VIEW_FRAME_TYPE_IN;

            memset(&stFrameUsrOut, 0, sizeof(stFrameUsrOut));
            stFrameUsrOut.eFormat = HI_PIXEL_FORMAT_YUV420SP_NV12;
            stFrameUsrOut.u32Width       = OUTPUT_WIDTH;
            stFrameUsrOut.u32Height      = OUTPUT_HEIGHT;
            stFrameUsrOut.u32YStride     = OUTPUT_WIDTH;
            stFrameUsrOut.u32CbCrStride  = stFrameUsrOut.u32YStride/2;
            stFrameUsrOut.u64YPhyAddr    = HI_MPI_VB_Handle2PhysAddr(Block);
            stFrameUsrOut.u64CbCrPhyAddr = stFrameUsrOut.u64YPhyAddr + stFrameUsrOut.u32YStride * stFrameUsrOut.u32Height;
            stFrameUsrOut.eFrameType = HI_PANO2VIEW_FRAME_TYPE_OUT;

            hBufferHandleIn = HI_PANO2VIEW_RegisterBuffer(stFrameUsrIn);
            if(!hBufferHandleIn)
            {
                printf("fail to get handle for input buffer");
            }

            /* create handle for output buffer */
            hBufferHandleOut = HI_PANO2VIEW_RegisterBuffer(stFrameUsrOut);
            if(!hBufferHandleOut)
            {
                printf("fail to get handle for output buffer");
            }

            stRyfaUsr.fYaw  = stRyfaUsr.fYaw + 0.3;
            if(stRyfaUsr.fYaw >= 180)
            {
                stRyfaUsr.fYaw = -180;
            }

            /* set rotation parameter */
            s32Ret = HI_PANO2VIEW_SetViewRotation(stRyfaUsr);
            if(HI_SUCCESS != s32Ret)
            {
                printf("<%s> Line %d: HI_PANO2VIEW_SetViewRotation failed, s32Ret=%d\n", __func__, __LINE__, s32Ret);
            }

            /* set scale parameter */
            s32Ret = HI_PANO2VIEW_SetViewScale(stRyfaUsr);
            if(HI_SUCCESS != s32Ret)
            {
                printf("<%s> Line %d: HI_PANO2VIEW_SetViewScale failed, s32Ret=%d\n", __func__, __LINE__, s32Ret);
            }

            s32Ret = HI_PANO2VIEW_GeneView(hBufferHandleIn, hBufferHandleOut);
            if(HI_SUCCESS != s32Ret)
            {
                printf("<%s> Line %d: HI_PANO2VIEW_GeneView failed, s32Ret=%d\n", __func__, __LINE__, s32Ret);
            }

            memset(&stVFrame2, 0, sizeof(VIDEO_FRAME_INFO_S));
            stVFrame2.u32PoolId = HI_MPI_VB_Handle2PoolId(Block);
            stVFrame2.enModId   = HI_ID_USER;
            stVFrame2.stVFrame.u32Width = stFrameUsrOut.u32Width;
            stVFrame2.stVFrame.u32Height = stFrameUsrOut.u32Height;
            stVFrame2.stVFrame.enField = VIDEO_FIELD_FRAME;
            stVFrame2.stVFrame.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            stVFrame2.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;
            stVFrame2.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
            stVFrame2.stVFrame.enDynamicRange = stVFrame1.stVFrame.enDynamicRange;
            stVFrame2.stVFrame.enColorGamut   = stVFrame1.stVFrame.enColorGamut;
            stVFrame2.stVFrame.u32Stride[0]   = stFrameUsrOut.u32YStride;
            stVFrame2.stVFrame.u32Stride[1]   = stFrameUsrOut.u32YStride;
            stVFrame2.stVFrame.u64PhyAddr[0]  = stFrameUsrOut.u64YPhyAddr;
            stVFrame2.stVFrame.u64PhyAddr[1]  = stFrameUsrOut.u64CbCrPhyAddr;

            /* unregister handle for input buffer */
            s32Ret = HI_PANO2VIEW_UnRegisterBuffer(hBufferHandleIn);
            if(HI_SUCCESS != s32Ret)
            {
                printf("fail to unregister handle for input buffer");
            }

            /* unregister handle for output buffer */
            s32Ret = HI_PANO2VIEW_UnRegisterBuffer(hBufferHandleOut);
            if(HI_SUCCESS != s32Ret)
            {
                printf("fail to unregister handle for output buffer");
            }

            s32Ret = HI_MPI_VO_SendFrame(SAMPLE_VO_DEV_UHD, pstVdecThreadParam->s32ChnId, &stVFrame2, -1);
            if (HI_SUCCESS != s32Ret)
            {
                printf("----------chn %d HI_MPI_VO_SendFrame fail!  s32Ret=0x%x\n", pstVdecThreadParam->s32ChnId, s32Ret);
            }

            s32Ret = HI_MPI_VPSS_ReleaseChnFrame(pstVdecThreadParam->s32ChnId, VPSS_CHN0, &stVFrame1);
            if (HI_SUCCESS != s32Ret)
            {
                printf("----------chn %d HI_MPI_VPSS_ReleaseChnFrame fail!  s32Ret=0x%x\n", pstVdecThreadParam->s32ChnId, s32Ret);
            }
            usleep(10000);
        }

        s32Ret = HI_MPI_VB_ReleaseBlock(Block);
        if (HI_SUCCESS != s32Ret)
        {
            printf("----------HI_MPI_VB_ReleaseBlock fail!  s32Ret=0x%x\n", s32Ret);
        }
    }

    HI_PANO2VIEW_DeInit();

    printf("\033[0;35m chn %d send steam thread return ...  \033[0;39m\n", pstVdecThreadParam->s32ChnId);
    fflush(stdout);

    return (HI_VOID *)HI_SUCCESS;
}

HI_VOID SAMPLE_GPU_StartImageProcessing(VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread)
{
    *pVdecThread = 0;
    pthread_create(pVdecThread, 0, SAMPLE_GPU_ImageProcessing, (HI_VOID *)pstVdecSend);
}

HI_VOID SAMPLE_GPU_StopImageProcessing(VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread)
{
    pstVdecSend->eThreadCtrl = THREAD_CTRL_STOP;
    if(0 != *pVdecThread)
    {
        pthread_join(*pVdecThread, HI_NULL);
        *pVdecThread = 0;
    }
}

int main()
{
    VB_CONFIG_S stVbConfig;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32VdecChnNum;
    VDEC_THREAD_PARAM_S stVdecSend;
    SIZE_S stDispSize;
    VDEC_CHN VdecChn;
    VPSS_GRP VpssGrp;
    pthread_t   VdecThread[2];
    PIC_SIZE_E enDispPicSize;
    SAMPLE_VDEC_ATTR stSampleVdec;
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
    SAMPLE_VO_CONFIG_S stVoConfig;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
    VO_INTF_SYNC_E enIntfSync;

    /************************************************
    step1:  init SYS, init common VB(for VPSS and VO)
    *************************************************/
    enDispPicSize = PIC_1080P;
    enIntfSync    = VO_OUTPUT_1080P30;

    s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("sys get pic size fail for %#x!\n", s32Ret);
        goto END1;
    }

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 2;
    stVbConfig.astCommPool[0].u32BlkCnt  = 10;
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(INPUT_WIDTH, INPUT_HEIGHT,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, 0);
    stVbConfig.astCommPool[0].u64BlkSize = ALIGN_UP(stVbConfig.astCommPool[0].u64BlkSize, 4096);

    stVbConfig.astCommPool[1].u32BlkCnt  = 5;
    stVbConfig.astCommPool[1].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, 0);
    stVbConfig.astCommPool[1].u64BlkSize = ALIGN_UP(stVbConfig.astCommPool[0].u64BlkSize, 4096);

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto END1;
    }

    /************************************************
    step2:  init module VB or user VB(for VDEC)
    *************************************************/
    u32VdecChnNum = 1;
    VdecChn = 0;
    stSampleVdec.enType                           = PT_H265;
    stSampleVdec.u32Width                         = INPUT_WIDTH;
    stSampleVdec.u32Height                        = INPUT_HEIGHT;
    stSampleVdec.enMode                           = VIDEO_MODE_FRAME;
    stSampleVdec.stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IPB;
    stSampleVdec.stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;
    stSampleVdec.stSapmleVdecVideo.u32RefFrameNum = 5;
    stSampleVdec.u32DisplayFrameNum               = 3;
    stSampleVdec.u32FrameBufCnt = stSampleVdec.stSapmleVdecVideo.u32RefFrameNum + stSampleVdec.u32DisplayFrameNum + 1;
    s32Ret = SAMPLE_COMM_VDEC_InitVBPool(1, &stSampleVdec);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init mod common vb fail for %#x!\n", s32Ret);
        goto END2;
    }

    /************************************************
    step3:  start VDEC
    *************************************************/
    s32Ret = SAMPLE_COMM_VDEC_Start(u32VdecChnNum, &stSampleVdec);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
        goto END3;
    }

    /************************************************
    step4:  start VPSS
    *************************************************/
    VpssGrp = 0;
    stVpssGrpAttr.u32MaxW = INPUT_WIDTH;
    stVpssGrpAttr.u32MaxH = INPUT_HEIGHT;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.bNrEn   = HI_FALSE;
    memset(abChnEnable, 0, sizeof(abChnEnable));
    abChnEnable[0] = HI_TRUE;
    astVpssChnAttr[0].u32Width                    = INPUT_WIDTH;
    astVpssChnAttr[0].u32Height                   = INPUT_HEIGHT;
    astVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode              = COMPRESS_MODE_NONE;
    astVpssChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;
    astVpssChnAttr[0].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    astVpssChnAttr[0].u32Depth                    = 5;
    astVpssChnAttr[0].bMirror                     = HI_FALSE;
    astVpssChnAttr[0].bFlip                       = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    astVpssChnAttr[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, &abChnEnable[0], &stVpssGrpAttr, &astVpssChnAttr[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
        goto END4;
    }


    /************************************************
    step5:  start VO
    *************************************************/
    stVoConfig.VoDev                 = SAMPLE_VO_DEV_UHD;
    stVoConfig.enVoIntfType          = VO_INTF_HDMI;
    stVoConfig.enIntfSync            = enIntfSync;
    stVoConfig.enPicSize             = enDispPicSize;
    stVoConfig.u32BgColor            = COLOR_RGB_BLUE;
    stVoConfig.u32DisBufLen          = 3;
    stVoConfig.enDstDynamicRange     = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode              = VO_MODE_1MUX;
    stVoConfig.enPixFormat           = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVoConfig.stDispRect.s32X       = 0;
    stVoConfig.stDispRect.s32Y       = 0;
    stVoConfig.stDispRect.u32Width   = stDispSize.u32Width;
    stVoConfig.stDispRect.u32Height  = stDispSize.u32Height;
    stVoConfig.stImageSize.u32Width  = stDispSize.u32Width;
    stVoConfig.stImageSize.u32Height = stDispSize.u32Height;
    stVoConfig.enVoPartMode          = VO_PART_MODE_SINGLE;
    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VO fail for %#x!\n", s32Ret);
        goto END5;
    }

    /************************************************
    step6:  VDEC bind VPSS
    *************************************************/

    s32Ret = SAMPLE_COMM_VDEC_Bind_VPSS(VdecChn, VpssGrp);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("vdec bind vpss fail for %#x!\n", s32Ret);
        goto END6;
    }

    /************************************************
    step7:  send stream to VDEC
    *************************************************/
    snprintf(stVdecSend.cFileName, sizeof(stVdecSend.cFileName), SAMPLE_STREAM_NAME);
    snprintf(stVdecSend.cFilePath, sizeof(stVdecSend.cFilePath), "%s", SAMPLE_STREAM_PATH);
    stVdecSend.enType          = stSampleVdec.enType;
    stVdecSend.s32StreamMode   = stSampleVdec.enMode;
    stVdecSend.s32ChnId        = VdecChn;
    stVdecSend.s32IntervalTime = 1000;
    stVdecSend.u64PtsInit      = 0;
    stVdecSend.u64PtsIncrease  = 0;
    stVdecSend.eThreadCtrl     = THREAD_CTRL_START;
    stVdecSend.bCircleSend     = HI_TRUE;
    stVdecSend.s32MilliSec     = 0;
    stVdecSend.s32MinBufSize   = (stSampleVdec.u32Width * stSampleVdec.u32Height * 3)>>1;
    SAMPLE_COMM_VDEC_StartSendStream(u32VdecChnNum, &stVdecSend, &VdecThread[0]);

    SAMPLE_GPU_StartImageProcessing(&stVdecSend, &VdecThread[1]);

    SAMPLE_COMM_VDEC_CmdCtrl(u32VdecChnNum, &stVdecSend, &VdecThread[0]);

    SAMPLE_GPU_StopImageProcessing(&stVdecSend, &VdecThread[1]);

    SAMPLE_COMM_VDEC_StopSendStream(u32VdecChnNum, &stVdecSend, &VdecThread[0]);


END6:
    s32Ret = SAMPLE_COMM_VDEC_UnBind_VPSS(VdecChn, VpssGrp);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("vdec unbind vpss fail for %#x!\n", s32Ret);
    }

END5:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

END4:
    SAMPLE_COMM_VPSS_Stop(VpssGrp, &abChnEnable[0]);

END3:
    SAMPLE_COMM_VDEC_Stop(u32VdecChnNum);

END2:
    SAMPLE_COMM_VDEC_ExitVBPool();

END1:
    SAMPLE_COMM_SYS_Exit();

    SAMPLE_PRT("program exit here!\n");
    return s32Ret;
}
