#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_comm_video.h"
#include "sample_comm_nnie.h"
#include "sample_media_nnie.h"
#include "sample_media_server.h"

#define SAMPLE_MEIDA_MSG_ID SAMPLE_Media_MSG_GetSiId()

/******************************************************************************
* function : NNIE message process:(1.fill rect;2.send venc;3.send vo)
******************************************************************************/
static HI_S32 SAMPLE_MEDIA_NNIE_Process(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    VO_LAYER VoLayer = 0;
    VO_CHN VoChn = 0;
    VENC_CHN VeChn = 0;
    HI_BOOL bEncode;
    HI_BOOL bVo;
    HI_S32 s32MilliSec = 20000;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstRespMsg;
    SAMPLE_NNIE_DRAW_RECT_MSG_S *pstDrawRectMsg;

    bVo = pstMsg->as32PrivData[0];
    bEncode = pstMsg->as32PrivData[1];
    pstDrawRectMsg = (SAMPLE_NNIE_DRAW_RECT_MSG_S *)pstMsg->pBody;
    (HI_VOID)SAMPLE_COMM_SVP_NNIE_FillRect(&pstDrawRectMsg->stFrameInfo, pstDrawRectMsg->pstRegion, 0x0000FF00);

    if (HI_TRUE == bEncode)
    {
        s32Ret = HI_MPI_VENC_SendFrame(VeChn, &pstDrawRectMsg->stFrameInfo, s32MilliSec);
    }

    if (HI_TRUE == bVo)
    {
        s32Ret = HI_MPI_VO_SendFrame(VoLayer, VoChn, &pstDrawRectMsg->stFrameInfo, s32MilliSec);
    }

    pstRespMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);

    HI_IPCMSG_SendAsync(SAMPLE_MEIDA_MSG_ID, pstRespMsg, NULL);
    HI_IPCMSG_DestroyMessage(pstRespMsg);

    return HI_SUCCESS;
}
/******************************************************************************
* function : NNIE message process
******************************************************************************/
HI_S32 SAMPLE_NNIE_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch(pstMsg->u32CMD)
    {
        case SAMPLE_MEDIA_CMD_NNIE_DRAW_RECT_PROC:
            s32Ret = SAMPLE_MEDIA_NNIE_Process(pstMsg);
            break;

         default:
            SAMPLE_PRT("SAMPLE_NNIE_MSG_PROC cmd:%d is error!\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


