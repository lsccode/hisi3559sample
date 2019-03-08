#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vgs.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_vgs.h"

#include "sample_comm_ive.h"

static HI_BOOL bMpiInit = HI_FALSE;

HI_U16 SAMPLE_COMM_IVE_CalcStride(HI_U32 u32Width, HI_U8 u8Align)
{
    return (u32Width + (u8Align - u32Width % u8Align) % u8Align);
}

static HI_S32 SAMPLE_IVE_MPI_Init(HI_VOID)
{
    HI_S32 s32Ret;

    HI_MPI_SYS_Exit();

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}


HI_VOID SAMPLE_COMM_IVE_CheckIveMpiInit(HI_VOID)
{
    if (HI_FALSE == bMpiInit)
    {
        if (SAMPLE_IVE_MPI_Init())
        {
            SAMPLE_PRT("Ive mpi init failed!\n");
            exit(-1);
        }
        bMpiInit = HI_TRUE;
    }
}
HI_S32 SAMPLE_COMM_IVE_IveMpiExit(HI_VOID)
{
    bMpiInit = HI_FALSE;
    if (HI_MPI_SYS_Exit())
    {
        SAMPLE_PRT("Sys exit failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VGS_FillRect(VIDEO_FRAME_INFO_S* pstFrmInfo, SAMPLE_RECT_ARRAY_S* pstRect, HI_U32 u32Color)
{
    VGS_HANDLE VgsHandle = -1;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U16 i;
    VGS_TASK_ATTR_S stVgsTask;
    VGS_ADD_COVER_S stVgsAddCover;

    if (0 == pstRect->u16Num)
    {
        return s32Ret;
    }
    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Vgs begin job fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    memcpy(&stVgsTask.stImgIn, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&stVgsTask.stImgOut, pstFrmInfo, sizeof(VIDEO_FRAME_INFO_S));

    stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
    stVgsAddCover.u32Color = u32Color;
    for (i = 0; i < pstRect->u16Num; i++)
    {
        stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
        stVgsAddCover.stQuadRangle.u32Thick = 2;
        memcpy(stVgsAddCover.stQuadRangle.stPoint, pstRect->astRect[i].astPoint, sizeof(pstRect->astRect[i].astPoint));
        s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
            HI_MPI_VGS_CancelJob(VgsHandle);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
        HI_MPI_VGS_CancelJob(VgsHandle);
        return s32Ret;
    }

    return s32Ret;

}

/******************************************************************************
* function : Init Vb
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_VbInit(PIC_SIZE_E *paenSize,SIZE_S *pastSize,HI_U32 u32VpssChnNum)
{
    HI_S32 s32Ret;
    HI_U32 i;
    HI_U64 u64BlkSize;
    VB_CONFIG_S stVbConf;

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    for (i = 0; i < u32VpssChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_SYS_GetPicSize(paenSize[i], &pastSize[i]);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VB_FAIL_0,
            "SAMPLE_COMM_SYS_GetPicSize failed,Error(%#x)!\n",s32Ret);

        u64BlkSize = COMMON_GetPicBufferSize(pastSize[i].u32Width, pastSize[i].u32Height,
                            SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
        /* comm video buffer */
        stVbConf.astCommPool[i].u64BlkSize = u64BlkSize;
        stVbConf.astCommPool[i].u32BlkCnt  = 10;
    }

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VB_FAIL_1,
        "SAMPLE_COMM_SYS_Init failed,Error(%#x)!\n", s32Ret);

    return s32Ret;
VB_FAIL_1:
    SAMPLE_COMM_SYS_Exit();
VB_FAIL_0:

    return s32Ret;

}

/******************************************************************************
* function : Start Vpss
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartVpss(SIZE_S *pastSize,HI_U32 u32VpssChnNum)
{
    HI_S32 i;
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};
    VPSS_GRP VpssGrp = 0;
    
    stVpssGrpAttr.u32MaxW = 3840;
    stVpssGrpAttr.u32MaxH = 2160;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    for (i = 0; i < u32VpssChnNum; i++)
    {
        abChnEnable[i] = HI_TRUE;
    }
    
    for(i = 0; i < VPSS_MAX_CHN_NUM; i++)
    {
        astVpssChnAttr[i].u32Width                    = pastSize[i].u32Width;
        astVpssChnAttr[i].u32Height                   = pastSize[i].u32Height;
        astVpssChnAttr[i].enChnMode                   = VPSS_CHN_MODE_USER;
        astVpssChnAttr[i].enCompressMode              = COMPRESS_MODE_NONE;
        astVpssChnAttr[i].enDynamicRange              = DYNAMIC_RANGE_SDR8;
        astVpssChnAttr[i].enVideoFormat               = VIDEO_FORMAT_LINEAR;
        astVpssChnAttr[i].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        astVpssChnAttr[i].stFrameRate.s32SrcFrameRate = -1;
        astVpssChnAttr[i].stFrameRate.s32DstFrameRate = -1;
        astVpssChnAttr[i].u32Depth                    = 1;
        astVpssChnAttr[i].bMirror                     = HI_FALSE;
        astVpssChnAttr[i].bFlip                       = HI_FALSE;
        astVpssChnAttr[i].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    }

    return SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, &astVpssChnAttr[0]);

}
/******************************************************************************
* function : Stop Vpss
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_StopVpss(HI_U32 u32VpssChnNum)
{
    VPSS_GRP VpssGrp = 0;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM] = {HI_FALSE, HI_FALSE, HI_FALSE, HI_FALSE};
    HI_S32 i = 0;

    for (i = 0; i < u32VpssChnNum; i++)
    {
        abChnEnable[i] = HI_TRUE;
    }

    (HI_VOID)SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEnable);

    return;
}

/******************************************************************************
* function : Start Vo
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartVo(HI_VOID)
{
    HI_S32 s32Ret;
    VO_DEV VoDev = SAMPLE_VO_DEV_DHD0;
    VO_LAYER VoLayer = 0;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    HI_U32 u32DisBufLen = 3;

    stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
    stVoPubAttr.enIntfType = VO_INTF_HDMI;
    stVoPubAttr.u32BgColor = COLOR_RGB_BLUE;
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VO_FAIL_0,
        "SAMPLE_COMM_VO_StartDev failed,Error(%#x)!\n",s32Ret);

    s32Ret = SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VO_FAIL_1,
        "SAMPLE_COMM_VO_HdmiStart failed,Error(%#x)!\n",s32Ret);

    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync,&stLayerAttr.stDispRect.u32Width,
                                    &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VO_FAIL_2,
        "SAMPLE_COMM_VO_GetWH failed,Error(%#x)!\n",s32Ret);

    s32Ret = HI_MPI_VO_SetDisplayBufLen(VoLayer, u32DisBufLen);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VO_FAIL_2,
        "HI_MPI_VO_SetDisplayBufLen failed,Error(%#x)!\n",s32Ret);

    stLayerAttr.stDispRect.s32X        = 0;
    stLayerAttr.stDispRect.s32Y        = 0;
    stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;
    stLayerAttr.bDoubleFrame        = HI_FALSE;
    stLayerAttr.bClusterMode        = HI_FALSE;
    stLayerAttr.enPixFormat            = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stLayerAttr.enDstDynamicRange     = DYNAMIC_RANGE_SDR8;

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VO_FAIL_2,
        "SAMPLE_COMM_VO_StartLayer failed,Error(%#x)!\n",s32Ret);

    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, VO_FAIL_3,
        "SAMPLE_COMM_VO_StartChn failed,Error(%#x)!\n",s32Ret);

    return s32Ret;
VO_FAIL_3:
     SAMPLE_COMM_VO_StopLayer(VoLayer);
VO_FAIL_2:
     SAMPLE_COMM_VO_HdmiStop();
VO_FAIL_1:
     SAMPLE_COMM_VO_StopDev(VoDev);
VO_FAIL_0:
    return s32Ret;
}
/******************************************************************************
* function : Stop Vo
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_StopVo(HI_VOID)
{
    VO_DEV VoDev = SAMPLE_VO_DEV_DHD0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;

    (HI_VOID)SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    (HI_VOID)SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_HdmiStop();
    (HI_VOID)SAMPLE_COMM_VO_StopDev(VoDev);
}
/******************************************************************************
* function : Start Vi/Vpss/Venc/Vo
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartViVpssVencVo(SAMPLE_VI_CONFIG_S *pstViConfig,
    SAMPLE_IVE_SWITCH_S *pstSwitch,PIC_SIZE_E *penExtPicSize)
{
    SIZE_S astSize[VPSS_CHN_NUM];
    PIC_SIZE_E aenSize[VPSS_CHN_NUM];
    VI_CHN_ATTR_S stViChnAttr;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    PAYLOAD_TYPE_E enStreamType = PT_H264;
    VENC_GOP_ATTR_S stGopAttr;
    VI_DEV ViDev0 = 0;
    VI_PIPE ViPipe0 = 0;
    VI_CHN ViChn = 0;
    HI_S32 s32ViCnt = 1;
    HI_S32 s32WorkSnsId  = 0;
    VPSS_GRP VpssGrp = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    VENC_CHN VeH264Chn = 0;
    WDR_MODE_E enWDRMode = WDR_MODE_NONE;
    DYNAMIC_RANGE_E enDynamicRange = DYNAMIC_RANGE_SDR8;
    PIXEL_FORMAT_E enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VIDEO_FORMAT_E enVideoFormat = VIDEO_FORMAT_LINEAR;
    COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;
    VI_VPSS_MODE_E enMastPipeMode = VI_ONLINE_VPSS_OFFLINE;

    memset(pstViConfig,0,sizeof(*pstViConfig));

    SAMPLE_COMM_VI_GetSensorInfo(pstViConfig);
    pstViConfig->s32WorkingViNum                           = s32ViCnt;

    pstViConfig->as32WorkingViId[0]                        = 0;
    pstViConfig->astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(pstViConfig->astViInfo[0].stSnsInfo.enSnsType, 0);
    pstViConfig->astViInfo[0].stSnsInfo.s32BusId           = 0;

    pstViConfig->astViInfo[0].stDevInfo.ViDev              = ViDev0;
    pstViConfig->astViInfo[0].stDevInfo.enWDRMode          = enWDRMode;

    pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode    = enMastPipeMode;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe0;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[1]          = -1;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    pstViConfig->astViInfo[0].stChnInfo.ViChn              = ViChn;
    pstViConfig->astViInfo[0].stChnInfo.enPixFormat        = enPixFormat;
    pstViConfig->astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    pstViConfig->astViInfo[0].stChnInfo.enVideoFormat      = enVideoFormat;
    pstViConfig->astViInfo[0].stChnInfo.enCompressMode     = enCompressMode;

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(pstViConfig->astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &aenSize[0]);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_0,
        "Error(%#x),SAMPLE_COMM_VI_GetSizeBySensor failed!\n",s32Ret);
    aenSize[1] = *penExtPicSize;

    /******************************************
     step  1: Init vb
    ******************************************/
    s32Ret = SAMPLE_COMM_IVE_VbInit(aenSize,astSize,VPSS_CHN_NUM);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_0,
        "Error(%#x),SAMPLE_COMM_IVE_VbInit failed!\n",s32Ret);
    /******************************************
     step 2: Start vi
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_1,
        "Error(%#x),SAMPLE_COMM_VI_SetParam failed!\n",s32Ret);
    
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_1,
        "Error(%#x),SAMPLE_COMM_VI_StartVi failed!\n",s32Ret);
    /******************************************
     step 3: Start vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_2,
        "Error(%#x),SAMPLE_IVS_StartVpss failed!\n",s32Ret);
    /******************************************
      step 4: Bind vpss to vi
     ******************************************/
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe0, ViChn, VpssGrp);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_3,
        "Error(%#x),SAMPLE_COMM_VI_BindVpss failed!\n",s32Ret);
    //Set vi frame
    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe0, ViChn,&stViChnAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
        "Error(%#x),HI_MPI_VI_GetChnAttr failed!\n",s32Ret);

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe0, ViChn,&stViChnAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
        "Error(%#x),HI_MPI_VI_SetChnAttr failed!\n",s32Ret);
    /******************************************
     step 5: Start Vo
     ******************************************/
    if (HI_TRUE == pstSwitch->bVo)
    {
        s32Ret = SAMPLE_COMM_IVE_StartVo();
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
            "Error(%#x),SAMPLE_COMM_IVE_StartVo failed!\n", s32Ret);
    }
    /******************************************
     step 6: Start Venc
    ******************************************/
    if (HI_TRUE == pstSwitch->bVenc)
    {
        s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_5,
            "Error(%#x),SAMPLE_COMM_VENC_GetGopAttr failed!\n",s32Ret);
        s32Ret = SAMPLE_COMM_VENC_Start(VeH264Chn, enStreamType,aenSize[0],enRcMode,0,&stGopAttr);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_5,
            "Error(%#x),SAMPLE_COMM_VENC_Start failed!\n",s32Ret);
        s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VeH264Chn, 1);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_6,
            "Error(%#x),SAMPLE_COMM_VENC_StartGetStream failed!\n",s32Ret);
    }

    return HI_SUCCESS;

END_INIT_6:
    if (HI_TRUE == pstSwitch->bVenc)
    {
        SAMPLE_COMM_VENC_Stop(VeH264Chn);
    }
END_INIT_5:
    if (HI_TRUE == pstSwitch->bVo)
    {
        SAMPLE_COMM_IVE_StopVo();
    }
END_INIT_4:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe0, ViChn, VpssGrp);
END_INIT_3:
    SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_INIT_2:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
END_INIT_1:    //system exit
    SAMPLE_COMM_SYS_Exit();
    memset(pstViConfig,0,sizeof(*pstViConfig));
END_INIT_0:

    return s32Ret;
}
/******************************************************************************
* function : Stop Vi/Vpss/Venc/Vo
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_StopViVpssVencVo(SAMPLE_VI_CONFIG_S *pstViConfig,SAMPLE_IVE_SWITCH_S *pstSwitch)
{
    if (HI_TRUE == pstSwitch->bVenc)
    {
        SAMPLE_COMM_VENC_StopGetStream();
        SAMPLE_COMM_VENC_Stop(0);
    }
    if (HI_TRUE == pstSwitch->bVo)
    {
        SAMPLE_COMM_IVE_StopVo();
    }

    SAMPLE_COMM_VI_UnBind_VPSS(pstViConfig->astViInfo[0].stPipeInfo.aPipe[0], 
        pstViConfig->astViInfo[0].stChnInfo.ViChn, 0);
    SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
    SAMPLE_COMM_VI_StopVi(pstViConfig);
    SAMPLE_COMM_SYS_Exit();

    memset(pstViConfig,0,sizeof(*pstViConfig));
}


