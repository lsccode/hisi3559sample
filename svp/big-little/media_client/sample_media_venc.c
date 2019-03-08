#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_common.h"
#include "hi_comm_video.h"
#include "sample_media_venc.h"
#include "sample_media_client.h"
#include "sample_comm_ive.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#define SAMPLE_MEIDA_MSG_ID SAMPLE_Media_MSG_GetSiId()

#define HI_IPCMSG_SEND_SYNC_TIMEOUT 10000

/******************************************************************************
* function : VENC Send frame
******************************************************************************/
HI_S32 SAMPLE_VENC_SendFrame(HI_S32 VencChn,
        VIDEO_FRAME_INFO_S *pstVFrame,HI_S32 s32MilliSec)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstReq = NULL;
    HI_IPCMSG_MESSAGE_S *pstResp = NULL;

    pstReq = HI_IPCMSG_CreateMessage(HI_ID_VENC, SAMPLE_MEDIA_CMD_VENC_SENDFRAME,
            pstVFrame, sizeof(VIDEO_FRAME_INFO_S));
    SAMPLE_CHECK_EXPR_RET(NULL == pstReq,HI_FAILURE,"Error,HI_IPCMSG_CreateMessage failed!\n");

    pstReq->as32PrivData[0] = VencChn;
    pstReq->as32PrivData[1] = s32MilliSec;

    s32Ret = HI_IPCMSG_SendSync(SAMPLE_MEIDA_MSG_ID, pstReq, &pstResp, HI_IPCMSG_SEND_SYNC_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),HI_IPCMSG_SendSync failed!\n", s32Ret);
        HI_IPCMSG_DestroyMessage(pstReq);
        return s32Ret;
    }

    s32Ret = pstResp->s32RetVal;

    HI_IPCMSG_DestroyMessage(pstReq);
    HI_IPCMSG_DestroyMessage(pstResp);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

