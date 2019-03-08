

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
#include <sys/prctl.h>
#include <sys/ioctl.h>

#include "mpi_vgs.h"
#include "mpi_gdc.h"
#include "hi_comm_gdc.h"
#include "hi_comm_vgs.h"
#include "sample_comm.h"


PAYLOAD_TYPE_E g_enVencType   = PT_H265;
SAMPLE_RC_E    g_enRcMode     = SAMPLE_RC_CBR;
PIC_SIZE_E     g_enPicSize    = PIC_1080P;
DATA_BITWIDTH_E g_enDataWidth = DATA_BITWIDTH_8;

HI_U16 g_au16LMFCoef[128] = {0, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175,
191, 207, 223, 239, 255, 271, 286, 302, 318, 334, 350, 365, 381, 397, 412,
428, 443, 459, 474, 490, 505, 520, 536, 551, 566, 581, 596, 611, 626, 641,
656, 670, 685, 699, 713, 728, 742, 756, 769, 783, 797, 810, 823, 836, 848,
861, 873, 885, 896, 908, 919, 929, 940, 950, 959, 969, 984, 998, 1013, 1027,
1042, 1056, 1071, 1085, 1100, 1114, 1129, 1143, 1158, 1172, 1187, 1201, 1215,
1230, 1244, 1259, 1273, 1288, 1302, 1317, 1331, 1346, 1360, 1375, 1389, 1404,
1418, 1433, 1447, 1462, 1476, 1491, 1505, 1519, 1534, 1548, 1563, 1577, 1592,
1606, 1621, 1635, 1650, 1664, 1679, 1693, 1708, 1722, 1737, 1751, 1766, 1780, 1795, 1809, 1823, 1838};


pthread_t ThreadId;
HI_BOOL bSetFisheyeAttr = HI_FALSE;

typedef struct hiFISHEYE_SET_ATTR_THREAD_INFO
{
    VI_PIPE     ViPipe;
    VI_CHN         ViChn;
}FISHEYE_SET_ATTR_THRD_INFO;

SAMPLE_VO_CONFIG_S g_stVoConfig = {0};

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_FISHEYE_Usage(char* sPrgNm)
{
    printf("Usage : %s <index> <vo intf> <venc type>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0) fisheye 360 panorama 2 half with ceiling mount.\n");
    printf("\t 1) fisheye 360 panorama and 2 normal PTZ with desktop mount.\n");
    printf("\t 2) fisheye 180 panorama and 2 normal dynamic PTZ with wall mount.\n");
    printf("\t 3) fisheye source picture and 3 normal PTZ with wall mount.\n");
    printf("\t 4) nine_lattice preview(Only images larger than or equal to 8M are supported).\n");

    printf("vo intf:\n");
    printf("\t 0) vo HDMI output, default.\n");
    printf("\t 1) vo BT1120 output.\n");

    printf("venc type:\n");
    printf("\t 0) H265, default.\n");
    printf("\t 1) H264.\n");
    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_FISHEYE_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        bSetFisheyeAttr = HI_FALSE;
        SAMPLE_COMM_All_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
 * Function:    SAMPLE_VIO_FISHEYE_StartViVo
 * Description: online mode / offline mode. Embeded isp, phychn preview
******************************************************************************/
static HI_S32 SAMPLE_FISHEYE_StartViVoVenc(SAMPLE_VI_CONFIG_S* pstViConfig,VI_PIPE ViPipe, VI_CHN ViExtChn, VENC_CHN VencChn, VO_CHN VoChn,SIZE_S *pstDstSize)
{
    HI_S32              s32Ret          = HI_SUCCESS;
    VI_CHN_ATTR_S       stChnAttr;
    VI_EXT_CHN_ATTR_S   stExtChnAttr;
    VENC_GOP_ATTR_S     stGopAttr;
    VO_LAYER            VoLayer         = g_stVoConfig.VoDev;
    VI_CHN              ViChn           = pstViConfig->astViInfo[0].stChnInfo.ViChn;
    HI_U32              u32Profile      = 0;

    if(HI_NULL == pstViConfig)
    {
        SAMPLE_PRT("pstViConfig is NULL\n");
        return HI_FAILURE;
    }
    /******************************************
     step 1: start vi dev & chn to capture
    ******************************************/
    memset(&stChnAttr,0,sizeof(VI_CHN_ATTR_S));

    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto exit1;
    }

    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe, ViChn, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get vi chn:%d attr failed with:0x%x!\n", ViChn, s32Ret);
        goto exit2;
    }

    stExtChnAttr.s32BindChn                   = ViChn;
    stExtChnAttr.enCompressMode               = stChnAttr.enCompressMode;
    stExtChnAttr.enPixFormat                  = stChnAttr.enPixelFormat;
    stExtChnAttr.stFrameRate.s32SrcFrameRate  = stChnAttr.stFrameRate.s32SrcFrameRate;
    stExtChnAttr.stFrameRate.s32DstFrameRate  = stChnAttr.stFrameRate.s32DstFrameRate;
    stExtChnAttr.stSize.u32Width              = stChnAttr.stSize.u32Width;
    stExtChnAttr.stSize.u32Height             = stChnAttr.stSize.u32Height;
    stExtChnAttr.u32Depth                     = 0;
    stExtChnAttr.enDynamicRange               = stChnAttr.enDynamicRange;
    stExtChnAttr.enSource                     = VI_EXT_CHN_SOURCE_TAIL;

    pstDstSize->u32Width = stExtChnAttr.stSize.u32Width;
    pstDstSize->u32Height= stExtChnAttr.stSize.u32Height;

    /* start vi dev extern chn */
    s32Ret = HI_MPI_VI_SetExtChnAttr(ViPipe, ViExtChn, &stExtChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set vi extern chn attr failed with: 0x%x.!\n", s32Ret);
        goto exit2;
    }

    /*Enable ext-channel*/
    s32Ret = HI_MPI_VI_EnableChn(ViPipe,ViExtChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("enable vi extern chn failed with: 0x%x.!\n", s32Ret);
        goto exit2;
    }

    /******************************************
    step 2: start VENC
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO Get GopAttr failed!\n");
        goto exit3;
    }

    u32Profile = (0==g_enDataWidth)?0:1;

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, g_enVencType, \
                                        g_enPicSize, g_enRcMode,u32Profile,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VENC failed with %#x!\n", s32Ret);
        goto exit3;
    }

    /******************************************
    step 3: start VO
    ******************************************/
    s32Ret = SAMPLE_COMM_VO_StartVO(&g_stVoConfig);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VO failed with %#x!\n", s32Ret);
        goto exit4;
    }


    /******************************************
    step 4: Bind Venc
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Bind_VENC(ViPipe,ViExtChn, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIU_FISHEYE_BindVenc failed with %#x!\n", s32Ret);
        goto exit5;
    }

    s32Ret = SAMPLE_COMM_VI_Bind_VO(ViPipe,ViExtChn,VoLayer,VoChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("VIO sys bind failed with %#x!\n", s32Ret);
        goto exit1;
    }

    return HI_SUCCESS;

exit5:
    SAMPLE_COMM_VO_StopVO(&g_stVoConfig);
exit4:
    SAMPLE_COMM_VENC_Stop(VencChn);
exit3:
    s32Ret = HI_MPI_VI_DisableChn(ViPipe, ViExtChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn extchn:%d failed with %#x\n", ViExtChn, s32Ret);
    }
 exit2:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
 exit1:
    return s32Ret;
}

HI_S32 SAMPLE_FISHEYE_StopViVoVenc(SAMPLE_VI_CONFIG_S* pstViConfig, VI_PIPE ViPipe, VI_CHN ViExtChn,VO_CHN VoChn, VENC_CHN VencChn)
{
    HI_S32   s32Ret     = HI_SUCCESS;
    VO_LAYER VoLayer = g_stVoConfig.VoDev;

    s32Ret = HI_MPI_VI_DisableChn(ViPipe,ViExtChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn extchn:%d failed with %#x\n", ViExtChn, s32Ret);
        return HI_FAILURE;
    }

    SAMPLE_COMM_VI_UnBind_VENC(ViPipe, ViExtChn, VencChn);
    SAMPLE_COMM_VI_UnBind_VO(ViPipe,ViExtChn,VoLayer,VoChn);

    SAMPLE_COMM_VO_StopVO(&g_stVoConfig);

    SAMPLE_COMM_VI_StopVi(pstViConfig);

    return HI_SUCCESS;
}

HI_S32 SAMPLE_FISHEYE_StartViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VO_CONFIG_S* pstVoConfig)
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

HI_S32 SAMPLE_FISHEYE_StopViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VO_CONFIG_S* pstVoConfig)
{
    SAMPLE_COMM_VO_StopVO(pstVoConfig);

    SAMPLE_COMM_VI_StopVi(pstViConfig);

    return HI_SUCCESS;
}
static HI_VOID *SAMPLE_Proc_SetFisheyeAttrThread(HI_VOID *arg)
{
    HI_S32      i       = 0;
    VI_PIPE     ViPipe  = 0;
    VI_CHN      ViChn   = 0;
    HI_S32      s32Ret  = HI_SUCCESS;
    FISHEYE_ATTR_S  stFisheyeAttr;
    FISHEYE_SET_ATTR_THRD_INFO     *pstFisheyeAttrThreadInfo = HI_NULL;

    if(HI_NULL == arg)
    {
        printf("arg is NULL\n");
        return HI_NULL;
    }

    prctl(PR_SET_NAME, "FISHEYE_Cruise", 0,0,0);

    memset(&stFisheyeAttr,0,sizeof(FISHEYE_ATTR_S));

    pstFisheyeAttrThreadInfo = (FISHEYE_SET_ATTR_THRD_INFO *)arg;

    ViPipe  = pstFisheyeAttrThreadInfo->ViPipe;
    ViChn   = pstFisheyeAttrThreadInfo->ViChn;

    while (HI_TRUE == bSetFisheyeAttr)
    {
        s32Ret = HI_MPI_VI_GetExtChnFisheye(ViPipe, ViChn, &stFisheyeAttr);
        if (HI_SUCCESS != s32Ret)
        {
            return HI_NULL;
        }

        for (i = 1; i < 3; i++)
        {
            if (360 == stFisheyeAttr.astFishEyeRegionAttr[i].u32Pan)
            {
                stFisheyeAttr.astFishEyeRegionAttr[i].u32Pan = 0;
            }
            else
            {
                stFisheyeAttr.astFishEyeRegionAttr[i].u32Pan++;
            }

            if (360 == stFisheyeAttr.astFishEyeRegionAttr[i].u32Tilt)
            {
                stFisheyeAttr.astFishEyeRegionAttr[i].u32Tilt = 0;
            }
            else
            {
                stFisheyeAttr.astFishEyeRegionAttr[i].u32Tilt++;
            }
        }

        s32Ret = HI_MPI_VI_SetExtChnFisheye(ViPipe,ViChn,&stFisheyeAttr);
        if (HI_SUCCESS != s32Ret)
        {
            return HI_NULL;
        }

        sleep(1);

    #if 0
        printf("Func:%s, line:%d, pan1:%d, tilt1:%d, pan2:%d, tilt2:%d\n", __FUNCTION__, __LINE__,
        stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan, stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt,
        stFisheyeAttr.astFishEyeRegionAttr[2].u32Pan, stFisheyeAttr.astFishEyeRegionAttr[2].u32Tilt);
    #endif
    }

    return HI_NULL;
}

HI_S32 SAMPLE_FISHEYE_StartSetFisheyeAttrThrd(VI_PIPE ViPipe, VI_CHN ViChn)
{
    FISHEYE_SET_ATTR_THRD_INFO stFisheyeAttrThreadInfo;

    stFisheyeAttrThreadInfo.ViPipe  = ViPipe;
    stFisheyeAttrThreadInfo.ViChn   = ViChn;

    bSetFisheyeAttr = HI_TRUE;

    pthread_create(&ThreadId, HI_NULL, SAMPLE_Proc_SetFisheyeAttrThread, &stFisheyeAttrThreadInfo);

    sleep(1);

    return HI_SUCCESS;
}

HI_S32 SAMPLE_FISHEYE_StopSetFisheyeAttrThrd(void)
{
    if (HI_FALSE != bSetFisheyeAttr)
    {
        bSetFisheyeAttr = HI_FALSE;
        pthread_join(ThreadId, HI_NULL);
    }

    return HI_SUCCESS;
}

static HI_VOID *SAMPLE_FISHEYE_Nine_Lattice_Thread(HI_VOID *arg)
{
    HI_S32              s32Ret          = HI_SUCCESS;
    GDC_HANDLE          hHandle;
    GDC_TASK_ATTR_S     stTask;
    VGS_TASK_ATTR_S     stVgsTask;
    FISHEYE_ATTR_S      stFisheyeAttr;
    VI_CHN_ATTR_S       stChnAttr       = {0};
    HI_U32              u32BufSize;
    HI_CHAR             *pMmzName       = HI_NULL;
    HI_U64              u64OutPhyAddr;
    HI_U8               *pu8OutVirtAddr;
    VB_BLK              vbOutBlk;
    HI_U32              u32Width;
    HI_U32              u32Height;
    HI_U32              u32OutWidth ;
    HI_U32              u32OutHeight;
    HI_U32              u32OutStride;
    HI_U32              u32XAlign       = 0;
    HI_U64              u64AddrAlign       = 0;
    VI_PIPE             ViPipe          = 0;
    VI_CHN              ViChn           = 0;
    HI_U32              u32OldDepth     = 2;
    HI_U32              u32Depth        = 2;
    VO_LAYER            VoLayer         = g_stVoConfig.VoDev;
    VO_CHN              VoChn           = 0;
    HI_S32              s32MilliSec     = -1;
    SIZE_S              *pstSize;

    if(NULL == arg)
    {
        SAMPLE_PRT("arg is NULL\n");
        return HI_NULL;
    }

    prctl(PR_SET_NAME, "FISHEYE_Frame", 0,0,0);

    pstSize         = (SIZE_S *)arg;

    if(DATA_BITWIDTH_10 ==  g_enDataWidth)
    {
        u32XAlign       = 64;
        u64AddrAlign    = 80;   //u64AddrAlign = u32XAlign*10/8;

        u32Width        = pstSize->u32Width;
        u32Height       = pstSize->u32Height;

        u32OutWidth     = pstSize->u32Width;
        u32OutHeight    = pstSize->u32Height;
        u32OutStride    = ALIGN_UP(pstSize->u32Width*5/4, u64AddrAlign);
    }
    else
    {
        u32XAlign       = 16;
        u64AddrAlign    = 16;

        u32Width        = pstSize->u32Width;
        u32Height       = pstSize->u32Height;

        u32OutWidth     = pstSize->u32Width;
        u32OutHeight    = pstSize->u32Height;
        u32OutStride    = ALIGN_UP(pstSize->u32Width, u64AddrAlign);
    }

    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe,ViChn,&stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
    }

    u32OldDepth         = stChnAttr.u32Depth;
    stChnAttr.u32Depth  = u32Depth;

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe,ViChn,&stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
    }

    while ( HI_TRUE == bSetFisheyeAttr )
    {
        s32Ret = HI_MPI_GDC_BeginJob(&hHandle);
        if(s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            return HI_NULL;
        }

        u32BufSize = COMMON_GetPicBufferSize(u32Width, u32Height,PIXEL_FORMAT_YVU_SEMIPLANAR_420,g_enDataWidth,COMPRESS_MODE_NONE, 0);

        vbOutBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32BufSize, pMmzName);
        if (VB_INVALID_HANDLE == vbOutBlk)
        {
            SAMPLE_PRT("Info:HI_MPI_VB_GetBlock(size:%d) fail\n", u32BufSize);
            return HI_NULL;
        }

        u64OutPhyAddr = HI_MPI_VB_Handle2PhysAddr(vbOutBlk);
        if (0 == u64OutPhyAddr)
        {
            SAMPLE_PRT("Info:HI_MPI_VB_Handle2PhysAddr fail, u32OutPhyAddr:0x%llx\n",u64OutPhyAddr);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        pu8OutVirtAddr = (HI_U8 *)HI_MPI_SYS_Mmap(u64OutPhyAddr, u32BufSize);
        if (NULL == pu8OutVirtAddr)
        {
            SAMPLE_PRT("Info:HI_MPI_SYS_Mmap fail, u8OutVirtAddr:0x%llx\n",(HI_U64)pu8OutVirtAddr);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        s32Ret = HI_MPI_VI_GetChnFrame(ViPipe, ViChn, &stTask.stImgIn, s32MilliSec);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        memcpy(&stTask.stImgOut, &stTask.stImgIn, sizeof(VIDEO_FRAME_INFO_S));

        stTask.stImgOut.u32PoolId                  = HI_MPI_VB_Handle2PoolId(vbOutBlk);
        stTask.stImgOut.stVFrame.u64PhyAddr[0]   = u64OutPhyAddr;
        stTask.stImgOut.stVFrame.u64PhyAddr[1]   = u64OutPhyAddr + u32OutStride * u32OutHeight;
        stTask.stImgOut.stVFrame.u64PhyAddr[2]   = u64OutPhyAddr + u32OutStride * u32OutHeight;
        stTask.stImgOut.stVFrame.u64VirAddr[0]   = (HI_UL)pu8OutVirtAddr;
        stTask.stImgOut.stVFrame.u64VirAddr[1]   = (HI_UL)pu8OutVirtAddr + u32OutStride * u32OutHeight;
        stTask.stImgOut.stVFrame.u64VirAddr[2]   = (HI_UL)pu8OutVirtAddr + u32OutStride * u32OutHeight;
        stTask.stImgOut.stVFrame.u32Stride[0]    = u32OutStride;
        stTask.stImgOut.stVFrame.u32Stride[1]    = u32OutStride;
        stTask.stImgOut.stVFrame.u32Stride[2]    = u32OutStride;
        stTask.stImgOut.stVFrame.u32Width         = u32OutWidth;
        stTask.stImgOut.stVFrame.u32Height         = u32OutHeight;

        stFisheyeAttr.bEnable           = HI_TRUE;
        stFisheyeAttr.bLMF              = HI_FALSE;
        stFisheyeAttr.bBgColor          = HI_TRUE;
        stFisheyeAttr.u32BgColor        = COLOR_RGB_BLUE;
        stFisheyeAttr.s32HorOffset      = 0;
        stFisheyeAttr.s32VerOffset      = 0;
        stFisheyeAttr.u32TrapezoidCoef  = 0;
        stFisheyeAttr.s32FanStrength    = 0;
        stFisheyeAttr.enMountMode       = FISHEYE_WALL_MOUNT;
        stFisheyeAttr.u32RegionNum      = 4;

        stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode          = FISHEYE_VIEW_180_PANORAMA;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X      = 0;
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y      = 0;
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height = u32Height/3;

        stFisheyeAttr.astFishEyeRegionAttr[1].enViewMode          = FISHEYE_VIEW_NORMAL;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32X      = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32Y      = 0;
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        stFisheyeAttr.astFishEyeRegionAttr[2].enViewMode          = FISHEYE_VIEW_NORMAL;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32X      = ALIGN_DOWN(u32Width*2/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32Y      = 0;
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        stFisheyeAttr.astFishEyeRegionAttr[3].enViewMode          = FISHEYE_VIEW_NORMAL;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.s32X      = 0;
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.s32Y      = ALIGN_DOWN(u32Height/3,2);
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        s32Ret = HI_MPI_GDC_AddCorrectionTask(hHandle, &stTask, &stFisheyeAttr);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        stFisheyeAttr.bEnable           = HI_TRUE;
        stFisheyeAttr.bLMF              = HI_FALSE;
        stFisheyeAttr.bBgColor          = HI_TRUE;
        stFisheyeAttr.u32BgColor        = COLOR_RGB_BLUE;
        stFisheyeAttr.s32HorOffset      = 0;
        stFisheyeAttr.s32VerOffset      = 0;
        stFisheyeAttr.u32TrapezoidCoef  = 0;
        stFisheyeAttr.s32FanStrength    = 0;
        stFisheyeAttr.enMountMode       = FISHEYE_WALL_MOUNT;
        stFisheyeAttr.u32RegionNum      = 4;

        stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode          = FISHEYE_VIEW_180_PANORAMA;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X      = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y      = ALIGN_DOWN(u32Height/3,2);
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        stFisheyeAttr.astFishEyeRegionAttr[1].enViewMode          = FISHEYE_VIEW_NORMAL;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[1].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32X      = ALIGN_DOWN(u32Width*2/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32Y      = ALIGN_DOWN(u32Height/3,2);
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        stFisheyeAttr.astFishEyeRegionAttr[2].enViewMode          = FISHEYE_VIEW_NORMAL;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[2].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32X      = 0;
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32Y      = ALIGN_DOWN(u32Height*2/3,2);
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        stFisheyeAttr.astFishEyeRegionAttr[3].enViewMode          = FISHEYE_VIEW_NORMAL;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32InRadius         = 0;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32OutRadius        = 1200;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32Pan              = 180;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32Tilt             = 180;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32HorZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[3].u32VerZoom          = 4095;
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.s32X      = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.s32Y      = ALIGN_DOWN(u32Height*2/3,2);
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.u32Width  = ALIGN_DOWN(u32Width/3,u32XAlign);
        stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.u32Height = ALIGN_DOWN(u32Height/3,2);

        s32Ret = HI_MPI_GDC_AddCorrectionTask(hHandle, &stTask, &stFisheyeAttr);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        s32Ret = HI_MPI_GDC_EndJob(hHandle);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        /*Add Scale Task*/
        s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }


        memcpy(&stVgsTask.stImgIn, &stTask.stImgIn, sizeof(VIDEO_FRAME_INFO_S));
        memcpy(&stVgsTask.stImgOut, &stTask.stImgIn, sizeof(VIDEO_FRAME_INFO_S));

        stVgsTask.stImgOut.u32PoolId                = HI_MPI_VB_Handle2PoolId(vbOutBlk);
        stVgsTask.stImgOut.stVFrame.u32Width        = ALIGN_DOWN(u32Width/3,u32XAlign);
        stVgsTask.stImgOut.stVFrame.u32Height        = ALIGN_DOWN(u32Height/3,2);
        stVgsTask.stImgOut.stVFrame.enField            = VIDEO_FIELD_FRAME;
        stVgsTask.stImgOut.stVFrame.enPixelFormat    = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        stVgsTask.stImgOut.stVFrame.u32Stride[0]    = u32OutStride;
        stVgsTask.stImgOut.stVFrame.u32Stride[1]    = u32OutStride;
        stVgsTask.stImgOut.stVFrame.u32Stride[2]    = u32OutStride;

        if (DATA_BITWIDTH_10 ==  g_enDataWidth)
        {
            stVgsTask.stImgOut.stVFrame.u64PhyAddr[0]    = u64OutPhyAddr + (HI_U64)u32OutStride * u32OutHeight * 2 / 3 + ALIGN_DOWN(u32OutWidth *2/3,u32XAlign)* 5/4;
            stVgsTask.stImgOut.stVFrame.u64PhyAddr[1]    = u64OutPhyAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign)* 5/4;
            stVgsTask.stImgOut.stVFrame.u64PhyAddr[2]    = u64OutPhyAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign)* 5/4;
            stVgsTask.stImgOut.stVFrame.u64VirAddr[0]    = (HI_U64)pu8OutVirtAddr + (HI_U64)u32OutStride * u32OutHeight * 2 / 3 + ALIGN_DOWN(u32OutWidth *2/3,u32XAlign)* 5/4;
            stVgsTask.stImgOut.stVFrame.u64VirAddr[1]    = (HI_U64)pu8OutVirtAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign)* 5/4;
            stVgsTask.stImgOut.stVFrame.u64VirAddr[2]    = (HI_U64)pu8OutVirtAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign)* 5/4;
        }
        else
        {
            stVgsTask.stImgOut.stVFrame.u64PhyAddr[0]    = u64OutPhyAddr + (HI_U64)u32OutStride * u32OutHeight * 2 / 3 + ALIGN_DOWN(u32OutWidth *2/3,u32XAlign);
            stVgsTask.stImgOut.stVFrame.u64PhyAddr[1]    = u64OutPhyAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign);
            stVgsTask.stImgOut.stVFrame.u64PhyAddr[2]    = u64OutPhyAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign);
            stVgsTask.stImgOut.stVFrame.u64VirAddr[0]    = (HI_U64)pu8OutVirtAddr + (HI_U64)u32OutStride * u32OutHeight * 2 / 3 + ALIGN_DOWN(u32OutWidth *2/3,u32XAlign);
            stVgsTask.stImgOut.stVFrame.u64VirAddr[1]    = (HI_U64)pu8OutVirtAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign);
            stVgsTask.stImgOut.stVFrame.u64VirAddr[2]    = (HI_U64)pu8OutVirtAddr + (HI_U64)u32OutStride * u32OutHeight + ((HI_U64)u32OutStride * u32OutHeight * 2/3) / 2 +  ALIGN_DOWN(u32OutWidth*2/3,u32XAlign);
        }

        s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stVgsTask, VGS_SCLCOEF_NORMAL);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        s32Ret = HI_MPI_VGS_EndJob(hHandle);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
            HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        s32Ret = HI_MPI_VO_SendFrame(VoLayer, VoChn, &stTask.stImgOut, s32MilliSec);
        if (HI_SUCCESS != s32Ret)
        {
            continue;
        }

        s32Ret = HI_MPI_VI_ReleaseChnFrame(ViPipe,ViChn,&stTask.stImgIn);
        if(HI_SUCCESS != s32Ret)
        {
            printf("Info:HI_MPI_VI_ReleaseFrame fail, s32Ret:0x%x\n",s32Ret );
            HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }

        s32Ret = HI_MPI_SYS_Munmap(pu8OutVirtAddr,u32BufSize);
        if(HI_SUCCESS != s32Ret)
        {
            printf("Info:HI_MPI_SYS_Munmap fail,s32Ret:0x%x\n", s32Ret);
            HI_MPI_VB_ReleaseBlock(vbOutBlk);
            return HI_NULL;
        }


        s32Ret = HI_MPI_VB_ReleaseBlock(vbOutBlk);
        if(HI_SUCCESS != s32Ret)
        {
            printf("Info:HI_MPI_VB_ReleaseBlock fail,s32Ret:0x%x\n", s32Ret);
            return HI_NULL;
        }

        usleep(20000);
    }

    stChnAttr.u32Depth = u32OldDepth;
    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe,ViChn,&stChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Err, s32Ret:0x%x\n",s32Ret);
    }
    return HI_NULL;
}

HI_S32 SAMPLE_FISHEYE_StopSwitchModeThrd(void)
{
    if (HI_FALSE != bSetFisheyeAttr)
    {
        bSetFisheyeAttr = HI_FALSE;
        pthread_join(ThreadId, NULL);
    }

    return HI_SUCCESS;
}


/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_FISHEYE_360Panorama_Celing_2half(SAMPLE_VI_CONFIG_S* pstViConfig,VI_PIPE ViPipe)
{
    VI_CHN              ViExtChn        = VI_EXT_CHN_START;
    VENC_CHN            VencChn         = 0;
    VO_CHN              VoChn           = 0;
    SIZE_S              stSize;
    HI_U64              u64BlkSize;
    VB_CONFIG_S         stVbConf;
    PIC_SIZE_E          enPicSize       = g_enPicSize;
    DATA_BITWIDTH_E     enBitWidth      = g_enDataWidth;
    HI_S32              s32ChnNum       = 1;
    HI_S32              s32Ret          = HI_SUCCESS;
    SIZE_S              stDstSize;
    FISHEYE_CONFIG_S    stFisheyeConfig;
    FISHEYE_ATTR_S      stFisheyeAttr;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step  1: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    /* comm video buffer */
    u64BlkSize = COMMON_GetPicBufferSize(stSize.u32Width,stSize.u32Height,SAMPLE_PIXEL_FORMAT,enBitWidth,COMPRESS_MODE_NONE, 0);
    stVbConf.astCommPool[0].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 10;

    /*vb for vi raw*/
    u64BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, HI_FALSE, 0);
    stVbConf.astCommPool[1].u64BlkSize  = u64BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  1: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_FISHEYE_StartViVoVenc(pstViConfig,ViPipe, ViExtChn, VencChn, VoChn,&stDstSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;
    }


     /******************************************
     step   2: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VencChn,s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed with %d\n", s32Ret);
        goto exit1;
    }


    /******************************************
      step  3: set fisheye Attr
     ******************************************/
    memcpy(stFisheyeConfig.au16LMFCoef,g_au16LMFCoef,sizeof(g_au16LMFCoef));

    stFisheyeAttr.bEnable              = HI_TRUE;
    stFisheyeAttr.bLMF                 = HI_TRUE;
    stFisheyeAttr.bBgColor             = HI_TRUE;
    stFisheyeAttr.u32BgColor           = COLOR_RGB_BLUE;
    stFisheyeAttr.s32HorOffset         = 0;
    stFisheyeAttr.s32FanStrength       = 0;
    stFisheyeAttr.s32VerOffset         = 0;
    stFisheyeAttr.u32TrapezoidCoef     = 0;
    stFisheyeAttr.s32FanStrength       = 0;
    stFisheyeAttr.enMountMode          = FISHEYE_CEILING_MOUNT;
    stFisheyeAttr.u32RegionNum         = 2;

    stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode             = FISHEYE_VIEW_360_PANORAMA;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius            = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius           = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan                 = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom             = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom             = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X         = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y         = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width     = stDstSize.u32Width;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height     = ALIGN_DOWN(stDstSize.u32Height/3, 2);

    stFisheyeAttr.astFishEyeRegionAttr[1].enViewMode             = FISHEYE_VIEW_360_PANORAMA;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32InRadius            = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32OutRadius           = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan                 = 180;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32HorZoom             = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32VerZoom             = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32X         = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32Y         = ALIGN_DOWN(stDstSize.u32Height/3, 2);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Width     = stDstSize.u32Width;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Height     = ALIGN_DOWN(stDstSize.u32Height/3, 2);

    s32Ret = HI_MPI_VI_SetPipeFisheyeConfig(ViPipe,&stFisheyeConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye config failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    printf("\nplease press enter, disable fisheye\n\n");
    getchar();

    stFisheyeAttr.bEnable = HI_FALSE;
    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    printf("\nplease press enter, enable fisheye\n");
    getchar();

    stFisheyeAttr.bEnable = HI_TRUE;
    s32Ret = HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    PAUSE();

    SAMPLE_COMM_VENC_StopGetStream();

exit1:
    SAMPLE_FISHEYE_StopViVoVenc(pstViConfig,ViPipe, ViExtChn, VencChn,VoChn);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}

/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_FISHEYE_360Panorama_Desktop_and_2normal(SAMPLE_VI_CONFIG_S* pstViConfig, VI_PIPE ViPipe)
{
    VI_CHN          ViExtChn    = VI_EXT_CHN_START;
    VO_CHN          VoChn       = 0;
    VENC_CHN        VencChn     = 0;
    HI_U32          u32XAlign   = 0;
    SIZE_S          stSize;
    HI_U32          u64BlkSize;
    VB_CONFIG_S     stVbConf;
    PIC_SIZE_E      enPicSize   = g_enPicSize;
    DATA_BITWIDTH_E enBitWidth  = g_enDataWidth;
    HI_S32          s32ChnNum   = 1;
    HI_S32          s32Ret      = HI_SUCCESS;
    SIZE_S          stDstSize;
    FISHEYE_ATTR_S  stFisheyeAttr;

    if(DATA_BITWIDTH_10 ==  enBitWidth)
    {
        u32XAlign = 64;
    }
    else
    {
        u32XAlign = 16;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize( enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step    1: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    /* comm video buffer */
    u64BlkSize = COMMON_GetPicBufferSize(stSize.u32Width,stSize.u32Height,SAMPLE_PIXEL_FORMAT,enBitWidth,HI_FALSE,0);
    stVbConf.astCommPool[0].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 15;

    /*vb for vi raw*/
    u64BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, HI_FALSE, 0);
    stVbConf.astCommPool[1].u64BlkSize  = u64BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;


    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  2: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_FISHEYE_StartViVoVenc(pstViConfig,ViPipe, ViExtChn, VencChn, VoChn, &stDstSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;
    }

     /******************************************
     step   3: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VencChn,s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;
    }

    /******************************************
      step  4: set fisheye Attr
     ******************************************/

    stFisheyeAttr.bEnable            = HI_TRUE;
    stFisheyeAttr.bLMF                = HI_FALSE;
    stFisheyeAttr.bBgColor            = HI_TRUE;
    stFisheyeAttr.u32BgColor        = COLOR_RGB_BLUE;
    stFisheyeAttr.s32HorOffset        = 0;
    stFisheyeAttr.s32FanStrength    = 0;
    stFisheyeAttr.s32VerOffset        = 0;
    stFisheyeAttr.u32TrapezoidCoef    = 0;
    stFisheyeAttr.s32FanStrength     = 0;
    stFisheyeAttr.enMountMode        = FISHEYE_DESKTOP_MOUNT;
    stFisheyeAttr.u32RegionNum        = 3;

    stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode            = FISHEYE_VIEW_360_PANORAMA;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius            = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius            =  1200;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width    = stDstSize.u32Width;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height    =  ALIGN_DOWN(stDstSize.u32Height/3, 2);

    stFisheyeAttr.astFishEyeRegionAttr[1].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32InRadius            = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32OutRadius            =  1200;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan                = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt                = 90;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32HorZoom            = 2048;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32X        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32Y        =  ALIGN_DOWN(stDstSize.u32Height/3, 2);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Width    =  ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Height    =  ALIGN_DOWN(stDstSize.u32Height/2, 2);

    stFisheyeAttr.astFishEyeRegionAttr[2].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32InRadius            = 0;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32OutRadius            = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32Pan                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32Tilt                = 270;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32HorZoom            = 2048;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32X        = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32Y        = ALIGN_DOWN(stDstSize.u32Height/3, 2);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Height    = ALIGN_DOWN(stDstSize.u32Height/2, 2);


    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    printf("\nplease press enter, disable fisheye\n\n");
    getchar();

    stFisheyeAttr.bEnable = HI_FALSE;
    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    printf("\nplease press enter, enable fisheye\n");
    getchar();

    stFisheyeAttr.bEnable = HI_TRUE;
    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    PAUSE();
    SAMPLE_COMM_VENC_StopGetStream();

exit1:
    SAMPLE_FISHEYE_StopViVoVenc(pstViConfig, ViPipe, ViExtChn,  VencChn,VoChn);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}


/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_FISHEYE_180Panorama_Wall_and_2DynamicNormal(SAMPLE_VI_CONFIG_S* pstViConfig,VI_PIPE ViPipe)
{
    VI_CHN              ViExtChn        = VI_EXT_CHN_START;
    VENC_CHN            VencChn         = 0;
    VO_CHN              VoChn           = 0;
    HI_U32              u32XAlign       = 0;
    SIZE_S              stSize;
    HI_U32              u64BlkSize;
    VB_CONFIG_S         stVbConf;
    PIC_SIZE_E          enPicSize       = g_enPicSize;
    DATA_BITWIDTH_E     enBitWidth      = g_enDataWidth;
    HI_S32              s32ChnNum       = 1;
    HI_S32              s32Ret          = HI_SUCCESS;
    SIZE_S              stDstSize;
    FISHEYE_ATTR_S      stFisheyeAttr;

    if(DATA_BITWIDTH_10 ==  enBitWidth)
    {
        u32XAlign = 64;
    }
    else
    {
        u32XAlign = 16;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize( enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step 1: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    /* comm video buffer */
    u64BlkSize = COMMON_GetPicBufferSize(stSize.u32Width,stSize.u32Height,SAMPLE_PIXEL_FORMAT,enBitWidth,COMPRESS_MODE_NONE,0);
    stVbConf.astCommPool[0].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 15;

    /*vb for vi raw*/
    u64BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, HI_FALSE, 0);
    stVbConf.astCommPool[1].u64BlkSize  = u64BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  2: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_FISHEYE_StartViVoVenc(pstViConfig, ViPipe, ViExtChn, VencChn,VoChn,&stDstSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;
    }

     /******************************************
     step   3: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VencChn,s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;
    }

    /******************************************
      step  4: set fisheye Attr
     ******************************************/

    stFisheyeAttr.bEnable             = HI_TRUE;
    stFisheyeAttr.bLMF                = HI_FALSE;
    stFisheyeAttr.bBgColor            = HI_FALSE;
    stFisheyeAttr.u32BgColor          = COLOR_RGB_BLUE;
    stFisheyeAttr.s32HorOffset        = 0;
    stFisheyeAttr.s32VerOffset        = 0;
    stFisheyeAttr.u32TrapezoidCoef    = 10;
    stFisheyeAttr.s32FanStrength      = 300;
    stFisheyeAttr.enMountMode         = FISHEYE_WALL_MOUNT;
    stFisheyeAttr.u32RegionNum        = 3;

    stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode            = FISHEYE_VIEW_180_PANORAMA;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt               = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width    = stDstSize.u32Width;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height    = ALIGN_DOWN(stDstSize.u32Height/2, 2);

    stFisheyeAttr.astFishEyeRegionAttr[1].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt               = 180;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32HorZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32X        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32Y        = ALIGN_DOWN(stDstSize.u32Height/2, 2);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Height   = ALIGN_DOWN(stDstSize.u32Height/2, 2);

    stFisheyeAttr.astFishEyeRegionAttr[2].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32Pan                = 200;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32Tilt               = 200;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32HorZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32X        = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32Y        = ALIGN_DOWN(stDstSize.u32Height/2, 2);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Height   = ALIGN_DOWN(stDstSize.u32Height/2, 2);


    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    /* create a pthread to change the fisheye attr */
    SAMPLE_FISHEYE_StartSetFisheyeAttrThrd(ViPipe, ViExtChn);

    PAUSE();

    SAMPLE_COMM_VENC_StopGetStream();
    SAMPLE_FISHEYE_StopSetFisheyeAttrThrd();

exit1:
    SAMPLE_FISHEYE_StopViVoVenc(pstViConfig, ViPipe, ViExtChn, VencChn,VoChn);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}


/******************************************************************************
* function : vi/vpss: offline/online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_FISHEYE_Source_and_3Normal(SAMPLE_VI_CONFIG_S* pstViConfig,VI_PIPE ViPipe)
{
    VI_CHN             ViExtChn    = VI_EXT_CHN_START;
    VENC_CHN           VencChn     = 0;
    VO_CHN             VoChn       = 0;
    HI_U32             u32XAlign   = 0;
    SIZE_S             stSize;
    HI_U32             u64BlkSize;
    VB_CONFIG_S        stVbConf;
    PIC_SIZE_E         enPicSize   = g_enPicSize;
    DATA_BITWIDTH_E    enBitWidth  = g_enDataWidth;
    HI_S32             s32ChnNum   = 1;
    HI_S32             s32Ret      = HI_SUCCESS;
    SIZE_S             stDstSize;
    FISHEYE_ATTR_S     stFisheyeAttr;

    if(DATA_BITWIDTH_10 ==  enBitWidth)
    {
        u32XAlign = 64;
    }
    else
    {
        u32XAlign = 16;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step    1: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    /* comm video buffer */
    u64BlkSize = COMMON_GetPicBufferSize(stSize.u32Width,stSize.u32Height,SAMPLE_PIXEL_FORMAT,enBitWidth,COMPRESS_MODE_NONE,0);
    stVbConf.astCommPool[0].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  =  15;

    /*vb for vi raw*/
    u64BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, HI_FALSE, 0);
    stVbConf.astCommPool[1].u64BlkSize  = u64BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }


    /******************************************
      step  1: start VI VO  VENC
     ******************************************/
    s32Ret = SAMPLE_FISHEYE_StartViVoVenc(pstViConfig,ViPipe, ViExtChn, VencChn,VoChn,&stDstSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_FISHEYE_StartViVo failed witfh %d\n", s32Ret);
        goto exit;
    }

     /******************************************
     step   2: stream venc process -- get stream, then save it to file.
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VencChn,s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_StartGetStream failed witfh %d\n", s32Ret);
        goto exit1;
    }

    /******************************************
      step  3: set fisheye Attr
     ******************************************/

    stFisheyeAttr.bEnable             = HI_TRUE;
    stFisheyeAttr.bLMF                = HI_FALSE;
    stFisheyeAttr.bBgColor            = HI_TRUE;
    stFisheyeAttr.u32BgColor          = COLOR_RGB_BLUE;
    stFisheyeAttr.s32HorOffset        = 0;
    stFisheyeAttr.s32VerOffset        = 0;
    stFisheyeAttr.u32TrapezoidCoef    = 10;
    stFisheyeAttr.s32FanStrength      = 0;
    stFisheyeAttr.enMountMode         = FISHEYE_WALL_MOUNT;
    stFisheyeAttr.u32RegionNum        = 4;

    stFisheyeAttr.astFishEyeRegionAttr[0].enViewMode            = FISHEYE_NO_TRANSFORMATION;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Pan                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32Tilt                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32HorZoom            = 2048;
    stFisheyeAttr.astFishEyeRegionAttr[0].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32X        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.s32Y        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height    = ALIGN_DOWN(stDstSize.u32Height/2, 2);

    stFisheyeAttr.astFishEyeRegionAttr[1].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Pan                = 180;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32Tilt               = 135;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32HorZoom            = 2048;
    stFisheyeAttr.astFishEyeRegionAttr[1].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32X        = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.s32Y        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[1].stOutRect.u32Height   = ALIGN_DOWN(stDstSize.u32Height/2, 2);

    stFisheyeAttr.astFishEyeRegionAttr[2].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32Pan                = 135;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32Tilt               = 180;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32HorZoom            = 2048;
    stFisheyeAttr.astFishEyeRegionAttr[2].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32X        = 0;
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.s32Y        = ALIGN_DOWN(stDstSize.u32Height/2, 2);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[2].stOutRect.u32Height   = ALIGN_DOWN(stDstSize.u32Height/2, 2);

    stFisheyeAttr.astFishEyeRegionAttr[3].enViewMode            = FISHEYE_VIEW_NORMAL;
    stFisheyeAttr.astFishEyeRegionAttr[3].u32InRadius           = 0;
    stFisheyeAttr.astFishEyeRegionAttr[3].u32OutRadius          = 1200;
    stFisheyeAttr.astFishEyeRegionAttr[3].u32Pan                = 215;
    stFisheyeAttr.astFishEyeRegionAttr[3].u32Tilt               = 180;
    stFisheyeAttr.astFishEyeRegionAttr[3].u32HorZoom            = 2048;
    stFisheyeAttr.astFishEyeRegionAttr[3].u32VerZoom            = 4095;
    stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.s32X        = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.s32Y        = ALIGN_DOWN(stDstSize.u32Height/2, 2);
    stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.u32Width    = ALIGN_DOWN(stDstSize.u32Width/2, u32XAlign);
    stFisheyeAttr.astFishEyeRegionAttr[3].stOutRect.u32Height   = ALIGN_DOWN(stDstSize.u32Height/2, 2);

    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    printf("\nplease press enter, disable fisheye\n\n");
    getchar();

    stFisheyeAttr.bEnable = HI_FALSE;
    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    printf("\nplease press enter, enable fisheye\n");
    getchar();

    stFisheyeAttr.bEnable = HI_TRUE;
    s32Ret =  HI_MPI_VI_SetExtChnFisheye(ViPipe,ViExtChn,&stFisheyeAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set fisheye attr failed with s32Ret:0x%x!\n", s32Ret);
        goto exit1;
    }

    PAUSE();

    SAMPLE_COMM_VENC_StopGetStream();

exit1:
    SAMPLE_FISHEYE_StopViVoVenc(pstViConfig, ViPipe, ViExtChn, VencChn,VoChn);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}

/******************************************************************************
* function : vi/: online fisheye mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_FISHEYE_Nine_Lattice(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    SIZE_S            stSize;
    HI_U32            u64BlkSize;
    VB_CONFIG_S        stVbConf;
    PIC_SIZE_E        enPicSize   = g_enPicSize;
    DATA_BITWIDTH_E enBitWidth  = g_enDataWidth;
    HI_S32          s32Ret      = HI_SUCCESS;

    if(HI_NULL == pstViConfig)
    {
        SAMPLE_PRT("pstViConfig is NULL!\n");
        return HI_FAILURE;
    }

    /******************************************
      step    1: mpp system init
     ******************************************/
    bSetFisheyeAttr = HI_TRUE;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize( enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {

        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt = 128;

    /* comm video buffer */
    u64BlkSize = COMMON_GetPicBufferSize(stSize.u32Width,stSize.u32Height,SAMPLE_PIXEL_FORMAT,enBitWidth,COMPRESS_MODE_NONE,0);
    stVbConf.astCommPool[0].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 15;

    /*vb for vi raw*/
    u64BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, HI_FALSE, 0);
    stVbConf.astCommPool[1].u64BlkSize  = u64BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step    2: start vi vo
     ******************************************/
    SAMPLE_FISHEYE_StartViVo(pstViConfig,&g_stVoConfig);

    /******************************************
      step    3: start a thread
     ******************************************/
    pthread_create(&ThreadId, NULL, SAMPLE_FISHEYE_Nine_Lattice_Thread, &stSize);

    PAUSE();
    SAMPLE_FISHEYE_StopSwitchModeThrd();

    SAMPLE_FISHEYE_StopViVo(pstViConfig,&g_stVoConfig);
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}

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
    HI_S32             s32Ret        = HI_FAILURE;
    VI_DEV             ViDev         = 0;
    VI_PIPE            ViPipe        = 0;
    VI_CHN             ViChn         = 0;
    SAMPLE_VI_CONFIG_S stViConfig;

    if ( (argc < 2) || (1 != strlen(argv[1])) )
    {
        SAMPLE_FISHEYE_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_FISHEYE_HandleSig);
    signal(SIGTERM, SAMPLE_FISHEYE_HandleSig);
#endif

    if ((argc > 2) && *argv[2] == '1')  /* '1': HDMI, else: BT1120 */
    {
        g_stVoConfig.enVoIntfType = VO_INTF_BT1120;
    }
    if ((argc > 3) && *argv[3] == '1')  /* '1': PT_H265, else: PT_H264 */
    {
        g_enVencType   = PT_H264;
    }

    SAMPLE_COMM_VO_GetDefConfig(&g_stVoConfig);

    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);
    stViConfig.s32WorkingViNum                           = 1;

    stViConfig.as32WorkingViId[0]                        = 0;
    stViConfig.astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, 0);
    stViConfig.astViInfo[0].stSnsInfo.s32BusId           = 0;

    stViConfig.astViInfo[0].stDevInfo.ViDev              = ViDev;
    stViConfig.astViInfo[0].stDevInfo.enWDRMode          = WDR_MODE_NONE;

    stViConfig.astViInfo[0].stPipeInfo.enMastPipeMode    = VI_ONLINE_VPSS_OFFLINE;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[1]          = -1;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[2]          = -1;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    stViConfig.astViInfo[0].stChnInfo.ViChn              = ViChn;
    stViConfig.astViInfo[0].stChnInfo.enPixFormat        = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stViConfig.astViInfo[0].stChnInfo.enDynamicRange     = DYNAMIC_RANGE_SDR8;
    stViConfig.astViInfo[0].stChnInfo.enVideoFormat      = VIDEO_FORMAT_LINEAR;
    stViConfig.astViInfo[0].stChnInfo.enCompressMode     = COMPRESS_MODE_NONE;


    /************************************************
        step1:  Get  input size
      *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType, &g_enPicSize);

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }

    switch (*argv[1])
    {
        /* VI/VPSS - VO. Embeded isp, phychn channel preview. */
        case '0':
            s32Ret = SAMPLE_FISHEYE_360Panorama_Celing_2half(&stViConfig,ViPipe);
            break;

        case '1':
            s32Ret = SAMPLE_FISHEYE_360Panorama_Desktop_and_2normal(&stViConfig,ViPipe);
            break;

        case '2':
            s32Ret = SAMPLE_FISHEYE_180Panorama_Wall_and_2DynamicNormal(&stViConfig,ViPipe);
            break;

        case '3':
            s32Ret = SAMPLE_FISHEYE_Source_and_3Normal(&stViConfig,ViPipe);
            break;

        case '4':
            s32Ret = SAMPLE_FISHEYE_Nine_Lattice(&stViConfig);
            break;

        default:
            SAMPLE_PRT("the index is invaild!\n");
            SAMPLE_FISHEYE_Usage(argv[0]);
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

