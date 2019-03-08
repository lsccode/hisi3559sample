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
#include <pthread.h>
#include <sys/prctl.h>
#include <math.h>

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_svp.h"
#include "sample_comm.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include "sample_nnie_main.h"
#include "sample_svp_nnie_software.h"
#include "sample_comm_ive.h"

#define M_EXTER_LOOP_NUMBER (10)
#define M_INTER_LOOP_NUMBER (3)

/*cnn para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stCnnModel = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stCnnNnieParam = {0};
static SAMPLE_SVP_NNIE_CNN_SOFTWARE_PARAM_S s_stCnnSoftwareParam = {0};
/*segment para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stSegnetModel = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stSegnetNnieParam = {0};
/*fasterrcnn para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stFasterRcnnModel = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stFasterRcnnNnieParam = {0};
static SAMPLE_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S s_stFasterRcnnSoftwareParam = {0};
static SAMPLE_SVP_NNIE_NET_TYPE_E s_enNetType;
/*rfcn para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stRfcnModel = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stRfcnNnieParam = {0};
static SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S s_stRfcnSoftwareParam = {0};
static SAMPLE_IVE_SWITCH_S s_stRfcnSwitch = {HI_FALSE,HI_FALSE};
static HI_BOOL s_bNnieStopSignal = HI_FALSE;
static pthread_t s_hNnieThread = 0;
static SAMPLE_VI_CONFIG_S s_stViConfig = {0};

/*ssd para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stSsdModel0 = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stSsdNnieParam0 = {0};
static SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S s_stSsdSoftwareParam0 = {0};

static SAMPLE_SVP_NNIE_MODEL_S s_stSsdModel1 = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stSsdNnieParam1 = {0};
static SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S s_stSsdSoftwareParam1 = {0};

pthread_mutex_t mtSSD = PTHREAD_MUTEX_INITIALIZER;
/*yolov1 para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stYolov1Model = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stYolov1NnieParam = {0};
static SAMPLE_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S s_stYolov1SoftwareParam = {0};
/*yolov2 para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stYolov2Model = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stYolov2NnieParam = {0};
static SAMPLE_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S s_stYolov2SoftwareParam = {0};
/*lstm para*/
static SAMPLE_SVP_NNIE_MODEL_S s_stLstmModel = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stLstmNnieParam = {0};

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_VENC_NNIE_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_VENC_StopGetStream();
        SAMPLE_COMM_VENC_StopSendQpmapFrame();
        SAMPLE_COMM_All_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

VENC_GOP_MODE_E SAMPLE_VENC_GetGopMode(void)
{
    char c;
    VENC_GOP_MODE_E enGopMode = 0;

Begin_Get:

    printf("please input choose gop mode!\n");
    printf("\t 0) NORMALP.\n");
    printf("\t 1) DUALP.\n");
    printf("\t 2) SMARTP.\n");
    printf("\t 3) ADVSMARTP\n");
    printf("\t 4) BIPREDB\n");

    while((c = getchar()) != '\n' && c != EOF)
        switch(c)
        {
            case '0':
                enGopMode = VENC_GOPMODE_NORMALP;
                break;
            case '1':
                enGopMode = VENC_GOPMODE_DUALP;
                break;
            case '2':
                enGopMode = VENC_GOPMODE_SMARTP;
                break;
            case '3':
                enGopMode = VENC_GOPMODE_ADVSMARTP;
                break;
            case '4':
                enGopMode = VENC_GOPMODE_BIPREDB;
                break;
            default:
                SAMPLE_PRT("input rcmode: %c, is invaild!\n",c);
                goto Begin_Get;
        }

    return enGopMode;
}

SAMPLE_RC_E SAMPLE_VENC_GetRcMode(void)
{
    char c;
    SAMPLE_RC_E  enRcMode = 0;

Begin_Get:

    printf("please input choose rc mode!\n");
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
    printf("\t a) avbr.\n");
    printf("\t f) fixQp\n");

    while((c = getchar()) != '\n' && c != EOF)
        switch(c)
        {
            case 'c':
                enRcMode = SAMPLE_RC_CBR;
                break;
            case 'v':
                enRcMode = SAMPLE_RC_VBR;
                break;
            case 'a':
                enRcMode = SAMPLE_RC_AVBR;
                break;
            case 'f':
                enRcMode = SAMPLE_RC_FIXQP;
                break;
            default:
                SAMPLE_PRT("input rcmode: %c, is invaild!\n",c);
                goto Begin_Get;
        }
    return enRcMode;
}

HI_S32 SAMPLE_VENC_SYS_Init(HI_U32 u32SupplementConfig,SAMPLE_SNS_TYPE_E  enSnsType)
{
    HI_S32 s32Ret;
    HI_U64 u64BlkSize;
    VB_CONFIG_S stVbConf;
    PIC_SIZE_E enSnsSize;
    SIZE_S     stSnsSize;

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(enSnsType, &enSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSnsSize, &stSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    u64BlkSize = COMMON_GetPicBufferSize(stSnsSize.u32Width, stSnsSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_10, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt    = 15;
#if 0
    u64BlkSize = COMMON_GetPicBufferSize(1920, 1080, PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_10, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt    = 15;

    stVbConf.astCommPool[2].u64BlkSize = 768*576*2;
    stVbConf.astCommPool[2].u32BlkCnt  = 1;
    stVbConf.u32MaxPoolCnt = 4;
#else
    stVbConf.astCommPool[1].u64BlkSize   = 768*576*2;
    stVbConf.astCommPool[1].u32BlkCnt    = 15;

    stVbConf.u32MaxPoolCnt = 4;
#endif

    if(0 == u32SupplementConfig)
    {
        s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    }
    else
    {
        s32Ret = SAMPLE_COMM_SYS_InitWithVbSupplement(&stVbConf,u32SupplementConfig);
    }
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_VOID SAMPLE_VENC_SetDCFInfo(VI_PIPE ViPipe)
{
    ISP_DCF_INFO_S stIspDCF;

    HI_MPI_ISP_GetDCFInfo(0, &stIspDCF);

    //description: Thumbnail test
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8ImageDescription,"Thumbnail test",DCF_DRSCRIPTION_LENGTH);
    //manufacturer: Hisilicon
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8Make,"Hisilicon",DCF_DRSCRIPTION_LENGTH);
    //model number: Hisilicon IP Camera
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8Model,"Hisilicon IP Camera",DCF_DRSCRIPTION_LENGTH);
    //firmware version: v.1.1.0
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8Software,"v.1.1.0",DCF_DRSCRIPTION_LENGTH);


    stIspDCF.stIspDCFConstInfo.u32FocalLength             = 0x00640001;
    stIspDCF.stIspDCFConstInfo.u8Contrast                 = 5;
    stIspDCF.stIspDCFConstInfo.u8CustomRendered           = 0;
    stIspDCF.stIspDCFConstInfo.u8FocalLengthIn35mmFilm    = 1;
    stIspDCF.stIspDCFConstInfo.u8GainControl              = 1;
    stIspDCF.stIspDCFConstInfo.u8LightSource              = 1;
    stIspDCF.stIspDCFConstInfo.u8MeteringMode             = 1;
    stIspDCF.stIspDCFConstInfo.u8Saturation               = 1;
    stIspDCF.stIspDCFConstInfo.u8SceneCaptureType         = 1;
    stIspDCF.stIspDCFConstInfo.u8SceneType                = 0;
    stIspDCF.stIspDCFConstInfo.u8Sharpness                = 5;
    stIspDCF.stIspDCFUpdateInfo.u32ISOSpeedRatings         = 500;
    stIspDCF.stIspDCFUpdateInfo.u32ExposureBiasValue       = 5;
    stIspDCF.stIspDCFUpdateInfo.u32ExposureTime            = 0x00010004;
    stIspDCF.stIspDCFUpdateInfo.u32FNumber                 = 0x0001000f;
    stIspDCF.stIspDCFUpdateInfo.u8WhiteBalance             = 1;
    stIspDCF.stIspDCFUpdateInfo.u8ExposureMode             = 0;
    stIspDCF.stIspDCFUpdateInfo.u8ExposureProgram          = 1;
    stIspDCF.stIspDCFUpdateInfo.u32MaxApertureValue        = 0x00010001;

    HI_MPI_ISP_SetDCFInfo(0, &stIspDCF);

    return;
}

HI_S32 SAMPLE_VENC_VI_Init( SAMPLE_VI_CONFIG_S *pstViConfig, HI_BOOL bLowDelay, HI_U32 u32SupplementConfig)
{
    HI_S32              s32Ret;
    SAMPLE_SNS_TYPE_E   enSnsType;
    ISP_CTRL_PARAM_S    stIspCtrlParam;
    HI_U32              u32FrameRate;


    enSnsType = pstViConfig->astViInfo[0].stSnsInfo.enSnsType;

    pstViConfig->as32WorkingViId[0]                           = 0;
    //pstViConfig->s32WorkingViNum                              = 1;

    pstViConfig->astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(pstViConfig->astViInfo[0].stSnsInfo.enSnsType, 0);
    pstViConfig->astViInfo[0].stSnsInfo.s32BusId           = 0;

    //pstViConfig->astViInfo[0].stDevInfo.ViDev              = ViDev0;
    pstViConfig->astViInfo[0].stDevInfo.enWDRMode          = WDR_MODE_NONE;

    if(HI_TRUE == bLowDelay)
    {
        pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode     = VI_ONLINE_VPSS_ONLINE;
        if(SONY_IMX277_SLVS_8M_120FPS_10BIT == enSnsType)
        {
            pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode = VI_PARALLEL_VPSS_PARALLEL;
        }
    }
    else
    {
        pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode     = VI_OFFLINE_VPSS_OFFLINE;
        if(SONY_IMX277_SLVS_8M_120FPS_10BIT == enSnsType)
        {
            pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode = VI_PARALLEL_VPSS_OFFLINE;
        }
    }

    //if(8k == enSnsType)
    //{
    //    pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode       = VI_PARALLEL_VPSS_OFFLINE;
    //}

    //pstViConfig->astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe0;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[1]          = -1;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    //pstViConfig->astViInfo[0].stChnInfo.ViChn              = ViChn;
    //pstViConfig->astViInfo[0].stChnInfo.enPixFormat        = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    //pstViConfig->astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    pstViConfig->astViInfo[0].stChnInfo.enVideoFormat      = VIDEO_FORMAT_LINEAR;
    pstViConfig->astViInfo[0].stChnInfo.enCompressMode     = COMPRESS_MODE_SEG;//COMPRESS_MODE_SEG;
    s32Ret = SAMPLE_VENC_SYS_Init(u32SupplementConfig,enSnsType);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Init SYS err for %#x!\n", s32Ret);
        return s32Ret;
    }

    SAMPLE_COMM_VI_GetFrameRateBySensor(enSnsType, &u32FrameRate);

    s32Ret = HI_MPI_ISP_GetCtrlParam(pstViConfig->astViInfo[0].stPipeInfo.aPipe[0], &stIspCtrlParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_GetCtrlParam failed with %d!\n", s32Ret);
        return s32Ret;
    }
    stIspCtrlParam.u32StatIntvl  = u32FrameRate/30;

    s32Ret = HI_MPI_ISP_SetCtrlParam(pstViConfig->astViInfo[0].stPipeInfo.aPipe[0], &stIspCtrlParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_SetCtrlParam failed with %d!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed with %d!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}
HI_S32 SAMPLE_VENC_VPSS_Init(VPSS_GRP VpssGrp, HI_BOOL* pabChnEnable, DYNAMIC_RANGE_E enDynamicRange,PIXEL_FORMAT_E enPixelFormat,SIZE_S stSize[],SAMPLE_SNS_TYPE_E enSnsType)
{
    HI_S32 i;
    HI_S32 s32Ret;
    PIC_SIZE_E      enSnsSize;
    SIZE_S          stSnsSize;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(enSnsType, &enSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSnsSize, &stSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }


    stVpssGrpAttr.enDynamicRange = enDynamicRange;
    stVpssGrpAttr.enPixelFormat  = enPixelFormat;
    stVpssGrpAttr.u32MaxW        = stSnsSize.u32Width;
    stVpssGrpAttr.u32MaxH        = stSnsSize.u32Height;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;


    for(i=0; i<VPSS_MAX_PHY_CHN_NUM; i++)
    {
        if(HI_TRUE == pabChnEnable[i])
        {
            stVpssChnAttr[i].u32Width                     = stSize[i].u32Width;
            stVpssChnAttr[i].u32Height                    = stSize[i].u32Height;
            stVpssChnAttr[i].enChnMode                    = VPSS_CHN_MODE_USER;
            stVpssChnAttr[i].enCompressMode               = COMPRESS_MODE_NONE;//COMPRESS_MODE_SEG;
            stVpssChnAttr[i].enDynamicRange               = enDynamicRange;
            stVpssChnAttr[i].enPixelFormat                = enPixelFormat;
            stVpssChnAttr[i].stFrameRate.s32SrcFrameRate  = -1;
            stVpssChnAttr[i].stFrameRate.s32DstFrameRate  = -1;
            stVpssChnAttr[i].u32Depth                     = 0;
            stVpssChnAttr[i].bMirror                      = HI_FALSE;
            stVpssChnAttr[i].bFlip                        = HI_FALSE;
            stVpssChnAttr[i].enVideoFormat                = VIDEO_FORMAT_LINEAR;
            stVpssChnAttr[i].stAspectRatio.enMode         = ASPECT_RATIO_NONE;
        }
    }

    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, pabChnEnable,&stVpssGrpAttr,stVpssChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
    }

    return s32Ret;
}


HI_S32 SAMPLE_VENC_CheckSensor(SAMPLE_SNS_TYPE_E   enSnsType,SIZE_S  stSize)
{
    HI_S32 s32Ret;
    SIZE_S          stSnsSize;
    PIC_SIZE_E      enSnsSize;

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(enSnsType, &enSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSnsSize, &stSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    if((stSnsSize.u32Width < stSize.u32Width) || (stSnsSize.u32Height < stSize.u32Height))
    {
        SAMPLE_PRT("Sensor size is (%d,%d), but encode chnl is (%d,%d) !\n",
                   stSnsSize.u32Width,stSnsSize.u32Height,stSize.u32Width,stSize.u32Height);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VENC_4K120(void *arg)
{
    HI_S32 i;
    HI_S32 s32Ret;
    SIZE_S          stSize[2];
    PIC_SIZE_E      enSize[2]     = {PIC_3840x2160, PIC_1080P};
    HI_S32          s32ChnNum     = 2;
    VENC_CHN        VencChn[2]    = {0,1};
    HI_U32          u32Profile[2] = {0,1};
    PAYLOAD_TYPE_E  enPayLoad[2]  = {PT_H265, PT_H264};
    VENC_GOP_MODE_E enGopMode;
    VENC_GOP_ATTR_S stGopAttr;
    SAMPLE_RC_E     enRcMode;

    VI_DEV          ViDev        = 0;
    VI_PIPE         ViPipe       = 0;
    VI_CHN          ViChn        = 0;
    SAMPLE_VI_CONFIG_S stViConfig;

    VPSS_GRP        VpssGrp        = 0;
    VPSS_CHN        VpssChn[2]     = {0,1};
    HI_BOOL         abChnEnable[4] = {1,1,0,0};

    HI_U32 u32SupplementConfig = HI_FALSE;

    for(i=0; i<s32ChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSize[i], &stSize[i]);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
            return s32Ret;
        }
    }

    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);

    if(SAMPLE_SNS_TYPE_BUTT == stViConfig.astViInfo[0].stSnsInfo.enSnsType)
    {
        SAMPLE_PRT("Not set SENSOR%d_TYPE !\n",0);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_VENC_CheckSensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType,stSize[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Check Sensor err!\n");
        return HI_FAILURE;
    }

    stViConfig.s32WorkingViNum       = 1;
    stViConfig.astViInfo[0].stDevInfo.ViDev     = ViDev;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0] = ViPipe;
    stViConfig.astViInfo[0].stChnInfo.ViChn     = ViChn;
    stViConfig.astViInfo[0].stChnInfo.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stViConfig.astViInfo[0].stChnInfo.enPixFormat    = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    s32Ret = SAMPLE_VENC_VI_Init(&stViConfig, HI_FALSE,u32SupplementConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("Init VI err for %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    SAMPLE_COMM_SVP_SetSysInit();
    s32Ret = SAMPLE_VENC_VPSS_Init(VpssGrp,abChnEnable,DYNAMIC_RANGE_SDR8,PIXEL_FORMAT_YVU_SEMIPLANAR_420,stSize,stViConfig.astViInfo[0].stSnsInfo.enSnsType);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Init VPSS err for %#x!\n", s32Ret);
        goto EXIT_VI_STOP;
    }

    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, VpssGrp);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("VI Bind VPSS err for %#x!\n", s32Ret);
        goto EXIT_VPSS_STOP;
    }

    /******************************************
     start stream venc
     ******************************************/

    enRcMode = SAMPLE_VENC_GetRcMode();

    enGopMode = SAMPLE_VENC_GetGopMode();
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(enGopMode,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Get GopAttr for %#x!\n", s32Ret);
        goto EXIT_VI_VPSS_UNBIND;
    }

    /***encode h.265 **/
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn[0], enPayLoad[0],enSize[0], enRcMode,u32Profile[0],&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
        goto EXIT_VI_VPSS_UNBIND;
    }

    s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp, VpssChn[0],VencChn[0]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Venc Get GopAttr failed for %#x!\n", s32Ret);
        goto EXIT_VENC_H265_STOP;
    }

    /***encode h.264 **/
    //s32Ret = SAMPLE_COMM_VENC_Start(VencChn[1], enPayLoad[1], enSize[1], enRcMode,u32Profile[1],&stGopAttr);
    //if (HI_SUCCESS != s32Ret)
    //{
    //    SAMPLE_PRT("Venc Start failed for %#x!\n", s32Ret);
    //    goto EXIT_VENC_H265_UnBind;
    //}

    //s32Ret = SAMPLE_COMM_VPSS_Bind_VENC(VpssGrp, VpssChn[1],VencChn[1]);
    //if (HI_SUCCESS != s32Ret)
    //{
    //    SAMPLE_PRT("Venc bind Vpss failed for %#x!\n", s32Ret);
    //    goto EXIT_VENC_H264_STOP;
    //}

    /******************************************
     stream save process
    ******************************************/
#if 0
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(VencChn,s32ChnNum);
#else
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(VencChn,1);
#endif
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto EXIT_VENC_H264_UnBind;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();

EXIT_VENC_H264_UnBind:
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp,VpssChn[1],VencChn[1]);
EXIT_VENC_H264_STOP:
    SAMPLE_COMM_VENC_Stop(VencChn[1]);
EXIT_VENC_H265_UnBind:
    SAMPLE_COMM_VPSS_UnBind_VENC(VpssGrp,VpssChn[0],VencChn[0]);
EXIT_VENC_H265_STOP:
    SAMPLE_COMM_VENC_Stop(VencChn[0]);
EXIT_VI_VPSS_UNBIND:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe,ViChn,VpssGrp);
EXIT_VPSS_STOP:
    SAMPLE_COMM_VPSS_Stop(VpssGrp,abChnEnable);
EXIT_VI_STOP:
    SAMPLE_COMM_VI_StopVi(&stViConfig);
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

HI_DOUBLE SAMPLE_SVP_GetDiff(struct timespec *pstTimeStart,struct timespec *pstTimeEnd)
{
    if (NULL == pstTimeStart || NULL == pstTimeEnd)
        return 1.0e-10;

    HI_U64 ullDiff = pstTimeEnd->tv_sec*1e3*1e3 +
                     pstTimeEnd->tv_nsec/1e3 -
                     pstTimeStart->tv_sec*1e3*1e3-
                     pstTimeStart->tv_nsec/1e3;

    HI_DOUBLE dbDiff = ullDiff*1.0/1e3;

    return dbDiff;
}

#define M_ACCURACY (1.0E-6)
void SAMPLE_SVP_PrintPerformance(HI_CHAR *pszNetName,
                                 HI_DOUBLE dbPerTotalTime,HI_DOUBLE dbPerFillSrcTime,
                                 HI_DOUBLE dbPerNnieTime,HI_DOUBLE dbPerCpuTime
                                )
{
    HI_DOUBLE dbPerTotalFps = 0.0;
    HI_DOUBLE dbPerFillSrcFps = 0.0;
    HI_DOUBLE dbPerNnieFps = 0.0;
    HI_DOUBLE dbPerCpuFps = 0.0;

    if(dbPerTotalTime > M_ACCURACY)
        dbPerTotalFps = 1000/dbPerTotalTime;

    if(dbPerFillSrcTime > M_ACCURACY)
        dbPerFillSrcFps = 1000/dbPerFillSrcTime;

    if(dbPerNnieTime > M_ACCURACY)
        dbPerNnieFps = 1000/dbPerNnieTime;

    if(dbPerCpuTime > M_ACCURACY)
        dbPerCpuFps = 1000/dbPerCpuTime;

    fprintf(stderr,"cnn net name      : %s       \n",pszNetName);
    fprintf(stderr,"total elapse time : %.2f ms  \n",dbPerTotalTime);
    fprintf(stderr,"total fps         : %.2f fps \n",dbPerTotalFps);
    fprintf(stderr,"fill elapse time  : %.2f ms  \n",dbPerFillSrcTime);
    fprintf(stderr,"fill fps          : %.2f fps \n",dbPerFillSrcFps);
    fprintf(stderr,"nnie elapse time  : %.2f ms  \n",dbPerNnieTime);
    fprintf(stderr,"nnie fps          : %.2f fps \n",dbPerNnieFps);
    fprintf(stderr,"cpu elapse time   : %.2f ms  \n",dbPerCpuTime);
    fprintf(stderr,"cpu fps           : %.2f fps \n",dbPerCpuFps);

    return ;
}

/******************************************************************************
* function : NNIE Forward
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                                      SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
                                      SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    HI_BOOL bFinish = HI_FALSE;
    SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    HI_U32 u32TotalStepNum = 0;

    SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
                               (HI_VOID *) pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr,
                               pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                                pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                                SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                                      HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                                      pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
                                     pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
                                     pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
                                     &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        fprintf(stderr,"line = %d,NNIE id = %d\n",__LINE__,pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId);
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
                                                hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                             "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }

    bFinish = HI_FALSE;
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
    {
        for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
        {
            u32TotalStepNum += *((HI_U8*)(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr)+j);
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                                   (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                                   u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

    }
    else
    {
        for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
        {
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                                       (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }

    return s32Ret;
}

/******************************************************************************
* function : NNIE ForwardWithBbox
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_ForwardWithBbox(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,SVP_SRC_BLOB_S astBbox[],
        SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bFinish = HI_FALSE;
    SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    HI_U32 u32TotalStepNum = 0;
    HI_U32 i, j;

    SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astForwardWithBboxCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
                               (HI_VOID *) pstNnieParam->astForwardWithBboxCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr,
                               pstNnieParam->astForwardWithBboxCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                                pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                                SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                                      HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                                      pstProcSegIdx->u32SegIdx,i);
        }
    }
    /*NNIE_ForwardWithBbox*/
    s32Ret = HI_MPI_SVP_NNIE_ForwardWithBbox(&hSvpNnieHandle,
             pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,astBbox,
             pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
             &pstNnieParam->astForwardWithBboxCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,HI_MPI_SVP_NNIE_ForwardWithBbox failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardWithBboxCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
                                                hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                             "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }

    bFinish = HI_FALSE;


    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *((HI_U8*)(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr)+j);
            }
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                                       (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                                       u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
        else
        {
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                                       (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                                       pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }

    return s32Ret;
}


/******************************************************************************
* function : Fill Src Data
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FillSrcData(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx)
{
    static int enter = 0;
    FILE* fp = NULL;
    HI_U32 i =0, j = 0, n = 0;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;
    HI_U32 u32VarSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8*pu8PicAddr = NULL;
    HI_U32*pu32StepAddr = NULL;
    HI_U32 u32SegIdx = pstInputDataIdx->u32SegIdx;
    HI_U32 u32NodeIdx = pstInputDataIdx->u32NodeIdx;
    HI_U32 u32TotalStepNum = 0;

    /*open file*/
    if (NULL != pstNnieCfg->pszPic)
    {
        fp = fopen(pstNnieCfg->pszPic,"rb");
        SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                  "Error, open file failed!\n");
    }

    /*get data size*/
    if(SVP_BLOB_TYPE_U8 <= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType &&
            SVP_BLOB_TYPE_YVU422SP >= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32VarSize = sizeof(HI_U8);
    }
    else
    {
        u32VarSize = sizeof(HI_U32);
    }

    /*fill src data*/
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32Dim = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u32Dim;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu32StepAddr = (HI_U32*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
        {
            for(i = 0;i < *(pu32StepAddr+n); i++)
            {
                s32Ret = fread(pu8PicAddr,u32Dim*u32VarSize,1,fp);
                SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                pu8PicAddr += u32Stride;
            }
            u32TotalStepNum += *(pu32StepAddr+n);
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
                                   (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
                                   u32TotalStepNum*u32Stride);
    }
    else
    {
        u32Height = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Height;
        u32Width = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Width;
        u32Chn = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Chn;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
        {
            fseek(fp,0,SEEK_SET);
            for(i = 0;i < u32Chn; i++)
            {

                for(j = 0; j < u32Height; j++)
                {
                    s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                    SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed,s32Ret = %d,%s!\n",s32Ret,strerror(errno));
                    pu8PicAddr += u32Stride;
                }
            }
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
                                   (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
                                   pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num*u32Chn*u32Height*u32Stride);
    }
    enter = 1;
    fclose(fp);
    return HI_SUCCESS;
FAIL:

    fclose(fp);
    return HI_FAILURE;
}

/******************************************************************************
* function : print report result
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_PrintReportResult(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
    HI_U32 u32SegNum = pstNnieParam->pstModel->u32NetSegNum;
    HI_U32 i = 0, j = 0, k = 0, n = 0;
    HI_U32 u32SegIdx = 0, u32NodeIdx = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR acReportFileName[SAMPLE_SVP_NNIE_REPORT_NAME_LENGTH] = {'\0'};
    FILE* fp = NULL;
    HI_U32*pu32StepAddr = NULL;
    HI_S32*ps32ResultAddr = NULL;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;

    for(u32SegIdx = 0; u32SegIdx < u32SegNum; u32SegIdx++)
    {
        for(u32NodeIdx = 0; u32NodeIdx < pstNnieParam->pstModel->astSeg[u32SegIdx].u16DstNum; u32NodeIdx++)
        {
            s32Ret = snprintf(acReportFileName,SAMPLE_SVP_NNIE_REPORT_NAME_LENGTH,
                              "seg%d_layer%d_output%d_inst.linear.hex",u32SegIdx,
                              pstNnieParam->pstModel->astSeg[u32SegIdx].astDstNode[u32NodeIdx].u32NodeId,0);
            SAMPLE_SVP_CHECK_EXPR_RET(s32Ret < 0,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                      "Error,create file name failed!\n");

            fp = fopen(acReportFileName,"w");
            SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                      "Error,open file failed!\n");

            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].enType)
            {
                u32Dim = pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].unShape.stSeq.u32Dim;
                u32Stride = pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].u32Stride;
                pu32StepAddr = (HI_U32*)(pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
                ps32ResultAddr = (HI_S32*)(pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].u64VirAddr);

                for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].u32Num; n++)
                {
                    for(i = 0;i < *(pu32StepAddr+n); i++)
                    {
                        for(j = 0; j < u32Dim; j++)
                        {
                            s32Ret = fprintf(fp ,"%08x\n",*(ps32ResultAddr+j));
                            SAMPLE_SVP_CHECK_EXPR_GOTO(s32Ret < 0,PRINT_FAIL,
                                                       SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,write report result file failed!\n");
                        }
                        ps32ResultAddr += u32Stride/sizeof(HI_U32);
                    }
                }
            }
            else
            {
                u32Height = pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].unShape.stWhc.u32Height;
                u32Width = pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].unShape.stWhc.u32Width;
                u32Chn = pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].unShape.stWhc.u32Chn;
                u32Stride = pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].u32Stride;
                ps32ResultAddr = (HI_S32*)(pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].u64VirAddr);
                for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astDst[u32NodeIdx].u32Num; n++)
                {
                    for(i = 0;i < u32Chn; i++)
                    {
                        for(j = 0; j < u32Height; j++)
                        {
                            for(k = 0; k < u32Width; k++)
                            {
                                s32Ret = fprintf(fp,"%08x\n",*(ps32ResultAddr+k));
                                SAMPLE_SVP_CHECK_EXPR_GOTO(s32Ret < 0,PRINT_FAIL,
                                                           SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,write report result file failed!\n");
                            }
                            ps32ResultAddr += u32Stride/sizeof(HI_U32);
                        }
                    }
                }
            }
            fclose(fp);
        }
    }
    return HI_SUCCESS;

PRINT_FAIL:
    fclose(fp);
    return HI_FAILURE;
}


/******************************************************************************
* function : Cnn software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Cnn_SoftwareDeinit(SAMPLE_SVP_NNIE_CNN_SOFTWARE_PARAM_S* pstCnnSoftWarePara)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstCnnSoftWarePara,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error, pstCnnSoftWarePara can't be NULL!\n");
    if(0!=pstCnnSoftWarePara->stGetTopN.u64PhyAddr && 0!=pstCnnSoftWarePara->stGetTopN.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstCnnSoftWarePara->stGetTopN.u64PhyAddr,
                            pstCnnSoftWarePara->stGetTopN.u64VirAddr);
        pstCnnSoftWarePara->stGetTopN.u64PhyAddr = 0;
        pstCnnSoftWarePara->stGetTopN.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : Cnn Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Cnn_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_CNN_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S* pstNnieModel)
{

    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware para deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software para deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Cnn_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_Cnn_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}

/******************************************************************************
* function : Cnn software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Cnn_SoftwareParaInit(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstCnnPara, SAMPLE_SVP_NNIE_CNN_SOFTWARE_PARAM_S* pstCnnSoftWarePara)
{
    HI_U32 u32GetTopNMemSize = 0;
    HI_U32 u32GetTopNAssistBufSize = 0;
    HI_U32 u32GetTopNPerFrameSize = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32ClassNum = pstCnnPara->pstModel->astSeg[0].astDstNode[0].unShape.stWhc.u32Width;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    /*get mem size*/
    u32GetTopNPerFrameSize = pstCnnSoftWarePara->u32TopN*sizeof(SAMPLE_SVP_NNIE_CNN_GETTOPN_UNIT_S);
    u32GetTopNMemSize = SAMPLE_SVP_NNIE_ALIGN16(u32GetTopNPerFrameSize)*pstNnieCfg->u32MaxInputNum;
    u32GetTopNAssistBufSize = u32ClassNum*sizeof(SAMPLE_SVP_NNIE_CNN_GETTOPN_UNIT_S);
    u32TotalSize = u32GetTopNMemSize+u32GetTopNAssistBufSize;

    /*malloc mem*/
    s32Ret = SAMPLE_COMM_SVP_MallocMem("SAMPLE_CNN_INIT",NULL,(HI_U64*)&u64PhyAddr,
                                       (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,Malloc memory failed!\n");
    memset(pu8VirAddr, 0, u32TotalSize);

    /*init GetTopn */
    pstCnnSoftWarePara->stGetTopN.u32Num= pstNnieCfg->u32MaxInputNum;
    pstCnnSoftWarePara->stGetTopN.unShape.stWhc.u32Chn = 1;
    pstCnnSoftWarePara->stGetTopN.unShape.stWhc.u32Height = 1;
    pstCnnSoftWarePara->stGetTopN.unShape.stWhc.u32Width = u32GetTopNPerFrameSize/sizeof(HI_U32);
    pstCnnSoftWarePara->stGetTopN.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32GetTopNPerFrameSize);
    pstCnnSoftWarePara->stGetTopN.u64PhyAddr = u64PhyAddr;
    pstCnnSoftWarePara->stGetTopN.u64VirAddr = (HI_U64)pu8VirAddr;

    /*init AssistBuf */
    pstCnnSoftWarePara->stAssistBuf.u32Size = u32GetTopNAssistBufSize;
    pstCnnSoftWarePara->stAssistBuf.u64PhyAddr = u64PhyAddr+u32GetTopNMemSize;
    pstCnnSoftWarePara->stAssistBuf.u64VirAddr = (HI_U64)pu8VirAddr+u32GetTopNMemSize;

    return s32Ret;
}

/******************************************************************************
* function : Cnn init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Cnn_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstCnnPara, SAMPLE_SVP_NNIE_CNN_SOFTWARE_PARAM_S* pstCnnSoftWarePara)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstNnieCfg,pstCnnPara);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    if(pstCnnSoftWarePara!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Cnn_SoftwareParaInit(pstNnieCfg,pstCnnPara,pstCnnSoftWarePara);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error(%#x),SAMPLE_SVP_NNIE_Cnn_SoftwareParaInit failed!\n",s32Ret);
    }

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Cnn_Deinit(pstCnnPara,pstCnnSoftWarePara,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_Cnn_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}

/******************************************************************************
* function : Cnn process
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Cnn_PrintResult(SVP_BLOB_S *pstGetTopN,HI_U32 u32TopN)
{
    HI_U32 i = 0, j = 0;
    HI_U32 *pu32Tmp = NULL;
    HI_U32 u32Stride = pstGetTopN->u32Stride;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstGetTopN,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,pstGetTopN can't be NULL!\n");
    for(j = 0; j < pstGetTopN->u32Num; j++)
    {
        SAMPLE_SVP_TRACE_INFO("==== The %dth image info====\n",j);
        pu32Tmp = (HI_U32*)(pstGetTopN->u64VirAddr + j * u32Stride);
        for(i = 0; i < u32TopN * 2; i+= 2)
        {
            SAMPLE_SVP_TRACE_INFO("%d:%d\n",pu32Tmp[i],pu32Tmp[i+1]);
        }
    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : show Cnn sample(image 28x28 U8_C1)
******************************************************************************/
void SAMPLE_SVP_NNIE_Cnn(void)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/y/0_28x28.y";
    HI_CHAR *pcModelName = "./data/nnie_model/classification/inst_mnist_cycle.wk";
    HI_U32 u32PicNum = 1;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core
    s_stCnnSoftwareParam.u32TopN = 5;

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*CNN Load model*/
    SAMPLE_SVP_TRACE_INFO("Cnn Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stCnnModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    /*CNN parameter initialization*/
    /*Cnn software parameters are set in SAMPLE_SVP_NNIE_Cnn_SoftwareParaInit,
     if user has changed net struct, please make sure the parameter settings in
     SAMPLE_SVP_NNIE_Cnn_SoftwareParaInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Cnn parameter initialization!\n");
    s_stCnnNnieParam.pstModel = &s_stCnnModel.stModel;
    s32Ret = SAMPLE_SVP_NNIE_Cnn_ParamInit(&stNnieCfg,&s_stCnnNnieParam,&s_stCnnSoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Cnn_ParamInit failed!\n");

    /*Fill src data*/
    SAMPLE_SVP_TRACE_INFO("Cnn start!\n");
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stCnnNnieParam,&stInputDataIdx);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");

    /*NNIE process(process the 0-th segment)*/
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stCnnNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

    /*Software process*/
    /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Cnn_GetTopN
     function's input datas are correct*/
    s32Ret = SAMPLE_SVP_NNIE_Cnn_GetTopN(&s_stCnnNnieParam,&s_stCnnSoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_CnnGetTopN failed!\n");

    /*Print result*/
    SAMPLE_SVP_TRACE_INFO("Cnn result:\n");
    s32Ret = SAMPLE_SVP_NNIE_Cnn_PrintResult(&(s_stCnnSoftwareParam.stGetTopN),
             s_stCnnSoftwareParam.u32TopN);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,CNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Cnn_PrintResult failed!\n");

CNN_FAIL_0:
    SAMPLE_SVP_NNIE_Cnn_Deinit(&s_stCnnNnieParam,&s_stCnnSoftwareParam,&s_stCnnModel);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Cnn sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Cnn_HandleSig(void)
{
    SAMPLE_SVP_NNIE_Cnn_Deinit(&s_stCnnNnieParam,&s_stCnnSoftwareParam,&s_stCnnModel);
    memset(&s_stCnnNnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stCnnSoftwareParam,0,sizeof(SAMPLE_SVP_NNIE_CNN_SOFTWARE_PARAM_S));
    memset(&s_stCnnModel,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}


/******************************************************************************
* function : show Segnet sample(image 224x224 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_Segnet(void)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/segnet_image_224x224.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/segmentation/inst_segnet_cycle.wk";
    HI_U32 u32PicNum = 1;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Segnet Load model*/
    SAMPLE_SVP_TRACE_INFO("Segnet Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stSegnetModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SEGNET_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    /*Segnet parameter initialization*/
    SAMPLE_SVP_TRACE_INFO("Segnet parameter initialization!\n");
    s_stSegnetNnieParam.pstModel = &s_stSegnetModel.stModel;
    s32Ret = SAMPLE_SVP_NNIE_Cnn_ParamInit(&stNnieCfg,&s_stSegnetNnieParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SEGNET_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Cnn_ParamInit failed!\n");

    /*Fill src data*/
    SAMPLE_SVP_TRACE_INFO("Segnet start!\n");

    HI_U32 index = 0;
    while(index++ < M_INTER_LOOP_NUMBER)
    {
        struct timespec stStartFillSrcTs;
        clock_gettime(CLOCK_MONOTONIC, &stStartFillSrcTs);
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stSegnetNnieParam,&stInputDataIdx);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SEGNET_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");

        struct timespec stStartForwardTs;
        struct timespec stEndForwardTs;
        /*NNIE process(process the 0-th segment)*/
        clock_gettime(CLOCK_MONOTONIC, &stStartForwardTs);
        stProcSegIdx.u32SegIdx = 0;
        s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stSegnetNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SEGNET_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
        clock_gettime(CLOCK_MONOTONIC, &stEndForwardTs);

        HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartFillSrcTs,&stEndForwardTs);
        HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartFillSrcTs,&stStartForwardTs);
        HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartForwardTs,&stEndForwardTs);
        HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(NULL,NULL);

        SAMPLE_SVP_PrintPerformance("Segnet",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
        fprintf(stderr,"\n");
        //struct timespec stEndTimeSpc;
        //clock_gettime(CLOCK_MONOTONIC, &stEndTimeSpc);
        //HI_U64 diff = stEndTimeSpc.tv_sec*1000*1000 +
        //              stEndTimeSpc.tv_nsec/1000 -
        //              stStartTimeSpc.tv_sec*1000*1000-
        //              stStartTimeSpc.tv_nsec/1000;
        //SAMPLE_SVP_TRACE_INFO("Segnet result,multi-core,start(%llu:%llu),end(%llu:%llu)\n",
        //                      stStartTimeSpc.tv_sec,stStartTimeSpc.tv_nsec,
        //                      stEndTimeSpc.tv_sec,stEndTimeSpc.tv_nsec);
        //SAMPLE_SVP_TRACE_INFO("Segnet print result,multi-core,pic resolution (224*224) bgr,"
        //                      "time difference(%llu us ~= %llu ms), %f fps\n",diff,diff/1000,1000*1000*1.0/diff);
    }

    /*print report result*/
    s32Ret = SAMPLE_SVP_NNIE_PrintReportResult(&s_stSegnetNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SEGNET_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_PrintReportResult failed!\n");

    SAMPLE_SVP_TRACE_INFO("Segnet is successfully processed!\n");

SEGNET_FAIL_0:
    SAMPLE_SVP_NNIE_Cnn_Deinit(&s_stSegnetNnieParam,NULL,&s_stSegnetModel);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Segnet sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Segnet_HandleSig(void)
{
    SAMPLE_SVP_NNIE_Cnn_Deinit(&s_stSegnetNnieParam,NULL,&s_stSegnetModel);
    memset(&s_stSegnetNnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stSegnetModel,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : print detection result
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Detection_PrintResult(SVP_BLOB_S *pstDstScore,
        SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, HI_FLOAT f32PrintResultThresh)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = (HI_S32*)pstDstScore->u64VirAddr;
    HI_S32* ps32Roi = (HI_S32*)pstDstRoi->u64VirAddr;
    HI_S32* ps32ClassRoiNum = (HI_S32*)pstClassRoiNum->u64VirAddr;
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

    u32RoiNumBias += ps32ClassRoiNum[0];
    for (i = 1; i < u32ClassNum; i++)
    {
        u32ScoreBias = u32RoiNumBias;
        u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;
        /*if the confidence score greater than result threshold, the result will be printed*/
        if((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >=
                f32PrintResultThresh && ps32ClassRoiNum[i]!=0)
        {
            SAMPLE_SVP_TRACE_INFO("==== The %dth class box info====\n", i);
        }
        for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
        {
            f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
            if (f32Score < f32PrintResultThresh)
            {
                break;
            }
            s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
            s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
            s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
            s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
            SAMPLE_SVP_TRACE_INFO("%d %d %d %d %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
        }
        u32RoiNumBias += ps32ClassRoiNum[i];
    }
    return HI_SUCCESS;
}


/******************************************************************************
* function : FasterRcnn software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FasterRcnn_SoftwareDeinit(SAMPLE_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stRpnTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stRpnTmpBuf.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stRpnTmpBuf.u64PhyAddr,
                            pstSoftWareParam->stRpnTmpBuf.u64VirAddr);
        pstSoftWareParam->stRpnTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stRpnTmpBuf.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : FasterRcnn Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FasterRcnn_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S* pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_FasterRcnn_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}


/******************************************************************************
* function : FasterRcnn software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FasterRcnn_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RpnTmpBufSize = 0;
    HI_U32 u32RpnBboxBufSize = 0;
    HI_U32 u32GetResultTmpBufSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    /*RPN parameter init*/
    pstSoftWareParam->u32MaxRoiNum = pstCfg->u32MaxRoiNum;
    if(SAMPLE_SVP_NNIE_VGG16_FASTER_RCNN == s_enNetType)
    {
        pstSoftWareParam->u32ClassNum = 4;
        pstSoftWareParam->u32NumRatioAnchors = 3;
        pstSoftWareParam->u32NumScaleAnchors = 3;
        pstSoftWareParam->au32Scales[0] = 8 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[1] = 16 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[2] = 32 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Ratios[0] = 0.5 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Ratios[1] = 1 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Ratios[2] = 2 * SAMPLE_SVP_QUANT_BASE;
    }
    else
    {
        pstSoftWareParam->u32ClassNum = 2;
        pstSoftWareParam->u32NumRatioAnchors = 1;
        pstSoftWareParam->u32NumScaleAnchors = 9;
        pstSoftWareParam->au32Scales[0] = 1.5 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[1] = 2.1 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[2] = 2.9 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[3] = 4.1 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[4] = 5.8 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[5] = 8.0 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[6] = 11.3 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[7] = 15.8 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Scales[8] = 22.1 * SAMPLE_SVP_QUANT_BASE;
        pstSoftWareParam->au32Ratios[0] = 2.44 * SAMPLE_SVP_QUANT_BASE;
    }

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height,
                      pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width,
                                        pstSoftWareParam->u32MinSize = 16;
    pstSoftWareParam->u32FilterThresh = 16;
    pstSoftWareParam->u32SpatialScale = (HI_U32)(0.0625 * SAMPLE_SVP_QUANT_BASE);
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.7 * SAMPLE_SVP_QUANT_BASE);
    pstSoftWareParam->u32FilterThresh = 0;
    pstSoftWareParam->u32NumBeforeNms = 6000;
    for(i = 0; i < pstSoftWareParam->u32ClassNum; i++)
    {
        pstSoftWareParam->au32ConfThresh[i] = 1;
    }
    pstSoftWareParam->u32ValidNmsThresh = (HI_U32)(0.3 * SAMPLE_SVP_QUANT_BASE);
    pstSoftWareParam->stRpnBbox.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Height = pstCfg->u32MaxRoiNum;
    pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Width = SAMPLE_SVP_COORDI_NUM;
    pstSoftWareParam->stRpnBbox.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(SAMPLE_SVP_COORDI_NUM*sizeof(HI_U32));
    pstSoftWareParam->stRpnBbox.u32Num = 1;
    for(i = 0; i < 2; i++)
    {
        for(j = 0; j < pstNnieParam->pstModel->astSeg[0].u16DstNum; j++)
        {
            if(0 == strncmp(pstNnieParam->pstModel->astSeg[0].astDstNode[j].szName,
                            pstSoftWareParam->apcRpnDataLayerName[i],
                            SVP_NNIE_NODE_NAME_LEN))
            {
                pstSoftWareParam->aps32Conv[i] =(HI_S32*)pstNnieParam->astSegData[0].astDst[j].u64VirAddr;
                pstSoftWareParam->au32ConvHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[j].unShape.stWhc.u32Height;
                pstSoftWareParam->au32ConvWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[j].unShape.stWhc.u32Width;
                pstSoftWareParam->au32ConvChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[j].unShape.stWhc.u32Chn;
                break;
            }
        }
        SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[0].u16DstNum),
                                  HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,failed to find report node %s!\n",
                                  pstSoftWareParam->apcRpnDataLayerName[i]);
        if(0 == i)
        {
            pstSoftWareParam->u32ConvStride = pstNnieParam->astSegData[0].astDst[j].u32Stride;
        }
    }

    /*calculate software mem size*/
    u32ClassNum = pstSoftWareParam->u32ClassNum;
    u32RpnTmpBufSize = SAMPLE_SVP_NNIE_RpnTmpBufSize(pstSoftWareParam->u32NumRatioAnchors,
                       pstSoftWareParam->u32NumScaleAnchors,pstSoftWareParam->au32ConvHeight[0],
                       pstSoftWareParam->au32ConvWidth[0]);
    u32RpnTmpBufSize = SAMPLE_SVP_NNIE_ALIGN16(u32RpnTmpBufSize);
    u32RpnBboxBufSize = pstSoftWareParam->stRpnBbox.u32Num*
                        pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Height*pstSoftWareParam->stRpnBbox.u32Stride;
    u32GetResultTmpBufSize = SAMPLE_SVP_NNIE_FasterRcnn_GetResultTmpBufSize(pstCfg->u32MaxRoiNum,u32ClassNum);
    u32GetResultTmpBufSize = SAMPLE_SVP_NNIE_ALIGN16(u32GetResultTmpBufSize);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstCfg->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstCfg->u32MaxRoiNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32RpnTmpBufSize + u32RpnBboxBufSize + u32GetResultTmpBufSize + u32DstRoiSize +
                   u32DstScoreSize + u32ClassRoiNumSize;

    /*malloc mem*/
    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_RCNN_INIT",NULL,(HI_U64*)&u64PhyAddr,
                                          (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

    /*set addr*/
    pstSoftWareParam->stRpnTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stRpnTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);
    pstSoftWareParam->stRpnTmpBuf.u32Size = u32RpnTmpBufSize;

    pstSoftWareParam->stRpnBbox.u64PhyAddr = u64PhyAddr+u32RpnTmpBufSize;
    pstSoftWareParam->stRpnBbox.u64VirAddr = (HI_U64)(pu8VirAddr)+u32RpnTmpBufSize;

    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr+u32RpnBboxBufSize+u32RpnTmpBufSize;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnBboxBufSize+u32RpnTmpBufSize);
    pstSoftWareParam->stGetResultTmpBuf.u32Size = u32GetResultTmpBufSize;

    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32RpnBboxBufSize+u32RpnTmpBufSize+u32GetResultTmpBufSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnBboxBufSize+u32RpnTmpBufSize+u32GetResultTmpBufSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum*SAMPLE_SVP_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32RpnBboxBufSize+u32RpnTmpBufSize+u32GetResultTmpBufSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnBboxBufSize+u32RpnTmpBufSize+u32GetResultTmpBufSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32RpnBboxBufSize+u32RpnTmpBufSize+u32GetResultTmpBufSize+u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnBboxBufSize+u32RpnTmpBufSize+u32GetResultTmpBufSize+u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return s32Ret;
}

/******************************************************************************
* function : FasterRcnn parameter initialization
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FasterRcnn_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstFasterRcnnCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware parameter*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstFasterRcnnCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software parameter*/
    s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_SoftwareInit(pstFasterRcnnCfg,pstNnieParam,
             pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_SVP_NNIE_FasterRcnn_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_FasterRcnn_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}

/******************************************************************************
* function : show fasterRcnn sample(image 1240x375 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_FasterRcnn(void)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/single_person_1240x375.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_alexnet_frcnn_cycle.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    s_enNetType = SAMPLE_SVP_NNIE_ALEXNET_FASTER_RCNN;
    f32PrintResultThresh = 0.8f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 300;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0; //set NNIE core for 0-th Seg
    stNnieCfg.aenNnieCoreId[1] = SVP_NNIE_ID_0; //set NNIE core for 1-th Seg

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*FasterRcnn Load model*/
    SAMPLE_SVP_TRACE_INFO("FasterRcnn Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stFasterRcnnModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");
    printstModel(&s_stFasterRcnnModel);
    /*FasterRcnn para init*/
    /*apcRpnDataLayerName is used to set RPN data layer name
      and search RPN input data,if user has changed network struct, please
      make sure the data layer names are correct*/
    /*FasterRcnn parameters are set in SAMPLE_SVP_NNIE_FasterRcnn_SoftwareInit,
     if user has changed network struct, please make sure the parameter settings in
     SAMPLE_SVP_NNIE_FasterRcnn_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("FasterRcnn parameter initialization!\n");
    s_stFasterRcnnNnieParam.pstModel = &s_stFasterRcnnModel.stModel;
    s_stFasterRcnnSoftwareParam.apcRpnDataLayerName[0] = "rpn_cls_score";
    s_stFasterRcnnSoftwareParam.apcRpnDataLayerName[1] = "rpn_bbox_pred";
    s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_ParamInit(&stNnieCfg,&s_stFasterRcnnNnieParam,
             &s_stFasterRcnnSoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_FasterRcnn_ParamInit failed!\n");
    printstNnieCfg(&stNnieCfg);
    printstNnieParam(&s_stFasterRcnnNnieParam,s_stFasterRcnnModel.stModel.u32NetSegNum);
    /*Fill 0-th input node of 0-th seg*/
    SAMPLE_SVP_TRACE_INFO("FasterRcnn start!\n");
    HI_U32 index = 0;
    while(index++ < M_INTER_LOOP_NUMBER)
    {
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        struct timespec stStartTimeSpc;
        clock_gettime(CLOCK_MONOTONIC, &stStartTimeSpc);
        s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stFasterRcnnNnieParam,&stInputDataIdx);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");

        struct timespec stStartForwardTs;
        clock_gettime(CLOCK_MONOTONIC, &stStartForwardTs);
        /*NNIE process 0-th seg*/
        stProcSegIdx.u32SegIdx = 0;
        s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stFasterRcnnNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
        /*RPN*/
        struct timespec stStartRpnTs;
        clock_gettime(CLOCK_MONOTONIC, &stStartRpnTs);
        s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_Rpn(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FasterRcnn_Rpn failed!\n");
        /*NNIE process 1-th seg, the input conv data comes from 0-th seg's 0-th report node,
         the input roi comes from RPN results*/
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        stProcSegIdx.u32SegIdx = 1;

        struct timespec stStartForwardBboxTs;
        clock_gettime(CLOCK_MONOTONIC, &stStartForwardBboxTs);
        s32Ret = SAMPLE_SVP_NNIE_ForwardWithBbox(&s_stFasterRcnnNnieParam,&stInputDataIdx,
                 &s_stFasterRcnnSoftwareParam.stRpnBbox,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
        struct timespec stStartGetResBboxTs;
        clock_gettime(CLOCK_MONOTONIC, &stStartGetResBboxTs);
        /*GetResult*/
        /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_FasterRcnn_GetResult
         function's input datas are correct*/
        s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_GetResult(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FasterRcnn_GetResult failed!\n");

        /*print result, Alexnet_FasterRcnn has 2 classes:
         class 0:background     class 1:pedestrian */
        struct timespec stEndTimeSpc;
        clock_gettime(CLOCK_MONOTONIC, &stEndTimeSpc);

        HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartTimeSpc,&stEndTimeSpc);
        HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartTimeSpc,&stStartForwardTs);
        HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartForwardTs,&stStartRpnTs);
        HI_DOUBLE dbRpnDiff = SAMPLE_SVP_GetDiff(&stStartRpnTs,&stStartForwardBboxTs);
        HI_DOUBLE dbForwardBboxDiff = SAMPLE_SVP_GetDiff(&stStartForwardBboxTs,&stStartGetResBboxTs);
        HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartGetResBboxTs,&stEndTimeSpc);

        SAMPLE_SVP_PrintPerformance("FasterRcnn",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
        fprintf(stderr,"cpu rpn time      : %.2f ms  \n",dbRpnDiff);
        fprintf(stderr,"cpu rpn fps       : %.2f fps \n",1000/dbRpnDiff);
        fprintf(stderr,"nnie bbox time    : %.2f ms  \n",dbForwardBboxDiff);
        fprintf(stderr,"nnie bbox fps     : %.2f fps \n",1000/dbForwardBboxDiff);
        fprintf(stderr,"\n");
    }

    SAMPLE_SVP_TRACE_INFO("FasterRcnn result:\n");
    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stFasterRcnnSoftwareParam.stDstScore,
            &s_stFasterRcnnSoftwareParam.stDstRoi, &s_stFasterRcnnSoftwareParam.stClassRoiNum,
            f32PrintResultThresh);

FRCNN_FAIL_0:
    SAMPLE_SVP_NNIE_FasterRcnn_Deinit(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam,
                                      &s_stFasterRcnnModel);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function :show fasterrcnn double_roipooling sample(image 224x224 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_FasterRcnn_DoubleRoiPooling(void)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/double_roipooling_224_224.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_fasterrcnn_double_roipooling_cycle.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    s_enNetType = SAMPLE_SVP_NNIE_VGG16_FASTER_RCNN;
    f32PrintResultThresh = 0.8f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 300;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0; //set NNIE core for 0-th Seg
    stNnieCfg.aenNnieCoreId[1] = SVP_NNIE_ID_0; //set NNIE core for 1-th Seg

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*FasterRcnn Load model*/
    SAMPLE_SVP_TRACE_INFO("FasterRcnn Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stFasterRcnnModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");
    printstModel(&s_stFasterRcnnModel);
    /*FasterRcnn para init*/
    /*apcRpnDataLayerName is used to set RPN data layer name
      and search RPN input data,if user has changed network struct, please
      make sure the data layer names are correct*/
    /*FasterRcnn parameters are set in SAMPLE_SVP_NNIE_FasterRcnn_SoftwareInit,
     if user has changed network struct, please make sure the parameter settings in
     SAMPLE_SVP_NNIE_FaasterRcnn_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("FasterRcnn parameter initialization!\n");
    s_stFasterRcnnNnieParam.pstModel = &s_stFasterRcnnModel.stModel;
    s_stFasterRcnnSoftwareParam.apcRpnDataLayerName[0] = "rpn_cls_score";
    s_stFasterRcnnSoftwareParam.apcRpnDataLayerName[1] = "rpn_bbox_pred";
    s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_ParamInit(&stNnieCfg,&s_stFasterRcnnNnieParam,
             &s_stFasterRcnnSoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_FasterRcnn_ParamInit failed!\n");

    /*Fill 0-th input node of 0-th seg*/
    SAMPLE_SVP_TRACE_INFO("FasterRcnn start!\n");
    HI_U32 index = 0;
    while(index++ < M_INTER_LOOP_NUMBER)
    {
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        struct timespec stStartSpcTime;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
        s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stFasterRcnnNnieParam,&stInputDataIdx);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
        struct timespec stStartSpcForward;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);
        /*NNIE process 0-th seg*/
        stProcSegIdx.u32SegIdx = 0;
        s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stFasterRcnnNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
        struct timespec stStartSpcRpn;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcRpn);
        /*RPN*/
        s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_Rpn(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FasterRcnn_Rpn failed!\n");

        /*NNIE process 1-st seg, the input conv data comes from 0-th seg's 0-th and
          1-st report node,the input roi comes from RPN results*/
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        stProcSegIdx.u32SegIdx = 1;
        struct timespec stStartSpcForwardBbox;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcForwardBbox);
        s32Ret = SAMPLE_SVP_NNIE_ForwardWithBbox(&s_stFasterRcnnNnieParam,&stInputDataIdx,
                 &s_stFasterRcnnSoftwareParam.stRpnBbox,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

        /*GetResult*/
        /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_FasterRcnn_GetResult
         function's input datas are correct*/
        struct timespec stStartSpcGetRes;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);

        s32Ret = SAMPLE_SVP_NNIE_FasterRcnn_GetResult(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FRCNN_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FasterRcnn_GetResult failed!\n");
        struct timespec stEndSpcTime;
        clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);

        HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
        HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
        HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcRpn);
        HI_DOUBLE dbRpnDiff = SAMPLE_SVP_GetDiff(&stStartSpcRpn,&stStartSpcForwardBbox);
        HI_DOUBLE dbForwardBboxDiff = SAMPLE_SVP_GetDiff(&stStartSpcForwardBbox,&stStartSpcGetRes);
        HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);

        SAMPLE_SVP_PrintPerformance("FasterRcnn Double RoiPooling",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
        fprintf(stderr,"cpu rpn time      : %.2f ms  \n",dbRpnDiff);
        fprintf(stderr,"cpu rpn fps       : %.2f fps \n",1000/dbRpnDiff);
        fprintf(stderr,"nnie bbox time    : %.2f ms  \n",dbForwardBboxDiff);
        fprintf(stderr,"nnie bbox fps     : %.2f fps \n",1000/dbForwardBboxDiff);
        fprintf(stderr,"\n");
    }


    /*print result, FasterRcnn has 4 classes:
     class 0:background  class 1:person  class 2:people  class 3:person sitting */
    SAMPLE_SVP_TRACE_INFO("FasterRcnn result:\n");
    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stFasterRcnnSoftwareParam.stDstScore,
            &s_stFasterRcnnSoftwareParam.stDstRoi, &s_stFasterRcnnSoftwareParam.stClassRoiNum,
            f32PrintResultThresh);
FRCNN_FAIL_0:
    SAMPLE_SVP_NNIE_FasterRcnn_Deinit(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam,
                                      &s_stFasterRcnnModel);
    SAMPLE_COMM_SVP_CheckSysExit();
}


/******************************************************************************
* function : fasterRcnn sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_FasterRcnn_HandleSig(void)
{
    SAMPLE_SVP_NNIE_FasterRcnn_Deinit(&s_stFasterRcnnNnieParam,&s_stFasterRcnnSoftwareParam,
                                      &s_stFasterRcnnModel);
    memset(&s_stFasterRcnnNnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stFasterRcnnSoftwareParam,0,sizeof(SAMPLE_SVP_NNIE_FASTERRCNN_SOFTWARE_PARAM_S));
    memset(&s_stFasterRcnnModel,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Rfcn software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Rfcn_SoftwareDeinit(SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stRpnTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stRpnTmpBuf.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stRpnTmpBuf.u64PhyAddr,
                            pstSoftWareParam->stRpnTmpBuf.u64VirAddr);
        pstSoftWareParam->stRpnTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stRpnTmpBuf.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : Rfcn Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Rfcn_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Rfcn_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_Rfcn_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}

/******************************************************************************
* function : Rfcn software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Rfcn_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RpnTmpBufSize = 0;
    HI_U32 u32RpnBboxBufSize = 0;
    HI_U32 u32GetResultTmpBufSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    /*init Rpn para*/
    pstSoftWareParam->u32MaxRoiNum = pstCfg->u32MaxRoiNum;
    pstSoftWareParam->u32ClassNum = 21;
    pstSoftWareParam->u32NumRatioAnchors = 3;
    pstSoftWareParam->u32NumScaleAnchors = 3;
    pstSoftWareParam->au32Scales[0] = 8 * SAMPLE_SVP_NNIE_QUANT_BASE;
    pstSoftWareParam->au32Scales[1] = 16 * SAMPLE_SVP_NNIE_QUANT_BASE;
    pstSoftWareParam->au32Scales[2] = 32 * SAMPLE_SVP_NNIE_QUANT_BASE;
    pstSoftWareParam->au32Ratios[0] = 0.5 * SAMPLE_SVP_NNIE_QUANT_BASE;
    pstSoftWareParam->au32Ratios[1] = 1 * SAMPLE_SVP_NNIE_QUANT_BASE;
    pstSoftWareParam->au32Ratios[2] = 2 * SAMPLE_SVP_NNIE_QUANT_BASE;
    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height,
                      pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width,
                                        pstSoftWareParam->u32MinSize = 16;
    pstSoftWareParam->u32FilterThresh = 0;
    pstSoftWareParam->u32SpatialScale = (HI_U32)(0.0625 * SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.7 * SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32FilterThresh = 0;
    pstSoftWareParam->u32NumBeforeNms = 6000;
    for(i = 0; i < pstSoftWareParam->u32ClassNum; i++)
    {
        pstSoftWareParam->au32ConfThresh[i] = 1;
        pstSoftWareParam->af32ScoreThr[i] = 0.8f;
    }
    pstSoftWareParam->u32ValidNmsThresh = (HI_U32)(0.3 * 4096);

    /*set rpn input data info, the input info is set according to RPN data layers' name*/
    for(i = 0; i < 2; i++)
    {
        for(j = 0; j < pstNnieParam->pstModel->astSeg[0].u16DstNum; j++)
        {
            if(0 == strncmp(pstNnieParam->pstModel->astSeg[0].astDstNode[j].szName,
                            pstSoftWareParam->apcRpnDataLayerName[i],
                            SVP_NNIE_NODE_NAME_LEN))
            {
                pstSoftWareParam->aps32Conv[i] =(HI_S32*)pstNnieParam->astSegData[0].astDst[j].u64VirAddr;
                pstSoftWareParam->au32ConvHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[j].unShape.stWhc.u32Height;
                pstSoftWareParam->au32ConvWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[j].unShape.stWhc.u32Width;
                pstSoftWareParam->au32ConvChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[j].unShape.stWhc.u32Chn;
                break;
            }
        }
        SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[0].u16DstNum),
                                  HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,failed to find report node %s!\n",
                                  pstSoftWareParam->apcRpnDataLayerName[i]);
        if(0 == i)
        {
            pstSoftWareParam->u32ConvStride = pstNnieParam->astSegData[0].astDst[j].u32Stride;
        }
    }

    pstSoftWareParam->stRpnBbox.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Height = pstCfg->u32MaxRoiNum;
    pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Width = SAMPLE_SVP_COORDI_NUM;
    pstSoftWareParam->stRpnBbox.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(SAMPLE_SVP_COORDI_NUM*sizeof(HI_U32));
    pstSoftWareParam->stRpnBbox.u32Num = 1;

    /*malloc software mem*/
    u32RpnTmpBufSize = SAMPLE_SVP_NNIE_RpnTmpBufSize(pstSoftWareParam->u32NumRatioAnchors,
                       pstSoftWareParam->u32NumScaleAnchors,pstSoftWareParam->au32ConvHeight[0],
                       pstSoftWareParam->au32ConvWidth[0]);
    u32RpnTmpBufSize = SAMPLE_SVP_NNIE_ALIGN16(u32RpnTmpBufSize);
    u32RpnBboxBufSize = pstSoftWareParam->stRpnBbox.u32Num*
                        pstSoftWareParam->stRpnBbox.unShape.stWhc.u32Height*pstSoftWareParam->stRpnBbox.u32Stride;
    u32GetResultTmpBufSize = SAMPLE_SVP_NNIE_Rfcn_GetResultTmpBuf(pstCfg->u32MaxRoiNum,pstSoftWareParam->u32ClassNum);
    u32GetResultTmpBufSize = SAMPLE_SVP_NNIE_ALIGN16(u32GetResultTmpBufSize);
    u32ClassNum = pstSoftWareParam->u32ClassNum;
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstCfg->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstCfg->u32MaxRoiNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32RpnTmpBufSize + u32RpnBboxBufSize + u32GetResultTmpBufSize + u32DstRoiSize +
                   u32DstScoreSize + u32ClassRoiNumSize;

    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_RFCN_INIT",NULL,(HI_U64*)&u64PhyAddr,
                                          (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

    pstSoftWareParam->stRpnTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stRpnTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);
    pstSoftWareParam->stRpnTmpBuf.u32Size = u32RpnTmpBufSize;

    pstSoftWareParam->stRpnBbox.u64PhyAddr = u64PhyAddr+u32RpnTmpBufSize;
    pstSoftWareParam->stRpnBbox.u64VirAddr = (HI_U64)(pu8VirAddr)+u32RpnTmpBufSize;

    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr+u32RpnTmpBufSize+u32RpnBboxBufSize;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnTmpBufSize+u32RpnBboxBufSize);
    pstSoftWareParam->stGetResultTmpBuf.u32Size = u32GetResultTmpBufSize;

    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32RpnTmpBufSize+u32RpnBboxBufSize+u32GetResultTmpBufSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnTmpBufSize+u32RpnBboxBufSize+u32GetResultTmpBufSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32RpnTmpBufSize+u32RpnBboxBufSize+u32GetResultTmpBufSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnTmpBufSize+u32RpnBboxBufSize+u32GetResultTmpBufSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32RpnTmpBufSize+u32RpnBboxBufSize+u32GetResultTmpBufSize+u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32RpnTmpBufSize+u32RpnBboxBufSize+u32GetResultTmpBufSize+u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;
    return s32Ret;
}

/******************************************************************************
* function : Rfcn init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Rfcn_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Rfcn_SoftwareInit(pstCfg,pstNnieParam,pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_SVP_NNIE_Rfcn_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Rfcn_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_Rfcn_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}

/******************************************************************************
* function : roi to rect
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_RoiToRect(SVP_BLOB_S *pstDstScore,
                                 SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, HI_FLOAT *paf32ScoreThr,
                                 HI_BOOL bRmBg,SAMPLE_SVP_NNIE_RECT_ARRAY_S *pstRect,
                                 HI_U32 u32SrcWidth, HI_U32 u32SrcHeight,HI_U32 u32DstWidth,HI_U32 u32DstHeight)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = (HI_S32*)pstDstScore->u64VirAddr;
    HI_S32* ps32Roi = (HI_S32*)pstDstRoi->u64VirAddr;
    HI_S32* ps32ClassRoiNum = (HI_S32*)pstClassRoiNum->u64VirAddr;
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    HI_U32 u32RoiNumTmp = 0;

    SAMPLE_SVP_CHECK_EXPR_RET(u32ClassNum > SAMPLE_SVP_NNIE_MAX_CLASS_NUM ,HI_ERR_SVP_NNIE_ILLEGAL_PARAM,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),u32ClassNum(%u) must be less than or equal %u to!\n",HI_ERR_SVP_NNIE_ILLEGAL_PARAM,u32ClassNum, SAMPLE_SVP_NNIE_MAX_CLASS_NUM);
    pstRect->u32TotalNum = 0;
    pstRect->u32ClsNum = u32ClassNum;
    if (bRmBg)
    {
        pstRect->au32RoiNum[0] = 0;
        u32RoiNumBias += ps32ClassRoiNum[0];
        for (i = 1; i < u32ClassNum; i++)
        {
            u32ScoreBias = u32RoiNumBias;
            u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;
            u32RoiNumTmp = 0;
            /*if the confidence score greater than result thresh, the result will be drawed*/
            if(((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >=
                    paf32ScoreThr[i])  &&  (ps32ClassRoiNum[i] != 0))
            {
                for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
                {
                    /*Score is descend order*/
                    f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
                    if ((f32Score < paf32ScoreThr[i]) || (u32RoiNumTmp >= SAMPLE_SVP_NNIE_MAX_ROI_NUM_OF_CLASS))
                    {
                        break;
                    }

                    pstRect->astRect[i][u32RoiNumTmp].astPoint[0].s32X = (HI_U32)((HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM] / (HI_FLOAT)u32SrcWidth * (HI_FLOAT)u32DstWidth) & (~1) ;
                    pstRect->astRect[i][u32RoiNumTmp].astPoint[0].s32Y = (HI_U32)((HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1] / (HI_FLOAT)u32SrcHeight * (HI_FLOAT)u32DstHeight) & (~1);

                    pstRect->astRect[i][u32RoiNumTmp].astPoint[1].s32X = (HI_U32)((HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2]/ (HI_FLOAT)u32SrcWidth * (HI_FLOAT)u32DstWidth) & (~1);
                    pstRect->astRect[i][u32RoiNumTmp].astPoint[1].s32Y = pstRect->astRect[i][u32RoiNumTmp].astPoint[0].s32Y;

                    pstRect->astRect[i][u32RoiNumTmp].astPoint[2].s32X = pstRect->astRect[i][u32RoiNumTmp].astPoint[1].s32X;
                    pstRect->astRect[i][u32RoiNumTmp].astPoint[2].s32Y = (HI_U32)((HI_FLOAT)ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3] / (HI_FLOAT)u32SrcHeight * (HI_FLOAT)u32DstHeight) & (~1);

                    pstRect->astRect[i][u32RoiNumTmp].astPoint[3].s32X =  pstRect->astRect[i][u32RoiNumTmp].astPoint[0].s32X;
                    pstRect->astRect[i][u32RoiNumTmp].astPoint[3].s32Y =  pstRect->astRect[i][u32RoiNumTmp].astPoint[2].s32Y;

                    u32RoiNumTmp++;
                }

            }

            pstRect->au32RoiNum[i] = u32RoiNumTmp;
            pstRect->u32TotalNum += u32RoiNumTmp;
            u32RoiNumBias += ps32ClassRoiNum[i];
        }

    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : Rfcn Proc
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Rfcn_Proc(SAMPLE_SVP_NNIE_PARAM_S *pstParam,
                                        SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S *pstSwParam, VIDEO_FRAME_INFO_S* pstExtFrmInfo,
                                        HI_U32 u32BaseWidth,HI_U32 u32BaseHeight)
{
    HI_S32 s32Ret = HI_FAILURE;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    /*SP420*/
    pstParam->astSegData[stInputDataIdx.u32SegIdx].astSrc[stInputDataIdx.u32NodeIdx].u64VirAddr = pstExtFrmInfo->stVFrame.u64VirAddr[0];
    pstParam->astSegData[stInputDataIdx.u32SegIdx].astSrc[stInputDataIdx.u32NodeIdx].u64PhyAddr = pstExtFrmInfo->stVFrame.u64PhyAddr[0];
    pstParam->astSegData[stInputDataIdx.u32SegIdx].astSrc[stInputDataIdx.u32NodeIdx].u32Stride  = pstExtFrmInfo->stVFrame.u32Stride[0];

    /*NNIE process 0-th seg*/
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(pstParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

    /*RPN*/
    s32Ret = SAMPLE_SVP_NNIE_Rfcn_Rpn(pstParam, pstSwParam);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,SAMPLE_SVP_NNIE_RFCN_Rpn failed!\n");

    /*NNIE process 1-th seg, the input data comes from 3-rd report node of 0-th seg,
      the input roi comes from RPN results*/
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 3;
    stProcSegIdx.u32SegIdx = 1;
    s32Ret = SAMPLE_SVP_NNIE_ForwardWithBbox(pstParam,&stInputDataIdx,
             &pstSwParam->stRpnBbox,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

    /*NNIE process 2-nd seg, the input data comes from 4-th report node of 0-th seg
      the input roi comes from RPN results*/
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 4;
    stProcSegIdx.u32SegIdx = 2;
    s32Ret = SAMPLE_SVP_NNIE_ForwardWithBbox(pstParam,&stInputDataIdx,
             &pstSwParam->stRpnBbox,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

    /*GetResult*/
    /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Rfcn_GetResult
     function's input datas are correct*/
    s32Ret = SAMPLE_SVP_NNIE_Rfcn_GetResult(pstParam,pstSwParam);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,SAMPLE_SVP_NNIE_Rfcn_GetResult failed!\n");

    /*draw result, this sample has 21 classes:
     class 0:background     class 1:plane           class 2:bicycle
     class 3:bird           class 4:boat            class 5:bottle
     class 6:bus            class 7:car             class 8:cat
     class 9:chair          class10:cow             class11:diningtable
     class 12:dog           class13:horse           class14:motorbike
     class 15:person        class16:pottedplant     class17:sheep
     class 18:sofa          class19:train           class20:tvmonitor*/
    s32Ret = SAMPLE_SVP_NNIE_RoiToRect(&(pstSwParam->stDstScore),
                                       &(pstSwParam->stDstRoi), &(pstSwParam->stClassRoiNum), pstSwParam->af32ScoreThr,HI_TRUE,&(pstSwParam->stRect),
                                       pstExtFrmInfo->stVFrame.u32Width, pstExtFrmInfo->stVFrame.u32Height,u32BaseWidth,u32BaseHeight);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_RoiToRect failed!\n",s32Ret);

    return s32Ret;

}
/******************************************************************************
* function : Rfcn vi to vo thread entry
******************************************************************************/
static HI_VOID* SAMPLE_SVP_NNIE_Rfcn_ViToVo(HI_VOID* pArgs)
{
    HI_S32 s32Ret;
    SAMPLE_SVP_NNIE_PARAM_S *pstParam;
    SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S *pstSwParam;
    VIDEO_FRAME_INFO_S stBaseFrmInfo;
    VIDEO_FRAME_INFO_S stExtFrmInfo;
    HI_S32 s32MilliSec = 20000;
    VO_LAYER voLayer = 0;
    VO_CHN voChn = 0;
    HI_S32 s32VpssGrp = 0;
    HI_S32 as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};

    pstParam = &s_stRfcnNnieParam;
    pstSwParam = &s_stRfcnSoftwareParam;

    while (HI_FALSE == s_bNnieStopSignal)
    {
        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[1], &stExtFrmInfo, s32MilliSec);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
                       s32Ret,s32VpssGrp, as32VpssChn[1]);
            continue;
        }

        s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[0], &stBaseFrmInfo, s32MilliSec);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, EXT_RELEASE,
                               "Error(%#x),HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
                               s32Ret,s32VpssGrp, as32VpssChn[0]);

        s32Ret = SAMPLE_SVP_NNIE_Rfcn_Proc(pstParam,pstSwParam, &stExtFrmInfo,
                                           stBaseFrmInfo.stVFrame.u32Width,stBaseFrmInfo.stVFrame.u32Height);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
                               "Error(%#x),SAMPLE_SVP_NNIE_Rfcn_Proc failed!\n", s32Ret);

        //Draw rect
        s32Ret = SAMPLE_COMM_SVP_NNIE_FillRect(&stBaseFrmInfo, &(pstSwParam->stRect), 0x0000FF00);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
                               "SAMPLE_COMM_SVP_NNIE_FillRect failed, Error(%#x)!\n", s32Ret);

        s32Ret = HI_MPI_VO_SendFrame(voLayer, voChn, &stBaseFrmInfo, s32MilliSec);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
                               "HI_MPI_VO_SendFrame failed, Error(%#x)!\n", s32Ret);

BASE_RELEASE:
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp,as32VpssChn[0], &stBaseFrmInfo);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VPSS_ReleaseChnFrame failed,Grp(%d) chn(%d)!\n",
                       s32Ret,s32VpssGrp,as32VpssChn[0]);
        }

EXT_RELEASE:
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp,as32VpssChn[1], &stExtFrmInfo);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),HI_MPI_VPSS_ReleaseChnFrame failed,Grp(%d) chn(%d)!\n",
                       s32Ret,s32VpssGrp,as32VpssChn[1]);
        }

    }

    return HI_NULL;
}

/******************************************************************************
* function : Rfcn
******************************************************************************/
void SAMPLE_SVP_NNIE_Rfcn(void)
{
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_rfcn_resnet50_cycle_352x288.wk";
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SIZE_S stSize;
    PIC_SIZE_E enSize = PIC_CIF;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR acThreadName[16] = {0};

    memset(&s_stRfcnModel,0,sizeof(s_stRfcnModel));
    memset(&s_stRfcnNnieParam,0,sizeof(s_stRfcnNnieParam));
    memset(&s_stRfcnSoftwareParam,0,sizeof(s_stRfcnSoftwareParam));

    /******************************************
     step 1: start vi vpss vo
     ******************************************/
    s_stRfcnSwitch.bVenc = HI_FALSE;
    s_stRfcnSwitch.bVo   = HI_TRUE;
    s32Ret = SAMPLE_COMM_IVE_StartViVpssVencVo(&s_stViConfig,&s_stRfcnSwitch,&enSize);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_RFCN_0,
                           "Error(%#x),SAMPLE_COMM_IVE_StartViVpssVencVo failed!\n", s32Ret);

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enSize, &stSize);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_RFCN_0,
                           "Error(%#x),SAMPLE_COMM_SYS_GetPicSize failed!\n", s32Ret);

    /******************************************
     step 2: init NNIE param
     ******************************************/
    stNnieCfg.pszPic= NULL;
    stNnieCfg.u32MaxInputNum = 1; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 300;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0; //set NNIE core for 0-th Seg
    stNnieCfg.aenNnieCoreId[1] = SVP_NNIE_ID_0; //set NNIE core for 1-th Seg
    stNnieCfg.aenNnieCoreId[2] = SVP_NNIE_ID_0; //set NNIE core for 2-th Seg

    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stRfcnModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,END_RFCN_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    /*apcRpnDataLayerName is used to set RPN data layer name
      and search RPN input data,if user has changed network struct, please
      make sure the data layer names are correct*/
    s_stRfcnNnieParam.pstModel = &s_stRfcnModel.stModel;
    s_stRfcnSoftwareParam.apcRpnDataLayerName[0] = "rpn_cls_score";
    s_stRfcnSoftwareParam.apcRpnDataLayerName[1] = "rpn_bbox_pred";
    s32Ret = SAMPLE_SVP_NNIE_Rfcn_ParamInit(&stNnieCfg,&s_stRfcnNnieParam,
                                            &s_stRfcnSoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,END_RFCN_1,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Rfcn_ParamInit failed!\n");

    s_bNnieStopSignal = HI_FALSE;

    /******************************************
      step 3: Create work thread
     ******************************************/
    snprintf(acThreadName, 16, "NNIE_ViToVo");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0,0,0);
    pthread_create(&s_hNnieThread, 0, SAMPLE_SVP_NNIE_Rfcn_ViToVo, NULL);

    SAMPLE_PAUSE();

    s_bNnieStopSignal = HI_TRUE;
    pthread_join(s_hNnieThread, HI_NULL);
    s_hNnieThread = 0;
END_RFCN_1:

    SAMPLE_SVP_NNIE_Rfcn_Deinit(&s_stRfcnNnieParam,&s_stRfcnSoftwareParam,&s_stRfcnModel);
END_RFCN_0:
    SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stRfcnSwitch);
    return ;

}


/******************************************************************************
* function : rfcn sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Rfcn_HandleSig(void)
{
    s_bNnieStopSignal = HI_TRUE;
    if (0 != s_hNnieThread)
    {
        pthread_join(s_hNnieThread, HI_NULL);
        s_hNnieThread = 0;
    }

    SAMPLE_SVP_NNIE_Rfcn_Deinit(&s_stRfcnNnieParam,&s_stRfcnSoftwareParam,&s_stRfcnModel);
    memset(&s_stRfcnNnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stRfcnSoftwareParam,0,sizeof(SAMPLE_SVP_NNIE_RFCN_SOFTWARE_PARAM_S));
    memset(&s_stRfcnModel,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));

    SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stRfcnSwitch);

}


/******************************************************************************
* function : SSD software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Ssd_SoftwareDeinit(SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stPriorBoxTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stPriorBoxTmpBuf.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stPriorBoxTmpBuf.u64PhyAddr,
                            pstSoftWareParam->stPriorBoxTmpBuf.u64VirAddr);
        pstSoftWareParam->stPriorBoxTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stPriorBoxTmpBuf.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : Ssd Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Ssd_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Ssd_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_Ssd_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}


/******************************************************************************
* function : Ssd software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Ssd_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_U32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    /*Set Conv Parameters*/
    /*the SSD sample report resule is after permute operation,
     conv result is (C, H, W), after permute, the report node's
     (C1, H1, W1) is (H, W, C), the stride of report result is aligned according to C dim*/
    for(i = 0; i < 12; i++)
    {
        pstSoftWareParam->au32ConvHeight[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        pstSoftWareParam->au32ConvWidth[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height;
        pstSoftWareParam->au32ConvChannel[i] = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width;
        if(i%2==1)
        {
            pstSoftWareParam->au32ConvStride[i/2] = SAMPLE_SVP_NNIE_ALIGN16(pstSoftWareParam->au32ConvChannel[i]*sizeof(HI_U32))/sizeof(HI_U32);
        }
    }

    /*Set PriorBox Parameters*/
    pstSoftWareParam->au32PriorBoxWidth[0] = 38;
    pstSoftWareParam->au32PriorBoxWidth[1] = 19;
    pstSoftWareParam->au32PriorBoxWidth[2] = 10;
    pstSoftWareParam->au32PriorBoxWidth[3] = 5;
    pstSoftWareParam->au32PriorBoxWidth[4] = 3;
    pstSoftWareParam->au32PriorBoxWidth[5] = 1;

    pstSoftWareParam->au32PriorBoxHeight[0] = 38;
    pstSoftWareParam->au32PriorBoxHeight[1] = 19;
    pstSoftWareParam->au32PriorBoxHeight[2] = 10;
    pstSoftWareParam->au32PriorBoxHeight[3] = 5;
    pstSoftWareParam->au32PriorBoxHeight[4] = 3;
    pstSoftWareParam->au32PriorBoxHeight[5] = 1;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;

    pstSoftWareParam->af32PriorBoxMinSize[0][0] = 30.0f;
    pstSoftWareParam->af32PriorBoxMinSize[1][0] = 60.0f;
    pstSoftWareParam->af32PriorBoxMinSize[2][0] = 111.0f;
    pstSoftWareParam->af32PriorBoxMinSize[3][0] = 162.0f;
    pstSoftWareParam->af32PriorBoxMinSize[4][0] = 213.0f;
    pstSoftWareParam->af32PriorBoxMinSize[5][0] = 264.0f;

    pstSoftWareParam->af32PriorBoxMaxSize[0][0] = 60.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[1][0] = 111.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[2][0] = 162.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[3][0] = 213.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[4][0] = 264.0f;
    pstSoftWareParam->af32PriorBoxMaxSize[5][0] = 315.0f;

    pstSoftWareParam->u32MinSizeNum = 1;
    pstSoftWareParam->u32MaxSizeNum = 1;
    pstSoftWareParam->bFlip= HI_TRUE;
    pstSoftWareParam->bClip= HI_FALSE;

    pstSoftWareParam->au32InputAspectRatioNum[0] = 1;
    pstSoftWareParam->au32InputAspectRatioNum[1] = 2;
    pstSoftWareParam->au32InputAspectRatioNum[2] = 2;
    pstSoftWareParam->au32InputAspectRatioNum[3] = 2;
    pstSoftWareParam->au32InputAspectRatioNum[4] = 1;
    pstSoftWareParam->au32InputAspectRatioNum[5] = 1;

    pstSoftWareParam->af32PriorBoxAspectRatio[0][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[0][1] = 0;
    pstSoftWareParam->af32PriorBoxAspectRatio[1][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[1][1] = 3;
    pstSoftWareParam->af32PriorBoxAspectRatio[2][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[2][1] = 3;
    pstSoftWareParam->af32PriorBoxAspectRatio[3][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[3][1] = 3;
    pstSoftWareParam->af32PriorBoxAspectRatio[4][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[4][1] = 0;
    pstSoftWareParam->af32PriorBoxAspectRatio[5][0] = 2;
    pstSoftWareParam->af32PriorBoxAspectRatio[5][1] = 0;

    pstSoftWareParam->af32PriorBoxStepWidth[0] = 8;
    pstSoftWareParam->af32PriorBoxStepWidth[1] = 16;
    pstSoftWareParam->af32PriorBoxStepWidth[2] = 32;
    pstSoftWareParam->af32PriorBoxStepWidth[3] = 64;
    pstSoftWareParam->af32PriorBoxStepWidth[4] = 100;
    pstSoftWareParam->af32PriorBoxStepWidth[5] = 300;

    pstSoftWareParam->af32PriorBoxStepHeight[0] = 8;
    pstSoftWareParam->af32PriorBoxStepHeight[1] = 16;
    pstSoftWareParam->af32PriorBoxStepHeight[2] = 32;
    pstSoftWareParam->af32PriorBoxStepHeight[3] = 64;
    pstSoftWareParam->af32PriorBoxStepHeight[4] = 100;
    pstSoftWareParam->af32PriorBoxStepHeight[5] = 300;

    pstSoftWareParam->f32Offset = 0.5f;

    pstSoftWareParam->as32PriorBoxVar[0] = (HI_S32)(0.1f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->as32PriorBoxVar[1] = (HI_S32)(0.1f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->as32PriorBoxVar[2] = (HI_S32)(0.2f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->as32PriorBoxVar[3] = (HI_S32)(0.2f*SAMPLE_SVP_NNIE_QUANT_BASE);

    /*Set Softmax Parameters*/
    pstSoftWareParam->u32SoftMaxInHeight = 21;
    pstSoftWareParam->au32SoftMaxInChn[0] = 121296;
    pstSoftWareParam->au32SoftMaxInChn[1] = 45486;
    pstSoftWareParam->au32SoftMaxInChn[2] = 12600;
    pstSoftWareParam->au32SoftMaxInChn[3] = 3150;
    pstSoftWareParam->au32SoftMaxInChn[4] = 756;
    pstSoftWareParam->au32SoftMaxInChn[5] = 84;

    pstSoftWareParam->u32ConcatNum = 6;
    pstSoftWareParam->u32SoftMaxOutWidth = 1;
    pstSoftWareParam->u32SoftMaxOutHeight = 21;
    pstSoftWareParam->u32SoftMaxOutChn = 8732;

    /*Set DetectionOut Parameters*/
    pstSoftWareParam->u32ClassNum = 21;
    pstSoftWareParam->u32TopK = 400;
    pstSoftWareParam->u32KeepTopK = 200;
    pstSoftWareParam->u32NmsThresh = (HI_U16)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = 1;
    pstSoftWareParam->au32DetectInputChn[0] = 23104;
    pstSoftWareParam->au32DetectInputChn[1] = 8664;
    pstSoftWareParam->au32DetectInputChn[2] = 2400;
    pstSoftWareParam->au32DetectInputChn[3] = 600;
    pstSoftWareParam->au32DetectInputChn[4] = 144;
    pstSoftWareParam->au32DetectInputChn[5] = 16;

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum;
    u32TotalSize = SAMPLE_SVP_NNIE_Ssd_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstCfg->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstCfg->u32MaxRoiNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize;

    if(pstCfg->aenNnieCoreId[0] == SVP_NNIE_ID_0)
    {
        s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_SSD_INIT0",NULL,(HI_U64*)&u64PhyAddr,
                                              (void**)&pu8VirAddr,u32TotalSize);
        SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                  "Error,Malloc memory failed!\n");
    }
    else if(pstCfg->aenNnieCoreId[0] == SVP_NNIE_ID_1)
    {
        s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_SSD_INIT1",NULL,(HI_U64*)&u64PhyAddr,
                                              (void**)&pu8VirAddr,u32TotalSize);
        SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                  "Error,Malloc memory failed!\n");
    }

    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

    /*set each tmp buffer addr*/
    pstSoftWareParam->stPriorBoxTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stPriorBoxTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);

    pstSoftWareParam->stSoftMaxTmpBuf.u64PhyAddr = u64PhyAddr+
            pstSoftWareParam->stPriorBoxTmpBuf.u32Size;
    pstSoftWareParam->stSoftMaxTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr+
            pstSoftWareParam->stPriorBoxTmpBuf.u32Size);

    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr+
            pstSoftWareParam->stPriorBoxTmpBuf.u32Size+pstSoftWareParam->stSoftMaxTmpBuf.u32Size;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr+
            pstSoftWareParam->stPriorBoxTmpBuf.u32Size+ pstSoftWareParam->stSoftMaxTmpBuf.u32Size);

    u32TmpBufTotalSize = pstSoftWareParam->stPriorBoxTmpBuf.u32Size+
                         pstSoftWareParam->stSoftMaxTmpBuf.u32Size + pstSoftWareParam->stGetResultTmpBuf.u32Size;

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
                                           pstSoftWareParam->u32TopK*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
            pstSoftWareParam->u32TopK*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
            pstSoftWareParam->u32TopK*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*
            pstSoftWareParam->u32TopK;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
            u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+
            u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return s32Ret;
}


/******************************************************************************
* function : Ssd init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Ssd_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Ssd_SoftwareInit(pstCfg,pstNnieParam,
             pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_SVP_NNIE_Ssd_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Ssd_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_Ssd_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}

/******************************************************************************
* function : show SSD sample(image 300x300 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_Ssd0(void *arg)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/dog_bike_car_300x300.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_cycle.wk";
//    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_inst.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};
    struct timespec stStartTimeSpc;
    struct timespec stEndTimeSpc;

    /*Set configuration parameter*/
    f32PrintResultThresh = 0.8f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core
    //stNnieCfg.aenNnieCoreId[1] = SVP_NNIE_ID_1;//set NNIE core

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Ssd Load model*/
    SAMPLE_SVP_TRACE_INFO("Ssd Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stSsdModel0);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    // printstModel(&s_stSsdModel);
    /*Ssd parameter initialization*/
    /*Ssd parameters are set in SAMPLE_SVP_NNIE_Ssd_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Ssd_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Ssd parameter initialization!\n");
    s_stSsdNnieParam0.pstModel = &s_stSsdModel0.stModel;

    struct timespec stStartTimeParamInit;
    struct timespec stEndTimeParamInit;
    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);
    s32Ret = SAMPLE_SVP_NNIE_Ssd_ParamInit(&stNnieCfg,&s_stSsdNnieParam0,&s_stSsdSoftwareParam0);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Ssd_ParamInit failed!\n");

    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);

     printstNnieCfg(&stNnieCfg);
     printstNnieParam(&s_stSsdNnieParam0,s_stSsdModel0.stModel.u32NetSegNum);
    /*Fill src data*/

//    fprintf(stderr,"Ssd start!\n");

    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;

    unsigned int countEx = 0;
    unsigned int countIn = 0;

    while(1)
    {
        fprintf(stderr,"\n################ Hi3559 ARFCV100 SSD0 start procesing ##############\n");
        fprintf(stderr,"Image resolution : 300*300\n");
        //clock_gettime(CLOCK_MONOTONIC, &stStartTimeSpc);
        countIn = 0;
        while(countIn++ < M_INTER_LOOP_NUMBER)
        {
            struct timespec stStartSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
#if 0
            s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stSsdNnieParam0,&stInputDataIdx);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
#else

#endif
            struct timespec stStartSpcForward;
            /*NNIE process(process the 0-th segment)*/
            stProcSegIdx.u32SegIdx = 0;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);
            s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stSsdNnieParam0,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
            /*software process*/
            /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Ssd_GetResult
             function's input datas are correct*/
            struct timespec stStartSpcGetRes;

            clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
#if 0
            pthread_mutex_lock(&mtSSD);
            s32Ret = SAMPLE_SVP_NNIE_Ssd_GetResult("SSD0", &s_stSsdNnieParam0,&s_stSsdSoftwareParam0);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_Ssd_GetResult failed!\n");
            pthread_mutex_unlock(&mtSSD);
#else
           // usleep(80*1000);
           // fprintf(stderr,"wait ... \n");
#endif
            struct timespec stEndSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);
            /*print result, this sample has 21 classes:
             class 0:background     class 1:plane           class 2:bicycle
             class 3:bird           class 4:boat            class 5:bottle
             class 6:bus            class 7:car             class 8:cat
             class 9:chair          class10:cow             class11:diningtable
             class 12:dog           class13:horse           class14:motorbike
             class 15:person        class16:pottedplant     class17:sheep
             class 18:sofa          class19:train           class20:tvmonitor*/
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
            HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
            HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
            HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcGetRes);
            HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
            SAMPLE_SVP_PrintPerformance("SSD0",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
        }
//        clock_gettime(CLOCK_MONOTONIC, &stEndTimeSpc);
//        HI_U64 diff = stEndTimeSpc.tv_sec*1000*1000 +
//                      stEndTimeSpc.tv_nsec/1000 -
//                      stStartTimeSpc.tv_sec*1000*1000-
//                      stStartTimeSpc.tv_nsec/1000;
////        fprintf(stderr,"start:%llu,end %llu\n",stStartTimeSpc.tv_sec,stEndTimeSpc.tv_sec);

//        fprintf(stderr,"  Elapse time      : %.2f ms \n",diff*1.0/(M_INTER_LOOP_NUMBER*1000));
//        fprintf(stderr,"  CNN Performance  : %.2f fps\n",1000*1000*1.0*M_INTER_LOOP_NUMBER/diff);

        fprintf(stderr,"################ Hi3559 ARFCV100 SSD0 end procesing ################\n");
        //(void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stSsdSoftwareParam0.stDstScore,
        //        &s_stSsdSoftwareParam0.stDstRoi, &s_stSsdSoftwareParam0.stClassRoiNum,f32PrintResultThresh);
    }

    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stSsdSoftwareParam0.stDstScore,
            &s_stSsdSoftwareParam0.stDstRoi, &s_stSsdSoftwareParam0.stClassRoiNum,f32PrintResultThresh);

SSD_FAIL_0:
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam0,&s_stSsdSoftwareParam0,&s_stSsdModel0);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : SSD sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Ssd_HandleSig0(void)
{
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam0,&s_stSsdSoftwareParam0,&s_stSsdModel0);
    memset(&s_stSsdNnieParam0,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stSsdSoftwareParam0,0,sizeof(SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S));
    memset(&s_stSsdModel0,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : show SSD sample(image 300x300 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_Ssd1(void *arg)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/dog_bike_car_300x300.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_cycle.wk";
//    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_inst.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};
    struct timespec stStartTimeSpc;
    struct timespec stEndTimeSpc;

    /*Set configuration parameter*/
    f32PrintResultThresh = 0.8f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_1;//set NNIE core

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Ssd Load model*/
    SAMPLE_SVP_TRACE_INFO("Ssd Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stSsdModel1);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    // printstModel(&s_stSsdModel);
    /*Ssd parameter initialization*/
    /*Ssd parameters are set in SAMPLE_SVP_NNIE_Ssd_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Ssd_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Ssd parameter initialization!\n");
    s_stSsdNnieParam1.pstModel = &s_stSsdModel1.stModel;

    struct timespec stStartTimeParamInit;
    struct timespec stEndTimeParamInit;
    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);
    s32Ret = SAMPLE_SVP_NNIE_Ssd_ParamInit(&stNnieCfg,&s_stSsdNnieParam1,&s_stSsdSoftwareParam1);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Ssd_ParamInit failed!\n");

    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);

    // printstNnieCfg(&stNnieCfg);
    // printstNnieParam(&s_stSsdNnieParam,s_stSsdModel.stModel.u32NetSegNum);
    /*Fill src data*/

//    fprintf(stderr,"Ssd start!\n");

    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;

    unsigned int countEx = 0;
    unsigned int countIn = 0;

    while(1)
    {
        fprintf(stderr,"\n################ Hi3559 ARFCV100 SSD1 start procesing ##############\n");
        fprintf(stderr,"Image resolution : 300*300\n");
        //clock_gettime(CLOCK_MONOTONIC, &stStartTimeSpc);
        countIn = 0;
        while(countIn++ < M_INTER_LOOP_NUMBER)
        {
            struct timespec stStartSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
#if 0
            s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stSsdNnieParam1,&stInputDataIdx);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
            fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
#else

#endif
            struct timespec stStartSpcForward;
            /*NNIE process(process the 0-th segment)*/
            stProcSegIdx.u32SegIdx = 0;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);
            s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stSsdNnieParam1,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
            fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
            /*software process*/
            /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Ssd_GetResult
             function's input datas are correct*/
            struct timespec stStartSpcGetRes;
            fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);
#if 0
            pthread_mutex_lock(&mtSSD);
            s32Ret = SAMPLE_SVP_NNIE_Ssd_GetResult("SSD1",&s_stSsdNnieParam1,&s_stSsdSoftwareParam1);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_Ssd_GetResult failed!\n");

            pthread_mutex_unlock(&mtSSD);
#else
            //usleep(80*1000);
            //fprintf(stderr,"wait ... \n");
#endif
            struct timespec stEndSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);
            /*print result, this sample has 21 classes:
             class 0:background     class 1:plane           class 2:bicycle
             class 3:bird           class 4:boat            class 5:bottle
             class 6:bus            class 7:car             class 8:cat
             class 9:chair          class10:cow             class11:diningtable
             class 12:dog           class13:horse           class14:motorbike
             class 15:person        class16:pottedplant     class17:sheep
             class 18:sofa          class19:train           class20:tvmonitor*/
            fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
            HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
            HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
            HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcGetRes);
            HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);

            SAMPLE_SVP_PrintPerformance("SSD1",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
            fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
        }
//        clock_gettime(CLOCK_MONOTONIC, &stEndTimeSpc);
//        HI_U64 diff = stEndTimeSpc.tv_sec*1000*1000 +
//                      stEndTimeSpc.tv_nsec/1000 -
//                      stStartTimeSpc.tv_sec*1000*1000-
//                      stStartTimeSpc.tv_nsec/1000;
////        fprintf(stderr,"start:%llu,end %llu\n",stStartTimeSpc.tv_sec,stEndTimeSpc.tv_sec);

//        fprintf(stderr,"  Elapse time      : %.2f ms \n",diff*1.0/(M_INTER_LOOP_NUMBER*1000));
//        fprintf(stderr,"  CNN Performance  : %.2f fps\n",1000*1000*1.0*M_INTER_LOOP_NUMBER/diff);

        fprintf(stderr,"################ Hi3559 ARFCV100 SSD1 end procesing ################\n");
    }

    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stSsdSoftwareParam1.stDstScore,
            &s_stSsdSoftwareParam1.stDstRoi, &s_stSsdSoftwareParam1.stClassRoiNum,f32PrintResultThresh);

SSD_FAIL_0:
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam1,&s_stSsdSoftwareParam1,&s_stSsdModel1);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : SSD sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Ssd_HandleSig1(void)
{
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam1,&s_stSsdSoftwareParam1,&s_stSsdModel1);
    memset(&s_stSsdNnieParam1,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stSsdSoftwareParam1,0,sizeof(SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S));
    memset(&s_stSsdModel1,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : show SSD sample(image 300x300 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_SsdForward0(void *arg)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/dog_bike_car_300x300.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_cycle.wk";
//    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_inst.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};
    struct timespec stStartTimeSpc;
    struct timespec stEndTimeSpc;

    /*Set configuration parameter*/
    f32PrintResultThresh = 0.8f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Ssd Load model*/
    SAMPLE_SVP_TRACE_INFO("Ssd Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stSsdModel0);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    // printstModel(&s_stSsdModel);
    /*Ssd parameter initialization*/
    /*Ssd parameters are set in SAMPLE_SVP_NNIE_Ssd_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Ssd_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Ssd parameter initialization!\n");
    s_stSsdNnieParam0.pstModel = &s_stSsdModel0.stModel;

    struct timespec stStartTimeParamInit;
    struct timespec stEndTimeParamInit;
    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);
    s32Ret = SAMPLE_SVP_NNIE_Ssd_ParamInit(&stNnieCfg,&s_stSsdNnieParam0,&s_stSsdSoftwareParam0);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Ssd_ParamInit failed!\n");

    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);

    // printstNnieCfg(&stNnieCfg);
    // printstNnieParam(&s_stSsdNnieParam,s_stSsdModel.stModel.u32NetSegNum);
    /*Fill src data*/

//    fprintf(stderr,"Ssd start!\n");

    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;

    unsigned int countEx = 0;
    unsigned int countIn = 0;

    while(1)
    {
        fprintf(stderr,"\n################ Hi3559 ARFCV100 SSDfd0 start procesing ##############\n");
        fprintf(stderr,"Image resolution : 300*300\n");
        //clock_gettime(CLOCK_MONOTONIC, &stStartTimeSpc);
        countIn = 0;
        while(countIn++ < M_INTER_LOOP_NUMBER)
        {

            struct timespec stStartSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
            //s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stSsdNnieParam0,&stInputDataIdx);
            //SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
            //                           "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
            //fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
            /*NNIE process(process the 0-th segment)*/

            struct timespec stStartSpcForward;
            stProcSegIdx.u32SegIdx = 0;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);
            s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stSsdNnieParam0,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
            fprintf(stderr,"SSD1 Line = %d\n",__LINE__);
            /*software process*/
            /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Ssd_GetResult
             function's input datas are correct*/


            struct timespec stStartSpcGetRes;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);
            //pthread_mutex_lock(&mtSSD);
            //s32Ret = SAMPLE_SVP_NNIE_Ssd_GetResult("SSDfd0",&s_stSsdNnieParam0,&s_stSsdSoftwareParam0);
            //SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
            //                           "Error,SAMPLE_SVP_NNIE_Ssd_GetResult failed!\n");
            //pthread_mutex_unlock(&mtSSD);
            //usleep(80*1000);
            struct timespec stEndSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);
            HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
            HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
            HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcGetRes);
            HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);
            SAMPLE_SVP_PrintPerformance("SSDfd0",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
            fprintf(stderr,"SSD0 Line = %d\n",__LINE__);
        }
//        clock_gettime(CLOCK_MONOTONIC, &stEndTimeSpc);
//        HI_U64 diff = stEndTimeSpc.tv_sec*1000*1000 +
//                      stEndTimeSpc.tv_nsec/1000 -
//                      stStartTimeSpc.tv_sec*1000*1000-
//                      stStartTimeSpc.tv_nsec/1000;
////        fprintf(stderr,"start:%llu,end %llu\n",stStartTimeSpc.tv_sec,stEndTimeSpc.tv_sec);

//        fprintf(stderr,"  Elapse time      : %.2f ms \n",diff*1.0/(M_INTER_LOOP_NUMBER*1000));
//        fprintf(stderr,"  CNN Performance  : %.2f fps\n",1000*1000*1.0*M_INTER_LOOP_NUMBER/diff);

        fprintf(stderr,"################ Hi3559 ARFCV100 SSD0 end procesing ################\n");
    }


    /*print result, this sample has 21 classes:
     class 0:background     class 1:plane           class 2:bicycle
     class 3:bird           class 4:boat            class 5:bottle
     class 6:bus            class 7:car             class 8:cat
     class 9:chair          class10:cow             class11:diningtable
     class 12:dog           class13:horse           class14:motorbike
     class 15:person        class16:pottedplant     class17:sheep
     class 18:sofa          class19:train           class20:tvmonitor*/

    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stSsdSoftwareParam0.stDstScore,
            &s_stSsdSoftwareParam0.stDstRoi, &s_stSsdSoftwareParam0.stClassRoiNum,f32PrintResultThresh);

SSD_FAIL_0:
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam0,&s_stSsdSoftwareParam0,&s_stSsdModel0);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : SSD sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Ssd_HandleSigFoward0(void)
{
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam0,&s_stSsdSoftwareParam0,&s_stSsdModel0);
    memset(&s_stSsdNnieParam0,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stSsdSoftwareParam0,0,sizeof(SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S));
    memset(&s_stSsdModel0,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : show SSD sample(image 300x300 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_SsdForward1(void *arg)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/dog_bike_car_300x300.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_cycle.wk";
//    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_ssd_inst.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};
    struct timespec stStartTimeSpc;
    struct timespec stEndTimeSpc;

    /*Set configuration parameter*/
    f32PrintResultThresh = 0.8f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_1;//set NNIE core

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Ssd Load model*/
    SAMPLE_SVP_TRACE_INFO("Ssd Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stSsdModel1);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    // printstModel(&s_stSsdModel);
    /*Ssd parameter initialization*/
    /*Ssd parameters are set in SAMPLE_SVP_NNIE_Ssd_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Ssd_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Ssd parameter initialization!\n");
    s_stSsdNnieParam1.pstModel = &s_stSsdModel1.stModel;

    struct timespec stStartTimeParamInit;
    struct timespec stEndTimeParamInit;
    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);
    s32Ret = SAMPLE_SVP_NNIE_Ssd_ParamInit(&stNnieCfg,&s_stSsdNnieParam1,&s_stSsdSoftwareParam1);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Ssd_ParamInit failed!\n");

    clock_gettime(CLOCK_MONOTONIC, &stStartTimeParamInit);

    // printstNnieCfg(&stNnieCfg);
    // printstNnieParam(&s_stSsdNnieParam,s_stSsdModel.stModel.u32NetSegNum);
    /*Fill src data*/

//    fprintf(stderr,"Ssd start!\n");

    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;

    unsigned int countEx = 0;
    unsigned int countIn = 0;

    while(1)
    {
        fprintf(stderr,"\n################ Hi3559 ARFCV100 SSDfd1 start procesing ##############\n");
        fprintf(stderr,"Image resolution : 300*300\n");
        //clock_gettime(CLOCK_MONOTONIC, &stStartTimeSpc);
        countIn = 0;
        while(countIn++ < M_INTER_LOOP_NUMBER)
        {

            struct timespec stStartSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
            //s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stSsdNnieParam1,&stInputDataIdx);
            //SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
            //                           "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
            //fprintf(stderr,"SSD1 Line = %d\n",__LINE__);

            /*NNIE process(process the 0-th segment)*/
            struct timespec stStartSpcForward;
            stProcSegIdx.u32SegIdx = 0;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);
            s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stSsdNnieParam1,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
            SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                       "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
            /*software process*/
            /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Ssd_GetResult
             function's input datas are correct*/

            //usleep(80*1000);

            struct timespec stStartSpcGetRes;
            clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);
            //pthread_mutex_lock(&mtSSD);
            //s32Ret = SAMPLE_SVP_NNIE_Ssd_GetResult("SSDfd1",&s_stSsdNnieParam1,&s_stSsdSoftwareParam1);
            //SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,SSD_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
            //                           "Error,SAMPLE_SVP_NNIE_Ssd_GetResult failed!\n");
            //pthread_mutex_unlock(&mtSSD);
            //usleep(80*1000);
            struct timespec stEndSpcTime;
            clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);
            HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
            HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
            HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcGetRes);
            HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);
            SAMPLE_SVP_PrintPerformance("SSDfd1",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);

        }
//        clock_gettime(CLOCK_MONOTONIC, &stEndTimeSpc);
//        HI_U64 diff = stEndTimeSpc.tv_sec*1000*1000 +
//                      stEndTimeSpc.tv_nsec/1000 -
//                      stStartTimeSpc.tv_sec*1000*1000-
//                      stStartTimeSpc.tv_nsec/1000;
////        fprintf(stderr,"start:%llu,end %llu\n",stStartTimeSpc.tv_sec,stEndTimeSpc.tv_sec);

//        fprintf(stderr,"  Elapse time      : %.2f ms \n",diff*1.0/(M_INTER_LOOP_NUMBER*1000));
//        fprintf(stderr,"  CNN Performance  : %.2f fps\n",1000*1000*1.0*M_INTER_LOOP_NUMBER/diff);

        fprintf(stderr,"################ Hi3559 ARFCV100 SSD1 end procesing ################\n");
    }


    /*print result, this sample has 21 classes:
     class 0:background     class 1:plane           class 2:bicycle
     class 3:bird           class 4:boat            class 5:bottle
     class 6:bus            class 7:car             class 8:cat
     class 9:chair          class10:cow             class11:diningtable
     class 12:dog           class13:horse           class14:motorbike
     class 15:person        class16:pottedplant     class17:sheep
     class 18:sofa          class19:train           class20:tvmonitor*/

    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stSsdSoftwareParam1.stDstScore,
            &s_stSsdSoftwareParam1.stDstRoi, &s_stSsdSoftwareParam1.stClassRoiNum,f32PrintResultThresh);

SSD_FAIL_0:
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam1,&s_stSsdSoftwareParam1,&s_stSsdModel1);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : SSD sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Ssd_HandleSigFoward1(void)
{
    SAMPLE_SVP_NNIE_Ssd_Deinit(&s_stSsdNnieParam1,&s_stSsdSoftwareParam1,&s_stSsdModel1);
    memset(&s_stSsdNnieParam1,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stSsdSoftwareParam1,0,sizeof(SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S));
    memset(&s_stSsdModel1,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Yolov1 software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov1_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stGetResultTmpBuf.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr,
                            pstSoftWareParam->stGetResultTmpBuf.u64VirAddr);
        pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = 0;
        pstSoftWareParam->stDstRoi.u64PhyAddr = 0;
        pstSoftWareParam->stDstRoi.u64VirAddr = 0;
        pstSoftWareParam->stDstScore.u64PhyAddr = 0;
        pstSoftWareParam->stDstScore.u64VirAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64PhyAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : Yolov1 Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov1_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Yolov1_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_Yolov1_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}


/******************************************************************************
* function : Yolov1 software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov1_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32BboxNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    pstSoftWareParam->u32BboxNumEachGrid = 2;
    pstSoftWareParam->u32ClassNum = 20;
    pstSoftWareParam->u32GridNumHeight = 7;
    pstSoftWareParam->u32GridNumWidth = 7;
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.5f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = (HI_U32)(0.2f*SAMPLE_SVP_NNIE_QUANT_BASE);

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum+1;
    u32BboxNum = pstSoftWareParam->u32BboxNumEachGrid*pstSoftWareParam->u32GridNumHeight*
                 pstSoftWareParam->u32GridNumWidth;
    u32TmpBufTotalSize = SAMPLE_SVP_NNIE_Yolov1_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*u32BboxNum*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*u32BboxNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize+u32TmpBufTotalSize;
    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_YOLOV1_INIT",NULL,(HI_U64*)&u64PhyAddr,
                                          (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

    /*set each tmp buffer addr*/
    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
                                           u32BboxNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
            u32BboxNum*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
            u32BboxNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*u32BboxNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
            u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+
            u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return s32Ret;
}


/******************************************************************************
* function : Yolov1 init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov1_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Yolov1_SoftwareInit(pstCfg,pstNnieParam,
             pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_SVP_NNIE_Yolov1_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Yolov1_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_Yolov1_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}


/******************************************************************************
* function : show YOLOV1 sample(image 448x448 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_Yolov1(void)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/dog_bike_car_448x448.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_yolov1_cycle.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    f32PrintResultThresh = 0.3f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Yolov1 Load model*/
    SAMPLE_SVP_TRACE_INFO("Yolov1 Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stYolov1Model);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV1_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    printstModel(&s_stYolov1Model);
    /*Yolov1 parameter initialization*/
    /*Yolov1 software parameters are set in SAMPLE_SVP_NNIE_Yolov1_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Yolov1_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Yolov1 parameter initialization!\n");
    s_stYolov1NnieParam.pstModel = &s_stYolov1Model.stModel;
    s32Ret = SAMPLE_SVP_NNIE_Yolov1_ParamInit(&stNnieCfg,&s_stYolov1NnieParam,&s_stYolov1SoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV1_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Yolov1_ParamInit failed!\n");
    printstNnieCfg(&stNnieCfg);
    printstNnieParam(&s_stYolov1NnieParam,s_stYolov1Model.stModel.u32NetSegNum);
    /*Fill src data*/
    SAMPLE_SVP_TRACE_INFO("Yolov1 start!\n");
    HI_U32 index = 0;
    while(index++ < M_INTER_LOOP_NUMBER)
    {
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        struct timespec stStartSpcTime;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
        s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stYolov1NnieParam,&stInputDataIdx);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV1_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
        struct timespec stStartSpcForward;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);

        /*NNIE process(process the 0-th segment)*/
        stProcSegIdx.u32SegIdx = 0;
        s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stYolov1NnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV1_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

        struct timespec stStartSpcGetRes;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);
        /*software process*/
        /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Yolov1_GetResult
         function input datas are correct*/
        s32Ret = SAMPLE_SVP_NNIE_Yolov1_GetResult(&s_stYolov1NnieParam,&s_stYolov1SoftwareParam);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV1_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Yolov1_GetResult failed!\n");

        /*print result, this sample has 21 classes:
         class 0:background     class 1:plane           class 2:bicycle
         class 3:bird           class 4:boat            class 5:bottle
         class 6:bus            class 7:car             class 8:cat
         class 9:chair          class10:cow             class11:diningtable
         class 12:dog           class13:horse           class14:motorbike
         class 15:person        class16:pottedplant     class17:sheep
         class 18:sofa          class19:train           class20:tvmonitor*/
        struct timespec stEndSpcTime;
        clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);
        HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
        HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
        HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcGetRes);
        HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);

        SAMPLE_SVP_PrintPerformance("Yolov1",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
        fprintf(stderr,"\n");
    }

    SAMPLE_SVP_TRACE_INFO("Yolov1 result:\n");
    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stYolov1SoftwareParam.stDstScore,
            &s_stYolov1SoftwareParam.stDstRoi, &s_stYolov1SoftwareParam.stClassRoiNum,f32PrintResultThresh);


YOLOV1_FAIL_0:
    SAMPLE_SVP_NNIE_Yolov1_Deinit(&s_stYolov1NnieParam,&s_stYolov1SoftwareParam,&s_stYolov1Model);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Yolov1 sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Yolov1_HandleSig(void)
{
    SAMPLE_SVP_NNIE_Yolov1_Deinit(&s_stYolov1NnieParam,&s_stYolov1SoftwareParam,&s_stYolov1Model);
    memset(&s_stYolov1NnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stYolov1SoftwareParam,0,sizeof(SAMPLE_SVP_NNIE_YOLOV1_SOFTWARE_PARAM_S));
    memset(&s_stYolov1Model,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Yolov2 software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov2_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stGetResultTmpBuf.u64VirAddr)
    {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr,
                            pstSoftWareParam->stGetResultTmpBuf.u64VirAddr);
        pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = 0;
        pstSoftWareParam->stDstRoi.u64PhyAddr = 0;
        pstSoftWareParam->stDstRoi.u64VirAddr = 0;
        pstSoftWareParam->stDstScore.u64PhyAddr = 0;
        pstSoftWareParam->stDstScore.u64VirAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64PhyAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64VirAddr = 0;
    }
    return s32Ret;
}


/******************************************************************************
* function : Yolov2 Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov2_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
        SAMPLE_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Yolov2_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_SVP_NNIE_Yolov2_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}


/******************************************************************************
* function : Yolov2 software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov2_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32BboxNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    pstSoftWareParam->u32BboxNumEachGrid = 5;
    pstSoftWareParam->u32ClassNum = 5;
    pstSoftWareParam->u32GridNumHeight = 13;
    pstSoftWareParam->u32GridNumWidth = 13;
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = (HI_U32)(0.25f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32MaxRoiNum = 10;
    pstSoftWareParam->af32Bias[0] = 1.08;
    pstSoftWareParam->af32Bias[1] = 1.19;
    pstSoftWareParam->af32Bias[2] = 3.42;
    pstSoftWareParam->af32Bias[3] = 4.41;
    pstSoftWareParam->af32Bias[4] = 6.63;
    pstSoftWareParam->af32Bias[5] = 11.38;
    pstSoftWareParam->af32Bias[6] = 9.42;
    pstSoftWareParam->af32Bias[7] = 5.11;
    pstSoftWareParam->af32Bias[8] = 16.62;
    pstSoftWareParam->af32Bias[9] = 10.52;

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum+1;
    u32BboxNum = pstSoftWareParam->u32BboxNumEachGrid*pstSoftWareParam->u32GridNumHeight*
                 pstSoftWareParam->u32GridNumWidth;
    u32TmpBufTotalSize = SAMPLE_SVP_NNIE_Yolov2_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*u32BboxNum*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*u32BboxNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize+u32TmpBufTotalSize;
    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_YOLOV2_INIT",NULL,(HI_U64*)&u64PhyAddr,
                                          (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

    /*set each tmp buffer addr*/
    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
                                           u32BboxNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
            u32BboxNum*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
            u32BboxNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*u32BboxNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
            u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+
            u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return s32Ret;
}


/******************************************************************************
* function : Yolov1 init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov2_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Yolov2_SoftwareInit(pstCfg,pstNnieParam,
             pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_SVP_NNIE_Yolov1_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Yolov2_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_Yolov1_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}


/******************************************************************************
* function : show YOLOV2 sample(image 416x416 U8_C3)
******************************************************************************/
void SAMPLE_SVP_NNIE_Yolov2(void)
{
    HI_CHAR *pcSrcFile = "./data/nnie_image/rgb_planar/street_cars_416x416.bgr";
    HI_CHAR *pcModelName = "./data/nnie_model/detection/inst_yolov2_cycle.wk";
    HI_U32 u32PicNum = 1;
    HI_FLOAT f32PrintResultThresh = 0.0f;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    f32PrintResultThresh = 0.25f;
    stNnieCfg.pszPic= pcSrcFile;
    stNnieCfg.u32MaxInputNum = u32PicNum; //max input image num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Yolov2 Load model*/
    SAMPLE_SVP_TRACE_INFO("Yolov2 Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelName,&s_stYolov2Model);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV2_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    /*Yolov2 parameter initialization*/
    /*Yolov2 software parameters are set in SAMPLE_SVP_NNIE_Yolov2_SoftwareInit,
      if user has changed net struct, please make sure the parameter settings in
      SAMPLE_SVP_NNIE_Yolov2_SoftwareInit function are correct*/
    SAMPLE_SVP_TRACE_INFO("Yolov2 parameter initialization!\n");
    s_stYolov2NnieParam.pstModel = &s_stYolov2Model.stModel;
    s32Ret = SAMPLE_SVP_NNIE_Yolov2_ParamInit(&stNnieCfg,&s_stYolov2NnieParam,&s_stYolov2SoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV2_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Yolov2_ParamInit failed!\n");

    /*Fill src data*/
    SAMPLE_SVP_TRACE_INFO("Yolov2 start!\n");
    HI_U32 index = 0;
    while(index++ < M_INTER_LOOP_NUMBER)
    {
        stInputDataIdx.u32SegIdx = 0;
        stInputDataIdx.u32NodeIdx = 0;
        struct timespec stStartSpcTime;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcTime);
        s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stYolov2NnieParam,&stInputDataIdx);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV2_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
        struct timespec stStartSpcForward;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcForward);
        /*NNIE process(process the 0-th segment)*/
        stProcSegIdx.u32SegIdx = 0;
        s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stYolov2NnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV2_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Forward failed!\n");
        struct timespec stStartSpcGetRes;
        clock_gettime(CLOCK_MONOTONIC, &stStartSpcGetRes);
        /*Software process*/
        /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Yolov2_GetResult
         function input datas are correct*/
        s32Ret = SAMPLE_SVP_NNIE_Yolov2_GetResult(&s_stYolov2NnieParam,&s_stYolov2SoftwareParam);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV2_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_Yolov2_GetResult failed!\n");

        /*print result, this sample has 6 classes:
         class 0:background     class 1:Carclass           class 2:Vanclass
         class 3:Truckclass     class 4:Pedestrianclass    class 5:Cyclist*/
        struct timespec stEndSpcTime;
        clock_gettime(CLOCK_MONOTONIC, &stEndSpcTime);
        HI_DOUBLE dbTotalDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stEndSpcTime);
        HI_DOUBLE dbFillSrcDiff = SAMPLE_SVP_GetDiff(&stStartSpcTime,&stStartSpcForward);
        HI_DOUBLE dbFrowardDiff = SAMPLE_SVP_GetDiff(&stStartSpcForward,&stStartSpcGetRes);
        HI_DOUBLE dbGetResDiff = SAMPLE_SVP_GetDiff(&stStartSpcGetRes,&stEndSpcTime);

        SAMPLE_SVP_PrintPerformance("Yolov2",dbTotalDiff,dbFillSrcDiff,dbFrowardDiff,dbGetResDiff);
        fprintf(stderr,"\n");
    }

    SAMPLE_SVP_TRACE_INFO("Yolov2 result:\n");
    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stYolov2SoftwareParam.stDstScore,
            &s_stYolov2SoftwareParam.stDstRoi, &s_stYolov2SoftwareParam.stClassRoiNum,f32PrintResultThresh);


YOLOV2_FAIL_0:
    SAMPLE_SVP_NNIE_Yolov2_Deinit(&s_stYolov2NnieParam,&s_stYolov2SoftwareParam,&s_stYolov2Model);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Yolov2 sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Yolov2_HandleSig(void)
{
    SAMPLE_SVP_NNIE_Yolov2_Deinit(&s_stYolov2NnieParam,&s_stYolov2SoftwareParam,&s_stYolov2Model);
    memset(&s_stYolov2NnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stYolov2SoftwareParam,0,sizeof(SAMPLE_SVP_NNIE_YOLOV2_SOFTWARE_PARAM_S));
    memset(&s_stYolov2Model,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}


/******************************************************************************
* function : Lstm Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Lstm_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParamm,
        SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParamm!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParamm);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                    "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}


/******************************************************************************
* function : Lstm init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Lstm_ParamInit(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
        SAMPLE_SVP_NNIE_PARAM_S *pstLstmPara)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstNnieCfg,pstLstmPara);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);
    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Lstm_Deinit(pstLstmPara,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
                              "Error(%#x),SAMPLE_SVP_NNIE_Lstm_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}

/******************************************************************************
* function : show Lstm sample(vector)
******************************************************************************/
void SAMPLE_SVP_NNIE_Lstm(void)
{
    HI_CHAR *apcSrcFile[3] = {"./data/nnie_image/vector/Seq.SEQ_S32",
                              "./data/nnie_image/vector/Vec1.VEC_S32",
                             "./data/nnie_image/vector/Vec2.VEC_S32"};
    HI_CHAR *pchModelName = "./data/nnie_model/recurrent/lstm_3_3.wk";
    HI_U8* pu8VirAddr = NULL;
    HI_U32 u32SegNum = 0;
    HI_U32 u32Step = 0;
    HI_U32 u32Offset = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 i = 0, j = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_CFG_S   stNnieCfg = {0};
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    stNnieCfg.u32MaxInputNum = 16; //max input data num in each batch
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core
    u32Step = 20; //time step

    /*Sys init*/
    SAMPLE_COMM_SVP_CheckSysInit();

    /*Lstm Load model*/
    SAMPLE_SVP_TRACE_INFO("Lstm Load model!\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pchModelName,&s_stLstmModel);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,LSTM_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_COMM_SVP_NNIE_LoadModel failed!\n");

    /*Lstm step initialization*/
    u32SegNum = s_stLstmModel.stModel.u32NetSegNum;
    u32TotalSize = stNnieCfg.u32MaxInputNum*sizeof(HI_S32)*u32SegNum*2;
    s32Ret = SAMPLE_COMM_SVP_MallocMem("SVP_NNIE_STEP",NULL,(HI_U64*)&s_stLstmNnieParam.stStepBuf.u64PhyAddr,
                                       (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,LSTM_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,Malloc memory failed!\n");
    /*Get step virtual addr*/
    s_stLstmNnieParam.stStepBuf.u64VirAddr = (HI_U64)(pu8VirAddr);
    for(i = 0; i < u32SegNum*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM; i++)
    {
        stNnieCfg.au64StepVirAddr[i] = s_stLstmNnieParam.stStepBuf.u64VirAddr +
                                       i*stNnieCfg.u32MaxInputNum*sizeof(HI_S32);
    }
    /*Set step value, in this sample, the step values are set to be 20,
    if user has changed input network, please set correct step
    values according to the input network*/
    for(i = 0; i < u32SegNum; i++)
    {
        u32Offset = i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM;
        for(j = 0; j < stNnieCfg.u32MaxInputNum; j++)
        {
            *((HI_U32*)(stNnieCfg.au64StepVirAddr[u32Offset])+j) = u32Step;//step of input x_t
            *((HI_U32*)(stNnieCfg.au64StepVirAddr[u32Offset+1])+j) = u32Step;//step of output h_t
        }
    }

    /*Lstm parameter initialization */
    SAMPLE_SVP_TRACE_INFO("Lstm parameter initialization!\n");
    s_stLstmNnieParam.pstModel = &s_stLstmModel.stModel;
    s32Ret = SAMPLE_SVP_NNIE_Lstm_ParamInit(&stNnieCfg,&s_stLstmNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,LSTM_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Lstm_ParamInit failed!\n");

    /*Fill src data, in this sample, the 0-th seg is lstm network,if user has
     changed input network,please make sure the value of stInputDataIdx.u32SegIdx
     is correct*/
    SAMPLE_SVP_TRACE_INFO("Lstm start!\n");
    stInputDataIdx.u32SegIdx = 0;
    for(i = 0; i < s_stLstmNnieParam.pstModel->astSeg[stInputDataIdx.u32SegIdx].u16SrcNum; i++)
    {
        stNnieCfg.pszPic = apcSrcFile[i];
        stInputDataIdx.u32NodeIdx = i;
        s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stLstmNnieParam,&stInputDataIdx);
        SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,LSTM_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                                   "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");
    }

    /*NNIE process(process the 0-th segment)*/
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stLstmNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,LSTM_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

    /*print report result*/
    s32Ret = SAMPLE_SVP_NNIE_PrintReportResult(&s_stLstmNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,LSTM_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
                               "Error,SAMPLE_SVP_NNIE_PrintReportResult failed!\n");

    SAMPLE_SVP_TRACE_INFO("Lstm is successfully processed!\n");

LSTM_FAIL_0:
    SAMPLE_SVP_NNIE_Lstm_Deinit(&s_stLstmNnieParam,&s_stLstmModel);
    SAMPLE_COMM_SVP_CheckSysExit();
}

/******************************************************************************
* function : Lstm sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Lstm_HandleSig(void)
{
    SAMPLE_SVP_NNIE_Lstm_Deinit(&s_stLstmNnieParam,&s_stLstmModel);
    memset(&s_stLstmNnieParam,0,sizeof(SAMPLE_SVP_NNIE_PARAM_S));
    memset(&s_stLstmModel,0,sizeof(SAMPLE_SVP_NNIE_MODEL_S));
    SAMPLE_COMM_SVP_CheckSysExit();
}

