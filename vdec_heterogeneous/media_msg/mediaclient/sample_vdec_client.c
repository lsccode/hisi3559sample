#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "sample_msg_api.h"
#include "sample_vdec_api.h"
#include "hi_comm_vb.h"
#include "hi_datafifo.h"
#include "hi_ipcmsg.h"
#include "hi_comm_ipcmsg.h"
#include "sample_msg_comm_define.h"
#include "sample_common_vdec.h"


extern HI_S32 g_MCmsgId;


static HI_DATAFIFO_HANDLE g_hDataFifo[VDEC_MAX_CHN_NUM] = {[0 ... VDEC_MAX_CHN_NUM - 1] = HI_DATAFIFO_INVALID_HANDLE};

static HI_BOOL g_bRelease[VDEC_MAX_CHN_NUM] = {[0 ... VDEC_MAX_CHN_NUM - 1] = HI_TRUE};



void SAMPLE_VDEC_ReleaseStream(void* pStream)
{
    SAMPLE_VDEC_STREAM_S *pstSampleStream = (SAMPLE_VDEC_STREAM_S *)pStream;

    if(0 != pstSampleStream->stStream.u32Len)
    {
        g_bRelease[pstSampleStream->s32ChnId] = HI_TRUE;
    }

    return;
}

HI_VOID SAMPLE_VDEC_SetReleaseStreamFlag(HI_S32 s32ChnId, HI_BOOL bFlag)
{
    g_bRelease[s32ChnId] = bFlag;
    return;
}

HI_BOOL SAMPLE_VDEC_GetReleaseStreamFlag(HI_S32 s32ChnId)
{
    HI_DATAFIFO_Write(g_hDataFifo[s32ChnId], HI_NULL);
    return g_bRelease[s32ChnId];
}


HI_S32 SAMPLE_VDEC_DatafifoInit(HI_S32 s32ChnId, HI_U64 *pu64Addr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_DATAFIFO_PARAMS_S stDatafifo;
    HI_U64 phyAddr = 0;

    stDatafifo.u32EntriesNum        = 10;
    stDatafifo.u32CacheLineSize     = sizeof(SAMPLE_VDEC_STREAM_S);
    stDatafifo.bDataReleaseByWriter = HI_TRUE;
    stDatafifo.enOpenMode           = DATAFIFO_WRITER;

    s32Ret = HI_DATAFIFO_Open(&g_hDataFifo[s32ChnId], &stDatafifo);

    if (HI_SUCCESS != s32Ret)
    {
        printf("open datafifo error:%x\n", s32Ret);
        return -1;
    }

    s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[s32ChnId], DATAFIFO_CMD_GET_PHY_ADDR, &phyAddr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("get datafifo phy addr error:%x\n", s32Ret);
        return -1;
    }

    s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[s32ChnId], DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, SAMPLE_VDEC_ReleaseStream);
    if (HI_SUCCESS != s32Ret)
    {
        printf("register callback funtion fail!\n", s32Ret);
        return -1;
    }

    *pu64Addr = phyAddr;

    return HI_SUCCESS;
}


void SAMPLE_VDEC_DatafifoDeinit(HI_S32 s32ChnId)
{
    HI_S32 s32Ret = HI_SUCCESS;

    // call write NULL to flush and release stream buffer.
    s32Ret = HI_DATAFIFO_Write(g_hDataFifo[s32ChnId], NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("write error:%x\n", s32Ret);
    }

    HI_DATAFIFO_Close(g_hDataFifo[s32ChnId]);
    printf("datafifo_deinit finish\n");
    g_hDataFifo[s32ChnId] = HI_DATAFIFO_INVALID_HANDLE;

    return;
}



HI_S32 SAMPLE_VDEC_MSG_DatafifoInit(HI_S32 s32ChnId, HI_U64 u64PhyAdr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;
    HI_U64 u64DatafifoPhy = u64PhyAdr;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VDEC, SAMPLE_VDEC_CMD_DATAFIFO_INIT, &u64DatafifoPhy, sizeof(HI_U64));
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = s32ChnId;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDEC_MSG_DatafifoDeinit(HI_S32 s32ChnId)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VDEC, SAMPLE_VDEC_CMD_DATAFIFO_DEINIT, HI_NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }
    pReq->as32PrivData[0] = s32ChnId;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;
    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    return HI_SUCCESS;
}







HI_S32 SAMPLE_VDEC_SendStreamByDatafifo(HI_S32 s32ChnId, SAMPLE_VDEC_STREAM_S *pstSampleStream)
{
    HI_S32 s32Ret;
    HI_U32 u32AvailWriteLen = 0;

    s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[s32ChnId], DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &u32AvailWriteLen);
    if (HI_SUCCESS != s32Ret)
    {
        printf("get available write len error:%x\n", s32Ret);
        return HI_FAILURE;
    }

    if (u32AvailWriteLen >= sizeof(SAMPLE_VDEC_STREAM_S))
    {
        s32Ret = HI_DATAFIFO_Write(g_hDataFifo[s32ChnId], pstSampleStream);
        if (HI_SUCCESS != s32Ret)
        {
            printf("write error:%x\n", s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_DATAFIFO_CMD(g_hDataFifo[s32ChnId], DATAFIFO_CMD_WRITE_DONE, NULL);
        if (HI_SUCCESS != s32Ret)
        {
            printf("write done error:%x\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VDEC_StartVideo(HI_S32 s32ChnNum)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VDEC, SAMPLE_VDEC_CMD_START_VIDEO, HI_NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }

    pReq->as32PrivData[0] = s32ChnNum;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    printf("msg: start video finish!\n");

    return HI_SUCCESS;
}


HI_S32 SAMPLE_VDEC_StopVideo(HI_S32 s32ChnNum)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pReq = NULL;
    HI_IPCMSG_MESSAGE_S *pResp = NULL;

    pReq = HI_IPCMSG_CreateMessage(SAMPLE_MOD_VDEC, SAMPLE_VDEC_CMD_STOP_VIDEO, HI_NULL, 0);
    if(NULL == pReq)
    {
        printf("HI_IPCMSG_CreateMessage failed!\n");
        return HI_FAILURE;
    }

    pReq->as32PrivData[0] = s32ChnNum;
    s32Ret = HI_IPCMSG_SendSync(g_MCmsgId, pReq, &pResp, SAMPLE_SEND_MSG_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_SendSync failed!\n");
        HI_IPCMSG_DestroyMessage(pReq);
        HI_IPCMSG_DestroyMessage(pResp);
        return s32Ret;
    }
    s32Ret = pResp->s32RetVal;

    HI_IPCMSG_DestroyMessage(pReq);
    HI_IPCMSG_DestroyMessage(pResp);

    printf("msg: stop video finish!\n");

    return s32Ret;
}





