#include "sample_comm.h"
#include "mpi_snap.h"

HI_S32 SAMPLE_PHOTO_COMM_Start_Video(void)
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
    VENC_RECV_PIC_PARAM_S stRecvParam;

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
    stVbConf.astCommPool[0].u32BlkCnt   = 15;

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
    stVpssChnAttr[0].u32Depth                    = 0;
    stVpssChnAttr[0].bMirror                     = HI_FALSE;
    stVpssChnAttr[0].bFlip                       = HI_FALSE;
    stVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp0, abChnEnable, &stVpssGrpAttr, stVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VPSS_Start Grp0 failed with %d!\n", s32Ret);
        goto EXIT1;
    }

    stVpssChnAttr[0].u32Depth = 3;
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

    stRecvParam.s32RecvPicNum = -1;
    s32Ret = HI_MPI_VENC_StartRecvFrame(VencChn, &stRecvParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvFrame failed witfh %d\n", s32Ret);
        goto EXIT5;
    }

    /************************************************
    step 8:  dump bnr set attr
    *************************************************/
    BNR_DUMP_ATTR_S stBnrDumpAttr;
    stBnrDumpAttr.bEnable = HI_TRUE;
    stBnrDumpAttr.u32Depth = 1;
    s32Ret = HI_MPI_SNAP_SetBNRRawDumpAttr(SnapPipe, &stBnrDumpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SNAP_SetBNRRawDumpAttr failed witfh %d\n", s32Ret);
        goto EXIT5;
    }

    return HI_SUCCESS;

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

HI_S32 SAMPLE_PHOTO_COMM_Stop_Video(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VI_PIPE                 VideoPipe           = 0;
    VI_PIPE                 SnapPipe            = VI_MAX_PIPE_NUM/2;
    VI_CHN                  ViChn               = 0;
    VPSS_GRP                VpssGrp0            = VideoPipe;
    VPSS_GRP                VpssGrp1            = SnapPipe;
    VPSS_CHN                VpssChn[4]          = {VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3};
    HI_BOOL                 abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    VI_VPSS_MODE_E          enVideoPipeMode     = VI_OFFLINE_VPSS_OFFLINE;
    VI_VPSS_MODE_E          enSnapPipeMode      = VI_OFFLINE_VPSS_OFFLINE;
    VENC_CHN VencChn = 0;
    VO_LAYER VoLayer = 0;
    VO_CHN VoChn = 0;
    SAMPLE_VI_CONFIG_S      stViConfig;
    SAMPLE_VO_CONFIG_S      stVoConfig = {0};
    BNR_DUMP_ATTR_S  stBnrDumpAttr;

    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);
    stViConfig.s32WorkingViNum                           = 1;

    stViConfig.as32WorkingViId[0]                        = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, 0);
    stViConfig.astViInfo[0].stSnsInfo.s32BusId           = 0;

    stViConfig.astViInfo[0].stDevInfo.ViDev              = 0;
    stViConfig.astViInfo[0].stDevInfo.enWDRMode          = WDR_MODE_NONE;

    stViConfig.astViInfo[0].stPipeInfo.enMastPipeMode    = enVideoPipeMode;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]          = VideoPipe;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[1]          = SnapPipe;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    stViConfig.astViInfo[0].stChnInfo.ViChn              = ViChn;
    stViConfig.astViInfo[0].stChnInfo.enPixFormat        = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stViConfig.astViInfo[0].stChnInfo.enDynamicRange     = DYNAMIC_RANGE_SDR8;
    stViConfig.astViInfo[0].stChnInfo.enVideoFormat      = VIDEO_FORMAT_LINEAR;
    stViConfig.astViInfo[0].stChnInfo.enCompressMode     = COMPRESS_MODE_NONE;

    stViConfig.astViInfo[0].stSnapInfo.bSnap = HI_TRUE;
    stViConfig.astViInfo[0].stSnapInfo.bDoublePipe = HI_TRUE;
    stViConfig.astViInfo[0].stSnapInfo.VideoPipe = VideoPipe;
    stViConfig.astViInfo[0].stSnapInfo.SnapPipe = SnapPipe;
    stViConfig.astViInfo[0].stSnapInfo.enVideoPipeMode = enVideoPipeMode;
    stViConfig.astViInfo[0].stSnapInfo.enSnapPipeMode = enSnapPipeMode;
    SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
    abChnEnable[0] = HI_TRUE;

    stBnrDumpAttr.bEnable = HI_FALSE;
    stBnrDumpAttr.u32Depth = 0;
    HI_MPI_SNAP_SetBNRRawDumpAttr(SnapPipe, &stBnrDumpAttr);

    SAMPLE_COMM_VENC_Stop(VencChn);
    SAMPLE_COMM_VPSS_UnBind_VO(VpssGrp0, VpssChn[0], stVoConfig.VoDev, VoChn);

    SAMPLE_COMM_VO_StopVO(&stVoConfig);

    SAMPLE_COMM_VPSS_Stop(VpssGrp1, abChnEnable);

    SAMPLE_COMM_VPSS_Stop(VpssGrp0, abChnEnable);
    SAMPLE_COMM_VI_UnBind_VPSS(VideoPipe, ViChn, VpssGrp0);
    SAMPLE_COMM_VI_UnBind_VPSS(SnapPipe, ViChn, VpssGrp1);

    SAMPLE_COMM_VI_StopVi(&stViConfig);

    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

