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
#include "sample_comm.h"
#include "sample_comm_dpu.h"
#include "sample_dpu_main.h"


static SAMPLE_DPU_CONFIG_S s_stSampleDpuConfig;
static SAMPLE_VI_CONFIG_S s_stSampleViConfig;
static pthread_t s_hDpuThread = 0;
static HI_BOOL s_bDpuStopSignal = HI_FALSE;


/* This case only for function design reference */
HI_S32 SAMPLE_DPU_FILE_RECT_MATCH(HI_VOID)
{
    VB_CONFIG_S stVbConfig;
    HI_S32 i, s32Ret = HI_SUCCESS;
    SIZE_S astSize[2];
    PIC_SIZE_E enPicSize = 0;
    HI_S32 s32PipeNum = 0;
    HI_CHAR *apcMapFileName[DPU_RECT_MAX_PIPE_NUM] = {"./data/input/lut/1050x560_LeftMap.dat",
        "./data/input/lut/1050x560_RightMap.dat"};
    HI_CHAR *apcSrcFileName[DPU_RECT_MAX_PIPE_NUM] = {"./data/input/src/1280x720_LeftSrc.yuv",
        "./data/input/src/1280x720_RightSrc.yuv"};
    HI_CHAR *pcMatchFileName = "./data/output/1050x560_sp400.yuv";
    VIDEO_FRAME_INFO_S astDpuRectSrcFrame[DPU_RECT_MAX_PIPE_NUM] = {0};
    DPU_RECT_CHN_ATTR_S astChnAttr[DPU_RECT_MAX_CHN_NUM] = {0};
    DPU_MATCH_CHN_ATTR_S stChnAttr = {0};
    HI_U16 u16DispNum = 0;
    HI_U32 u32Size = 0;
    HI_U32 u32Stride = 0;
    HI_S32 s32MilliSec = 2000;
    HI_U32 u32FrameNum = 0;

    memset(&s_stSampleDpuConfig, 0, sizeof(SAMPLE_DPU_CONFIG_S));
    /************************************************
     step1:  init SYS, init common VB(for DPU RECT and DPU MATCH)
     *************************************************/
    enPicSize = PIC_720P;
    s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enPicSize, &astSize[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("sys get pic size failed for %#x!\n", s32Ret);
        goto END1;
    }

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 128;
    stVbConfig.astCommPool[0].u32BlkCnt  = 10;
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(astSize[0].u32Width, astSize[0].u32Height,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, 0);

    astSize[1].u32Width = SAMPLE_DPU_DST_WIDTH;
    astSize[1].u32Height = SAMPLE_DPU_DST_HEIGHT;
    stVbConfig.astCommPool[1].u32BlkCnt  = 8;
    stVbConfig.astCommPool[1].u64BlkSize = COMMON_GetPicBufferSize(astSize[1].u32Width, astSize[1].u32Height,
                                                PIXEL_FORMAT_S16C1, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, 0);
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys failed for %#x!\n", s32Ret);
        goto END1;
    }

    u32Stride = SAMPLE_COMM_DPU_CalcStride(astSize[0].u32Width, SAMPLE_DPU_ALIGN_16);
    u32Size = u32Stride * astSize[0].u32Height;
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        s_stSampleDpuConfig.astPipeVbPool[i] = HI_MPI_VB_CreatePool(u32Size, 1, NULL);
        if (VB_INVALID_POOLID == s_stSampleDpuConfig.astPipeVbPool[i])
        {
            s32Ret = HI_FAILURE;
            SAMPLE_PRT("create vb pool failed!\n");
            goto END2;
        }
        
        s_stSampleDpuConfig.au32VbBlk[i] = HI_MPI_VB_GetBlock(s_stSampleDpuConfig.astPipeVbPool[i], u32Size, NULL);
        if (VB_INVALID_HANDLE == s_stSampleDpuConfig.au32VbBlk[i])
        {
            SAMPLE_PRT("HI_MPI_VB_GetBlock failed!\n");
            goto END3;
        }
    }

    /************************************************
     step2:  start DPU RECT
     *************************************************/
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        s32PipeNum = i;
        s32Ret = SAMPLE_COMM_DPU_RECT_LoadLut(apcMapFileName[i], 
                &s_stSampleDpuConfig.astDpuRectMemInfo[i], &s_stSampleDpuConfig.s32LutId[i]);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("load lut failed for %#x!\n", s32Ret);
            goto END4;
        }
    }
    s32PipeNum = DPU_RECT_MAX_PIPE_NUM;

    s_stSampleDpuConfig.stDpuRectGrpAttr.LeftLutId = s_stSampleDpuConfig.s32LutId[0];
    s_stSampleDpuConfig.stDpuRectGrpAttr.RightLutId = s_stSampleDpuConfig.s32LutId[1];
    s_stSampleDpuConfig.DpuRectGrp = 0;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stLeftImageSize.u32Width = astSize[0].u32Width;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stLeftImageSize.u32Height = astSize[0].u32Height;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stRightImageSize.u32Width = astSize[0].u32Width;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stRightImageSize.u32Height = astSize[0].u32Height;
    s_stSampleDpuConfig.stDpuRectGrpAttr.enRectMode = DPU_RECT_MODE_DOUBLE;
    s_stSampleDpuConfig.stDpuRectGrpAttr.u32Depth = 1;
    s_stSampleDpuConfig.stDpuRectGrpAttr.bNeedSrcFrame = HI_TRUE;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stFrameRate.s32DstFrameRate = -1;

    for (i = 0; i < DPU_RECT_MAX_CHN_NUM; i++)
    {
        astChnAttr[i].stImageSize.u32Width = astSize[1].u32Width;
        astChnAttr[i].stImageSize.u32Height = astSize[1].u32Height;
    }

    s32Ret = SAMPLE_COMM_DPU_RECT_Start(s_stSampleDpuConfig.DpuRectGrp, 
            &s_stSampleDpuConfig.stDpuRectGrpAttr, astChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start dpu rect failed for %#x!\n", s32Ret);
        goto END5;
    }

    /************************************************
     step3:  start DPU MATCH
     *************************************************/
    u16DispNum = SAMPLE_DPU_DISP_NUM;
    s32Ret = SAMPLE_COMM_DPU_MATCH_GetAssistBufSize(u16DispNum, astSize[1].u32Height, &u32Size);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("get dpu match assist buffer size failed for %#x!\n", s32Ret);
        goto END5;
    }

    s32Ret = SAMPLE_COMM_DPU_MATCH_CreateMemInfo(&s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf, u32Size);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("create dpu match assist buffer failed for %#x!\n", s32Ret);
        goto END5;
    }

    s_stSampleDpuConfig.DpuMatchGrp = 0;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.u16DispNum = u16DispNum;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.u16DispStartPos = 0;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.u32Depth = 1;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stLeftImageSize.u32Width = astSize[1].u32Width;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stLeftImageSize.u32Height = astSize[1].u32Height;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stRightImageSize.u32Width = astSize[1].u32Width;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stRightImageSize.u32Height = astSize[1].u32Height;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enDensAccuMode = DPU_MATCH_DENS_ACCU_MODE_D9_A0;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enDispSubpixelEn = DPU_MATCH_DISP_SUBPIXEL_ENABLE;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enMatchMaskMode = DPU_MATCH_MASK_9X9_MODE;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enSpeedAccuMode = DPU_MATCH_SPEED_ACCU_MODE_ACCU;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.bNeedSrcFrame = HI_TRUE;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stFrameRate.s32DstFrameRate = -1;

    stChnAttr.stImageSize.u32Width = astSize[1].u32Width;
    stChnAttr.stImageSize.u32Height = astSize[1].u32Height;
    s32Ret = SAMPLE_COMM_DPU_MATCH_Start(s_stSampleDpuConfig.DpuMatchGrp, 
            &s_stSampleDpuConfig.stDpuMatchGrpAttr, &stChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("dpu match start failed for %#x!\n", s32Ret);
        goto END6;
    }

    /************************************************
     step4:  bind DPU RECT to DPU MATCH
     *************************************************/
    s32Ret = SAMPLE_COMM_DPU_RECT_Bind_MATCH(s_stSampleDpuConfig.DpuRectGrp, s_stSampleDpuConfig.DpuMatchGrp);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("dpu rect bind match failed for %#x!\n", s32Ret);
        goto END6;
    }
    /************************************************
     step5:  send frame to DPU RECT
     *************************************************/
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        s_stSampleDpuConfig.apstPipeFile[i] = fopen(apcSrcFileName[i], "rb");
        if(NULL == s_stSampleDpuConfig.apstPipeFile[i])
        {
            s32Ret = HI_FAILURE;
            SAMPLE_PRT("fopen file %s failed!\n", apcSrcFileName[i]);
            goto END7;
        }
    }
   
    s_stSampleDpuConfig.pstMatchFile = fopen(pcMatchFileName, "wb");
    if(NULL == s_stSampleDpuConfig.pstMatchFile)
    {
        s32Ret = HI_FAILURE;
        SAMPLE_PRT("fopen file %s failed!\n", pcMatchFileName);
        goto END7;
    }
    
    for (u32FrameNum = 0; u32FrameNum < SAMPLE_DPU_FRAME_NUM; u32FrameNum++)
    {
        for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
        {
            s32Ret = SAMPLE_COMM_DPU_RECT_GetVFrameFromFile(s_stSampleDpuConfig.apstPipeFile[i], 
                        s_stSampleDpuConfig.astPipeVbPool[i], astSize[0].u32Width,
                        astSize[0].u32Height, u32Stride, &astDpuRectSrcFrame[i], 
                        s_stSampleDpuConfig.au32VbBlk[i]);
            if(s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("get frame failed for %#x!\n", s32Ret);
                goto END8;
            }
        }

        s32Ret = HI_MPI_DPU_RECT_SendFrame(s_stSampleDpuConfig.DpuRectGrp, &astDpuRectSrcFrame[DPU_RECT_LEFT_PIPE],
                &astDpuRectSrcFrame[DPU_RECT_RIGHT_PIPE], s32MilliSec);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("send dpu rect frame failed for %#x!\n", s32Ret);
            goto END8;
        }

       /************************************************
         step6:  get match frame from DPU MATCH
         *************************************************/
        s32MilliSec = -1;
        s32Ret = HI_MPI_DPU_MATCH_GetFrame(s_stSampleDpuConfig.DpuMatchGrp, 
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_LEFT_PIPE],
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_RIGHT_PIPE], 
                    &s_stSampleDpuConfig.stDpuMatchDstFrame, s32MilliSec);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("get dpu match frame failed for %#x!\n", s32Ret);
            goto END8;
        }

        /* use match frame to do something */        
        s32Ret = SAMPLE_COMM_DPU_WriteFrame2File(s_stSampleDpuConfig.pstMatchFile, sizeof(HI_U16),
                    &s_stSampleDpuConfig.stDpuMatchDstFrame);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("gSAMPLE_COMM_DPU_WriteFrame2File failed for %#x!\n", s32Ret);
            goto END9;
        }

        s32Ret = HI_MPI_DPU_MATCH_ReleaseFrame(s_stSampleDpuConfig.DpuMatchGrp, 
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_LEFT_PIPE],
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_RIGHT_PIPE],
                    &s_stSampleDpuConfig.stDpuMatchDstFrame);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("get dpu match frame failed for %#x!\n", s32Ret);
            goto END9;
        }
    }

    PAUSE();
    goto END8;

END9:
    (HI_VOID)HI_MPI_DPU_MATCH_ReleaseFrame(s_stSampleDpuConfig.DpuMatchGrp, 
                &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_LEFT_PIPE],
                &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_RIGHT_PIPE],
                &s_stSampleDpuConfig.stDpuMatchDstFrame);
END8:
    if (NULL != s_stSampleDpuConfig.pstMatchFile)
    {
        fclose(s_stSampleDpuConfig.pstMatchFile);
        s_stSampleDpuConfig.pstMatchFile = NULL;
    }
END7:
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        if (NULL != s_stSampleDpuConfig.apstPipeFile[i])
        {
            fclose(s_stSampleDpuConfig.apstPipeFile[i]);
            s_stSampleDpuConfig.apstPipeFile[i] = NULL;
        }
    }
    (HI_VOID)SAMPLE_COMM_DPU_RECT_UnBind_MATCH(s_stSampleDpuConfig.DpuRectGrp,
            s_stSampleDpuConfig.DpuMatchGrp);
END6:
    (HI_VOID)SAMPLE_COMM_DPU_MACTH_Stop(s_stSampleDpuConfig.DpuMatchGrp);
    SAMPLE_DPU_MMZ_FREE(s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64PhyAddr, 
            s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64VirAddr);
END5:
    (HI_VOID)SAMPLE_COMM_DPU_RECT_Stop(s_stSampleDpuConfig.DpuRectGrp, 
            s_stSampleDpuConfig.stDpuRectGrpAttr.enRectMode);
END4:
    for (i = 0; i < s32PipeNum; i++)
    {
        (HI_VOID)SAMPLE_COMM_DPU_RECT_UnloadLut(&s_stSampleDpuConfig.astDpuRectMemInfo[i],
            &s_stSampleDpuConfig.s32LutId[i]);
    }
    
END3:    
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        s32Ret = HI_MPI_VB_ReleaseBlock(s_stSampleDpuConfig.au32VbBlk[i]);
        s_stSampleDpuConfig.au32VbBlk[i] = VB_INVALID_HANDLE;
    }
END2:
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        if (VB_INVALID_POOLID != s_stSampleDpuConfig.astPipeVbPool[i])
        {
            (HI_VOID)HI_MPI_VB_DestroyPool(s_stSampleDpuConfig.astPipeVbPool[i]);
        }
    }    
END1:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

HI_VOID SAMPLE_DPU_FILE_RECT_MATCH_HandleSig(HI_VOID)
{
    HI_U32 i = 0;    
    
    if (NULL != s_stSampleDpuConfig.pstMatchFile)
    {
        fclose(s_stSampleDpuConfig.pstMatchFile);
        s_stSampleDpuConfig.pstMatchFile = NULL;
    }

    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        if (NULL != s_stSampleDpuConfig.apstPipeFile[i])
        {
            fclose(s_stSampleDpuConfig.apstPipeFile[i]);
            s_stSampleDpuConfig.apstPipeFile[i] = NULL;
        }
    }

    (HI_VOID)SAMPLE_COMM_DPU_RECT_UnBind_MATCH(s_stSampleDpuConfig.DpuRectGrp,
                s_stSampleDpuConfig.DpuMatchGrp);
    (HI_VOID)SAMPLE_COMM_DPU_MACTH_Stop(s_stSampleDpuConfig.DpuMatchGrp);
    SAMPLE_DPU_MMZ_FREE(s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64PhyAddr, 
            s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64VirAddr);
    (HI_VOID)SAMPLE_COMM_DPU_RECT_Stop(s_stSampleDpuConfig.DpuRectGrp, 
            s_stSampleDpuConfig.stDpuRectGrpAttr.enRectMode);

    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        (HI_VOID)SAMPLE_COMM_DPU_RECT_UnloadLut(&s_stSampleDpuConfig.astDpuRectMemInfo[i],
            &s_stSampleDpuConfig.s32LutId[i]);
    }

    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        (HI_VOID)HI_MPI_VB_ReleaseBlock(s_stSampleDpuConfig.au32VbBlk[i]);
        s_stSampleDpuConfig.au32VbBlk[i] = VB_INVALID_HANDLE;
    }
    
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        if (VB_INVALID_POOLID != s_stSampleDpuConfig.astPipeVbPool[i])
        {
            (HI_VOID)HI_MPI_VB_DestroyPool(s_stSampleDpuConfig.astPipeVbPool[i]);
        }
    }
    
    SAMPLE_COMM_SYS_Exit();

    return ;
}

HI_S32 SAMPLE_DPU_Bind(DPU_RECT_GRP DpuRectGrp, DPU_MATCH_GRP DpuMatchGrp, HI_S32 s32PipeNum)
{
    VI_PIPE  ViPipe;
    VI_CHN   ViChn = 0;
    HI_S32   s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    DPU_RECT_PIPE DpuRectPipe = 0;
    HI_S32 i;

    for (i = 0; i < s32PipeNum; i++)
    {
        ViPipe = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe, ViChn, VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VI bind VPSS failed with %#x!", s32Ret);
            return HI_FAILURE;
        }
    }

    for (i = 0; i < s32PipeNum; i++)
    {
        VpssGrp = i;
        VpssChn = 0;
        DpuRectPipe = i;
        s32Ret = SAMPLE_COMM_DPU_VPSS_Bind_RECT(VpssGrp, VpssChn, DpuRectGrp, DpuRectPipe);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VPSS bind RECT failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    s32Ret = SAMPLE_COMM_DPU_RECT_Bind_MATCH(DpuRectGrp, DpuMatchGrp);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("RECT bind MATCH failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_DPU_UnBind(DPU_RECT_GRP DpuRectGrp, DPU_MATCH_GRP DpuMatchGrp, HI_S32 s32PipeNum)
{
    VI_PIPE  ViPipe;
    VI_CHN   ViChn   = 0;
    HI_S32   s32Ret  = HI_SUCCESS;
    HI_U32   i;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    DPU_RECT_PIPE DpuRectPipe = 0;

    s32Ret = SAMPLE_COMM_DPU_RECT_UnBind_MATCH(DpuRectGrp, DpuMatchGrp);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("RECT unbind MATCH failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    for (i = 0; i < s32PipeNum; i++)
    {
        VpssGrp = i;
        VpssChn = 0;
        DpuRectPipe = i;
        s32Ret = SAMPLE_COMM_DPU_VPSS_UnBind_RECT(VpssGrp, VpssChn, DpuRectGrp, DpuRectPipe);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VPSS unbind RECT failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    
    for (i = 0; i < s32PipeNum; i++)
    {
        ViPipe = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VI_UnBind_VPSS(ViPipe, ViChn, VpssGrp);

        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("VI unbind VPSS failed with %#x", s32Ret);
            return HI_FAILURE;
        }
    }
    
    return HI_SUCCESS;
}

static HI_VOID* SAMPLE_DPU_GetFrameProc(HI_VOID *pArg)
{
    HI_S32 s32MilliSec = 2000;
    HI_S32 s32Ret = HI_SUCCESS;
    
    while(HI_FALSE == s_bDpuStopSignal)
    {
        s32Ret = HI_MPI_DPU_MATCH_GetFrame(s_stSampleDpuConfig.DpuMatchGrp, 
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_LEFT_PIPE],
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_RIGHT_PIPE], 
                    &s_stSampleDpuConfig.stDpuMatchDstFrame, s32MilliSec);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("get dpu match frame failed for %#x!\n", s32Ret);
            continue;
        }

        /* do something for match frame */
        
        s32Ret = HI_MPI_DPU_MATCH_ReleaseFrame(s_stSampleDpuConfig.DpuMatchGrp, 
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_LEFT_PIPE],
                    &s_stSampleDpuConfig.astDpuMatchSrcFrame[DPU_MATCH_RIGHT_PIPE],
                    &s_stSampleDpuConfig.stDpuMatchDstFrame);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("get dpu match frame failed for %#x!\n", s32Ret);
        }
            
    }

    return NULL;
}

/* This case only for function design reference */
HI_S32 SAMPLE_DPU_VI_VPSS_RECT_MATCH(HI_VOID)
{
    HI_U32               u32ViChnCnt = 2;
    SIZE_S               stSize;
    VB_CONFIG_S          stVbConfig;
    PIC_SIZE_E           enPicSize = 0;
    HI_S32               s32Ret = HI_SUCCESS;
    HI_S32               s32ViPipeNum = 2;
    HI_S32               s32VpssGrpNum = 2;
    HI_S32               s32VpssGrpNumTmp = 0;
    HI_S32               i;
    VPSS_GRP             VpssGrp = 0;
    VPSS_GRP_ATTR_S      stVpssGrpAttr;
    VPSS_CHN_ATTR_S      astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL              abChnEn[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};
    
    HI_S32               s32DpuRectPipeNum = 0;
    HI_CHAR *apcMapFileName[DPU_RECT_MAX_PIPE_NUM] = {"./data/input/lut/1050x560_LeftMap.dat",
        "./data/input/lut/1050x560_RightMap.dat"};
    DPU_RECT_CHN_ATTR_S astDpuRectChnAttr[DPU_RECT_MAX_CHN_NUM] = {0};
    DPU_MATCH_CHN_ATTR_S stDpuMatchChnAttr = {0};
    HI_U16 u16DispNum = 0;
    HI_U32 u32Size = 0;
    HI_CHAR acThreadName[16] = {0};
    
    /************************************************
     step1:  get all sensors information
     *************************************************/
    SAMPLE_COMM_VI_GetSensorInfo(&s_stSampleViConfig);

    s_stSampleViConfig.s32WorkingViNum = s32ViPipeNum;

    for (i = 0; i < s32ViPipeNum; i++)
    {
        s_stSampleViConfig.astViInfo[i].stDevInfo.enWDRMode        = WDR_MODE_NONE;

        s_stSampleViConfig.astViInfo[i].stChnInfo.enCompressMode   = COMPRESS_MODE_NONE;
        s_stSampleViConfig.astViInfo[i].stChnInfo.enDynamicRange   = DYNAMIC_RANGE_SDR8;
        s_stSampleViConfig.astViInfo[i].stChnInfo.enVideoFormat    = VIDEO_FORMAT_LINEAR;
        s_stSampleViConfig.astViInfo[i].stChnInfo.enPixFormat      = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        s_stSampleViConfig.astViInfo[i].stChnInfo.ViChn            = 0;

        s_stSampleViConfig.astViInfo[i].stPipeInfo.enMastPipeMode  = VI_OFFLINE_VPSS_OFFLINE;
        s_stSampleViConfig.astViInfo[i].stPipeInfo.aPipe[1]        = -1;
        s_stSampleViConfig.astViInfo[i].stPipeInfo.aPipe[2]        = -1;
        s_stSampleViConfig.astViInfo[i].stPipeInfo.aPipe[3]        = -1;
    }

    s_stSampleViConfig.as32WorkingViId[0]                   = 0;
    s_stSampleViConfig.astViInfo[0].stDevInfo.ViDev         = 0;
    s_stSampleViConfig.astViInfo[0].stPipeInfo.aPipe[0]     = 0;
    s_stSampleViConfig.astViInfo[0].stSnsInfo.MipiDev       = 0;
    s_stSampleViConfig.astViInfo[0].stSnsInfo.s32BusId      = 0;

    s_stSampleViConfig.as32WorkingViId[1]                   = 1;
    s_stSampleViConfig.astViInfo[1].stDevInfo.ViDev         = 2;
    s_stSampleViConfig.astViInfo[1].stPipeInfo.aPipe[0]     = 1;
    s_stSampleViConfig.astViInfo[1].stSnsInfo.MipiDev       = 2;
    s_stSampleViConfig.astViInfo[1].stSnsInfo.s32BusId      = 1;

    /************************************************
     step 2: get input size
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(s_stSampleViConfig.astViInfo[0].stSnsInfo.enSnsType, &enPicSize);
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
      step 3: mpp system init
     ******************************************/
    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 128;
    stVbConfig.astCommPool[0].u32BlkCnt  = 10;
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height,
                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConfig.astCommPool[1].u32BlkCnt  = 8;
    stVbConfig.astCommPool[1].u64BlkSize = COMMON_GetPicBufferSize(SAMPLE_DPU_DST_WIDTH, SAMPLE_DPU_DST_HEIGHT,
                PIXEL_FORMAT_S16C1, DATA_BITWIDTH_16, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

    stVbConfig.astCommPool[2].u32BlkCnt  = u32ViChnCnt * 4;
    stVbConfig.astCommPool[2].u64BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height,
                PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

    stVbConfig.astCommPool[3].u32BlkCnt  = 10;
    stVbConfig.astCommPool[3].u64BlkSize = COMMON_GetPicBufferSize(SAMPLE_DPU_IN_WIDTH, SAMPLE_DPU_IN_HEIGHT, 
                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %#x!\n", s32Ret);
        goto END1;
    }

    s32Ret = SAMPLE_COMM_VI_SetParam(&s_stSampleViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SetParam failed with %#x!\n", s32Ret);
        goto END1;
    }
    /******************************************
      step 4: start VI
     ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&s_stSampleViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartVi failed with %#x\n", s32Ret);
        goto END1;
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

    astVpssChnAttr[0].u32Width                     = SAMPLE_DPU_IN_WIDTH;
    astVpssChnAttr[0].u32Height                    = SAMPLE_DPU_IN_HEIGHT;
    astVpssChnAttr[0].enChnMode                    = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode               = COMPRESS_MODE_NONE;
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

    for (i = 0; i < s32VpssGrpNum; i++)
    {
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEn, &stVpssGrpAttr, astVpssChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
            goto END2;
        }
        s32VpssGrpNumTmp++;
    }

    /************************************************
     step6:  start DPU RECT
     *************************************************/
    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        s32Ret = SAMPLE_COMM_DPU_RECT_LoadLut(apcMapFileName[i], &s_stSampleDpuConfig.astDpuRectMemInfo[i],
                    &s_stSampleDpuConfig.s32LutId[i]);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("load lut failed for %#x!\n", s32Ret);
            goto END3;
        }
        s32DpuRectPipeNum++;
    }

    s_stSampleDpuConfig.stDpuRectGrpAttr.LeftLutId = s_stSampleDpuConfig.s32LutId[0];
    s_stSampleDpuConfig.stDpuRectGrpAttr.RightLutId = s_stSampleDpuConfig.s32LutId[1];
    s_stSampleDpuConfig.DpuRectGrp = 0;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stLeftImageSize.u32Width = SAMPLE_DPU_IN_WIDTH;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stLeftImageSize.u32Height = SAMPLE_DPU_IN_HEIGHT;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stRightImageSize.u32Width = SAMPLE_DPU_IN_WIDTH;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stRightImageSize.u32Height = SAMPLE_DPU_IN_HEIGHT;
    s_stSampleDpuConfig.stDpuRectGrpAttr.enRectMode = DPU_RECT_MODE_DOUBLE;
    s_stSampleDpuConfig.stDpuRectGrpAttr.u32Depth = 0;
    s_stSampleDpuConfig.stDpuRectGrpAttr.bNeedSrcFrame = HI_TRUE;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    s_stSampleDpuConfig.stDpuRectGrpAttr.stFrameRate.s32DstFrameRate = -1;

    for (i = 0; i < DPU_RECT_MAX_CHN_NUM; i++)
    {
        astDpuRectChnAttr[i].stImageSize.u32Width = SAMPLE_DPU_DST_WIDTH;
        astDpuRectChnAttr[i].stImageSize.u32Height = SAMPLE_DPU_DST_HEIGHT;
    }

    s32Ret = SAMPLE_COMM_DPU_RECT_Start(s_stSampleDpuConfig.DpuRectGrp, 
            &s_stSampleDpuConfig.stDpuRectGrpAttr, astDpuRectChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start dpu rect failed for %#x!\n", s32Ret);
        goto END4;
    }

    /************************************************
     step7:  start DPU MATCH
     *************************************************/
    u16DispNum = SAMPLE_DPU_DISP_NUM;
    s32Ret = SAMPLE_COMM_DPU_MATCH_GetAssistBufSize(u16DispNum, SAMPLE_DPU_DST_HEIGHT, &u32Size);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("get dpu match assist buffer size failed for %#x!\n", s32Ret);
        goto END4;
    }

    s32Ret = SAMPLE_COMM_DPU_MATCH_CreateMemInfo(&s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf, u32Size);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("create dpu match assist buffer failed for %#x!\n", s32Ret);
        goto END4;
    }

    s_stSampleDpuConfig.DpuMatchGrp = 0;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.u16DispNum = u16DispNum;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.u16DispStartPos = 0;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.u32Depth = 8;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stLeftImageSize.u32Width = SAMPLE_DPU_DST_WIDTH;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stLeftImageSize.u32Height = SAMPLE_DPU_DST_HEIGHT;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stRightImageSize.u32Width = SAMPLE_DPU_DST_WIDTH;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stRightImageSize.u32Height = SAMPLE_DPU_DST_HEIGHT;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enDensAccuMode = DPU_MATCH_DENS_ACCU_MODE_D9_A0;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enDispSubpixelEn = DPU_MATCH_DISP_SUBPIXEL_ENABLE;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enMatchMaskMode = DPU_MATCH_MASK_9X9_MODE;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.enSpeedAccuMode = DPU_MATCH_SPEED_ACCU_MODE_ACCU;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.bNeedSrcFrame = HI_TRUE;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    s_stSampleDpuConfig.stDpuMatchGrpAttr.stFrameRate.s32DstFrameRate = -1;

    stDpuMatchChnAttr.stImageSize.u32Width = SAMPLE_DPU_DST_WIDTH;
    stDpuMatchChnAttr.stImageSize.u32Height = SAMPLE_DPU_DST_HEIGHT;
    s32Ret = SAMPLE_COMM_DPU_MATCH_Start(s_stSampleDpuConfig.DpuMatchGrp, 
                &s_stSampleDpuConfig.stDpuMatchGrpAttr, &stDpuMatchChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("dpu match start failed for %#x!\n", s32Ret);
        goto END5;
    }

    /************************************************
     step8:  bind VI->VPSS->DPU RECT->DPU MATCH
     *************************************************/
    s32Ret = SAMPLE_DPU_Bind(s_stSampleDpuConfig.DpuRectGrp, s_stSampleDpuConfig.DpuMatchGrp, s32ViPipeNum);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("dpu bind failed for %#x!\n", s32Ret);
        goto END6;
    }

    /******************************************
      step 9: Create work thread
     ******************************************/
    snprintf(acThreadName, 16, "DPU_FrameProc");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0,0,0);
    pthread_create(&s_hDpuThread, 0, SAMPLE_DPU_GetFrameProc, NULL);
    
    PAUSE();
    s_bDpuStopSignal = HI_TRUE;
    pthread_join(s_hDpuThread, NULL);
    s_hDpuThread = 0;
    
END6:
    (HI_VOID)SAMPLE_DPU_UnBind(s_stSampleDpuConfig.DpuRectGrp, s_stSampleDpuConfig.DpuMatchGrp, s32ViPipeNum);
END5:
    (HI_VOID)SAMPLE_COMM_DPU_MACTH_Stop(s_stSampleDpuConfig.DpuMatchGrp);
    SAMPLE_DPU_MMZ_FREE(s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64PhyAddr, 
            s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64VirAddr);
END4:
    (HI_VOID)SAMPLE_COMM_DPU_RECT_Stop(s_stSampleDpuConfig.DpuRectGrp, 
            s_stSampleDpuConfig.stDpuRectGrpAttr.enRectMode);
END3:
    for (i = 0; i < s32DpuRectPipeNum; i++)
    {
        (HI_VOID)SAMPLE_COMM_DPU_RECT_UnloadLut(&s_stSampleDpuConfig.astDpuRectMemInfo[i], &s_stSampleDpuConfig.s32LutId[i]);
    }
END2:
    for (i = 0; i < s32VpssGrpNumTmp; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEn);
    }
    
    SAMPLE_COMM_VI_StopVi(&s_stSampleViConfig);
END1:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}

HI_VOID SAMPLE_DPU_VI_VPSS_RECT_MATCH_HandleSig(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32ViPipeNum = 2;
    HI_S32 s32VpssGrpNum = 2;
    VPSS_GRP VpssGrp = 0;
    HI_BOOL abChnEn[VPSS_MAX_PHY_CHN_NUM] = {1, 0, 0, 0};

    s_bDpuStopSignal = HI_TRUE;
    if (0 != s_hDpuThread)
    {
        pthread_join(s_hDpuThread, NULL);
        s_hDpuThread = 0;
    }

    (HI_VOID)SAMPLE_DPU_UnBind(s_stSampleDpuConfig.DpuRectGrp, s_stSampleDpuConfig.DpuMatchGrp, s32ViPipeNum);

    (HI_VOID)SAMPLE_COMM_DPU_MACTH_Stop(s_stSampleDpuConfig.DpuMatchGrp);

    SAMPLE_DPU_MMZ_FREE(s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64PhyAddr, 
            s_stSampleDpuConfig.stDpuMatchGrpAttr.stAssistBuf.u64VirAddr);

    (HI_VOID)SAMPLE_COMM_DPU_RECT_Stop(s_stSampleDpuConfig.DpuRectGrp, 
            s_stSampleDpuConfig.stDpuRectGrpAttr.enRectMode);

    for (i = 0; i < DPU_RECT_MAX_PIPE_NUM; i++)
    {
        (HI_VOID)SAMPLE_COMM_DPU_RECT_UnloadLut(&s_stSampleDpuConfig.astDpuRectMemInfo[i], &s_stSampleDpuConfig.s32LutId[i]);
    }

    for (i = 0; i < s32VpssGrpNum; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, abChnEn);
    }
   
    SAMPLE_COMM_VI_StopVi(&s_stSampleViConfig);
    SAMPLE_COMM_SYS_Exit();

    return ;
}



