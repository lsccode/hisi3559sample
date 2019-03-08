#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_comm_video.h"
#include "sample_media_vpss.h"
#include "mpi_vpss.h"
#include "sample_media_server.h"
#include "sample_comm_ive.h"

#define SAMPLE_MEIDA_MSG_ID SAMPLE_Media_MSG_GetSiId()

/******************************************************************************
* function : VPSS get frame
******************************************************************************/
static HI_S32 SAMPLE_MEDIA_VPSS_GetChnFrame(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VIDEO_FRAME_INFO_S  stFrameInfo;
    HI_S32 s32MilliSec;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstRespMsg;

    VpssGrp = pstMsg->as32PrivData[0];
    VpssChn = pstMsg->as32PrivData[1];
    s32MilliSec = pstMsg->as32PrivData[2];
    
    s32Ret = HI_MPI_VPSS_GetChnFrame(VpssGrp, VpssChn, &stFrameInfo, s32MilliSec);

    pstRespMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &stFrameInfo, sizeof(VIDEO_FRAME_INFO_S));
    
    HI_IPCMSG_SendAsync(SAMPLE_MEIDA_MSG_ID, pstRespMsg, NULL);
    HI_IPCMSG_DestroyMessage(pstRespMsg);

    return HI_SUCCESS;
}

/******************************************************************************
* function : VPSS release frame
******************************************************************************/
static HI_S32 SAMPLE_MEDIA_VPSS_ReleaseChnFrame(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_MESSAGE_S *pstRespMsg;

    VpssGrp = pstMsg->as32PrivData[0];
    VpssChn = pstMsg->as32PrivData[1];

    s32Ret = HI_MPI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, (VIDEO_FRAME_INFO_S *)pstMsg->pBody);

    pstRespMsg = HI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
    
    HI_IPCMSG_SendAsync(SAMPLE_MEIDA_MSG_ID, pstRespMsg, NULL);
    HI_IPCMSG_DestroyMessage(pstRespMsg);

    return HI_SUCCESS;
}

/******************************************************************************
* function : VPSS message process
******************************************************************************/
HI_S32 SAMPLE_VPSS_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    switch(pstMsg->u32CMD)
    {
        case SAMPLE_MEDIA_CMD_VPSS_GETCHNFRAME:
            s32Ret = SAMPLE_MEDIA_VPSS_GetChnFrame(pstMsg);
            break;
        case SAMPLE_MEDIA_CMD_VPSS_RELEASECHNFRAME:
            s32Ret = SAMPLE_MEDIA_VPSS_ReleaseChnFrame(pstMsg);
            break;
         default:
            SAMPLE_PRT("SAMPLE_VPSS_MSG_PROC cmd:%d is error!\n", pstMsg->u32CMD);
            break;
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

