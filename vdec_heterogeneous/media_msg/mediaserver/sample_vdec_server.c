#include <unistd.h>
#include "sample_comm.h"
#include "sample_msg_comm_define.h"
#include "hi_datafifo.h"
#include "sample_vdec_api.h"

VO_INTF_SYNC_E g_enIntfSync = VO_OUTPUT_3840x2160_30;

static HI_BOOL g_bStop = HI_FALSE;
static HI_BOOL g_bThReturn = HI_FALSE;

SAMPLE_BUF_S g_stBufInfo[VDEC_MAX_CHN_NUM] = {0};
static HI_DATAFIFO_HANDLE g_hDataFifo[VDEC_MAX_CHN_NUM] = {[0 ... VDEC_MAX_CHN_NUM - 1] = HI_DATAFIFO_INVALID_HANDLE};


SAMPLE_VO_MODE_E SAMPLE_VDEC_GetVOMode(HI_U32 u32ChnNum)
{
    if(u32ChnNum == 1)
    {
        return VO_MODE_1MUX;
    }
    else if(u32ChnNum <= 4)
    {
        return VO_MODE_4MUX;
    }
    else if(u32ChnNum <= 8)
    {
        return VO_MODE_8MUX;
    }
    else if(u32ChnNum <= 16)
    {
        return VO_MODE_16MUX;
    }
    else
    {
        return VO_MODE_16MUX;
    }
}

HI_S32 SAMPLE_VDEC_GetQuadraticRoot(HI_S32 s32Num)
{
    if (1 == s32Num) return 1;
    else if (4>=s32Num) return 2;
    else if (9>=s32Num) return 3;
    else if (16>=s32Num) return 4;
    else if (25>=s32Num) return 5;
    else if (36>=s32Num) return 6;
    else if (49>=s32Num) return 7;
    else if (64>=s32Num) return 8;
    else if (81>=s32Num) return 9;
    else if (100>=s32Num) return 10;
    else if (121>=s32Num) return 11;
    else if (144>=s32Num) return 12;
    else HI_ASSERT(0);
}


HI_S32 SAMPLE_VDEC_DatafifoInit(HI_S32 VdecChn, HI_U64 *pu64Addr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DATAFIFO_PARAMS_S stDatafifo;

    stDatafifo.u32EntriesNum        = 10;
    stDatafifo.u32CacheLineSize     = sizeof(SAMPLE_VDEC_STREAM_S);
    stDatafifo.bDataReleaseByWriter = HI_TRUE;
    stDatafifo.enOpenMode           = DATAFIFO_READER;

    g_hDataFifo[VdecChn] = HI_DATAFIFO_INVALID_HANDLE;

    s32Ret = HI_DATAFIFO_OpenByAddr(&g_hDataFifo[VdecChn], &stDatafifo, *pu64Addr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("chn %d open datafifo error:%x\n", VdecChn, s32Ret);
        return HI_FAILURE;
    }

    printf("chn %d datafifo_init finish\n", VdecChn);

    return HI_SUCCESS;
}



HI_VOID SAMPLE_VDEC_DatafifoDeinit(HI_S32 VdecChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 readLen;
    SAMPLE_VDEC_STREAM_S *pstSampleStream;

    while(1)
    {
        if(HI_TRUE == g_bThReturn)
        {
            readLen = 0;
            s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[VdecChn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
            if (readLen > 0)
            {
                s32Ret = HI_DATAFIFO_Read(g_hDataFifo[VdecChn], (HI_VOID**)&pstSampleStream);
                if (HI_SUCCESS == s32Ret)
                {
                    s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[VdecChn], DATAFIFO_CMD_READ_DONE, pstSampleStream);
                }
            }
            else
            {
                break;
            }
        }
        usleep(10);
    }
    s32Ret = HI_DATAFIFO_Close(g_hDataFifo[VdecChn]);
    if(s32Ret != HI_SUCCESS)
    {
        printf("chn %d datafifo close fail\n", VdecChn);
    }
    g_hDataFifo[VdecChn] = HI_DATAFIFO_INVALID_HANDLE;
    printf("chn %d datafifo_deinit finish\n", VdecChn);

    return;
}





HI_S32 SAMPLE_VDEC_MmapBuffer(HI_S32 VdecChn, SAMPLE_VDEC_STREAM_S *pstSampleStream)
{
    if(HI_NULL == g_stBufInfo[VdecChn].pVirAddr)
    {
        g_stBufInfo[VdecChn].u64PhyAddr = pstSampleStream->stBufInfo.u64PhyAddr;
        g_stBufInfo[VdecChn].u32Len     = pstSampleStream->stBufInfo.u32Len;

        g_stBufInfo[VdecChn].pVirAddr = HI_MPI_SYS_Mmap(g_stBufInfo[VdecChn].u64PhyAddr, g_stBufInfo[VdecChn].u32Len);
        if (HI_NULL == g_stBufInfo[VdecChn].pVirAddr)
        {
            printf("HI_MPI_SYS_Mmap size %d fail!\n", g_stBufInfo[VdecChn].u32Len);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID SAMPLE_VDEC_MunmapBuffer(HI_S32 VdecChn)
{
    HI_S32 s32Ret;

    if(0 != g_stBufInfo[VdecChn].pVirAddr)
    {
        s32Ret = HI_MPI_SYS_Munmap(g_stBufInfo[VdecChn].pVirAddr, g_stBufInfo[VdecChn].u32Len);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_SYS_Munmap fail!\n");
        }
        memset(&g_stBufInfo[VdecChn], 0, sizeof(SAMPLE_BUF_S));
    }

    return;
}



HI_S32 SAMPLE_VDEC_StartVideo(HI_S32 s32VdecChnNum)
{
    VB_CONFIG_S stVbConfig;
    HI_S32 i, s32SquareRoots, s32Ret = HI_SUCCESS;
    VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];
    SIZE_S stDispSize;
    VO_LAYER VoLayer;
    HI_S32 VpssGrpNum;
    VPSS_GRP VpssGrp=0;
    pthread_t   VdecThread[2*VDEC_MAX_CHN_NUM];
    PIC_SIZE_E enDispPicSize;
    SAMPLE_VDEC_ATTR astSampleVdec[VDEC_MAX_CHN_NUM];
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
    SAMPLE_VO_CONFIG_S stVoConfig;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
    VO_INTF_SYNC_E enIntfSync;

    VpssGrpNum    = s32VdecChnNum;
    /************************************************
    step1:  init SYS, init common VB(for VPSS and VO)
    *************************************************/

    if(VO_OUTPUT_3840x2160_30 == g_enIntfSync)
    {
        enDispPicSize = PIC_3840x2160;
        enIntfSync    = VO_OUTPUT_3840x2160_30;
    }
    else
    {
        enDispPicSize = PIC_1080P;
        enIntfSync    = VO_OUTPUT_1080P30;
    }

    s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("sys get pic size fail for %#x!\n", s32Ret);
        goto END1;
    }

    s32SquareRoots = SAMPLE_VDEC_GetQuadraticRoot(s32VdecChnNum);

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 1;
    stVbConfig.astCommPool[0].u32BlkCnt  = 0*s32VdecChnNum;
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width/s32SquareRoots, stDispSize.u32Height/s32SquareRoots,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, 0);
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto END1;
    }

    /************************************************
    step2:  init module VB or user VB(for VDEC)
    *************************************************/
    for(i=0; i<s32VdecChnNum; i++)
    {
        astSampleVdec[i].enType                           = PT_H265;
        astSampleVdec[i].u32Width                         = 1920;
        astSampleVdec[i].u32Height                        = 1080;
        astSampleVdec[i].enMode                           = VIDEO_MODE_FRAME;
        astSampleVdec[i].stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IPB;
        astSampleVdec[i].stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;
        astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum = 3;
        astSampleVdec[i].u32DisplayFrameNum               = 2;
        astSampleVdec[i].u32FrameBufCnt = astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + astSampleVdec[i].u32DisplayFrameNum + 1;
    }
    s32Ret = SAMPLE_COMM_VDEC_InitVBPool(s32VdecChnNum, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init mod common vb fail for %#x!\n", s32Ret);
        goto END2;
    }

    /************************************************
    step3:  start VDEC
    *************************************************/
    s32Ret = SAMPLE_COMM_VDEC_Start(s32VdecChnNum, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
        goto END3;
    }

    /************************************************
    step4:  start VPSS
    *************************************************/
    stVpssGrpAttr.u32MaxW = 1920;
    stVpssGrpAttr.u32MaxH = 1080;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.bNrEn = HI_FALSE;

    memset(abChnEnable, 0, sizeof(abChnEnable));
    abChnEnable[0] = HI_TRUE;
    astVpssChnAttr[0].u32Width                    = ALIGN_UP(stDispSize.u32Width/s32SquareRoots, 2);
    astVpssChnAttr[0].u32Height                   = ALIGN_UP(stDispSize.u32Height/s32SquareRoots, 2);
    astVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_AUTO;
    astVpssChnAttr[0].enCompressMode              = COMPRESS_MODE_SEG;
    astVpssChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;
    astVpssChnAttr[0].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    astVpssChnAttr[0].u32Depth                    = 0;
    astVpssChnAttr[0].bMirror                     = HI_FALSE;
    astVpssChnAttr[0].bFlip                       = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    astVpssChnAttr[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    for(i=0; i<s32VdecChnNum; i++)
    {
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, &abChnEnable[0], &stVpssGrpAttr, &astVpssChnAttr[0]);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
            goto END4;
        }
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
    stVoConfig.enVoMode              = SAMPLE_VDEC_GetVOMode(s32VdecChnNum);
    stVoConfig.enPixFormat           = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVoConfig.stDispRect.s32X       = 0;
    stVoConfig.stDispRect.s32Y       = 0;
    stVoConfig.stDispRect.u32Width   = stDispSize.u32Width;
    stVoConfig.stDispRect.u32Height  = stDispSize.u32Height;
    stVoConfig.stImageSize.u32Width  = stDispSize.u32Width;
    stVoConfig.stImageSize.u32Height = stDispSize.u32Height;
    stVoConfig.enVoPartMode          = VO_PART_MODE_MULTI;

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VO fail for %#x!\n", s32Ret);
        goto END5;
    }

    /************************************************
    step6:  VDEC bind VPSS
    *************************************************/
    for(i=0; i<s32VdecChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_Bind_VPSS(i, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec bind vpss fail for %#x!\n", s32Ret);
            goto END6;
        }
    }

    /************************************************
    step7:  VPSS bind VO
    *************************************************/
    VoLayer = stVoConfig.VoDev;
    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Bind_VO(i, 0, VoLayer, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss bind vo fail for %#x!\n", s32Ret);
            goto END7;
        }
    }

    g_bStop = HI_FALSE;
    s32Ret = SAMPLE_VDEC_CreateDatafifoThread();
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("create datafifo fail for %#x!\n", s32Ret);
        goto END7;
    }

    return HI_SUCCESS;

END7:
    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i, 0, VoLayer, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
        }
    }

END6:
    for(i=0; i<s32VdecChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_UnBind_VPSS(i, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec unbind vpss fail for %#x!\n", s32Ret);
        }
    }

END5:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

END4:
    for(i = VpssGrp; i >= 0; i--)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, &abChnEnable[0]);
    }
END3:
    SAMPLE_COMM_VDEC_Stop(s32VdecChnNum);

END2:
    SAMPLE_COMM_VDEC_ExitVBPool();

END1:
    SAMPLE_COMM_SYS_Exit();

    return HI_FAILURE;
}


HI_S32 SAMPLE_VDEC_StopVideo(HI_S32 s32VdecChnNum)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    HI_S32 VpssGrpNum;
    VPSS_GRP VpssGrp;
    VO_LAYER VoLayer;
    SAMPLE_VO_CONFIG_S stVoConfig;
    HI_BOOL abChnEnable[VPSS_MAX_CHN_NUM];
    PIC_SIZE_E enDispPicSize;
    VO_INTF_SYNC_E enIntfSync;
    SIZE_S stDispSize;

    g_bStop = HI_TRUE;
    while(HI_TRUE != g_bThReturn)
    {
        usleep(10);
    }

    VpssGrpNum    = s32VdecChnNum;

    if(VO_OUTPUT_3840x2160_30 == g_enIntfSync)
    {
        enDispPicSize = PIC_3840x2160;
        enIntfSync    = VO_OUTPUT_3840x2160_30;
    }
    else
    {
        enDispPicSize = PIC_1080P;
        enIntfSync    = VO_OUTPUT_1080P30;
    }

    s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("sys get pic size fail for %#x!\n", s32Ret);
    }

    VoLayer = SAMPLE_VO_DEV_UHD;

    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i, 0, VoLayer, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
        }
    }


    for(i=0; i<s32VdecChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_UnBind_VPSS(i, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec unbind vpss fail for %#x!\n", s32Ret);
        }
    }

    stVoConfig.VoDev                 = SAMPLE_VO_DEV_UHD;
    stVoConfig.enVoIntfType          = VO_INTF_HDMI;
    stVoConfig.enIntfSync            = enIntfSync;
    stVoConfig.enPicSize             = enDispPicSize;
    stVoConfig.u32BgColor            = COLOR_RGB_BLUE;
    stVoConfig.u32DisBufLen          = 3;
    stVoConfig.enDstDynamicRange     = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode              = SAMPLE_VDEC_GetVOMode(s32VdecChnNum);
    stVoConfig.enPixFormat           = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVoConfig.stDispRect.s32X       = 0;
    stVoConfig.stDispRect.s32Y       = 0;
    stVoConfig.stDispRect.u32Width   = stDispSize.u32Width;
    stVoConfig.stDispRect.u32Height  = stDispSize.u32Height;
    stVoConfig.stImageSize.u32Width  = stDispSize.u32Width;
    stVoConfig.stImageSize.u32Height = stDispSize.u32Height;
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

    memset(abChnEnable, 0, sizeof(abChnEnable));
    abChnEnable[0] = HI_TRUE;
    for(i=0; i<VpssGrpNum; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, &abChnEnable[0]);
    }

    SAMPLE_COMM_VDEC_Stop(s32VdecChnNum);

    SAMPLE_COMM_VDEC_ExitVBPool();

    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}


static void* SAMPLE_VDEC_DatafifoReceiveThread(void *arg)
{
    HI_U32 readLen = 0;
    HI_S32 VdecChn, s32Ret = HI_SUCCESS;
    SAMPLE_VDEC_STREAM_S *apstSpStream[VDEC_MAX_CHN_NUM];
    VDEC_STREAM_S astStream[VDEC_MAX_CHN_NUM];

    for(VdecChn=0; VdecChn<VDEC_MAX_CHN_NUM; VdecChn++)
    {
        apstSpStream[VdecChn] = HI_NULL;
    }

    printf("SAMPLE_VDEC_DatafifoReceiveThread start...\n");
    g_bThReturn = HI_FALSE;

    while (HI_FALSE == g_bStop)
    {
        for(VdecChn=0; VdecChn<VDEC_MAX_CHN_NUM; VdecChn++)
        {
            if(HI_TRUE == g_bStop)
            {
                break;
            }

            if(HI_DATAFIFO_INVALID_HANDLE == g_hDataFifo[VdecChn])
            {
                continue;
            }

            if(HI_NULL != apstSpStream[VdecChn])
            {
                goto SEND_STREAM;
            }

            readLen = 0;
            s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[VdecChn], DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
            if (HI_SUCCESS != s32Ret)
            {
                printf("get available read len error:%x\n", s32Ret);
                break;
            }

            if (readLen > 0)
            {
                s32Ret = HI_DATAFIFO_Read(g_hDataFifo[VdecChn], (HI_VOID**)&apstSpStream[VdecChn]);
                if (HI_SUCCESS != s32Ret)
                {
                    printf("read error:%x\n", s32Ret);
                    break;
                }

                s32Ret = SAMPLE_VDEC_MmapBuffer(VdecChn, apstSpStream[VdecChn]);
                if (HI_SUCCESS != s32Ret)
                {
                     printf("read error:%x\n", s32Ret);
                     break;
                }

                memcpy(&astStream[VdecChn], &apstSpStream[VdecChn]->stStream, sizeof(VDEC_STREAM_S));
                if(HI_NULL != apstSpStream[VdecChn]->stStream.pu8Addr)
                {
                    astStream[VdecChn].pu8Addr = (HI_U8 *)((HI_UL)g_stBufInfo[VdecChn].pVirAddr +
                        ((HI_UL)apstSpStream[VdecChn]->stStream.pu8Addr - (HI_UL)apstSpStream[VdecChn]->stBufInfo.pVirAddr));
                }
SEND_STREAM:
                s32Ret = HI_MPI_VDEC_SendStream(VdecChn, &astStream[VdecChn], 0);
                if (HI_ERR_VDEC_BUF_FULL == s32Ret)
                {
                    continue;
                }
                else if(HI_SUCCESS != s32Ret)
                {
                    printf("vdec chn %d send stream fail for 0x%x!\n", VdecChn, s32Ret);
                }

                s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[VdecChn], DATAFIFO_CMD_READ_DONE, apstSpStream[VdecChn]);
                if (HI_SUCCESS != s32Ret)
                {
                    printf("read done error:%x\n", s32Ret);
                    break;
                }
                apstSpStream[VdecChn] = HI_NULL;
            }
        }
        usleep(10000);
    }

    for(VdecChn=0; VdecChn<VDEC_MAX_CHN_NUM; VdecChn++)
    {
        if(HI_NULL != apstSpStream[VdecChn])
        {
            s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[VdecChn], DATAFIFO_CMD_READ_DONE, apstSpStream[VdecChn]);
            if (HI_SUCCESS != s32Ret)
            {
                printf("break: read done error:%x\n", s32Ret);
            }
            apstSpStream[VdecChn] = HI_NULL;
        }
        SAMPLE_VDEC_MunmapBuffer(VdecChn);
    }
    g_bThReturn = HI_TRUE;

    printf("SAMPLE_VDEC_DatafifoReceiveThread return...\n");

    return NULL;
}


HI_S32 SAMPLE_VDEC_CreateDatafifoThread(HI_VOID)
{
    HI_S32 s32Ret;

    pthread_t DatafifoThreadId;
    pthread_attr_t DatafifoThAttr;

    pthread_attr_init(&DatafifoThAttr);
    pthread_attr_setdetachstate(&DatafifoThAttr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&DatafifoThAttr, 0x200000);
    s32Ret = pthread_create(&DatafifoThreadId, &DatafifoThAttr, SAMPLE_VDEC_DatafifoReceiveThread, HI_NULL);
    if(HI_SUCCESS != s32Ret)
    {
        printf("Media_Server_Receive_thread create fail\n");
        return HI_FAILURE;
    }
    pthread_attr_destroy(&DatafifoThAttr);

    return HI_SUCCESS;
}














