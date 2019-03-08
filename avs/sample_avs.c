

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

#include "mpi_avs.h"
#include "sample_comm.h"
#include "mpi_vb.h"

#ifndef __HuaweiLite__
#include "hi_avs_lut_generate.h"
#endif

VO_INTF_TYPE_E g_enAVSVoIntf    = VO_INTF_HDMI;
VO_INTF_SYNC_E g_enAVSIntfSync  = VO_OUTPUT_1080P30;

PIC_SIZE_E     g_enPicSize  = PIC_3840x2160;
HI_U64         g_AVSLUTAddr = 0;
HI_U64         g_AVSMaskAddr = 0;

VB_BLK  g_hBlkHdl;
VB_POOL g_Pool;

HI_BOOL   g_AVSGo;
pthread_t AVSThreadId;

typedef struct hiSAMPLE_AVS_CONFIG_S
{
    HI_U32                u32OutW;
    HI_U32                u32OutH;
    AVS_GRP_ATTR_S        stGrpAttr;
    COMPRESS_MODE_E       enOutCmpMode;
    HI_BOOL               benChn1;
    HI_CHAR               pLUTName[AVS_PIPE_NUM][64];
} SAMPLE_AVS_CONFIG_S;


/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_AVS_Usage(char* sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0) 2 fisheye stitching, Normal projection.\n");
    printf("\t 1) 4 fisheye stitching, Cube map.\n");
    printf("\t 2) 4 pic no blend stitching.\n");
    printf("\t 3) 8 nonfisheye Equirectangular.\n");
    printf("\t 4) 2 fisheye stitching, Image stabilizing.\n");

#ifndef __HuaweiLite__
    printf("\t 5) Generate lut.\n");
#endif

    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_AVS_HandleSig(HI_S32 signo)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_VENC_StopGetStream();

        if (g_AVSLUTAddr)
        {
            s32Ret = HI_MPI_SYS_MmzFree(g_AVSLUTAddr, NULL);

            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("MMzFree fail!\n");
            }
            g_AVSLUTAddr = 0;
        }

        if (g_AVSMaskAddr)
        {
            s32Ret = HI_MPI_SYS_MmzFree(g_AVSMaskAddr, NULL);

            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("MMzFree fail!\n");
            }
            g_AVSMaskAddr = 0;
        }


        SAMPLE_COMM_All_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }

    exit(-1);
}


HI_S32 SAMPLE_AVS_StartVENC(PIC_SIZE_E enPicSize)
{
    VENC_GOP_ATTR_S stGopAttr;
    VENC_CHN VencChn = 0;
    HI_U32 u32Profile = 1;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP, &stGopAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get GopAttr failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, PT_H265, enPicSize, SAMPLE_RC_CBR, u32Profile, &stGopAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VENC failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}


HI_U32 SAMPLE_AVS_GetFileLen(const HI_CHAR* File)
{
    FILE* FILE;
    HI_U32 u32FileLen;
    HI_S32 s32Ret;

    FILE = fopen(File, "rb");

    if (NULL != FILE)
    {
        s32Ret = fseek(FILE, 0L, SEEK_END);
        if (0 != s32Ret)
        {
            SAMPLE_PRT("fseek err!\n");
            fclose(FILE);
            return 0;
        }

        u32FileLen = ftell(FILE);

        s32Ret = fseek(FILE, 0L, SEEK_SET);
        if (0 != s32Ret)
        {
            SAMPLE_PRT("fseek err!\n");
            fclose(FILE);
            return 0;
        }

        fclose(FILE);
    }
    else
    {
        SAMPLE_PRT("open file %s fail!\n", File);
        u32FileLen = 0;
    }

    return u32FileLen;
}

HI_S32 SAMPLE_AVS_LoadFile(const HI_CHAR* File, HI_VOID* Addr, HI_U32 u32Size)
{
    FILE* FILE;
    HI_U32 u32ReadBytes;

    FILE = fopen(File, "rb");

    if (FILE != NULL)
    {
        u32ReadBytes = fread(Addr, u32Size, 1, FILE);

        if (u32ReadBytes != 1)
        {
            SAMPLE_PRT("read file error!\n");
            fclose(FILE);
            return HI_FAILURE;
        }

        fclose(FILE);
    }
    else
    {
        SAMPLE_PRT("Error: Open file of %s failed!\n", File);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_AVS_Bind(HI_U32 u32PipeNum, HI_BOOL benChn1)
{
    AVS_GRP  AVSGrp = 0;
    AVS_PIPE AVSPipe = 0;
    AVS_CHN  AVSChn = 0;
    VI_PIPE  ViPipe;
    VI_CHN   ViChn = 0;
    HI_S32   s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    VENC_CHN VencChn = 0;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn = 0;
    HI_U32 i;

    for (i = 0; i < u32PipeNum; i++)
    {
        ViPipe = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, VpssGrp);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VI bind VPSS fail with %#x", s32Ret);
            return HI_FAILURE;
        }
    }

    for (i = 0; i < u32PipeNum; i++)
    {
        AVSPipe = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VPSS_Bind_AVS(VpssGrp, VpssChn, AVSGrp, AVSPipe);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("AVS bind VO fail with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s32Ret = SAMPLE_COMM_AVS_Bind_VENC(AVSGrp, AVSChn, VencChn);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VPSSs bind VENC fail with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if (benChn1)
    {
        AVSChn = 1;
    }
    else
    {
        AVSChn = 0;
    }

    s32Ret = SAMPLE_COMM_AVS_Bind_VO(AVSGrp, AVSChn, VoLayer, VoChn);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("AVS bind VO fail with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_AVS_UnBind(HI_U32 u32PipeNum)
{
    AVS_GRP  AVSGrp  = 0;
    AVS_PIPE AVSPipe;
    AVS_CHN  AVSChn  = 0;
    VI_PIPE  ViPipe;
    VI_CHN   ViChn   = 0;
    HI_S32   s32Ret  = HI_SUCCESS;
    VO_LAYER VoLayer = 0;
    VO_CHN   VoChn   = 0;
    HI_U32   i;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    VENC_CHN VencChn = 0;

    s32Ret = SAMPLE_COMM_AVS_UnBind_VENC(AVSGrp, AVSChn, VencChn);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("AVS unbind VENC fail with %#x", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AVS_UnBind_VO(VoLayer, VoChn);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("AVS unbind VO fail with %#x", s32Ret);
        return HI_FAILURE;
    }

    for (i = 0; i < u32PipeNum; i++)
    {
        ViPipe = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VI_UnBind_VPSS(ViPipe, ViChn, VpssGrp);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VI unbind VPSS fail with %#x", s32Ret);
            return HI_FAILURE;
        }
    }

    for (i = 0; i < u32PipeNum; i++)
    {
        AVSPipe = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VPSS_UnBind_AVS(VpssGrp, VpssChn, AVSGrp, AVSPipe);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VPSS unbind AVS fail with %#x", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_AVS_StartAVS(AVS_GRP AVSGrp, SAMPLE_AVS_CONFIG_S* pstAVSConfig)
{
    HI_U32         u32LUTSize;
    AVS_GRP_ATTR_S stAVSGrpAttr = {0};
    AVS_CHN        AVSChn = 0;
    AVS_CHN_ATTR_S stChnAttr = {0};
    HI_VOID*       pLUTVirAddr = NULL;
    HI_S32         s32Ret = HI_SUCCESS;
    HI_U32         u32PipeNum;
    HI_U32         i = 0;

    memcpy(&stAVSGrpAttr, &pstAVSConfig->stGrpAttr, sizeof(AVS_GRP_ATTR_S));

    u32PipeNum = stAVSGrpAttr.u32PipeNum;

    if (AVS_MODE_BLEND == stAVSGrpAttr.enMode)
    {
        u32LUTSize = SAMPLE_AVS_GetFileLen(pstAVSConfig->pLUTName[0]);

        if (0 == u32LUTSize)
        {
            SAMPLE_PRT("Open lut file fail!\n");
            return HI_FAILURE;
        }

        u32LUTSize = ALIGN_UP(u32LUTSize, 256);

        s32Ret = HI_MPI_SYS_MmzAlloc(&(g_AVSLUTAddr), &(pLUTVirAddr), NULL, NULL, u32LUTSize * u32PipeNum);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("alloc LUT buf fail with %#x!\n", s32Ret);
            goto exit;
        }

        for (i = 0; i < u32PipeNum; i++)
        {
            if (HI_SUCCESS != SAMPLE_AVS_LoadFile(pstAVSConfig->pLUTName[i], ((HI_CHAR*)pLUTVirAddr + u32LUTSize * i), u32LUTSize))
            {
                goto exit;
            }

            stAVSGrpAttr.stLUT.u64PhyAddr[i] = g_AVSLUTAddr + u32LUTSize * i;
        }
    }

    stAVSGrpAttr.bSyncPipe                     = HI_TRUE;
    stAVSGrpAttr.stFrameRate.s32SrcFrameRate   = -1;
    stAVSGrpAttr.stFrameRate.s32DstFrameRate   = -1;
    stAVSGrpAttr.stLUT.enAccuracy              = AVS_LUT_ACCURACY_HIGH;

    s32Ret = HI_MPI_AVS_CreateGrp(AVSGrp, &stAVSGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Creat grp failed with %#x!\n", s32Ret);
        goto exit;
    }

    stChnAttr.enCompressMode              = pstAVSConfig->enOutCmpMode;
    stChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stChnAttr.stFrameRate.s32DstFrameRate = -1;
    stChnAttr.u32Depth                    = 0;
    stChnAttr.u32Width                    = pstAVSConfig->u32OutW;
    stChnAttr.u32Height                   = pstAVSConfig->u32OutH;
    stChnAttr.enDynamicRange              = DYNAMIC_RANGE_SDR10;

    s32Ret = HI_MPI_AVS_SetChnAttr(AVSGrp, AVSChn, &stChnAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set chnattr failed with %#x!\n", s32Ret);
        goto exit;
    }

    s32Ret = HI_MPI_AVS_EnableChn(AVSGrp, AVSChn);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("AVS enable chn failed with %#x!\n", s32Ret);
        goto exit;
    }

    if (pstAVSConfig->benChn1)
    {
        AVSChn = 1;
        stChnAttr.u32Width  = 1920;
        stChnAttr.u32Height = 1080;
        stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
        s32Ret = HI_MPI_AVS_SetChnAttr(AVSGrp, AVSChn, &stChnAttr);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Set chnattr failed with %#x!\n", s32Ret);
            goto exit;
        }

        s32Ret = HI_MPI_AVS_EnableChn(AVSGrp, AVSChn);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("AVS enable chn failed with %#x!\n", s32Ret);
            goto exit;
        }
    }

    s32Ret = HI_MPI_AVS_StartGrp(AVSGrp);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("AVS start grp failed with %#x!\n", s32Ret);
        goto exit;
    }

    return HI_SUCCESS;

exit:
    if (g_AVSLUTAddr)
    {
        s32Ret = HI_MPI_SYS_MmzFree(g_AVSLUTAddr, NULL);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("MMzFree fail!\n");
        }
        g_AVSLUTAddr = 0;
    }

    return HI_FAILURE;

}


HI_S32 SAMPLE_AVS_StopAVS(AVS_GRP AVSGrp, HI_BOOL benChn1)
{
    AVS_CHN AVSChn = 0;
    HI_S32  s32Ret = HI_SUCCESS;

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_AVS_StopGrp(AVSGrp);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if (benChn1)
    {
        AVSChn = 1;
        s32Ret = HI_MPI_AVS_DisableChn(AVSGrp, AVSChn);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    AVSChn = 0;
    s32Ret = HI_MPI_AVS_DisableChn(AVSGrp, AVSChn);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_AVS_DestroyGrp(AVSGrp);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if (g_AVSLUTAddr)
    {
        s32Ret = HI_MPI_SYS_MmzFree(g_AVSLUTAddr, NULL);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        g_AVSLUTAddr = 0;
    }

    return HI_SUCCESS;
}

static HI_VOID* SAMPLE_AVS_SetGrpAttrThread(HI_VOID* arg)
{

    HI_S32         s32Ret = HI_SUCCESS;
    AVS_GRP        AVSGrp = 0;
    AVS_GRP_ATTR_S stAVSGrpAttr;
    HI_U32         u32Step = 1;

    prctl(PR_SET_NAME, "AVS_Cruise", 0,0,0);

    printf("\nStart to cruise......\n\n");

    while (HI_FALSE == g_AVSGo)
    {
        s32Ret = HI_MPI_AVS_GetGrpAttr(AVSGrp, &stAVSGrpAttr);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("get grp attr failed with %#x!\n", s32Ret);
            return HI_NULL;
        }

        stAVSGrpAttr.stOutAttr.stRotation.s32Yaw += u32Step;

        if (stAVSGrpAttr.stOutAttr.stRotation.s32Yaw > 18000)
        {
            stAVSGrpAttr.stOutAttr.stRotation.s32Yaw = -18000 + u32Step;
        }

        s32Ret = HI_MPI_AVS_SetGrpAttr(AVSGrp, &stAVSGrpAttr);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("set grp attr failed with %#x!\n", s32Ret);
            return HI_NULL;
        }

        usleep(10 * 1000);
    }

    return HI_NULL;
}

HI_VOID SAMPLE_AVS_StartSetGrpAttrThrd(HI_VOID)
{
    g_AVSGo = HI_FALSE;

    pthread_create(&AVSThreadId, HI_NULL, SAMPLE_AVS_SetGrpAttrThread, NULL);

    sleep(1);

    return;
}

HI_VOID SAMPLE_AVS_StopSetGrpAttrThrd(HI_VOID)
{
    if (HI_FALSE == g_AVSGo)
    {
        g_AVSGo = HI_TRUE;
        pthread_join(AVSThreadId, HI_NULL);
    }

    return;
}



/******************************************************************************
* function : 2 fisheye, channel encode and preview.
******************************************************************************/
HI_S32 SAMPLE_AVS_2Fisheye_Equirectangular_Cylindrical_Rectilinear(void)
{
    HI_U32               u32ViChnCnt = 2;
    SIZE_S               stSize;
    HI_U32               u32BlkSize;
    VB_CONFIG_S          stVbConf;
    SAMPLE_VI_CONFIG_S   stViConfig;
    VI_STITCH_GRP_ATTR_S stStitchGrpAttr;
    VI_STITCH_GRP        StitchGrp = 0;
    SAMPLE_VO_CONFIG_S   stVoConfig;
    PIC_SIZE_E           enPicSize = PIC_3000x3000;
    HI_S32               s32Ret = HI_SUCCESS;
    SAMPLE_AVS_CONFIG_S  stAVSConfig = {0};
    HI_U32               u32PipeNum = 2;
    HI_U32               u32OutW, u32OutH;
    HI_S32               s32ChnNum = 1;
    VENC_CHN             VencChn[1] = {0};
    AVS_GRP              AVSGrp = 0;
    HI_S32               i;
    AVS_GRP_ATTR_S*      pstGrpAttr;
    VPSS_GRP             VpssGrp = 0;
    VPSS_GRP_ATTR_S      stVpssGrpAttr;
    VPSS_CHN_ATTR_S      astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL              abChnEn[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};
    HI_BOOL              benChn1 = HI_TRUE;
    AVS_MOD_PARAM_S      stModParam;

    u32OutW = 3840;
    u32OutH = 2160;

    /************************************************
    step1:  get all sensors information
    *************************************************/

    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

    stViConfig.s32WorkingViNum = u32PipeNum;

    for (i = 0; i < u32PipeNum; i++)
    {
        stViConfig.astViInfo[i].stDevInfo.enWDRMode        = WDR_MODE_NONE;

        stViConfig.astViInfo[i].stChnInfo.enCompressMode   = COMPRESS_MODE_SEG;
        stViConfig.astViInfo[i].stChnInfo.enDynamicRange   = DYNAMIC_RANGE_SDR10;
        stViConfig.astViInfo[i].stChnInfo.enVideoFormat    = VIDEO_FORMAT_LINEAR;
        stViConfig.astViInfo[i].stChnInfo.enPixFormat      = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        stViConfig.astViInfo[i].stChnInfo.ViChn            = 0;

        stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode  = VI_OFFLINE_VPSS_OFFLINE;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[1]        = -1;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[2]        = -1;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[3]        = -1;
    }

    stViConfig.as32WorkingViId[0]                   = 0;
    stViConfig.astViInfo[0].stDevInfo.ViDev         = 0;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]     = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev       = 0;
    stViConfig.astViInfo[0].stSnsInfo.s32BusId      = 0;

    stViConfig.as32WorkingViId[1]                   = 1;
    stViConfig.astViInfo[1].stDevInfo.ViDev         = 2;
    stViConfig.astViInfo[1].stPipeInfo.aPipe[0]     = 1;
    stViConfig.astViInfo[1].stSnsInfo.MipiDev       = 2;
    stViConfig.astViInfo[1].stSnsInfo.s32BusId      = 1;


    /************************************************
    step2:  get  input size
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step  3: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = AVS_GetPicBufferSize(3000, 3000, COMPRESS_MODE_TILE, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 8;

    u32BlkSize = COMMON_GetPicBufferSize(u32OutW, u32OutH, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                        DATA_BITWIDTH_10, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt  = 6;

    u32BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP,
                                    COMPRESS_MODE_LINE, DEFAULT_ALIGN);
    stVbConf.astCommPool[2].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt  = u32ViChnCnt * 4;

    u32BlkSize = COMMON_GetPicBufferSize(1920, 1080, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                        DATA_BITWIDTH_10, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[3].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[3].u32BlkCnt  = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto exit;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        goto exit;
    }

    /******************************************
      step  4: start VI
     ******************************************/
    stStitchGrpAttr.bStitch    = HI_FALSE;
    stStitchGrpAttr.enMode   = VI_STITCH_ISP_CFG_NORMAL;
    stStitchGrpAttr.u32PipeNum = 2;
    stStitchGrpAttr.PipeId[0]  = 0;
    stStitchGrpAttr.PipeId[1]  = 1;

    s32Ret = HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("disable stitch fail!\n");
        goto exit;
    }

    stStitchGrpAttr.bStitch = HI_TRUE;
    s32Ret = HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set Stitch grp failed with %d\n", s32Ret);
        goto exit;
    }

    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartVi failed with %d\n", s32Ret);
        goto exit;
    }


    /******************************************
      step  5: start VO
     ******************************************/
    SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);

    stVoConfig.VoDev                  = SAMPLE_VO_DEV_UHD;
    stVoConfig.enVoIntfType           = g_enAVSVoIntf;
    stVoConfig.enIntfSync             = g_enAVSIntfSync;
    stVoConfig.enPicSize              = enPicSize;
    stVoConfig.stDispRect.s32X        = 0;
    stVoConfig.stDispRect.s32Y        = 0;
    stVoConfig.stDispRect.u32Width    = 1920;
    stVoConfig.stDispRect.u32Height   = 1080;
    stVoConfig.stImageSize.u32Width   = 1920;
    stVoConfig.stImageSize.u32Height  = 1080;
    stVoConfig.enPixFormat            = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVoConfig.u32DisBufLen           = 0;
    stVoConfig.enDstDynamicRange      = DYNAMIC_RANGE_SDR10;
    stVoConfig.enVoMode               = VO_MODE_1MUX;
    stVoConfig.enVoPartMode           = VO_PART_MODE_SINGLE;
    stVoConfig.u32BgColor             = COLOR_RGB_BLUE;

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartVO failed with %d\n", s32Ret);
        goto exit1;
    }

    /******************************************
      step  6: start VENC
     ******************************************/
    s32Ret = SAMPLE_AVS_StartVENC(PIC_3840x2160);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_StartVENC failed with %d\n", s32Ret);
        goto exit2;
    }

    /******************************************
      step  7: start VPSS
     ******************************************/

    stVpssGrpAttr.u32MaxW                          = 3840;
    stVpssGrpAttr.u32MaxH                          = 2160;
    stVpssGrpAttr.enDynamicRange                   = DYNAMIC_RANGE_SDR10;
    stVpssGrpAttr.enPixelFormat                    = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate      = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate      = -1;

    astVpssChnAttr[0].u32Width                     = 3000;
    astVpssChnAttr[0].u32Height                    = 3000;
    astVpssChnAttr[0].enChnMode                    = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode               = COMPRESS_MODE_TILE;
    astVpssChnAttr[0].enDynamicRange               = DYNAMIC_RANGE_SDR10;
    astVpssChnAttr[0].enVideoFormat                = VIDEO_FORMAT_TILE_16x8;
    astVpssChnAttr[0].enPixelFormat                = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate  = -1;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate  = -1;
    astVpssChnAttr[0].u32Depth                     = 0;
    astVpssChnAttr[0].bMirror                      = HI_FALSE;
    astVpssChnAttr[0].bFlip                        = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode         = ASPECT_RATIO_NONE;
    astVpssChnAttr[0].stAspectRatio.u32BgColor     = 0xff;

    for (i = 0; i < u32PipeNum; i++)
    {
        VpssGrp = i;

        s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEn, &stVpssGrpAttr, astVpssChnAttr);

        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
            goto exit3;
        }
    }

    /******************************************
      step  8: start AVS
     ******************************************/

    stModParam.u32WorkingSetSize = 67 * 1024;
    s32Ret = HI_MPI_AVS_SetModParam(&stModParam);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_AVS_SetModParam fail for %#x!\n", s32Ret);
        goto exit3;
    }

    for (i = 0; i < u32PipeNum; i++)
    {
        snprintf(stAVSConfig.pLUTName[i], 64, "./data/zmax2_mesh_%d.bin", i);
    }

    stAVSConfig.u32OutW                          = u32OutW;
    stAVSConfig.u32OutH                          = u32OutH;
    stAVSConfig.enOutCmpMode                     = COMPRESS_MODE_SEG;
    stAVSConfig.benChn1                          = benChn1;

    pstGrpAttr                                   = &stAVSConfig.stGrpAttr;

    pstGrpAttr->enMode                           = AVS_MODE_BLEND;
    pstGrpAttr->u32PipeNum                       = u32PipeNum;
    pstGrpAttr->stGainAttr.enMode                = AVS_GAIN_MODE_AUTO;

    pstGrpAttr->stOutAttr.enPrjMode              = AVS_PROJECTION_EQUIRECTANGULAR;
    pstGrpAttr->stOutAttr.stCenter.s32X          = u32OutW / 2;
    pstGrpAttr->stOutAttr.stCenter.s32Y          = u32OutH / 2;
    pstGrpAttr->stOutAttr.stFOV.u32FOVX          = 36000;
    pstGrpAttr->stOutAttr.stFOV.u32FOVY          = 18000;
    pstGrpAttr->stOutAttr.stORIRotation.s32Roll  = 9000;
    pstGrpAttr->stOutAttr.stORIRotation.s32Pitch = 9000;
    pstGrpAttr->stOutAttr.stORIRotation.s32Yaw   = 0;
    pstGrpAttr->stOutAttr.stRotation.s32Roll     = 0;
    pstGrpAttr->stOutAttr.stRotation.s32Pitch    = 0;
    pstGrpAttr->stOutAttr.stRotation.s32Yaw      = 0;

    s32Ret = SAMPLE_AVS_StartAVS(AVSGrp, &stAVSConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_StartAVS failed with %d\n", s32Ret);
        goto exit4;
    }


    s32Ret = SAMPLE_AVS_Bind(u32PipeNum, benChn1);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_Bind failed with %d\n", s32Ret);
        goto exit5;
    }

    /******************************************
    step   9: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(VencChn, s32ChnNum);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed with %d\n", s32Ret);
        goto exit5;
    }

    PAUSE();

    printf("Switch to cylindrical projection mode!\n");

    AVS_GRP_ATTR_S stGrpAttr;

    s32Ret = HI_MPI_AVS_GetGrpAttr(AVSGrp, &stGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get avs grp attr fail with %#x!\n", s32Ret);
        goto exit5;
    }

    stGrpAttr.stOutAttr.stFOV.u32FOVX = 36000;
    stGrpAttr.stOutAttr.stFOV.u32FOVY = 11200;
    stGrpAttr.stOutAttr.enPrjMode     = AVS_PROJECTION_CYLINDRICAL;

    s32Ret = HI_MPI_AVS_SetGrpAttr(AVSGrp, &stGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get avs grp attr fail with %#x!\n", s32Ret);
        goto exit5;
    }

    PAUSE();
    printf("Switch to rectilinear projection mode!\n");

    s32Ret = HI_MPI_AVS_GetGrpAttr(AVSGrp, &stGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get avs grp attr fail with %#x!\n", s32Ret);
        goto exit5;
    }

    stGrpAttr.stOutAttr.stFOV.u32FOVX = 11200;
    stGrpAttr.stOutAttr.stFOV.u32FOVY = 11200;
    stGrpAttr.stOutAttr.enPrjMode     = AVS_PROJECTION_RECTILINEAR;

    s32Ret = HI_MPI_AVS_SetGrpAttr(AVSGrp, &stGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set avs grp attr fail with %#x!\n", s32Ret);
        goto exit5;
    }

    SAMPLE_AVS_StartSetGrpAttrThrd();

    PAUSE();

    SAMPLE_AVS_StopSetGrpAttrThrd();

    SAMPLE_COMM_VENC_StopGetStream();

    SAMPLE_AVS_UnBind(u32PipeNum);

exit5:
    SAMPLE_AVS_StopAVS(AVSGrp, benChn1);

exit4:

    for (i = 0; i < u32PipeNum; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEn);
    }

exit3:
    SAMPLE_COMM_VENC_Stop(VencChn[0]);

exit2:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

exit1:
    SAMPLE_COMM_VI_StopVi(&stViConfig);

exit:
    stStitchGrpAttr.bStitch = HI_FALSE;
    HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}


/******************************************************************************
* function : 4 fisheye, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_AVS_4Fisheye_CubeMap(void)
{

    HI_U32               u32ViChnCnt = 4;
    SIZE_S               stSize;
    HI_U32               u32BlkSize;
    VB_CONFIG_S          stVbConf;
    SAMPLE_VI_CONFIG_S   stViConfig;
    VI_STITCH_GRP_ATTR_S stStitchGrpAttr;
    VI_STITCH_GRP        StitchGrp = 0;
    SAMPLE_VO_CONFIG_S   stVoConfig;
    PIC_SIZE_E           enPicSize = PIC_3000x3000;
    HI_S32               s32Ret = HI_SUCCESS;
    SAMPLE_AVS_CONFIG_S  stAVSConfig = {0};
    HI_U32               u32PipeNum = 4;
    HI_U32               u32OutW, u32OutH;
    HI_S32               s32ChnNum = 1;
    AVS_GRP              AVSGrp = 0;
    HI_S32               i;
    AVS_GRP_ATTR_S*      pstGrpAttr;

    VENC_CHN             VencChn[48]   = {-1};
    HI_BOOL              benChn1 = HI_FALSE;
    VPSS_GRP             VpssGrp = 0;
    VPSS_GRP_ATTR_S      stVpssGrpAttr;
    VPSS_CHN_ATTR_S      astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL              abChnEn[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};
    AVS_MOD_PARAM_S      stModParam;

    u32OutW = 3840;
    u32OutH = 2160;

    /************************************************
    step1:  get all sensors information
    *************************************************/

    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

    stViConfig.s32WorkingViNum                  = u32PipeNum;

    for (i = 0; i < u32PipeNum; i++)
    {
        stViConfig.astViInfo[i].stDevInfo.enWDRMode        = WDR_MODE_NONE;

        stViConfig.astViInfo[i].stChnInfo.enDynamicRange   = DYNAMIC_RANGE_SDR10;
        stViConfig.astViInfo[i].stChnInfo.enCompressMode   = COMPRESS_MODE_SEG;
        stViConfig.astViInfo[i].stChnInfo.enVideoFormat    = VIDEO_FORMAT_LINEAR;
        stViConfig.astViInfo[i].stChnInfo.enPixFormat      = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        stViConfig.astViInfo[i].stChnInfo.ViChn            = 0;

        stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode  = VI_OFFLINE_VPSS_OFFLINE;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[1]        = -1;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[2]        = -1;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[3]        = -1;
    }

    stViConfig.as32WorkingViId[0]                   = 0;
    stViConfig.astViInfo[0].stDevInfo.ViDev         = 0;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]     = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev       = 0;
    stViConfig.astViInfo[0].stSnsInfo.s32BusId      = 0;

    stViConfig.as32WorkingViId[1]                   = 1;
    stViConfig.astViInfo[1].stDevInfo.ViDev         = 2;
    stViConfig.astViInfo[1].stPipeInfo.aPipe[0]     = 1;
    stViConfig.astViInfo[1].stSnsInfo.MipiDev       = 2;
    stViConfig.astViInfo[1].stSnsInfo.s32BusId      = 1;

    stViConfig.as32WorkingViId[2]                   = 2;
    stViConfig.astViInfo[2].stDevInfo.ViDev         = 4;
    stViConfig.astViInfo[2].stPipeInfo.aPipe[0]     = 2;
    stViConfig.astViInfo[2].stSnsInfo.MipiDev       = 4;
    stViConfig.astViInfo[2].stSnsInfo.s32BusId      = 2;

    stViConfig.as32WorkingViId[3]                   = 3;
    stViConfig.astViInfo[3].stDevInfo.ViDev         = 6;
    stViConfig.astViInfo[3].stPipeInfo.aPipe[0]     = 3;
    stViConfig.astViInfo[3].stSnsInfo.MipiDev       = 6;
    stViConfig.astViInfo[3].stSnsInfo.s32BusId      = 3;

    /************************************************
    step2:  get  input size
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return HI_FAILURE;
    }

    /******************************************
      step  3: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = AVS_GetPicBufferSize(3840, 2160, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 16;

    u32BlkSize = COMMON_GetPicBufferSize(u32OutW, u32OutH, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                        DATA_BITWIDTH_10, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt  = 20;

    u32BlkSize = VI_GetRawBufferSize(3840, 2700, PIXEL_FORMAT_RGB_BAYER_16BPP,
                                    COMPRESS_MODE_LINE, DEFAULT_ALIGN);
    stVbConf.astCommPool[2].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt  = u32ViChnCnt * 3;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto exit;
    }


    s32Ret = SAMPLE_COMM_VI_SetParam(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        goto exit;
    }


    /******************************************
      step  4: start VI
     ******************************************/
    stStitchGrpAttr.bStitch    = HI_FALSE;
    stStitchGrpAttr.enMode   = VI_STITCH_ISP_CFG_NORMAL;
    stStitchGrpAttr.u32PipeNum = 4;
    stStitchGrpAttr.PipeId[0]  = 0;
    stStitchGrpAttr.PipeId[1]  = 1;
    stStitchGrpAttr.PipeId[2]  = 2;
    stStitchGrpAttr.PipeId[3]  = 3;

    s32Ret = HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("disable stitch fail!\n");
        goto exit;
    }

    stStitchGrpAttr.bStitch = HI_TRUE;
    s32Ret = HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set Stitch grp failed with %d\n", s32Ret);
        goto exit;
    }


    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartVi failed with %d\n", s32Ret);
        goto exit;
    }

    /******************************************
      step  5: start VO
     ******************************************/
    SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);

    stVoConfig.VoDev                   = SAMPLE_VO_DEV_UHD;
    stVoConfig.enVoIntfType            = g_enAVSVoIntf;
    stVoConfig.enIntfSync              = g_enAVSIntfSync;
    stVoConfig.enPicSize               = enPicSize;
    stVoConfig.stDispRect.s32X         = 0;
    stVoConfig.stDispRect.s32Y         = 0;
    stVoConfig.stDispRect.u32Width     = 1920;
    stVoConfig.stDispRect.u32Height    = 1080;
    stVoConfig.stImageSize.u32Width    = 1920;
    stVoConfig.stImageSize.u32Height   = 1080;
    stVoConfig.enPixFormat             = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVoConfig.u32DisBufLen            = 3;
    stVoConfig.enDstDynamicRange       = DYNAMIC_RANGE_SDR10;
    stVoConfig.enVoMode                = VO_MODE_1MUX;
    stVoConfig.enVoPartMode            = VO_PART_MODE_SINGLE;
    stVoConfig.u32BgColor              = COLOR_RGB_BLUE;

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartVO failed with %d\n", s32Ret);
        goto exit1;
    }

    /******************************************
      step  6: start VENC
     ******************************************/
    s32Ret = SAMPLE_AVS_StartVENC(PIC_3840x2160);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_StartVENC failed with %d\n", s32Ret);
        goto exit2;
    }

    /******************************************
      step  7: start VPSS
     ******************************************/
    stVpssGrpAttr.u32MaxW                         = 3840;
    stVpssGrpAttr.u32MaxH                         = 2160;
    stVpssGrpAttr.enDynamicRange                  = DYNAMIC_RANGE_SDR10;
    stVpssGrpAttr.enPixelFormat                   = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate     = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate     = -1;

    astVpssChnAttr[0].u32Width                     = 3840;
    astVpssChnAttr[0].u32Height                    = 2160;
    astVpssChnAttr[0].enChnMode                    = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode               = COMPRESS_MODE_TILE;
    astVpssChnAttr[0].enDynamicRange               = DYNAMIC_RANGE_SDR10;
    astVpssChnAttr[0].enVideoFormat                = VIDEO_FORMAT_TILE_16x8;
    astVpssChnAttr[0].enPixelFormat                = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate  = -1;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate  = -1;
    astVpssChnAttr[0].u32Depth                     = 0;
    astVpssChnAttr[0].bMirror                      = HI_FALSE;
    astVpssChnAttr[0].bFlip                        = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode         = ASPECT_RATIO_NONE;
    astVpssChnAttr[0].stAspectRatio.u32BgColor     = 0xff;

    for (i = 0; i < u32PipeNum; i++)
    {
        VpssGrp = i;

        s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEn, &stVpssGrpAttr, astVpssChnAttr);

        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
            goto exit3;
        }
    }


    /******************************************
      step  8: start AVS
     ******************************************/

    stModParam.u32WorkingSetSize = 32 * 1024;
    s32Ret = HI_MPI_AVS_SetModParam(&stModParam);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_AVS_SetModParam fail for %#x!\n", s32Ret);
        goto exit3;
    }

    for (i = 0; i < u32PipeNum; i++)
    {
        snprintf(stAVSConfig.pLUTName[i], 64, "./data/4fisheye_ring_mesh_%d.bin", i);
    }

    stAVSConfig.u32OutW                          = u32OutW;
    stAVSConfig.u32OutH                          = u32OutH;
    stAVSConfig.enOutCmpMode                     = COMPRESS_MODE_NONE;
    stAVSConfig.benChn1                          = benChn1;

    pstGrpAttr                                   = &stAVSConfig.stGrpAttr;

    pstGrpAttr->enMode                           = AVS_MODE_BLEND;
    pstGrpAttr->u32PipeNum                       = u32PipeNum;
    pstGrpAttr->stGainAttr.enMode                = AVS_GAIN_MODE_MANUAL;
    pstGrpAttr->stGainAttr.s32Coef[0]            = 0x4000;
    pstGrpAttr->stGainAttr.s32Coef[1]            = 0x4000;
    pstGrpAttr->stGainAttr.s32Coef[2]            = 0x4000;
    pstGrpAttr->stGainAttr.s32Coef[3]            = 0x4000;

    pstGrpAttr->stOutAttr.enPrjMode              = AVS_PROJECTION_CUBE_MAP;

    pstGrpAttr->stOutAttr.stCenter.s32X          = u32OutW / 2;
    pstGrpAttr->stOutAttr.stCenter.s32Y          = u32OutH / 2;

    pstGrpAttr->stOutAttr.stFOV.u32FOVX          = 9000;
    pstGrpAttr->stOutAttr.stFOV.u32FOVY          = 9000;

    pstGrpAttr->stOutAttr.stORIRotation.s32Roll  = 0;
    pstGrpAttr->stOutAttr.stORIRotation.s32Pitch = 9000;
    pstGrpAttr->stOutAttr.stORIRotation.s32Yaw   = 0;
    pstGrpAttr->stOutAttr.stRotation.s32Roll     = 0;
    pstGrpAttr->stOutAttr.stRotation.s32Pitch    = 0;
    pstGrpAttr->stOutAttr.stRotation.s32Yaw      = 0;

    pstGrpAttr->stOutAttr.stCubeMapAttr.bBgColor             = HI_TRUE;
    pstGrpAttr->stOutAttr.stCubeMapAttr.u32BgColor           = 0xFFFFFF;
    pstGrpAttr->stOutAttr.stCubeMapAttr.u32SurfaceLength     = 720;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[0].s32X = 1200;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[0].s32Y = 720;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[1].s32X = 480;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[1].s32Y = 720;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[2].s32X = 2640;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[2].s32Y = 720;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[3].s32X = 1920;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[3].s32Y = 720;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[4].s32X = 1200;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[4].s32Y = 0;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[5].s32X = 1200;
    pstGrpAttr->stOutAttr.stCubeMapAttr.stStartPoint[5].s32Y = 1440;

    s32Ret = SAMPLE_AVS_StartAVS(AVSGrp, &stAVSConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_StartAVS failed with %d\n", s32Ret);
        goto exit4;
    }

    s32Ret = SAMPLE_AVS_Bind(u32PipeNum, benChn1);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_Bind failed with %d\n", s32Ret);
        goto exit5;
    }

    VencChn[0] = 0;

    /******************************************
    step   9: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(VencChn, s32ChnNum);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed with %d\n", s32Ret);
        goto exit5;
    }

    PAUSE();


    SAMPLE_COMM_VENC_StopGetStream();

    SAMPLE_AVS_UnBind(u32PipeNum);

exit5:
    SAMPLE_AVS_StopAVS(AVSGrp, benChn1);

exit4:

    for (i = 0; i < u32PipeNum; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEn);
    }

exit3:
    SAMPLE_COMM_VENC_Stop(VencChn[0]);

exit2:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

exit1:
    SAMPLE_COMM_VI_StopVi(&stViConfig);

exit:
    stStitchGrpAttr.bStitch = HI_FALSE;
    HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}


HI_S32 SAMPLE_AVS_4NoBlend(void)
{
    HI_U32 u32ViChnCnt = 4;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VB_CONFIG_S stVbConf;
    SAMPLE_VI_CONFIG_S stViConfig;
    VI_STITCH_GRP_ATTR_S stStitchGrpAttr;
    VI_STITCH_GRP StitchGrp = 0;
    PIC_SIZE_E enPicSize = PIC_3840x8640;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32PipeNum = 4;
    HI_U32 u32OutW, u32OutH;
    HI_S32 i;
    AVS_GRP AVSGrp = 0;
    HI_S32 s32ChnNum = 1;
    SAMPLE_AVS_CONFIG_S  stAVSConfig = {0};
    VENC_CHN             VencChn[1]   = {0};
    HI_BOOL              benChn1 = HI_FALSE;
    VPSS_GRP        VpssGrp = 0;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL         abChnEn[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};

    u32OutW = 7680;
    u32OutH = 4320;

    /************************************************
    step1:  get all sensors information
    *************************************************/
    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

    stViConfig.s32WorkingViNum                  = u32PipeNum;


    for (i = 0; i < u32PipeNum; i++)
    {
        stViConfig.astViInfo[i].stDevInfo.enWDRMode        = WDR_MODE_NONE;

        stViConfig.astViInfo[i].stChnInfo.enDynamicRange   = DYNAMIC_RANGE_SDR8;
        stViConfig.astViInfo[i].stChnInfo.enCompressMode   = COMPRESS_MODE_SEG;
        stViConfig.astViInfo[i].stChnInfo.enVideoFormat    = VIDEO_FORMAT_LINEAR;
        stViConfig.astViInfo[i].stChnInfo.enPixFormat      = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        stViConfig.astViInfo[i].stChnInfo.ViChn            = 0;

        stViConfig.astViInfo[i].stPipeInfo.enMastPipeMode  = VI_OFFLINE_VPSS_OFFLINE;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[1]        = -1;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[2]        = -1;
        stViConfig.astViInfo[i].stPipeInfo.aPipe[3]        = -1;
    }

    stViConfig.as32WorkingViId[0]                   = 0;
    stViConfig.astViInfo[0].stDevInfo.ViDev         = 0;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]     = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev       = 0;
    stViConfig.astViInfo[0].stSnsInfo.s32BusId      = 0;

    stViConfig.as32WorkingViId[1]                   = 1;
    stViConfig.astViInfo[1].stDevInfo.ViDev         = 2;
    stViConfig.astViInfo[1].stPipeInfo.aPipe[0]     = 1;
    stViConfig.astViInfo[1].stSnsInfo.MipiDev       = 2;
    stViConfig.astViInfo[1].stSnsInfo.s32BusId      = 1;

    stViConfig.as32WorkingViId[2]                   = 2;
    stViConfig.astViInfo[2].stDevInfo.ViDev         = 4;
    stViConfig.astViInfo[2].stPipeInfo.aPipe[0]     = 2;
    stViConfig.astViInfo[2].stSnsInfo.MipiDev       = 4;
    stViConfig.astViInfo[2].stSnsInfo.s32BusId      = 2;

    stViConfig.as32WorkingViId[3]                   = 3;
    stViConfig.astViInfo[3].stDevInfo.ViDev         = 6;
    stViConfig.astViInfo[3].stPipeInfo.aPipe[0]     = 3;
    stViConfig.astViInfo[3].stSnsInfo.MipiDev       = 6;
    stViConfig.astViInfo[3].stSnsInfo.s32BusId      = 3;



    /************************************************
    step2:  get  input size
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }


    /******************************************
      step  3: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                        DATA_BITWIDTH_10, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize = u32BlkSize ;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 4;

    u32BlkSize = COMMON_GetPicBufferSize(u32OutW, u32OutH, PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                        DATA_BITWIDTH_10, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt  = 6;

    u32BlkSize = VI_GetRawBufferSize(3840, 2700, PIXEL_FORMAT_RGB_BAYER_16BPP,
                                        COMPRESS_MODE_LINE, DEFAULT_ALIGN);
    stVbConf.astCommPool[2].u64BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt  = u32ViChnCnt * 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto exit;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        goto exit;
    }


    /******************************************
      step  4: start VI
     ******************************************/
    stStitchGrpAttr.bStitch = HI_FALSE;
    stStitchGrpAttr.enMode   = VI_STITCH_ISP_CFG_NORMAL;
    stStitchGrpAttr.u32PipeNum = 4;
    stStitchGrpAttr.PipeId[0] = 0;
    stStitchGrpAttr.PipeId[1] = 1;
    stStitchGrpAttr.PipeId[2] = 2;
    stStitchGrpAttr.PipeId[3] = 3;

    s32Ret = HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("disable stitch fail!\n");
        goto exit;
    }

    stStitchGrpAttr.bStitch = HI_TRUE;

    s32Ret = HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Set Stitch grp failed with %d\n", s32Ret);
        goto exit;
    }

    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartVi failed with %d\n", s32Ret);
        goto exit;
    }

    /******************************************
      step  5: start VPSS
     ******************************************/

    stVpssGrpAttr.u32MaxW                          = 3840;
    stVpssGrpAttr.u32MaxH                          = 2160;
    stVpssGrpAttr.enDynamicRange                   = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat                    = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate      = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate      = -1;

    astVpssChnAttr[0].u32Width                     = 3840;
    astVpssChnAttr[0].u32Height                    = 2160;
    astVpssChnAttr[0].enChnMode                    = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode               = COMPRESS_MODE_SEG;
    astVpssChnAttr[0].enDynamicRange               = DYNAMIC_RANGE_SDR8;
    astVpssChnAttr[0].enVideoFormat                = VIDEO_FORMAT_LINEAR;
    astVpssChnAttr[0].enPixelFormat                = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate  = -1;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate  = -1;
    astVpssChnAttr[0].u32Depth                     = 0;
    astVpssChnAttr[0].bMirror                      = HI_FALSE;
    astVpssChnAttr[0].bFlip                        = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode         = ASPECT_RATIO_NONE;
    astVpssChnAttr[0].stAspectRatio.u32BgColor     = 0xff;

    for (i = 0; i < 4; i++)
    {
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEn, &stVpssGrpAttr, astVpssChnAttr);

        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
            goto exit1;
        }
    }

    /******************************************
      step  6: start VENC
     ******************************************/

    s32Ret = SAMPLE_AVS_StartVENC(PIC_7680x4320);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_StartVENC failed with %d\n", s32Ret);
        goto exit2;
    }

    /******************************************
      step  7: start AVS
     ******************************************/
    stAVSConfig.stGrpAttr.u32PipeNum       = 4;
    stAVSConfig.stGrpAttr.enMode           = AVS_MODE_NOBLEND_QR;
    stAVSConfig.enOutCmpMode               = COMPRESS_MODE_SEG;
    stAVSConfig.benChn1                    = benChn1;
    s32Ret = SAMPLE_AVS_StartAVS(AVSGrp, &stAVSConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_StartAVS failed with %d\n", s32Ret);
        goto exit3;
    }


    /******************************************
    step   8: bind
    ******************************************/
    s32Ret = SAMPLE_AVS_Bind(u32PipeNum, benChn1);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_AVS_Bind failed with %d\n", s32Ret);
        goto exit4;
    }


    /******************************************
    step   9: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(VencChn, s32ChnNum);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed with %d\n", s32Ret);
        goto exit5;
    }

    PAUSE();

    SAMPLE_COMM_VENC_StopGetStream();

exit5:
    SAMPLE_AVS_UnBind(u32PipeNum);

exit4:
    SAMPLE_AVS_StopAVS(AVSGrp, benChn1);

exit3:
    SAMPLE_COMM_VENC_Stop(VencChn[0]);

exit2:
    for (i = 0; i < 4; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEn);
    }

exit1:
    SAMPLE_COMM_VI_StopVi(&stViConfig);

exit:
    stStitchGrpAttr.bStitch = HI_FALSE;
    HI_MPI_VI_SetStitchGrpAttr(StitchGrp, &stStitchGrpAttr);
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}

HI_S32 SAMPLE_AVS_8NonFisheye(void)
{
    printf("Not support 8 stitching yet!\n");
    return HI_SUCCESS;
}

HI_S32 SAMPLE_AVS_2Fisheye_Image_Stabilizing(void)
{
    printf("Not support image stabilizing yet!\n");
    return HI_SUCCESS;
}

#ifndef __HuaweiLite__

HI_S32 SAMPLE_AVS_GenerateLUT(void)
{
    AVS_LUT_MASK_S stLutMask;
    HI_U32 camera_num = 2;
    HI_U32 u32Width = 3000;
    HI_U32 u32Height = 3000;
    HI_CHAR OutLut[2][64];
    HI_U64  u64LutOutput[AVS_MAX_CAMERA_NUMBER];
    FILE* LutFILE;
    AVS_STATUS_E enRet;
    HI_VOID*       pMaskVirAddr = NULL;
    HI_VOID*       pLUTVirAddr= NULL;
    HI_CHAR cFile2[AVS_CALIBRATION_FILE_LENGTH];
    int cnt2 = 0;
    FILE *fp2;
    int c2;
    HI_U32 i;
    HI_S32 s32Ret = HI_SUCCESS;

    stLutMask.bInputYuvMask = HI_FALSE;
    stLutMask.bSameMask = HI_FALSE;
    stLutMask.stMaskDefine[0].enMaskShape = AVS_MASK_SHAPE_RECT;
    stLutMask.stMaskDefine[0].s32OffsetH = 0;
    stLutMask.stMaskDefine[0].s32OffsetV = 0;
    stLutMask.stMaskDefine[0].u32HalfMajorAxis = u32Width / 2;
    stLutMask.stMaskDefine[0].u32HalfMinorAxis = u32Height / 2;

    stLutMask.stMaskDefine[1].enMaskShape = AVS_MASK_SHAPE_ELLIPSE;
    stLutMask.stMaskDefine[1].s32OffsetH = 500;
    stLutMask.stMaskDefine[1].s32OffsetV = 500;
    stLutMask.stMaskDefine[1].u32HalfMajorAxis = u32Width / 2;
    stLutMask.stMaskDefine[1].u32HalfMinorAxis = u32Height / 2;

    s32Ret = HI_MPI_SYS_MmzAlloc(&(g_AVSMaskAddr), &(pMaskVirAddr), NULL, NULL, sizeof(HI_U16) * u32Width * u32Height);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("alloc mask buf fail with %#x!\n", s32Ret);
        goto exit;
    }

    stLutMask.u64MaskVirAddr[0] = (HI_U64)pMaskVirAddr;

    //open .pto file
   fp2 = fopen("./data/2fisheye_3000x3000.cal", "r");

    while (cnt2 < AVS_CALIBRATION_FILE_LENGTH)
    {
        c2 = fgetc(fp2);
        if (c2 == EOF) break;
        cFile2[cnt2] = c2;
        cnt2++;
    }

    fclose(fp2);

    s32Ret = HI_MPI_SYS_MmzAlloc(&(g_AVSLUTAddr), &(pLUTVirAddr), NULL, NULL, AVS_LUT_SIZE * camera_num);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("alloc lut buf fail with %#x!\n", s32Ret);
        goto exit1;
    }


    for (i = 0; i < camera_num; i++)
    {
        u64LutOutput[i] = (HI_U64)pLUTVirAddr + i * AVS_LUT_SIZE;
    }

    printf("Generating Lut, please wait...\n");

    enRet = HI_AVS_LutGenerate(AVS_TYPE_AVSP,
                              (HI_U64)cFile2,
                              &stLutMask,
                              2.0,
                              AVS_LUT_ACCURACY_HIGH,
                              u64LutOutput);

    if (AVS_STATUS_OK != enRet)
    {
        SAMPLE_PRT("Generate lut error!\n");
        goto exit2;
    }

    //save the lut file
    for (i = 0; i < camera_num; i++)
    {

        snprintf(OutLut[i], 64, "./2fisheye_3000x3000_mesh_%d.bin", i);

        LutFILE = fopen(OutLut[i], "wb");

        fwrite((HI_CHAR*)u64LutOutput[i], AVS_LUT_SIZE, 1, LutFILE);

        fclose(LutFILE);
    }

exit2:
    if (g_AVSLUTAddr)
    {
        s32Ret = HI_MPI_SYS_MmzFree(g_AVSLUTAddr, NULL);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("MMzFree fail!\n");
        }
        g_AVSLUTAddr = 0;
    }

exit1:
    if (g_AVSMaskAddr)
    {
        s32Ret = HI_MPI_SYS_MmzFree(g_AVSMaskAddr, NULL);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("MMzFree fail!\n");
        }
        g_AVSMaskAddr = 0;
    }

exit:
    return s32Ret;
}

#endif


/******************************************************************************
* function    : main()
* Description : video fisheye preview sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;

    if (argc < 2)
    {
        SAMPLE_AVS_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_AVS_HandleSig);
    signal(SIGTERM, SAMPLE_AVS_HandleSig);
#endif

    switch (*argv[1])
    {
        case '0':
            s32Ret = SAMPLE_AVS_2Fisheye_Equirectangular_Cylindrical_Rectilinear();
            break;

        case '1':
            s32Ret = SAMPLE_AVS_4Fisheye_CubeMap();
            break;

        case '2':
            s32Ret = SAMPLE_AVS_4NoBlend();
            break;

        case '3':
            s32Ret = SAMPLE_AVS_8NonFisheye();
            break;

        case '4':
            s32Ret = SAMPLE_AVS_2Fisheye_Image_Stabilizing();
            break;

#ifndef __HuaweiLite__
        case '5':
            s32Ret = SAMPLE_AVS_GenerateLUT();
            break;
#endif
        default:
            SAMPLE_PRT("the index is invaild!\n");
            SAMPLE_AVS_Usage(argv[0]);
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

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

