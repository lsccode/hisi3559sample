#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_comm_video.h"
#include "sample_media_venc.h"
#include "mpi_venc.h"
#include "sample_media_server.h"
#include "sample_comm_ive.h"

#define SAMPLE_MEIDA_MSG_ID SAMPLE_Media_MSG_GetSiId()

/******************************************************************************
* function : Venc send frame
******************************************************************************/
HI_S32 SAMPLE_MEDIA_VENC_SendFrame(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    VENC_CHN VencChn = 0;
    HI_S32 s32MilliSec;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstRespMsg;

    VencChn = pstMsg->as32PrivData[0];
    s32MilliSec = pstMsg->as32PrivData[1];

    s32Ret = HI_MPI_VENC_SendFrame(VencChn, (VIDEO_FRAME_INFO_S *)pstMsg->pBody, s32MilliSec);

    pstRespMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    
    HI_IPCMSG_SendAsync(SAMPLE_MEIDA_MSG_ID, pstRespMsg, NULL);
    HI_IPCMSG_DestroyMessage(pstRespMsg);

    return HI_SUCCESS;
}
/******************************************************************************
* function : Venc message process
******************************************************************************/
HI_S32 SAMPLE_VENC_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    switch(pstMsg->u32CMD)
    {
        case SAMPLE_MEDIA_CMD_VENC_SENDFRAME:
            s32Ret = SAMPLE_MEDIA_VENC_SendFrame(pstMsg);
            break;

         default:
            SAMPLE_PRT("SAMPLE_VENC_MSG_PROC cmd:%d is error!\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

