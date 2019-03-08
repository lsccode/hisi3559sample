#include "hi_comm_video.h"
#include "sample_venc_api.h"
#include "sample_common_venc.h"
#include "mpi_sys.h"


static HI_S32 SAMPLE_COMM_VENC_SaveStream_PhyAddr(FILE* pFd, VENC_STREAM_BUF_INFO_S *pstStreamBuf, VENC_STREAM_S* pstStream)
{
    HI_U32 i,j;
    HI_U64 u64SrcPhyAddr;
    HI_U32 u32Left;
    HI_S32 s32Ret = 0;
    HI_VOID* pVirAddr = NULL;

    for(i=0; i<pstStream->u32PackCount; i++)
    {
        for(j=0; j<MAX_TILE_NUM; j++)
        {
            if((pstStream->pstPack[i].u64PhyAddr > pstStreamBuf->u64PhyAddr[j])&&\
                (pstStream->pstPack[i].u64PhyAddr <= pstStreamBuf->u64PhyAddr[j]+pstStreamBuf->u64BufSize[j]))
                break;
        }

        if(pstStream->pstPack[i].u64PhyAddr + pstStream->pstPack[i].u32Len >=
                pstStreamBuf->u64PhyAddr[j] + pstStreamBuf->u64BufSize[j])
        {
            if (pstStream->pstPack[i].u64PhyAddr + pstStream->pstPack[i].u32Offset >=
                pstStreamBuf->u64PhyAddr[j] + pstStreamBuf->u64BufSize[j])
            {
                /* physical address retrace in offset segment */
                u64SrcPhyAddr = pstStreamBuf->u64PhyAddr[j] +
                                ((pstStream->pstPack[i].u64PhyAddr + pstStream->pstPack[i].u32Offset) -
                                (pstStreamBuf->u64PhyAddr[j] + pstStreamBuf->u64BufSize[j]));
                pVirAddr = (HI_VOID*)((HI_U64)pstStreamBuf->pUserAddr[j] + (u64SrcPhyAddr - pstStreamBuf->u64PhyAddr[j]));
                s32Ret = fwrite (pVirAddr, pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset, 1, pFd);
                if(s32Ret<0)
                {
                    SAMPLE_PRT("fwrite err %d\n", s32Ret);
                    return s32Ret;
                }
            }
            else
            {
                /* physical address retrace in data segment */
                u32Left = (pstStreamBuf->u64PhyAddr[j] + pstStreamBuf->u64BufSize[j]) - pstStream->pstPack[i].u64PhyAddr;
                u64SrcPhyAddr = pstStream->pstPack[i].u64PhyAddr + pstStream->pstPack[i].u32Offset;
                pVirAddr = (HI_VOID*)((HI_U64)pstStreamBuf->pUserAddr[j] + (u64SrcPhyAddr - pstStreamBuf->u64PhyAddr[j]));
                s32Ret = fwrite(pVirAddr, u32Left - pstStream->pstPack[i].u32Offset, 1, pFd);
                if(s32Ret<0)
                {
                    SAMPLE_PRT("fwrite err %d\n", s32Ret);
                    return s32Ret;
                }

                u64SrcPhyAddr = pstStreamBuf->u64PhyAddr[j];
                pVirAddr = (HI_VOID*)((HI_U64)pstStreamBuf->pUserAddr[j] + (u64SrcPhyAddr - pstStreamBuf->u64PhyAddr[j]));
                s32Ret = fwrite(pVirAddr, pstStream->pstPack[i].u32Len - u32Left, 1, pFd);
                if(s32Ret<0)
                {
                    SAMPLE_PRT("fwrite err %d\n", s32Ret);
                    return s32Ret;
                }
            }
        }
        else
        {
            /* physical address retrace does not happen */
            u64SrcPhyAddr = pstStream->pstPack[i].u64PhyAddr + pstStream->pstPack[i].u32Offset;
            pVirAddr = (HI_VOID*)((HI_U64)pstStreamBuf->pUserAddr[j] + (u64SrcPhyAddr - pstStreamBuf->u64PhyAddr[j]));
            s32Ret = fwrite (pVirAddr, pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset, 1, pFd);
            if(s32Ret<0)
            {
                SAMPLE_PRT("fwrite err %d\n", s32Ret);
                return s32Ret;
            }
        }
            fflush(pFd);
    }

    return HI_SUCCESS;
}

#ifndef FILE_NAME_LEN
#define FILE_NAME_LEN 128
#endif

HI_S32 SAMPLE_COMM_VENC_SavePicture(VENC_CHN VeChn, HI_U32 u32Cnt, const char* cFileName)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VENC_STREAM_BUF_INFO_S stStreamBufInfo;
    VENC_STREAM_S stStream;
    HI_S32 s32MilliSec = -1;
    HI_U32 i,j;
    VENC_PACK_S stJpegPack[SAMPLE_JPEG_PACK_NUM];
    char acFile[FILE_NAME_LEN] = {0};
    FILE* pFile;

    s32Ret = SAMPLE_VENC_COMM_GetStreamBufInfo(VeChn, &stStreamBufInfo);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_VENC_COMM_GetStreamBufInfo fail.\n");
        return s32Ret;
    }

    stStreamBufInfo.pUserAddr[0] = HI_MPI_SYS_MmapCache(stStreamBufInfo.u64PhyAddr[0], stStreamBufInfo.u64BufSize[0]);
    if(stStreamBufInfo.pUserAddr[0] == NULL)
    {
        SAMPLE_PRT("jpeg buffer HI_MPI_SYS_MmapCache fail.\n");
        return s32Ret;
    }

    for(i=0; i<u32Cnt; i++)
    {
        s32Ret = SAMPLE_VENC_COMM_GetStream(VeChn, &stStream, s32MilliSec);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_VENC_COMM_GetStream fail.\n");
            return s32Ret;
        }

        s32Ret = SAMPLE_VENC_COMM_GetJpegPack(VeChn, stJpegPack);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_VENC_COMM_GetJpegPack fail.\n");
            return s32Ret;
        }

        for(j=0; j<SAMPLE_JPEG_PACK_NUM; j++)
        {
            stJpegPack[j].pu8Addr = stStreamBufInfo.pUserAddr[0] + (stJpegPack[j].u64PhyAddr - stStreamBufInfo.u64PhyAddr[0]);
        }

        snprintf(acFile, FILE_NAME_LEN, "%s%d.jpg", cFileName, i);
        pFile = fopen(acFile, "wb");
        if (pFile == NULL)
        {
            SAMPLE_PRT("open file %s err\n", acFile);
            return HI_FAILURE;
        }

        stStream.pstPack = stJpegPack;
        stStream.u32PackCount = SAMPLE_JPEG_PACK_NUM;
        s32Ret = SAMPLE_COMM_VENC_SaveStream_PhyAddr(pFile, &stStreamBufInfo, &stStream);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_COMM_VENC_SaveStream_PhyAddr fail.\n");
            return s32Ret;
        }

        s32Ret = SAMPLE_VENC_COMM_ReleaseStream(VeChn, &stStream);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("SAMPLE_VENC_COMM_ReleaseStream fail.\n");
            return s32Ret;
        }

        fclose(pFile);

        SAMPLE_PRT("save %s success.\n", acFile);
    }

    HI_MPI_SYS_Munmap(stStreamBufInfo.pUserAddr[0], stStreamBufInfo.u64BufSize[0]);

    return HI_SUCCESS;
}


