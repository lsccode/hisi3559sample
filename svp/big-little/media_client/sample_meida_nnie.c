#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_common.h"
#include "hi_comm_video.h"
#include "sample_comm_svp.h"
#include "sample_comm_nnie.h"
#include "sample_media_nnie.h"
#include "sample_media_client.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define SAMPLE_MEIDA_MSG_ID SAMPLE_Media_MSG_GetSiId()

#define HI_IPCMSG_SEND_SYNC_TIMEOUT 10000

/******************************************************************************
* function : IVE message process
******************************************************************************/
HI_S32 SAMPLE_NNIE_MsgProcess(HI_BOOL bVo, HI_BOOL bEncode,
    SAMPLE_NNIE_DRAW_RECT_MSG_S *pstDrawRectMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstReqMsg = NULL;
    HI_IPCMSG_MESSAGE_S *pstRespMsg = NULL;

    pstReqMsg = HI_IPCMSG_CreateMessage(HI_ID_SVP_NNIE, SAMPLE_MEDIA_CMD_NNIE_DRAW_RECT_PROC,
                pstDrawRectMsg, sizeof(SAMPLE_NNIE_DRAW_RECT_MSG_S));
    SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstReqMsg,HI_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,HI_IPCMSG_CreateMessage failed!\n");

    pstReqMsg->as32PrivData[0] = bVo;
    pstReqMsg->as32PrivData[1] = bEncode;

    s32Ret = HI_IPCMSG_SendSync(SAMPLE_MEIDA_MSG_ID, pstReqMsg, &pstRespMsg, HI_IPCMSG_SEND_SYNC_TIMEOUT);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),HI_IPCMSG_SendSync failed1\n", s32Ret);
        HI_IPCMSG_DestroyMessage(pstReqMsg);
        return s32Ret;
    }

    s32Ret = pstRespMsg->s32RetVal;
    HI_IPCMSG_DestroyMessage(pstReqMsg);
    HI_IPCMSG_DestroyMessage(pstRespMsg);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

