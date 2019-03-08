#include "hi_type.h"
#include "mpi_dsp.h"
#include "mpi_sys.h"
#include "sample_common_photo.h"

HI_S32 SAMPLE_COMM_PHOTO_LoadDspCoreBinary(SVP_DSP_ID_E enCoreId)
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_CHAR * aszBin[4][4] = {
    {"./dsp_bin/dsp0/hi_sram.bin","./dsp_bin/dsp0/hi_iram0.bin","./dsp_bin/dsp0/hi_dram0.bin","./dsp_bin/dsp0/hi_dram1.bin"},
    {"./dsp_bin/dsp1/hi_sram.bin","./dsp_bin/dsp1/hi_iram0.bin","./dsp_bin/dsp1/hi_dram0.bin","./dsp_bin/dsp1/hi_dram1.bin"},
    {"./dsp_bin/dsp2/hi_sram.bin","./dsp_bin/dsp2/hi_iram0.bin","./dsp_bin/dsp2/hi_dram0.bin","./dsp_bin/dsp2/hi_dram1.bin"},
    {"./dsp_bin/dsp3/hi_sram.bin","./dsp_bin/dsp3/hi_iram0.bin","./dsp_bin/dsp3/hi_dram0.bin","./dsp_bin/dsp3/hi_dram1.bin"}};
    s32Ret = HI_MPI_SVP_DSP_PowerOn(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_PowerOn failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[enCoreId][1], enCoreId * 4 + 1);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[enCoreId][0], enCoreId * 4 + 0);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[enCoreId][2], enCoreId * 4 + 2);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_LoadBin(aszBin[enCoreId][3], enCoreId * 4 + 3);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_EnableCore(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_LoadBin failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}



HI_S32 SAMPLE_COMM_PHOTO_UnloadDspCoreBinary(SVP_DSP_ID_E enCoreId)
{
    HI_S32 s32Ret = HI_FAILURE;
    s32Ret = HI_MPI_SVP_DSP_DisableCore(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_DisableCore failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SVP_DSP_PowerOff(enCoreId);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SVP_DSP_PowerOff failed with %#x!\n",  s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_PHOTO_SaveYUVFrame(FILE* pfd, VIDEO_FRAME_S* pVBuf)
{
    unsigned int w, h;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    unsigned char TmpBuff[8192]; //If this value is too small and the image is big, this memory may not be enough
    HI_U64 phy_addr;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/*When the storage format is a planar format, this variable is used to keep the height of the UV component */
    HI_U32 u32Size;
    HI_CHAR* pUserPageAddr[2];

    if (PIXEL_FORMAT_YVU_SEMIPLANAR_420 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 3 / 2;
        u32UvHeight = pVBuf->u32Height / 2;
    }
    else if(PIXEL_FORMAT_YVU_SEMIPLANAR_422 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height) * 2;
        u32UvHeight = pVBuf->u32Height;
    }
    else if(PIXEL_FORMAT_YUV_400 == enPixelFormat)
    {
        u32Size = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
        u32UvHeight = pVBuf->u32Height;
    }
    else
    {
        printf("enPixelFormat==%d error.\n", enPixelFormat);
        return HI_FAILURE;
    }

    phy_addr = pVBuf->u64PhyAddr[0];

    pUserPageAddr[0] = HI_MPI_SYS_Mmap(phy_addr, u32Size);
    if (HI_NULL == pUserPageAddr[0])
    {
        printf("HI_MPI_SYS_Mmap error.\n");
        return HI_FAILURE;
    }

    pVBufVirt_Y = pUserPageAddr[0];
    pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0]) * (pVBuf->u32Height);

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for (h = 0; h < pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }

    if(PIXEL_FORMAT_YUV_400 != enPixelFormat)
    {
        fflush(pfd);
        /* save U ----------------------------------------------------------------*/
        fprintf(stderr, "U......");
        fflush(stderr);

        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

            pMemContent += 1;

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
        }
        fflush(pfd);

        /* save V ----------------------------------------------------------------*/
        fprintf(stderr, "V......");
        fflush(stderr);
        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
        }
    }
    fflush(pfd);

    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pUserPageAddr[0], u32Size);
    pUserPageAddr[0] = HI_NULL;

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_PHOTO_GetIsoByVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrame, HI_U32* pu32Iso)
{
    HI_U64 u64JpegDCFPhyAddr = 0;
    JPEG_DCF_S* pstJpegDcf = HI_NULL;

    u64JpegDCFPhyAddr = pstVideoFrame->stVFrame.stSupplement.u64JpegDCFPhyAddr;

    pstJpegDcf = HI_MPI_SYS_Mmap(u64JpegDCFPhyAddr, sizeof(JPEG_DCF_S));

    *pu32Iso = pstJpegDcf->stIspDCFInfo.stIspDCFUpdateInfo.u32ISOSpeedRatings;

    HI_MPI_SYS_Munmap(pstJpegDcf, sizeof(JPEG_DCF_S));

    return HI_SUCCESS;
}


