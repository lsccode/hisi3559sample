#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_comm_video.h"
#include "sample_media_vo.h"
#include "mpi_vo.h"
#include "sample_media_server.h"
#include "sample_comm_ive.h"

#define SAMPLE_MEIDA_MSG_ID SAMPLE_Media_MSG_GetSiId()

/******************************************************************************
* function : VO Send frame
******************************************************************************/
static HI_S32 SAMPLE_MEDIA_VO_SendFrame(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    VO_LAYER VoLayer;
    VO_CHN VoChn;
    HI_S32 s32MilliSec;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstRespMsg;

    VoLayer = pstMsg->as32PrivData[0];
    VoChn = pstMsg->as32PrivData[1];
    s32MilliSec = pstMsg->as32PrivData[2];

    s32Ret = HI_MPI_VO_SendFrame(VoLayer, VoChn, (VIDEO_FRAME_INFO_S *)pstMsg->pBody, s32MilliSec);

    pstRespMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    
    HI_IPCMSG_SendAsync(SAMPLE_MEIDA_MSG_ID, pstRespMsg, NULL);
    HI_IPCMSG_DestroyMessage(pstRespMsg);

    return HI_SUCCESS;
}

/******************************************************************************
* function : VO message process
******************************************************************************/
HI_S32 SAMPLE_VO_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    switch(pstMsg->u32CMD)
    {
        case SAMPLE_MEDIA_CMD_VO_SENDFRAME:
            s32Ret = SAMPLE_MEDIA_VO_SendFrame(pstMsg);
            break;

         default:
            SAMPLE_PRT("SAMPLE_VO_MSG_PROC cmd:%d is error!\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

