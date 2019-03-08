#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include "mpi_sys.h"
#include "sample_common_photo.h"
#include "sample_common_venc.h"
#include "sample_msg_api.h"
#include "sample_photo_api.h"
#include "sample_vb_api.h"
#include "sample_snap_api.h"
#include "sample_vpss_api.h"
#include "sample_venc_api.h"
#include "mpi_photo.h"
#include "hi_comm_vb.h"

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_PHOTO_Usage(char* sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0)HDR.\n");
    printf("\t 1)MFNR+DE.\n");
    printf("\t 2)SFNR.\n");

    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_PHOTO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        HI_MPI_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_S32 SAMPLE_PHOTO_HDR_Process(void)
{
    HI_S32 s32Ret = HI_FAILURE;
    VI_PIPE SnapPipe = VI_MAX_PIPE_NUM/2;
    HI_S32 VpssGrp = SnapPipe;
    HI_S32 VpssChn = 0;
    VIDEO_FRAME_INFO_S stSrcVideoFrame[PHOTO_HDR_FRAME_NUM];
    HI_U32 u32imgWidth = 3840;
    HI_U32 u32imgHeight = 2160;
    HI_U32 u32HdrPubMemSize = 0;
    HI_U64 u64PubPhyAddr;
    HI_VOID* pPubVirAddr;
    HI_CHAR cBufName[32];
    PHOTO_ALG_TYPE_E enAlgType = PHOTO_ALG_TYPE_HDR;
    PHOTO_ALG_INIT_S stPhotoInit;
    VB_BLK VbBlk;
    HI_BOOL bAllocDesBuf = HI_FALSE;
    HI_U64 u64DesPhyAddr;
    VIDEO_FRAME_INFO_S stDesVideoFrame;
    HI_U32 i;
    PHOTO_ALG_ATTR_S  stPhotoAttr = {0};
    VENC_CHN VeChn = 0;
    HI_U32 u32ISO = 0;

    s32Ret = HI_MPI_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_PHOTO_LoadDspCoreBinary(SVP_DSP_ID_0);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_PHOTO_LoadDspCoreBinary failed!\n");
        goto EXIT;
    }

    s32Ret = Media_Msg_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Media_Msg_Init failed!\n");
        goto EXIT1;
    }

    snprintf(cBufName, 32, "HdrPubMem");
    u32HdrPubMemSize = HDR_GetPublicMemSize(u32imgWidth, u32imgHeight);
    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u64PubPhyAddr, &pPubVirAddr, cBufName, NULL, u32HdrPubMemSize);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached failed!\n");
        goto EXIT2;
    }

    stPhotoInit.u64PublicMemPhyAddr = u64PubPhyAddr;
    stPhotoInit.u64PublicMemVirAddr = (HI_U64)pPubVirAddr;
    stPhotoInit.u32PublicMemSize = u32HdrPubMemSize;
    stPhotoInit.bPrintDebugInfo = HI_FALSE;
    s32Ret = HI_MPI_PHOTO_AlgInit(enAlgType, &stPhotoInit);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT3;
    }

    s32Ret = SAMPLE_PHOTO_COMM_Start_Video();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT4;
    }

    SNAP_ATTR_S stSnapAttr;
    stSnapAttr.enSnapType = SNAP_TYPE_PRO;
    stSnapAttr.bLoadCCM = HI_TRUE;
    stSnapAttr.stProAttr.u32FrameCnt = PHOTO_HDR_FRAME_NUM;
    stSnapAttr.stProAttr.u32RepeatSendTimes = 1;
    stSnapAttr.stProAttr.stProParam.enOperationMode = OPERATION_MODE_AUTO;
    stSnapAttr.stProAttr.stProParam.stAutoParam.au16ProExpStep[0] = 256/4;
    stSnapAttr.stProAttr.stProParam.stAutoParam.au16ProExpStep[1] = 256;
    stSnapAttr.stProAttr.stProParam.stAutoParam.au16ProExpStep[2] = 256*4;
    s32Ret = SAMPLE_SNAP_COMM_SetAttr(SnapPipe, &stSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_SetAttr failed with %#x!\n", s32Ret);
        goto EXIT5;
    }

    s32Ret = SAMPLE_SNAP_COMM_Enable(SnapPipe);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_Enable failed with %#x!\n", s32Ret);
        goto EXIT5;
    }


    SAMPLE_PRT("press enter key to trigger.\n");
    getchar();

    s32Ret = SAMPLE_SNAP_COMM_Trigger(SnapPipe);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_Trigger failed!\n");
        goto EXIT5;
    }

    for(i=0; i<PHOTO_HDR_FRAME_NUM; i++)
    {
        s32Ret = SAMPLE_VPSS_COMM_GetChnFrame(VpssGrp, VpssChn, &stSrcVideoFrame[i], -1);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_VPSS_COMM_GetChnFrame failed!\n");
            goto EXIT5;
        }

        if(!bAllocDesBuf)
        {
            VbBlk = SAMPLE_VB_COMM_GetBlock(stSrcVideoFrame[i].u32PoolId, 0, NULL);
            if(VB_INVALID_HANDLE == VbBlk)
            {
                SAMPLE_PRT("SAMPLE_VB_COMM_GetBlock failed!\n");
                break;
            }

            u64DesPhyAddr = SAMPLE_VB_COMM_Handle2PhysAddr(VbBlk);
            stDesVideoFrame = stSrcVideoFrame[i];
            stDesVideoFrame.stVFrame.u64PhyAddr[0] = u64DesPhyAddr;
            stDesVideoFrame.stVFrame.u64PhyAddr[1] = u64DesPhyAddr + (stDesVideoFrame.stVFrame.u32Stride[0] * stDesVideoFrame.stVFrame.u32Height);

            bAllocDesBuf = HI_TRUE;
        }

        SAMPLE_COMM_PHOTO_GetIsoByVideoFrame(&stSrcVideoFrame[i], &u32ISO);

        stPhotoAttr.stHDRAttr.stSrcFrm = stSrcVideoFrame[i];
        stPhotoAttr.stHDRAttr.stDesFrm = stDesVideoFrame;
        stPhotoAttr.stHDRAttr.u32FrmIndex = i;
        stPhotoAttr.stHDRAttr.u32ISO = u32ISO;
        stPhotoAttr.stHDRAttr.u32FaceNum = 0;
        s32Ret = HI_MPI_PHOTO_AlgProcess(enAlgType, &stPhotoAttr);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_PHOTO_AlgProcess failed!\n");
            goto EXIT7;
        }
    }

    #if 0
    FILE* pDesfd = NULL;
    HI_CHAR acDesFileName[64];
    snprintf(acDesFileName, 64, "./%s_3840_2160_p420.yuv", __FUNCTION__);
    pDesfd = fopen(acDesFileName, "wb");
    if(pDesfd == NULL)
    {
        printf("open %s fail.\n", acDesFileName);
        goto EXIT6;
    }
    s32Ret = SAMPLE_COMM_PHOTO_SaveYUVFrame(pDesfd, &stDesVideoFrame.stVFrame);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_PHOTO_SaveYUVFrame failed:%#x!\n", s32Ret);
        goto EXIT6;
    }
    fclose(pDesfd);
    #endif

    s32Ret = SAMPLE_VENC_COMM_SendFrame(VeChn, &stDesVideoFrame, -1);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VENC_COMM_SendFrame failed:%#x!\n", s32Ret);
        goto EXIT7;
    }

    s32Ret = SAMPLE_COMM_VENC_SavePicture(VeChn, 1, __FUNCTION__);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_SavePicture failed!\n");
        goto EXIT7;
    }


    SAMPLE_PRT("press enter key to exit.\n");
    getchar();

EXIT7:
    for(i=0; i<PHOTO_HDR_FRAME_NUM; i++)
    {
        SAMPLE_VPSS_COMM_ReleaseChnFrame(VpssGrp, VpssChn, &stSrcVideoFrame[i]);
    }

    SAMPLE_VB_COMM_ReleaseBlock(VbBlk);
EXIT6:
    SAMPLE_SNAP_COMM_Disable(SnapPipe);
EXIT5:
    SAMPLE_PHOTO_COMM_Stop_Video();
EXIT4:
    HI_MPI_PHOTO_AlgDeinit(enAlgType);
EXIT3:
    HI_MPI_SYS_MmzFree(u64PubPhyAddr, pPubVirAddr);
EXIT2:
    Media_Msg_Deinit();
EXIT1:
    SAMPLE_COMM_PHOTO_UnloadDspCoreBinary(SVP_DSP_ID_0);
EXIT:
    HI_MPI_SYS_Exit();

    return s32Ret;
}

HI_S32 SAMPLE_PHOTO_MFNR_DE_Process(void)
{
    HI_S32 s32Ret = HI_FAILURE;
    VI_PIPE SnapPipe = VI_MAX_PIPE_NUM/2;
    HI_S32 VpssGrp = SnapPipe;
    HI_S32 VpssChn = 0;
    VIDEO_FRAME_INFO_S stSrcVideoFrame[PHOTO_MFNR_FRAME_NUM];
    HI_U32 u32imgWidth = 3840;
    HI_U32 u32imgHeight = 2160;
    HI_U32 u32PubMemSize = 0;
    HI_U64 u64PubPhyAddr;
    HI_VOID* pPubVirAddr;
    HI_CHAR cBufName[32];
    PHOTO_ALG_TYPE_E enAlgType = PHOTO_ALG_TYPE_MFNR;
    PHOTO_ALG_INIT_S stPhotoInit;
    VB_BLK VbBlk;
    HI_BOOL bAllocDesBuf = HI_FALSE;
    HI_U64 u64DesPhyAddr;
    VIDEO_FRAME_INFO_S stDesVideoFrame;
    VIDEO_FRAME_INFO_S stBnrRawVideoFrame;
    HI_U32 i;
    PHOTO_ALG_ATTR_S  stPhotoAttr = {0};
    VENC_CHN VeChn = 0;
    HI_U32 u32ISO = 0;

    s32Ret = HI_MPI_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_PHOTO_LoadDspCoreBinary(SVP_DSP_ID_0);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_PHOTO_LoadDspCoreBinary failed!\n");
        goto EXIT;
    }

    s32Ret = Media_Msg_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Media_Msg_Init failed!\n");
        goto EXIT1;
    }

    snprintf(cBufName, 32, "MfnrPubMem");
    u32PubMemSize = MFNR_GetPublicMemSize(u32imgWidth, u32imgHeight);
    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u64PubPhyAddr, &pPubVirAddr, cBufName, NULL, u32PubMemSize);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached failed!\n");
        goto EXIT2;
    }

    stPhotoInit.u64PublicMemPhyAddr = u64PubPhyAddr;
    stPhotoInit.u64PublicMemVirAddr = (HI_U64)pPubVirAddr;
    stPhotoInit.u32PublicMemSize = u32PubMemSize;
    stPhotoInit.bPrintDebugInfo = HI_FALSE;
    s32Ret = HI_MPI_PHOTO_AlgInit(enAlgType, &stPhotoInit);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT3;
    }

    s32Ret = SAMPLE_PHOTO_COMM_Start_Video();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT4;
    }

    SNAP_ATTR_S stSnapAttr;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.bLoadCCM = HI_TRUE;
    stSnapAttr.stNormalAttr.u32FrameCnt = PHOTO_MFNR_FRAME_NUM;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
    stSnapAttr.stNormalAttr.bZSL = HI_FALSE;
    s32Ret = SAMPLE_SNAP_COMM_SetAttr(SnapPipe, &stSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_SetAttr failed with %#x!\n", s32Ret);
        goto EXIT5;
    }

    s32Ret = SAMPLE_SNAP_COMM_Enable(SnapPipe);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_Enable failed with %#x!\n", s32Ret);
        goto EXIT5;
    }


    SAMPLE_PRT("press enter key to trigger.\n");
    getchar();

    s32Ret = SAMPLE_SNAP_COMM_Trigger(SnapPipe);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_Trigger failed!\n");
        goto EXIT5;
    }

    for(i=0; i<PHOTO_MFNR_FRAME_NUM; i++)
    {
        s32Ret = SAMPLE_VPSS_COMM_GetChnFrame(VpssGrp, VpssChn, &stSrcVideoFrame[i], -1);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_VPSS_COMM_GetChnFrame failed!\n");
            goto EXIT5;
        }

        if(!bAllocDesBuf)
        {
            VbBlk = SAMPLE_VB_COMM_GetBlock(stSrcVideoFrame[i].u32PoolId, 0, NULL);
            if(VB_INVALID_HANDLE == VbBlk)
            {
                SAMPLE_PRT("SAMPLE_VB_COMM_GetBlock failed!\n");
                break;
            }

            u64DesPhyAddr = SAMPLE_VB_COMM_Handle2PhysAddr(VbBlk);
            stDesVideoFrame = stSrcVideoFrame[i];
            stDesVideoFrame.stVFrame.u64PhyAddr[0] = u64DesPhyAddr;
            stDesVideoFrame.stVFrame.u64PhyAddr[1] = u64DesPhyAddr + (stDesVideoFrame.stVFrame.u32Stride[0] * stDesVideoFrame.stVFrame.u32Height);

            bAllocDesBuf = HI_TRUE;

            s32Ret = SAMPLE_SNAP_COMM_GetBNRRaw(SnapPipe, &stBnrRawVideoFrame, -1);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_SNAP_COMM_GetBNRRaw failed!\n");
                break;
            }
        }

        SAMPLE_COMM_PHOTO_GetIsoByVideoFrame(&stSrcVideoFrame[i], &u32ISO);

        stPhotoAttr.stMFNRAttr.stSrcFrm = stSrcVideoFrame[i];
        stPhotoAttr.stMFNRAttr.stDesFrm = stDesVideoFrame;
        stPhotoAttr.stMFNRAttr.u32FrmIndex = i;
        stPhotoAttr.stMFNRAttr.u32ISO = u32ISO;
        s32Ret = HI_MPI_PHOTO_AlgProcess(enAlgType, &stPhotoAttr);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_PHOTO_AlgProcess failed!\n");
            goto EXIT7;
        }
    }


#if 1
    HI_MPI_PHOTO_AlgDeinit(enAlgType);
    HI_MPI_SYS_MmzFree(u64PubPhyAddr, pPubVirAddr);

    SAMPLE_PRT("MFNR process finished.\n");

    enAlgType = PHOTO_ALG_TYPE_DE;
    snprintf(cBufName, 32, "DePubMem");
    u32PubMemSize = DE_GetPublicMemSize(u32imgWidth, u32imgHeight);
    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u64PubPhyAddr, &pPubVirAddr, cBufName, NULL, u32PubMemSize);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached failed!\n");
        goto EXIT7;
    }

    stPhotoInit.u64PublicMemPhyAddr = u64PubPhyAddr;
    stPhotoInit.u64PublicMemVirAddr = (HI_U64)pPubVirAddr;
    stPhotoInit.u32PublicMemSize = u32PubMemSize;
    stPhotoInit.bPrintDebugInfo = HI_FALSE;
    s32Ret = HI_MPI_PHOTO_AlgInit(enAlgType, &stPhotoInit);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT7;
    }

    stPhotoAttr.stDEAttr.stFrm = stDesVideoFrame;
    stPhotoAttr.stDEAttr.stRawFrm = stBnrRawVideoFrame;
    stPhotoAttr.stDEAttr.u32ISO = u32ISO;
    s32Ret = HI_MPI_PHOTO_AlgProcess(enAlgType, &stPhotoAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_PHOTO_AlgProcess failed!\n");
        goto EXIT7;
    }
#endif

    s32Ret = SAMPLE_VENC_COMM_SendFrame(VeChn, &stDesVideoFrame, -1);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VENC_COMM_SendFrame failed:%#x!\n", s32Ret);
        goto EXIT7;
    }

    s32Ret = SAMPLE_COMM_VENC_SavePicture(VeChn, 1, __FUNCTION__);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_SavePicture failed!\n");
        goto EXIT7;
    }


    SAMPLE_PRT("press enter key to exit.\n");
    getchar();

EXIT7:
    for(i=0; i<PHOTO_MFNR_FRAME_NUM; i++)
    {
        SAMPLE_VPSS_COMM_ReleaseChnFrame(VpssGrp, VpssChn, &stSrcVideoFrame[i]);
    }
    SAMPLE_SNAP_COMM_ReleaseBNRRaw(SnapPipe, &stBnrRawVideoFrame);

    SAMPLE_VB_COMM_ReleaseBlock(VbBlk);
EXIT6:
    SAMPLE_SNAP_COMM_Disable(SnapPipe);
EXIT5:
    SAMPLE_PHOTO_COMM_Stop_Video();
EXIT4:
    HI_MPI_PHOTO_AlgDeinit(enAlgType);
EXIT3:
    HI_MPI_SYS_MmzFree(u64PubPhyAddr, pPubVirAddr);
EXIT2:
    Media_Msg_Deinit();
EXIT1:
    SAMPLE_COMM_PHOTO_UnloadDspCoreBinary(SVP_DSP_ID_0);
EXIT:
    HI_MPI_SYS_Exit();

    return s32Ret;
}

HI_S32 SAMPLE_PHOTO_SFNR_Process(void)
{
    HI_S32 s32Ret = HI_FAILURE;
    VI_PIPE SnapPipe = VI_MAX_PIPE_NUM/2;
    HI_S32 VpssGrp = SnapPipe;
    HI_S32 VpssChn = 0;
    VIDEO_FRAME_INFO_S stSrcVideoFrame;
    HI_U32 u32imgWidth = 3840;
    HI_U32 u32imgHeight = 2160;
    HI_U32 u32PubMemSize = 0;
    HI_U64 u64PubPhyAddr;
    HI_VOID* pPubVirAddr;
    HI_CHAR cBufName[32];
    PHOTO_ALG_TYPE_E enAlgType = PHOTO_ALG_TYPE_SFNR;
    PHOTO_ALG_INIT_S stPhotoInit;
    HI_U32 i;
    PHOTO_ALG_ATTR_S  stPhotoAttr = {0};
    VENC_CHN VeChn = 0;
    HI_U32 u32ISO = 0;

    s32Ret = HI_MPI_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_PHOTO_LoadDspCoreBinary(SVP_DSP_ID_0);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_PHOTO_LoadDspCoreBinary failed!\n");
        goto EXIT;
    }

    s32Ret = Media_Msg_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Media_Msg_Init failed!\n");
        goto EXIT1;
    }

    snprintf(cBufName, 32, "SfnrPubMem");
    u32PubMemSize = SFNR_GetPublicMemSize(u32imgWidth, u32imgHeight);
    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u64PubPhyAddr, &pPubVirAddr, cBufName, NULL, u32PubMemSize);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached failed!\n");
        goto EXIT2;
    }

    stPhotoInit.u64PublicMemPhyAddr = u64PubPhyAddr;
    stPhotoInit.u64PublicMemVirAddr = (HI_U64)pPubVirAddr;
    stPhotoInit.u32PublicMemSize = u32PubMemSize;
    stPhotoInit.bPrintDebugInfo = HI_FALSE;
    s32Ret = HI_MPI_PHOTO_AlgInit(enAlgType, &stPhotoInit);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT3;
    }

    s32Ret = SAMPLE_PHOTO_COMM_Start_Video();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_PHOTO_COMM_Start_Video failed!\n");
        goto EXIT4;
    }

    SNAP_ATTR_S stSnapAttr;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.bLoadCCM = HI_TRUE;
    stSnapAttr.stNormalAttr.u32FrameCnt = 1;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
    stSnapAttr.stNormalAttr.bZSL = HI_FALSE;
    s32Ret = SAMPLE_SNAP_COMM_SetAttr(SnapPipe, &stSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_SetAttr failed with %#x!\n", s32Ret);
        goto EXIT5;
    }

    s32Ret = SAMPLE_SNAP_COMM_Enable(SnapPipe);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_Enable failed with %#x!\n", s32Ret);
        goto EXIT5;
    }

    SAMPLE_PRT("press enter key to trigger.\n");
    getchar();

    s32Ret = SAMPLE_SNAP_COMM_Trigger(SnapPipe);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_SNAP_COMM_Trigger failed!\n");
        goto EXIT5;
    }

    s32Ret = SAMPLE_VPSS_COMM_GetChnFrame(VpssGrp, VpssChn, &stSrcVideoFrame, -1);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VPSS_COMM_GetChnFrame failed!\n");
        goto EXIT5;
    }

    SAMPLE_COMM_PHOTO_GetIsoByVideoFrame(&stSrcVideoFrame, &u32ISO);

    stPhotoAttr.stSFNRAttr.stFrm = stSrcVideoFrame;
    stPhotoAttr.stSFNRAttr.u32ISO = u32ISO;
    s32Ret = HI_MPI_PHOTO_AlgProcess(enAlgType, &stPhotoAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_PHOTO_AlgProcess failed!\n");
        goto EXIT7;
    }

    s32Ret = SAMPLE_VENC_COMM_SendFrame(VeChn, &stSrcVideoFrame, -1);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VENC_COMM_SendFrame failed:%#x!\n", s32Ret);
        goto EXIT7;
    }

    s32Ret = SAMPLE_COMM_VENC_SavePicture(VeChn, 1, __FUNCTION__);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_SavePicture failed!\n");
        goto EXIT7;
    }


    SAMPLE_PRT("press enter key to exit.\n");
    getchar();

EXIT7:
    SAMPLE_VPSS_COMM_ReleaseChnFrame(VpssGrp, VpssChn, &stSrcVideoFrame);
EXIT6:
    SAMPLE_SNAP_COMM_Disable(SnapPipe);
EXIT5:
    SAMPLE_PHOTO_COMM_Stop_Video();
EXIT4:
    HI_MPI_PHOTO_AlgDeinit(enAlgType);
EXIT3:
    HI_MPI_SYS_MmzFree(u64PubPhyAddr, pPubVirAddr);
EXIT2:
    Media_Msg_Deinit();
EXIT1:
    SAMPLE_COMM_PHOTO_UnloadDspCoreBinary(SVP_DSP_ID_0);
EXIT:
    HI_MPI_SYS_Exit();

    return s32Ret;
}

void SAMPLE_PHOTO_ProcBindBigCPU(void)
{
    int ret;
    cpu_set_t cpuset;
    pthread_t threadid;

    threadid = pthread_self();
    CPU_ZERO(&cpuset);

    CPU_SET(2, &cpuset);
    CPU_SET(3, &cpuset);

    ret = pthread_setaffinity_np(threadid, sizeof(cpu_set_t), &cpuset);
    if(ret != 0)
    {
        SAMPLE_PRT("pthread_setaffinity_np error:%d\n",ret);
        return;
    }

}

#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 s32Index = 0;

    if (argc < 2)
    {
        SAMPLE_PHOTO_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_PHOTO_HandleSig);
    signal(SIGTERM, SAMPLE_PHOTO_HandleSig);
    SAMPLE_PHOTO_ProcBindBigCPU();
#endif


    s32Index = atoi(argv[1]);
    switch (s32Index)
    {
        case 0:
            s32Ret = SAMPLE_PHOTO_HDR_Process();
            break;

        case 1:
            s32Ret = SAMPLE_PHOTO_MFNR_DE_Process();
            break;

        case 2:
            s32Ret = SAMPLE_PHOTO_SFNR_Process();
            break;

        default:
            SAMPLE_PRT("the index %d is invaild!\n",s32Index);
            SAMPLE_PHOTO_Usage(argv[0]);
            return HI_FAILURE;
    }

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("sample_photo exit success!\n");
    }
    else
    {
        SAMPLE_PRT("sample_photo exit abnormally!\n");
    }

    return (s32Ret);
}



