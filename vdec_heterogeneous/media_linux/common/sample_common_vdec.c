#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "mpi_sys.h"
#include "sample_msg_api.h"
#include "sample_vdec_api.h"
#include "sample_common_vdec.h"



HI_S32 SAMPLE_COMM_VDEC_DatafifoInit(HI_S32 s32VdecChnNum)
{
    HI_U64 u64DatafifoPhy = 0;
    HI_S32 s32Ret, s32Cnt, s32ChnId;

    for(s32ChnId=0; s32ChnId<s32VdecChnNum; s32ChnId++)
    {
        //linux datafifo init
        s32Ret = SAMPLE_VDEC_DatafifoInit(s32ChnId, &u64DatafifoPhy);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("chn %d datafifo init fail!\n", s32ChnId);
            goto Fail;
        }

         //liteos datafifo init
        s32Cnt = 0;
        while(s32Cnt<5)
        {
            s32Ret = SAMPLE_VDEC_MSG_DatafifoInit(s32ChnId, u64DatafifoPhy);
            if(s32Ret == HI_SUCCESS)
            {
                break;
            }
            s32Cnt++;
        }
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("MSG: chn %d datafifo init fail!\n", s32ChnId);
            goto Fail;
        }
    }
    return HI_SUCCESS;

Fail:

    for(; s32ChnId>=0; s32ChnId--)
    {
        s32Cnt = 0;
        while(s32Cnt<5)
        {
            s32Ret = SAMPLE_VDEC_MSG_DatafifoDeinit(s32ChnId);
            if(s32Ret == HI_SUCCESS)
            {
                break;
            }
            s32Cnt++;
        }
        SAMPLE_VDEC_DatafifoDeinit(s32ChnId);
    }
    return HI_FAILURE;

}



HI_VOID SAMPLE_COMM_VDEC_DatafifoDeinit(HI_S32 s32VdecChnNum)
{
    HI_S32 s32ChnId, s32Cnt, s32Ret;

    for(s32ChnId=0; s32ChnId<s32VdecChnNum; s32ChnId++)
    {
        s32Cnt = 0;
        while(s32Cnt<5)
        {
            s32Ret = SAMPLE_VDEC_MSG_DatafifoDeinit(s32ChnId);
            if(s32Ret == HI_SUCCESS)
            {
                break;
            }
            s32Cnt++;
        }
        SAMPLE_VDEC_DatafifoDeinit(s32ChnId);
    }

    return;
}



HI_S32 SAMPLE_COMM_VDEC_MallocBuffer(HI_U64 *pu64PhyAddr, HI_VOID** ppVirAddr, HI_U32 u32Size)
{
    return HI_MPI_SYS_MmzAlloc_Cached(pu64PhyAddr, ppVirAddr, "vdec_stream", NULL, u32Size);
}

HI_S32 SAMPLE_COMM_VDEC_FreeBuffer(HI_U64 u64PhyAddr, HI_VOID *pVirAddr, HI_U32 u32Size)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_SYS_MmzFlushCache(u64PhyAddr, pVirAddr, u32Size);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_SYS_MmzFlushCache fail!\n");
    }

    return HI_MPI_SYS_MmzFree(u64PhyAddr, pVirAddr);
}


HI_S32 SAMPLE_COMM_VDEC_FlushBuffer(HI_U64 u64PhyAddr, HI_VOID *pVirAddr, HI_U32 u32Size)
{
    if(0 != u32Size)
    {
        return HI_MPI_SYS_MmzFlushCache(u64PhyAddr, pVirAddr, u32Size);
    }

    return HI_SUCCESS;
}

HI_VOID * SAMPLE_COMM_VDEC_SendStream(HI_VOID *pArgs)
{
    VDEC_THREAD_PARAM_S *pstVdecThreadParam =(VDEC_THREAD_PARAM_S *)pArgs;
    HI_BOOL bEndOfStream = HI_FALSE;
    HI_S32 s32UsedBytes = 0, s32ReadLen = 0;
    FILE *fpStrm=NULL;
    HI_U8 *pu8Buf = NULL;
    HI_U64 u64BufPhyAddr;
    VDEC_STREAM_S stStream;
    HI_BOOL bFindStart, bFindEnd;
    HI_U64 u64PTS = 0;
    HI_U32 u32Len, u32Start, u32Cnt;
    HI_S32 s32Ret,  i;
    HI_CHAR cStreamFile[256];
    SAMPLE_VDEC_STREAM_S stSampleStream;

    prctl(PR_SET_NAME, "VideoSendStream", 0,0,0);

    snprintf(cStreamFile, sizeof(cStreamFile), "%s/%s", pstVdecThreadParam->cFilePath,pstVdecThreadParam->cFileName);
    if(cStreamFile != 0)
    {
        fpStrm = fopen(cStreamFile, "rb");
        if(fpStrm == NULL)
        {
            SAMPLE_PRT("chn %d can't open file %s in send stream thread!\n", pstVdecThreadParam->s32ChnId, cStreamFile);
            return (HI_VOID *)(HI_FAILURE);
        }
    }
    printf("\n \033[0;36m chn %d, stream file:%s, userbufsize: %d \033[0;39m\n", pstVdecThreadParam->s32ChnId,
        pstVdecThreadParam->cFileName, pstVdecThreadParam->s32MinBufSize);

    s32Ret = SAMPLE_COMM_VDEC_MallocBuffer(&u64BufPhyAddr, (HI_VOID**)&pu8Buf, pstVdecThreadParam->s32MinBufSize);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("chn %d can't alloc %d in send stream thread!\n", pstVdecThreadParam->s32ChnId, pstVdecThreadParam->s32MinBufSize);
        fclose(fpStrm);
        return (HI_VOID *)(HI_FAILURE);
    }
    fflush(stdout);

    SAMPLE_VDEC_SetReleaseStreamFlag(pstVdecThreadParam->s32ChnId, HI_TRUE);

    stSampleStream.stBufInfo.pVirAddr   = pu8Buf;
    stSampleStream.stBufInfo.u64PhyAddr = u64BufPhyAddr;
    stSampleStream.stBufInfo.u32Len     = pstVdecThreadParam->s32MinBufSize;
    stSampleStream.s32ChnId             = pstVdecThreadParam->s32ChnId;

    u64PTS = pstVdecThreadParam->u64PtsInit;
    while (1)
    {
        if (pstVdecThreadParam->eThreadCtrl == THREAD_CTRL_STOP)
        {
            break;
        }
        else if (pstVdecThreadParam->eThreadCtrl == THREAD_CTRL_PAUSE)
        {
            sleep(1);
            continue;
        }

        if(HI_TRUE != SAMPLE_VDEC_GetReleaseStreamFlag(pstVdecThreadParam->s32ChnId))
        {
            usleep(1000);
            continue;
        }

        bEndOfStream = HI_FALSE;
        bFindStart   = HI_FALSE;
        bFindEnd     = HI_FALSE;
        u32Start     = 0;
        fseek(fpStrm, s32UsedBytes, SEEK_SET);
        s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
        if (s32ReadLen == 0)
        {
            if (pstVdecThreadParam->bCircleSend == HI_TRUE)
            {
                memset(&stStream, 0, sizeof(VDEC_STREAM_S) );
                stStream.bEndOfStream = HI_TRUE;
                memcpy(&stSampleStream.stStream, &stStream, sizeof(stStream));
                SAMPLE_VDEC_SendStreamByDatafifo(pstVdecThreadParam->s32ChnId, &stSampleStream);

                s32UsedBytes = 0;
                fseek(fpStrm, 0, SEEK_SET);
                s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
            }
            else
            {
                break;
            }
        }
        SAMPLE_VDEC_SetReleaseStreamFlag(pstVdecThreadParam->s32ChnId, HI_FALSE);

        if (pstVdecThreadParam->s32StreamMode==VIDEO_MODE_FRAME && pstVdecThreadParam->enType == PT_H264)
        {
            for (i=0; i<s32ReadLen-8; i++)
            {
                int tmp = pu8Buf[i+3] & 0x1F;
                if (  pu8Buf[i    ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 &&
                       (
                           ((tmp == 0x5 || tmp == 0x1) && ((pu8Buf[i+4]&0x80) == 0x80)) ||
                           (tmp == 20 && (pu8Buf[i+7]&0x80) == 0x80)
                        )
                   )
                {
                    bFindStart = HI_TRUE;
                    i += 8;
                    break;
                }
            }

            for (; i<s32ReadLen-8; i++)
            {
                int tmp = pu8Buf[i+3] & 0x1F;
                if (  pu8Buf[i    ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 &&
                            (
                                  tmp == 15 || tmp == 7 || tmp == 8 || tmp == 6 ||
                                  ((tmp == 5 || tmp == 1) && ((pu8Buf[i+4]&0x80) == 0x80)) ||
                                  (tmp == 20 && (pu8Buf[i+7]&0x80) == 0x80)
                              )
                   )
                {
                    bFindEnd = HI_TRUE;
                    break;
                }
            }

            if(i>0)s32ReadLen = i;
            if (bFindStart == HI_FALSE)
            {
                SAMPLE_PRT("chn %d can not find H264 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                    pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == HI_FALSE)
            {
                s32ReadLen = i+8;
            }

        }
        else if (pstVdecThreadParam->s32StreamMode==VIDEO_MODE_FRAME
            && pstVdecThreadParam->enType == PT_H265)
        {
            HI_BOOL  bNewPic = HI_FALSE;
            for (i=0; i<s32ReadLen-6; i++)
            {
                HI_U32 tmp = (pu8Buf[i+3]&0x7E)>>1;
                bNewPic = ( pu8Buf[i+0] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1
                            && (tmp >= 0 && tmp <= 21) && ((pu8Buf[i+5]&0x80) == 0x80) );

                if (bNewPic)
                {
                    bFindStart = HI_TRUE;
                    i += 6;
                    break;
                }
            }

            for (; i<s32ReadLen-6; i++)
            {
                HI_U32 tmp = (pu8Buf[i+3]&0x7E)>>1;
                bNewPic = (pu8Buf[i+0] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1
                            &&( tmp == 32 || tmp == 33 || tmp == 34 || tmp == 39 || tmp == 40 || ((tmp >= 0 && tmp <= 21) && (pu8Buf[i+5]&0x80) == 0x80) )
                             );

                if (bNewPic)
                {
                    bFindEnd = HI_TRUE;
                    break;
                }
            }
            if(i>0)s32ReadLen = i;

            if (bFindStart == HI_FALSE)
            {
                SAMPLE_PRT("chn %d can not find H265 start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                    pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
            }
            if (bFindEnd == HI_FALSE)
            {
                s32ReadLen = i+6;
            }

        }
        else if (pstVdecThreadParam->enType == PT_MJPEG || pstVdecThreadParam->enType == PT_JPEG)
        {
            for (i=0; i<s32ReadLen-1; i++)
            {
                if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8)
                {
                    u32Start = i;
                    bFindStart = HI_TRUE;
                    i = i + 2;
                    break;
                }
            }

            for (; i<s32ReadLen-3; i++)
            {
                if ((pu8Buf[i] == 0xFF) && (pu8Buf[i+1]& 0xF0) == 0xE0)
                {
                     u32Len = (pu8Buf[i+2]<<8) + pu8Buf[i+3];
                     i += 1 + u32Len;
                }
                else
                {
                    break;
                }
            }

            for (; i<s32ReadLen-1; i++)
            {
                if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD9)
                {
                    bFindEnd = HI_TRUE;
                    break;
                }
            }
            s32ReadLen = i+2;

            if (bFindStart == HI_FALSE)
            {
                SAMPLE_PRT("chn %d can not find JPEG start code!s32ReadLen %d, s32UsedBytes %d.!\n",
                    pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
            }
        }
        else
        {
            if((s32ReadLen != 0) && (s32ReadLen < pstVdecThreadParam->s32MinBufSize))
            {
                bEndOfStream = HI_TRUE;
            }
        }

        stStream.u64PTS       = u64PTS;
        stStream.pu8Addr      = pu8Buf + u32Start;
        stStream.u32Len       = s32ReadLen;
        stStream.bEndOfFrame  = (pstVdecThreadParam->s32StreamMode==VIDEO_MODE_FRAME)? HI_TRUE: HI_FALSE;
        stStream.bEndOfStream = bEndOfStream;
        stStream.bDisplay     = 1;
        memcpy(&stSampleStream.stStream, &stStream, sizeof(stStream));

        s32Ret = SAMPLE_COMM_VDEC_FlushBuffer(stSampleStream.stBufInfo.u64PhyAddr + ((HI_UL)stSampleStream.stStream.pu8Addr-(HI_UL)stSampleStream.stBufInfo.pVirAddr),
            (HI_VOID*)stSampleStream.stStream.pu8Addr, stSampleStream.stStream.u32Len);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("chn %d flush buffer fail for 0x%x!\n", pstVdecThreadParam->s32ChnId, s32Ret);
            break;
        }

SendAgain:
        s32Ret = SAMPLE_VDEC_SendStreamByDatafifo(pstVdecThreadParam->s32ChnId, &stSampleStream);
        if( (HI_SUCCESS != s32Ret) && (THREAD_CTRL_START == pstVdecThreadParam->eThreadCtrl) )
        {
            usleep(pstVdecThreadParam->s32IntervalTime);
            goto SendAgain;
        }
        else
        {
            bEndOfStream = HI_FALSE;
            s32UsedBytes = s32UsedBytes +s32ReadLen + u32Start;
            u64PTS += pstVdecThreadParam->u64PtsIncrease;
        }
        usleep(pstVdecThreadParam->s32IntervalTime);
    }

    /* send the flag of stream end */
    memset(&stStream, 0, sizeof(VDEC_STREAM_S) );
    stStream.bEndOfStream = HI_TRUE;
    memcpy(&stSampleStream.stStream, &stStream, sizeof(stStream));
    SAMPLE_VDEC_SendStreamByDatafifo(pstVdecThreadParam->s32ChnId, &stSampleStream);

    fclose(fpStrm);
    if (pu8Buf != HI_NULL)
    {
        SAMPLE_COMM_VDEC_FreeBuffer(u64BufPhyAddr, (HI_VOID*)pu8Buf, pstVdecThreadParam->s32MinBufSize);
        pu8Buf = HI_NULL;
    }
    printf("\033[0;35m chn %d send steam thread return ...  \033[0;39m\n", pstVdecThreadParam->s32ChnId);
    fflush(stdout);

    return (HI_VOID *)HI_SUCCESS;
}




HI_VOID SAMPLE_COMM_VDEC_CmdCtrl(HI_S32 s32ChnNum,VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread)
{
    char c=0;
    HI_S32  i;

    while(1)
    {
        printf("\nSAMPLE_TEST:press 'e' to exit; 'p' to pause; 'r' to resume!\n");
        c = getchar();

        if (c == 'e')
            break;
        else if (c == 'r')
        {
            for (i=0; i<s32ChnNum; i++)
            {
                pstVdecSend[i].eThreadCtrl = THREAD_CTRL_START;
            }

        }
        else if (c == 'p')
        {
            for (i=0; i<s32ChnNum; i++)
            {
                pstVdecSend[i].eThreadCtrl = THREAD_CTRL_PAUSE;
            }
        }
    }
    return;
}



HI_VOID SAMPLE_COMM_VDEC_StartSendStream(HI_S32 s32ChnNum, VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread)
{
    HI_S32  i;

    for(i=0; i<s32ChnNum; i++)
    {
        pVdecThread[i] = 0;
        pthread_create(&pVdecThread[i], 0, SAMPLE_COMM_VDEC_SendStream, (HI_VOID *)&pstVdecSend[i]);
    }
}

HI_VOID SAMPLE_COMM_VDEC_StopSendStream(HI_S32 s32ChnNum, VDEC_THREAD_PARAM_S *pstVdecSend, pthread_t *pVdecThread)
{
    HI_S32  i;

    for(i=0; i<s32ChnNum; i++)
    {
        pstVdecSend[i].eThreadCtrl = THREAD_CTRL_STOP;
        if(0 != pVdecThread[i])
        {
            pthread_join(pVdecThread[i], HI_NULL);
            pVdecThread[i] = 0;
        }
    }
}




