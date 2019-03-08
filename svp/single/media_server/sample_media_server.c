
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include "hi_type.h"
#include "hi_ipcmsg.h"
#include "hi_common.h"

#include "sample_media_vpss.h"
#include "sample_media_vo.h"
#include "sample_media_venc.h"
#include "sample_media_ive.h"
#include "sample_media_nnie.h"
#include "sample_comm_ive.h"


static HI_S32 s_s32MediaMsgId = -1;

/******************************************************************************
* function : IPCMG Receive message process
******************************************************************************/
static void SAMPLE_Media_MsgReceiveProc(HI_S32 siId, HI_IPCMSG_MESSAGE_S *pstMsg)
{
    HI_U32 u32ModID=0;

    u32ModID = pstMsg->u32Module;

    switch (u32ModID)
    {
        case HI_ID_VPSS:
            SAMPLE_VPSS_MSG_PROC(pstMsg);
            break;
        case HI_ID_VO:
            SAMPLE_VO_MSG_PROC(pstMsg);
            break;
        case HI_ID_VENC:
            SAMPLE_VENC_MSG_PROC(pstMsg);
            break;
        case HI_ID_IVE:
            SAMPLE_IVE_MSG_PROC(pstMsg);
            break;
        case HI_ID_SVP_NNIE:
            SAMPLE_NNIE_MSG_PROC(pstMsg);
            break;

        default:
            SAMPLE_PRT("receive u32ModID:%d msg %d error!\n", u32ModID, pstMsg->u32CMD);
        break;
    }
}

/******************************************************************************
* function : IPCMG Receive message thread
******************************************************************************/
static void* SAMPLE_Media_MsgReceiveThread(void *pvArg)
{
    prctl(PR_SET_NAME, "ReceiveThread", 0, 0, 0);
    HI_S32 s32Ret = HI_SUCCESS;

    do{
        if(HI_TRUE == HI_IPCMSG_IsConnected(s_s32MediaMsgId))
        {
            printf("id:%d Run...\n", s_s32MediaMsgId);
            HI_IPCMSG_Run(s_s32MediaMsgId);
            printf("after Run...\n");
        }
        else
        {
            HI_IPCMSG_Disconnect(s_s32MediaMsgId);

            s32Ret = HI_IPCMSG_Connect(&s_s32MediaMsgId,"HiMPP_MSG", SAMPLE_Media_MsgReceiveProc);
            if(HI_SUCCESS != s32Ret)
            {
                printf("HI_IPCMSG_Connect fail\n");
                return NULL;
            }
        }
    }while(1);

    return NULL;
}

/******************************************************************************
* function : IPCMG Init
******************************************************************************/
HI_S32 SAMPLE_Media_MSG_Init(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    pthread_t rcv_threadid;
    pthread_attr_t attr;

    HI_IPCMSG_CONNECT_S stConnect;
    stConnect.u32RemoteId = 0;
    stConnect.u32Port = 0;
    stConnect.u32Priority = 0;
    s32Ret = HI_IPCMSG_AddService("HiMPP_MSG", &stConnect);
    SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,"Error(%#x),HI_IPCMSG_AddService failed!\n",s32Ret);

    s32Ret = HI_IPCMSG_TryConnect(&s_s32MediaMsgId,"HiMPP_MSG", SAMPLE_Media_MsgReceiveProc);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, IPCMSG_INIT_FAIL0,
        "Error(%#x),HI_IPCMSG_Connect failed!\n", s32Ret);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    s32Ret = pthread_create(&rcv_threadid, &attr, SAMPLE_Media_MsgReceiveThread, &s_s32MediaMsgId);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, IPCMSG_INIT_FAIL1,
        "Error(%#x),pthread_create failed!\n", s32Ret);

    pthread_attr_destroy(&attr);

    return HI_SUCCESS;

IPCMSG_INIT_FAIL1:
    pthread_attr_destroy(&attr);
    (HI_VOID)HI_IPCMSG_Disconnect(s_s32MediaMsgId);
    s_s32MediaMsgId = -1;
IPCMSG_INIT_FAIL0:
    (HI_VOID)HI_IPCMSG_DelService("HiMPP_MSG");
    return s32Ret;
}

/******************************************************************************
* function : IPCMG Deinit
******************************************************************************/
HI_VOID SAMPLE_Media_MSG_DeInit(void)
{
    if (-1 != s_s32MediaMsgId)
    {
        (HI_VOID)HI_IPCMSG_Disconnect(s_s32MediaMsgId);
        s_s32MediaMsgId = -1;
    }

    (HI_VOID)HI_IPCMSG_DelService("HiMPP_MSG");
}

/******************************************************************************
* function : IPCMG Get connect Id
******************************************************************************/
HI_S32 SAMPLE_Media_MSG_GetSiId(void)
{
    return s_s32MediaMsgId;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */




