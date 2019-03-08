
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "sample_comm.h"
#include "mpi_snap.h"

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_SNAP_Usage(char* sPrgNm)
{
    printf("Usage : %s <index> \n", sPrgNm);
    printf("index:\n");
    printf("\t 0)double pipe offline, normal snap.\n");

    return;
}


/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_SNAP_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_All_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}


HI_S32 SAMPLE_COMM_Snap_StartJpeg(VENC_CHN VencChn, SIZE_S* pstSize, HI_BOOL bSupportDCF)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_RECV_PIC_PARAM_S  stRecvParam;

    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVencAttr.enType = PT_JPEG;
    stVencChnAttr.stVencAttr.u32MaxPicWidth     = pstSize->u32Width;
    stVencChnAttr.stVencAttr.u32MaxPicHeight    = pstSize->u32Height;
    stVencChnAttr.stVencAttr.u32PicWidth        = pstSize->u32Width;
    stVencChnAttr.stVencAttr.u32PicHeight       = pstSize->u32Height;
    stVencChnAttr.stVencAttr.u32BufSize         = pstSize->u32Width * pstSize->u32Height * 2;
    stVencChnAttr.stVencAttr.bByFrame           = HI_TRUE;/*get stream mode is field mode  or frame mode*/
    stVencChnAttr.stVencAttr.stAttrJpege.bSupportDCF = bSupportDCF;
    stVencChnAttr.stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum = 0;

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n", \
                   VencChn, s32Ret);
        return s32Ret;
    }

    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    stRecvParam.s32RecvPicNum = -1;
    s32Ret = HI_MPI_VENC_StartRecvFrame(VencChn, &stRecvParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_U32 g_u32SnapCnt = 0;
extern HI_S32 SAMPLE_COMM_VENC_SaveStream(FILE* fpJpegFile, VENC_STREAM_S* pstStream);

static HI_S32 SAMPLE_COMM_Snap_SaveJpegFile(VENC_STREAM_S* pstStream, const char* cFileName)
{
    char acFile[FILE_NAME_LEN]    = {0};
    FILE* pFile;
    HI_S32 s32Ret;

    snprintf(acFile, FILE_NAME_LEN, "%s_%d.jpg", cFileName, g_u32SnapCnt);

    pFile = fopen(acFile, "wb");
    if (pFile == NULL)
    {
        SAMPLE_PRT("open file err\n");
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_VENC_SaveStream(pFile, pstStream);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("save snap picture failed!\n");
        return HI_FAILURE;
    }

    fclose(pFile);
    printf("save file %s.\n", acFile);

    g_u32SnapCnt++;

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_Snap_GetJpeg(VENC_CHN VencChn, HI_U32 u32SnapNum, const char* cFileName)
{
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 s32VencFd;
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    int i;

    g_u32SnapCnt = 0;

    /******************************************
     step 3:  recv picture
    ******************************************/
    s32VencFd = HI_MPI_VENC_GetFd(VencChn);
    if (s32VencFd < 0)
    {
        SAMPLE_PRT("HI_MPI_VENC_GetFd faild with%#x!\n", s32VencFd);
        return HI_FAILURE;
    }

    for(i=0;i<u32SnapNum; i++)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32VencFd, &read_fds);
        TimeoutVal.tv_sec  = 20;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(s32VencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            SAMPLE_PRT("snap select failed!\n");
            return HI_FAILURE;
        }
        else if (0 == s32Ret)
        {
            SAMPLE_PRT("snap time out!\n");
            return HI_FAILURE;
        }
        else
        {
            if (FD_ISSET(s32VencFd, &read_fds))
            {
                s32Ret = HI_MPI_VENC_QueryStatus(VencChn, &stStat);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_VENC_QueryStatus failed with %#x!\n", s32Ret);
                    return HI_FAILURE;
                }

                /*******************************************************
                suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
                 if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
                 {                SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
                    return HI_SUCCESS;
                 }
                 *******************************************************/
                if (0 == stStat.u32CurPacks)
                {
                    SAMPLE_PRT("NOTE: Current  frame is NULL!\n");
                    return HI_SUCCESS;
                }

                stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                if (NULL == stStream.pstPack)
                {
                    SAMPLE_PRT("malloc memory failed!\n");
                    return HI_FAILURE;
                }

                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = HI_MPI_VENC_GetStream(VencChn, &stStream, -1);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    return HI_FAILURE;
                }

                s32Ret = SAMPLE_COMM_Snap_SaveJpegFile(&stStream, cFileName);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    return HI_FAILURE;
                }

                s32Ret = HI_MPI_VENC_ReleaseStream(VencChn, &stStream);
                if (s32Ret)
                {
                    SAMPLE_PRT("HI_MPI_VENC_ReleaseStream failed with %#x!\n", s32Ret);
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    return HI_FAILURE;
                }

                free(stStream.pstPack);
                stStream.pstPack = NULL;
            }
        }
    }

    /******************************************
     step 4:  stop recv picture
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvFrame(VencChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic failed with %#x!\n",  s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VIO_StartViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VO_CONFIG_S* pstVoConfig)
{
    HI_S32  s32Ret;

    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VO_StartVO(pstVoConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VO failed with %#x!\n", s32Ret);
        goto EXIT;
    }

    return s32Ret;

EXIT:
    SAMPLE_COMM_VI_StopVi(pstViConfig);

    return s32Ret;
}

HI_S32 SAMPLE_VIO_StopViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VO_CONFIG_S* pstVoConfig)
{
    SAMPLE_COMM_VO_StopVO(pstVoConfig);

    SAMPLE_COMM_VI_StopVi(pstViConfig);

    return HI_SUCCESS;
}

HI_S32 SAMPLE_SNAP_DoublePipeOffline(HI_VOID)
{
    HI_S32                  s32Ret              = HI_SUCCESS;
    VI_DEV                  ViDev0              = 0;
    VI_PIPE                 VideoPipe           = 0;
    VI_PIPE                 SnapPipe            = VI_MAX_PIPE_NUM/2;
    VI_CHN                  ViChn               = 0;
    HI_S32                  s32ViCnt            = 1;
    VPSS_GRP                VpssGrp0            = VideoPipe;
    VPSS_GRP                VpssGrp1            = SnapPipe;
    VPSS_CHN                VpssChn[4]          = {VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3};
    VPSS_GRP_ATTR_S         stVpssGrpAttr       = {0};
    VPSS_CHN_ATTR_S         stVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};
    HI_BOOL                 abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    VO_CHN                  VoChn               = 0;
    PIC_SIZE_E              enPicSize           = PIC_3840x2160;
    WDR_MODE_E              enWDRMode           = WDR_MODE_NONE;
    DYNAMIC_RANGE_E         enDynamicRange      = DYNAMIC_RANGE_SDR8;
    PIXEL_FORMAT_E          enPixFormat         = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VIDEO_FORMAT_E          enVideoFormat       = VIDEO_FORMAT_LINEAR;
    COMPRESS_MODE_E         enCompressMode      = COMPRESS_MODE_NONE;
    VI_VPSS_MODE_E          enVideoPipeMode     = VI_OFFLINE_VPSS_OFFLINE;
    VI_VPSS_MODE_E          enSnapPipeMode      = VI_OFFLINE_VPSS_OFFLINE;
    SIZE_S                  stSize;
    HI_U32                  u32BlkSize;
    VB_CONFIG_S             stVbConf;
    SAMPLE_VI_CONFIG_S      stViConfig;
    SAMPLE_VO_CONFIG_S      stVoConfig;
    HI_U32 u32SupplementConfig = VB_SUPPLEMENT_JPEG_MASK;
    VENC_CHN VencChn = 0;

    /************************************************
    step 1:  Get all sensors information, need two vi
        ,and need two mipi --
    *************************************************/
    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);
    stViConfig.s32WorkingViNum                           = s32ViCnt;

    stViConfig.as32WorkingViId[0]                        = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, 0);
    stViConfig.astViInfo[0].stSnsInfo.s32BusId           = 0;

    stViConfig.astViInfo[0].stDevInfo.ViDev              = ViDev0;
    stViConfig.astViInfo[0].stDevInfo.enWDRMode          = enWDRMode;

    stViConfig.astViInfo[0].stPipeInfo.enMastPipeMode    = enVideoPipeMode;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]          = VideoPipe;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[1]          = SnapPipe;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    stViConfig.astViInfo[0].stChnInfo.ViChn              = ViChn;
    stViConfig.astViInfo[0].stChnInfo.enPixFormat        = enPixFormat;
    stViConfig.astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    stViConfig.astViInfo[0].stChnInfo.enVideoFormat      = enVideoFormat;
    stViConfig.astViInfo[0].stChnInfo.enCompressMode     = enCompressMode;

    stViConfig.astViInfo[0].stSnapInfo.bSnap = HI_TRUE;
    stViConfig.astViInfo[0].stSnapInfo.bDoublePipe = HI_TRUE;
    stViConfig.astViInfo[0].stSnapInfo.VideoPipe = VideoPipe;
    stViConfig.astViInfo[0].stSnapInfo.SnapPipe = SnapPipe;
    stViConfig.astViInfo[0].stSnapInfo.enVideoPipeMode = enVideoPipeMode;
    stViConfig.astViInfo[0].stSnapInfo.enSnapPipeMode = enSnapPipeMode;

    /************************************************
    step 2:  Get  input size
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);
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

    /************************************************
    step3:  Init SYS and common VB
    *************************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt              = 2;

    u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_10, enCompressMode, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt   = 10;

    u32BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, 32);
    stVbConf.astCommPool[1].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_InitWithVbSupplement(&stVbConf, u32SupplementConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto EXIT;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_SetParam failed with %d!\n", s32Ret);
        goto EXIT;
    }


    /************************************************
    step 4: start VI
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed with %d!\n", s32Ret);
        goto EXIT1;
    }

    /************************************************
    step 5: start VPSS, need two grp
    *************************************************/
    stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
    stVpssGrpAttr.u32MaxH                        = stSize.u32Height;
    stVpssGrpAttr.enPixelFormat                  = enPixFormat;
    stVpssGrpAttr.enDynamicRange                 = enDynamicRange;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    abChnEnable[0]                               = HI_TRUE;
    stVpssChnAttr[0].u32Width                    = stSize.u32Width;
    stVpssChnAttr[0].u32Height                   = stSize.u32Height;
    stVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_USER;
    stVpssChnAttr[0].enCompressMode              = enCompressMode;
    stVpssChnAttr[0].enDynamicRange              = enDynamicRange;
    stVpssChnAttr[0].enPixelFormat               = enPixFormat;
    stVpssChnAttr[0].enVideoFormat               = enVideoFormat;
    stVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    stVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    stVpssChnAttr[0].u32Depth                    = 1;
    stVpssChnAttr[0].bMirror                     = HI_FALSE;
    stVpssChnAttr[0].bFlip                       = HI_FALSE;
    stVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp0, abChnEnable, &stVpssGrpAttr, stVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VPSS_Start Grp0 failed with %d!\n", s32Ret);
        goto EXIT1;
    }

    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp1, abChnEnable, &stVpssGrpAttr, stVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VPSS_Start Grp1 failed with %d!\n", s32Ret);
        goto EXIT2;
    }

    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(VideoPipe, ViChn, VpssGrp0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Bind_VPSS failed with %d!\n", s32Ret);
        goto EXIT3;
    }
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(SnapPipe, ViChn, VpssGrp1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_Bind_VPSS failed with %d!\n", s32Ret);
        goto EXIT3;
    }

    /************************************************
    step 6:  start VO
    *************************************************/
    s32Ret = SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %d!\n", s32Ret);
        goto EXIT3;
    }

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed with %d!\n", s32Ret);
        goto EXIT3;
    }

    s32Ret = SAMPLE_COMM_VPSS_Bind_VO(VpssGrp0, VpssChn[0], stVoConfig.VoDev, VoChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VO Grp0 failed with %d!\n", s32Ret);
        goto EXIT4;
    }

    /************************************************
    step 7:  start VENC
    *************************************************/
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencChn, &stSize, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_SnapStart failed witfh %d\n", s32Ret);
        goto EXIT4;
    }

    s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp1, VpssChn[0], VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VPSS_Bind_VENC failed with %d!\n", s32Ret);
        goto EXIT5;
    }

    /************************************************
    step 8:  snap
    *************************************************/
    SNAP_ATTR_S stSnapAttr;
    stSnapAttr.enSnapType = SNAP_TYPE_NORMAL;
    stSnapAttr.bLoadCCM = HI_TRUE;
    stSnapAttr.stNormalAttr.u32FrameCnt = 1;
    stSnapAttr.stNormalAttr.u32RepeatSendTimes = 1;
    stSnapAttr.stNormalAttr.bZSL = 0;
    s32Ret = HI_MPI_SNAP_SetPipeAttr(SnapPipe, &stSnapAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SNAP_SetPipeAttr failed with %#x!\n", s32Ret);
        goto EXIT5;
    }

    s32Ret = HI_MPI_SNAP_EnablePipe(SnapPipe);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SNAP_EnablePipe failed with %#x!\n", s32Ret);
        goto EXIT5;
    }

    printf("=======press any key to trigger=====\n");
    getchar();

    s32Ret = HI_MPI_SNAP_TriggerPipe(SnapPipe);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SNAP_TriggerPipe failed with %#x!\n", s32Ret);
        goto EXIT6;
    }

    s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencChn, stSnapAttr.stNormalAttr.u32FrameCnt, HI_TRUE, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: sanp process failed!\n", __FUNCTION__);
        goto EXIT6;
    }
    printf("snap success!\n");

    PAUSE();

    //SAMPLE_COMM_VPSS_UnBind_VO(VpssGrp1, VpssChn[0], stVoConfig.VoDev, VoChn);
EXIT6:
    HI_MPI_SNAP_DisablePipe(SnapPipe);
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp1, VpssChn[0], VencChn);
EXIT5:
    SAMPLE_COMM_VENC_Stop(VencChn);
    SAMPLE_COMM_VPSS_UnBind_VO(VpssGrp0, VpssChn[0], stVoConfig.VoDev, VoChn);
EXIT4:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);
EXIT3:
    SAMPLE_COMM_VPSS_Stop(VpssGrp1, abChnEnable);
EXIT2:
    SAMPLE_COMM_VPSS_Stop(VpssGrp0, abChnEnable);
    SAMPLE_COMM_VI_UnBind_VPSS(VideoPipe, ViChn, VpssGrp0);
    SAMPLE_COMM_VI_UnBind_VPSS(SnapPipe, ViChn, VpssGrp1);
EXIT1:
    SAMPLE_COMM_VI_StopVi(&stViConfig);
EXIT:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}




/******************************************************************************
* function    : main()
* Description : main
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 s32Index;

    if (argc < 2)
    {
        SAMPLE_SNAP_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_SNAP_HandleSig);
    signal(SIGTERM, SAMPLE_SNAP_HandleSig);
#endif

    s32Index = atoi(argv[1]);
    switch (s32Index)
    {
        case 0:
            s32Ret = SAMPLE_SNAP_DoublePipeOffline();
            break;

        default:
            SAMPLE_PRT("the index %d is invaild!\n",s32Index);
            SAMPLE_SNAP_Usage(argv[0]);
            return HI_FAILURE;
    }

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return (s32Ret);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

