#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifndef __HuaweiLite__
#include <sys/poll.h>
#endif
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "hi_common.h"
#include "sample_comm.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vi.h"
#include "hi_comm_isp.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_isp.h"


#define DUMP_RAW_AND_SAVE_LSC (1)

#define MAX_FRM_CNT     25
#define MAX_FRM_WIDTH   8192

#define ALIGN_BACK(x, a)              ((a) * (((x) / (a))))

#if 1
#define MEM_DEV_OPEN() \
    do {\
        if (s_s32MemDev <= 0)\
        {\
            s_s32MemDev = open("/dev/mem", O_CREAT|O_RDWR|O_SYNC);\
            if (s_s32MemDev < 0)\
            {\
                perror("Open dev/mem error");\
                return -1;\
            }\
        }\
    }while(0)

#define MEM_DEV_CLOSE() \
    do {\
        HI_S32 s32Ret;\
        if (s_s32MemDev > 0)\
        {\
            s32Ret = close(s_s32MemDev);\
            if(HI_SUCCESS != s32Ret)\
            {\
                perror("Close mem/dev Fail");\
                return s32Ret;\
            }\
            s_s32MemDev = -1;\
        }\
    }while(0)

void usage(void)
{
    printf(
        "\n"
        "*************************************************\n"
        "Usage: ./lsc_online_cali [Pipe] [nbit] [scale] \n"
        "ViDev: \n"
        "   0:Pipe_0 ~~ 5:Pipe_5\n"
        "nbit: \n"
        "   The bit num to be dump\n"        
        "scale: \n"
        "   scale value to be used to calculate gain\n"        
        "e.g : ./lsc_online_cali  0 12 1\n"
        "*************************************************\n"
        "\n");
    exit(1);
}
#endif

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_LSCCALI_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_All_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_S32 get_raw_pts(VIDEO_FRAME_INFO_S** pastFrame, HI_U64 u64RawPts, HI_U32 u32RowCnt,
                   HI_U32 u32ColCnt, HI_U32 u32Col,  HI_U32* pu32Index)
{
    HI_S32 i;

    for (i = 0; i < u32RowCnt; i++)
    {
        printf("get_raw_pts  --pts is %lld.\n", pastFrame[i][u32Col].stVFrame.u64PTS);

        if (u64RawPts == pastFrame[i][u32Col].stVFrame.u64PTS)
        {
            *pu32Index = i;
            return 0;
        }
    }

    return -1;
}

HI_S32 getDumpPipe(VI_DEV ViDev, WDR_MODE_E enInWDRMode, HI_U32 *pu32PipeNum, VI_PIPE ViPipeId[])
{
    HI_S32 s32Ret;
    HI_U32 u32PipeNum, i;
    VI_DEV_BIND_PIPE_S stDevBindPipe;

    memset(&stDevBindPipe, 0, sizeof(stDevBindPipe));
    s32Ret = HI_MPI_VI_GetDevBindPipe(ViDev, &stDevBindPipe);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_GetDevBindPipe error 0x%x !\n", s32Ret);
        return s32Ret;
    }

    u32PipeNum = 0;

    switch (enInWDRMode)
    {
        case WDR_MODE_NONE:
        case WDR_MODE_BUILT_IN:
            if (stDevBindPipe.u32Num < 1)
            {
                printf("PipeNum(%d) enInWDRMode(%d) don't match !\n", stDevBindPipe.u32Num, enInWDRMode);
                return HI_FAILURE;
            }
            u32PipeNum = 1;
            ViPipeId[0] = stDevBindPipe.PipeId[0];
            ViPipeId[4] = stDevBindPipe.PipeId[0];
            break;

        case WDR_MODE_2To1_LINE:
        case WDR_MODE_2To1_FRAME:
        case WDR_MODE_2To1_FRAME_FULL_RATE:
            if (2 != stDevBindPipe.u32Num)
            {
                printf("PipeNum(%d) enInWDRMode(%d) don't match !\n", stDevBindPipe.u32Num, enInWDRMode);
                return HI_FAILURE;
            }
            u32PipeNum = 2;
            for (i = 0; i < u32PipeNum; i++)
            {
                ViPipeId[i] = stDevBindPipe.PipeId[i];
            }
            ViPipeId[4] = stDevBindPipe.PipeId[0];
            break;

        case WDR_MODE_3To1_LINE:
        case WDR_MODE_3To1_FRAME:
        case WDR_MODE_3To1_FRAME_FULL_RATE:
            if (3 != stDevBindPipe.u32Num)
            {
                printf("PipeNum(%d) enInWDRMode(%d) don't match !\n", stDevBindPipe.u32Num, enInWDRMode);
                return HI_FAILURE;
            }
            u32PipeNum = 3;
            for (i = 0; i < u32PipeNum; i++)
            {
                ViPipeId[i] = stDevBindPipe.PipeId[i];
            }
            ViPipeId[4] = stDevBindPipe.PipeId[0];
            break;

        case WDR_MODE_4To1_LINE:
        case WDR_MODE_4To1_FRAME:
        case WDR_MODE_4To1_FRAME_FULL_RATE:
            if (4 != stDevBindPipe.u32Num)
            {
                printf("PipeNum(%d) enInWDRMode(%d) don't match !\n", stDevBindPipe.u32Num, enInWDRMode);
                return HI_FAILURE;
            }
            u32PipeNum = 4;
            for (i = 0; i < u32PipeNum; i++)
            {
                ViPipeId[i] = stDevBindPipe.PipeId[i];
            }
            ViPipeId[4] = stDevBindPipe.PipeId[0];
            break;

        default:
            printf("enInWDRMode(%d) error !\n", enInWDRMode);
            return HI_FAILURE;
    }

    *pu32PipeNum = u32PipeNum;

    return HI_SUCCESS;
}


static inline HI_S32 bitWidth2PixelFormat(HI_U32 u32Nbit, PIXEL_FORMAT_E *penPixelFormat)
{
    PIXEL_FORMAT_E enPixelFormat;

    if (8 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_8BPP;
    }
    else if (10 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_10BPP;
    }
    else if (12 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
    }
    else if (14 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_14BPP;
    }
    else if (16 == u32Nbit)
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_16BPP;
    }
    else
    {
        enPixelFormat = PIXEL_FORMAT_RGB_BAYER_16BPP;
    }

    *penPixelFormat = enPixelFormat;
    return HI_SUCCESS;
}

HI_S32 convertBitPixel(HI_U8 *pu8Data, HI_U32 u32DataNum, HI_U32 u32BitWidth, HI_U16 *pu16OutData)
{
    HI_S32 i, u32Tmp, s32OutCnt;
    HI_U32 u32Val;
    HI_U64 u64Val;
    HI_U8 *pu8Tmp = pu8Data;

    s32OutCnt = 0;
    switch(u32BitWidth)
    {
    case 10:
        {
            /* 4 pixels consist of 5 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte4 byte3 byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 5 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32);

                pu16OutData[s32OutCnt++] = u64Val & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 10) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 20) & 0x3ff;
                pu16OutData[s32OutCnt++] = (u64Val >> 30) & 0x3ff;
            }
        }
        break;
    case 12:
        {
            /* 2 pixels consist of 3 bytes  */
            u32Tmp = u32DataNum / 2;

            for (i = 0; i < u32Tmp; i++)
            {
                /* byte2 byte1 byte0 */
                pu8Tmp = pu8Data + 3 * i;
                u32Val = pu8Tmp[0] + (pu8Tmp[1] << 8) + (pu8Tmp[2] << 16);
                pu16OutData[s32OutCnt++] = u32Val & 0xfff;
                pu16OutData[s32OutCnt++] = (u32Val >> 12) & 0xfff;
            }
        }
        break;
    case 14:
        {
            /* 4 pixels consist of 7 bytes  */
            u32Tmp = u32DataNum / 4;

            for (i = 0; i < u32Tmp; i++)
            {
                pu8Tmp = pu8Data + 7 * i;
                u64Val = pu8Tmp[0] + ((HI_U32)pu8Tmp[1] << 8) + ((HI_U32)pu8Tmp[2] << 16) +
                         ((HI_U32)pu8Tmp[3] << 24) + ((HI_U64)pu8Tmp[4] << 32) +
                         ((HI_U64)pu8Tmp[5] << 40) + ((HI_U64)pu8Tmp[6] << 48);

                pu16OutData[s32OutCnt++] = u64Val & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 14) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 28) & 0x3fff;
                pu16OutData[s32OutCnt++] = (u64Val >> 42) & 0x3fff;
            }
        }
        break;
    default:
        fprintf(stderr, "unsuport bitWidth: %d\n", u32BitWidth);
        return -1;
        break;
    }

    return s32OutCnt;
}

int mesh_calibration_proc(VI_PIPE ViPipe,VIDEO_FRAME_S* pVBuf, HI_U32 u32Nbit, HI_U32 u32MeshScale, HI_U32 u32ByteAlign, ISP_MESH_SHADING_TABLE_S *pstMLSCTable)
{

    //HI_U32 u32H;
    HI_U16 *pu16Data = NULL;
    HI_U64 phy_addr, size;
    HI_U8* pUserPageAddr[2];
    HI_U8  *pu8Data;

    HI_S32 s32Ret;
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_BUTT;

    ISP_MLSC_CALIBRATION_CFG_S  stMLSCCaliCfg;
    ISP_PUB_ATTR_S    stIspPubAttr;
    ISP_BLACK_LEVEL_S stBlackLevel;

    bitWidth2PixelFormat(u32Nbit, &enPixelFormat);
    if (enPixelFormat != pVBuf->enPixelFormat)
    {
        fprintf(stderr, "invalid pixel format:%d, u32Nbit: %d\n", pVBuf->enPixelFormat, u32Nbit);
        return HI_FAILURE;
    }

    size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
    phy_addr = pVBuf->u64PhyAddr[0];

    pUserPageAddr[0] = (HI_U8*) HI_MPI_SYS_Mmap(phy_addr, size);
    if (NULL == pUserPageAddr[0])
    {
        return HI_FAILURE;
    }

#if  DUMP_RAW_AND_SAVE_LSC
    HI_U32 u32H;
    FILE* pfd;

    printf("Dump raw frame of vi  to file: \n");

    /* open file */
    pfd = fopen("lsc.raw", "wb");
    if (NULL == pfd)
    {
        printf("open file failed:%s!\n", strerror(errno));
        return HI_FAILURE;
    }

    pu8Data = pUserPageAddr[0];
    if (8 != u32Nbit)
    {
        pu16Data = (HI_U16*)malloc(pVBuf->u32Width * 2);
        if (NULL == pu16Data)
        {
            fprintf(stderr, "alloc memory failed\n");
            HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
        	pUserPageAddr[0] = NULL;
            return HI_FAILURE;
        }
    }

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......dump data......u32Stride[0]: %d, width: %d\n", pVBuf->u32Stride[0], pVBuf->u32Width);
    //fprintf(stderr, "pts: %lld\n", pVBuf->stSupplement.u64RawPts);
    fflush(stderr);

    for (u32H = 0; u32H < pVBuf->u32Height; u32H++)
    {
        if (8 == u32Nbit)
        {
            fwrite(pu8Data, pVBuf->u32Width, 1, pfd);
        }
        else if (16 == u32Nbit)
        {
            fwrite(pu8Data, pVBuf->u32Width, 2, pfd);
			fflush(pfd);
        }
        else
        {
            convertBitPixel(pu8Data, pVBuf->u32Width, u32Nbit, pu16Data);
            fwrite(pu16Data, pVBuf->u32Width, 2, pfd);
        }
        pu8Data += pVBuf->u32Stride[0];
    }
    fflush(pfd);

    fprintf(stderr, "done u32TimeRef: %d!\n", pVBuf->u32TimeRef);
    //fprintf(stderr, "vc num: %lld!\n", pVBuf->u64PrivateData);
    fflush(stderr);
    fclose(pfd);
#endif

    pu8Data = pUserPageAddr[0];
    pu16Data = (HI_U16*)malloc(sizeof(HI_U16)*pVBuf->u32Width * pVBuf->u32Height);
    if (NULL == pu16Data)
    {
        fprintf(stderr, "alloc memory failed\n");
        HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
    	pUserPageAddr[0] = NULL;
        return HI_FAILURE;
    }
    convertBitPixel(pu8Data, pVBuf->u32Width*pVBuf->u32Height, u32Nbit, pu16Data);

    /*Calibration parameter preset*/
    stMLSCCaliCfg.enRawBit = u32Nbit;
    stMLSCCaliCfg.u32MeshScale = u32MeshScale;
    
    HI_MPI_ISP_GetPubAttr(ViPipe, &stIspPubAttr);
    stMLSCCaliCfg.enBayer  = stIspPubAttr.enBayer;
    stMLSCCaliCfg.u16ImgWidth = pVBuf->u32Width;
    stMLSCCaliCfg.u16ImgHeight = pVBuf->u32Height;

    HI_MPI_ISP_GetBlackLevelAttr(ViPipe, &stBlackLevel);
    stMLSCCaliCfg.u16BLCOffsetR  = stBlackLevel.au16BlackLevel[0];
    stMLSCCaliCfg.u16BLCOffsetGr = stBlackLevel.au16BlackLevel[1];
    stMLSCCaliCfg.u16BLCOffsetGb = stBlackLevel.au16BlackLevel[2];
    stMLSCCaliCfg.u16BLCOffsetB  = stBlackLevel.au16BlackLevel[3];  

    //printf("BLC level is %d \n", stMLSCCaliCfg.u16BLCOffsetR);
    s32Ret = HI_MPI_ISP_MeshShadingCalibration(ViPipe, pu16Data, &stMLSCCaliCfg, pstMLSCTable);
	if(HI_SUCCESS != s32Ret)
    {
        if (NULL != pu16Data)
        {
            free(pu16Data);
        }
        HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
        pUserPageAddr[0] = NULL;
           
        return HI_FAILURE;
    }

    if (NULL != pu16Data)
    {
        free(pu16Data);
    }

	HI_MPI_SYS_Munmap(pUserPageAddr[0], size);
    pUserPageAddr[0] = NULL;
    fprintf(stderr, "------Done!\n");
    
    return HI_SUCCESS;

}



HI_S32 lsc_online_cali_proc(VI_PIPE ViPipe, HI_U32 u32Nbit, HI_U32 u32MeshScale, HI_U32 u32Cnt, HI_U32 u32ByteAlign, HI_U32 u32RatioShow)
{
    int                    i, j;
//  int                    ret;
    //HI_CHAR                szYuvName[300] = {0};
    //FILE*                  pfd = NULL;
    HI_S32                 s32MilliSec = 4000;
    HI_U32                 u32CapCnt = 0;
    //HI_U64                 u64IspInfoPhyAddr = 0;
    //VI_CMP_PARAM_S         stCmpPara;
    //ISP_FRAME_INFO_S*      pstIspFrameInfo;
    //ISP_PUB_ATTR_S         stPubAttr;
//  VIDEO_FRAME_INFO_S     stFrame;
    VIDEO_FRAME_INFO_S     astFrame[MAX_FRM_CNT];
    ISP_MESH_SHADING_TABLE_S  stIspMLSCTable;

    /* get VI frame  */
    for (i = 0; i < u32Cnt; i++)
    {
        if (HI_SUCCESS != HI_MPI_VI_GetPipeFrame(ViPipe, &astFrame[i], s32MilliSec))
        {
            printf("get vi Pipe %d frame err\n", ViPipe);
            printf("only get %d frame\n", i);
            break;
        }

        printf("get vi Pipe %d frame num %d ok\n",ViPipe,  i);
    }

    u32CapCnt = i;

    if (0 == u32CapCnt)
    {
        return -1;
    }

    //testIspDngInfo(ViPipe, &stFrame);

    //HI_MPI_VI_ReleasePipeFrame(ViPipe, &stFrame);

    /* dump file */


    for (j = 0; j < u32CapCnt; j++)
    {
        /* save VI frame to file */        
        mesh_calibration_proc(ViPipe, &astFrame[j].stVFrame, u32Nbit, u32MeshScale, u32ByteAlign, &stIspMLSCTable);

        /* release frame after using */
        HI_MPI_VI_ReleasePipeFrame(ViPipe, &astFrame[j]);
    }

#if DUMP_RAW_AND_SAVE_LSC
    FILE *pFile = fopen("gain.txt", "wb");
    if(!pFile)
    {
        printf("create file fails\n");
        return -1;
    }
    fprintf(pFile,"stIspShardingTable.au32XGridWidth = ");
    for(i= 0;i < 16;i++)
    {
        fprintf(pFile,"%d,",stIspMLSCTable.au16XGridWidth[i]);
    }
    fprintf(pFile,"\n");
    fprintf(pFile,"stIspShardingTable.au32YGridHeight = ");
    for(i= 0;i < 16;i++)
    {
        fprintf(pFile,"%d,",stIspMLSCTable.au16YGridWidth[i]);
    }
    fprintf(pFile,"\n");
    fprintf(pFile,"R = \n");
    for(i=0;i<33;i++)
    {
        for(j = 0;j<33;j++)
        {
            fprintf(pFile,"%d,",stIspMLSCTable.au16RGain[i*33+j]);
        }
        fprintf(pFile,"\n");
    }
    fprintf(pFile,"Gr\n");
    for(i=0;i<33;i++)
    {
        for(j = 0;j<33;j++)
        {
            fprintf(pFile,"%d,",stIspMLSCTable.au16GrGain[i*33+j]);
        }
        fprintf(pFile,"\n");
    }
    fprintf(pFile,"Gb\n");
    for(i=0;i<33;i++)
    {
        for(j = 0;j<33;j++)
        {
            fprintf(pFile,"%d,",stIspMLSCTable.au16GbGain[i*33+j]);
        }
        fprintf(pFile,"\n");
    }
    fprintf(pFile,"B\n");
    for(i=0;i<33;i++)
    {
        for(j = 0;j<33;j++)
        {
            fprintf(pFile,"%d,",stIspMLSCTable.au16BGain[i*33+j]);
        }
        fprintf(pFile,"\n");
    }
    fclose(pFile);

#endif

    return 0;
}

#ifdef __HuaweiLite__
HI_S32 app_main(int argc, char* argv[])
#else
HI_S32 main(int argc, char* argv[])
#endif
{
    VI_DEV            ViDev                = 0;
    VI_PIPE           ViPipe               = 0;
    VI_PIPE           ViPipeId[4]          = {0};  /* save main pipe to [4] */
    HI_S32            s32Ret               = 0;
    HI_U32            i                    = 0;
    HI_U32            u32Nbit              = 12;
    HI_U32            u32FrmCnt            = 1;
    HI_U32            u32RawDepth          = 2;
    HI_U32            u32ByteAlign         = 1;
    HI_U32            u32PipeNum           = 0;   /* LineMode -> 1, WDRMode -> 2~3 */
    HI_U32            u32RatioShow         = 1;
    HI_U32            u32MeshScale         = 1;
    COMPRESS_MODE_E   enCompressMode       = COMPRESS_MODE_NONE;
    PIXEL_FORMAT_E    stPixelFrt;
    VI_DUMP_ATTR_S    stRawDumpAttr;
    VI_DUMP_ATTR_S    stDumpAttr;
    VI_DEV_ATTR_S     stDevAttr;
    VI_PIPE_ATTR_S    astBackUpPipeAttr[4];
    VI_PIPE_ATTR_S    stPipeAttr;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\t To see more usage, please enter: ./lsc_online_cali -h\n\n");

    if (argc > 1)
    {
        if (!strncmp(argv[1], "-h", 2))
        {
            usage();
            exit(HI_SUCCESS);
        }
        else
        {
            ViPipe = atoi(argv[1]);    /* pipe*/
        }
    }

    if (argc > 2)
    {
        u32Nbit = atoi(argv[2]);    /* nbit of Raw data:8bit;10bit;12bit */

        if (10 != u32Nbit &&  12 != u32Nbit &&  14 != u32Nbit &&  16 != u32Nbit)
        {
            printf("can't not support %d bits, only support 10bits,12bits,14bits,16bits\n", u32Nbit);
            exit(HI_FAILURE);
        }

    }

    if (argc > 3)
    {
        u32MeshScale = atoi(argv[3]);    /* Scale value of Mesh calibration*/

        if (u32MeshScale > 7)
        {
            printf("can't not support scale mode %d, can choose only from 0~7!\n", u32MeshScale);
            exit(HI_FAILURE);
        }

    }

#ifdef __HuaweiLite__
#else
    signal(SIGINT, SAMPLE_LSCCALI_HandleSig);
    signal(SIGTERM, SAMPLE_LSCCALI_HandleSig);
#endif
    //enCompressMode = COMPRESS_MODE_NONE; /* non_compress_mode */
    u32FrmCnt      = 1;  /* frame number is 1 */
    u32ByteAlign   = 1;  /* convert to Byte align */
    u32RatioShow   = 0;  /* ratio not shown */

    if (1 > u32FrmCnt || MAX_FRM_CNT < u32FrmCnt)
    {
        printf("invalid FrmCnt %d, FrmCnt range from 1 to %d\n", u32FrmCnt, MAX_FRM_CNT);
        exit(HI_FAILURE);
    }

    s32Ret = HI_MPI_VI_GetDevAttr(ViDev, &stDevAttr);

    if (HI_SUCCESS != s32Ret)
    {
        printf("Get dev %d attr failed!\n", ViDev);
        return s32Ret;
    }

    s32Ret = getDumpPipe(ViDev, stDevAttr.stWDRAttr.enWDRMode, &u32PipeNum, ViPipeId);

    if (HI_SUCCESS != s32Ret)
    {
        printf("getDumpPipe failed 0x%x!\n", s32Ret);
        return HI_ERR_VI_INVALID_PARA;
    }

    bitWidth2PixelFormat(u32Nbit, &stPixelFrt);
    printf("Setting Parameter ==> u32Nbit=%d\n", u32Nbit);
    printf("Setting Parameter ==> ScaleMode =%d\n", u32MeshScale);

    for (i = 0; i < u32PipeNum; i++)
    {
        s32Ret = HI_MPI_VI_GetPipeDumpAttr(ViPipeId[i], &stRawDumpAttr);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Get Pipe %d dump attr failed!\n", ViPipe);
            return s32Ret;
        }

        memcpy(&stDumpAttr, &stRawDumpAttr, sizeof(VI_DUMP_ATTR_S));
        stDumpAttr.bEnable                       = HI_TRUE;
        stDumpAttr.u32Depth                      = u32RawDepth;
        //stDumpAttr.stDumpInfo.enDumpType         = VI_DUMP_TYPE_RAW;
        //stDumpAttr.stDumpInfo.enPixelFormat      = stPixelFrt;
        //stDumpAttr.stDumpInfo.enCompressMode     = enCompressMode;
        //stDumpAttr.stDumpInfo.stCropInfo.bEnable = HI_FALSE;
        s32Ret = HI_MPI_VI_SetPipeDumpAttr(ViPipeId[i], &stDumpAttr);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d dump attr failed!\n", ViPipeId[i]);
            return s32Ret;
        }

        s32Ret = HI_MPI_VI_GetPipeAttr(ViPipeId[i], &astBackUpPipeAttr[i]);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Get Pipe %d attr failed!\n", ViPipe);
            return s32Ret;
        }

        memcpy(&stPipeAttr, &astBackUpPipeAttr[i], sizeof(VI_PIPE_ATTR_S));
        stPipeAttr.enCompressMode = enCompressMode;
        s32Ret = HI_MPI_VI_SetPipeAttr(ViPipeId[i], &stPipeAttr);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d attr failed!\n", ViPipe);
            return s32Ret;
        }
    }

    sleep(1);
    printf("--> u32PipeNum=%d\n", u32PipeNum); 

    if (1 == u32PipeNum || 1 == u32FrmCnt)
    {
        lsc_online_cali_proc(ViPipe, u32Nbit, u32MeshScale, u32FrmCnt, u32ByteAlign, u32RatioShow);
    }
    else
    { 
        printf("Please check if PipeNum is equal to 1!\n");
        exit(HI_FAILURE);
    }


    for (i = 0; i < u32PipeNum; i++)
    {
        s32Ret = HI_MPI_VI_SetPipeAttr(ViPipeId[i], &astBackUpPipeAttr[i]);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d attr failed!\n", ViPipe);
            return s32Ret;
        }

        s32Ret = HI_MPI_VI_SetPipeDumpAttr(ViPipeId[i], &stRawDumpAttr);

        if (HI_SUCCESS != s32Ret)
        {
            printf("Set Pipe %d dump attr failed!\n", ViPipe);
            return s32Ret;
        }
    }

    return s32Ret;
}

