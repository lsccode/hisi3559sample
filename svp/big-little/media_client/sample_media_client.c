#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "hi_ipcmsg.h"
#include "sample_comm_ive.h"

static HI_S32 s_s32MediaMsgId = -1;
static pthread_t s_mediaReceivePid = -1;

/******************************************************************************
* function : IPCMG Connect handle message
******************************************************************************/
static void SAMPLE_MEDIA_HandleMessage(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* pstMsg)
{
    SAMPLE_PRT("receive pstMsg: %s\n", (HI_CHAR *)pstMsg->pBody);
}

/******************************************************************************
* function : IPCMG Receive thread
******************************************************************************/
static void* SAMPLE_MEDIA_ReceiveThread(void *pvArg)
{
    HI_S32 *ps32Id = (HI_S32*)pvArg;
    SAMPLE_PRT("Run...\n");
    HI_IPCMSG_Run(*ps32Id);
    SAMPLE_PRT("after Run...\n");
    return HI_NULL;
}

/******************************************************************************
* function : IPCMG Init
******************************************************************************/
HI_S32 SAMPLE_Media_MSG_Init(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_CONNECT_S stConnect;
    HI_CHAR acThreadName[16] = {0};

    stConnect.u32RemoteId = 1;
    stConnect.u32Port = 0;
    stConnect.u32Priority = 0;
    s32Ret = HI_IPCMSG_AddService("HiMPP_MSG", &stConnect);
    SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,"Error(%#x),HI_IPCMSG_AddService failed!\n",s32Ret);

    s32Ret = HI_IPCMSG_Connect(&s_s32MediaMsgId,"HiMPP_MSG", SAMPLE_MEDIA_HandleMessage);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, IPCMSG_INIT_FAIL0,
        "Error(%#x),HI_IPCMSG_Connect failed!\n", s32Ret);

    snprintf(acThreadName, 16, "ReceiveThread");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0,0,0);
    s32Ret = pthread_create(&s_mediaReceivePid, NULL, SAMPLE_MEDIA_ReceiveThread, &s_s32MediaMsgId);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, IPCMSG_INIT_FAIL1,
        "Error(%#x),pthread_create failed!\n", s32Ret);
    return HI_SUCCESS;

IPCMSG_INIT_FAIL1:
    (HI_VOID)HI_IPCMSG_Disconnect(s_s32MediaMsgId);
    s_s32MediaMsgId = -1;
IPCMSG_INIT_FAIL0:
    (HI_VOID)HI_IPCMSG_DelService("HiMPP_MSG");
    return s32Ret;
}

/******************************************************************************
* function : IPCMG Deinit
******************************************************************************/
HI_VOID SAMPLE_Media_MSG_DeInit()
{
    if (-1 != s_s32MediaMsgId)
    {
        (HI_VOID)HI_IPCMSG_Disconnect(s_s32MediaMsgId);
        s_s32MediaMsgId = -1;
    }
    if(s_mediaReceivePid != -1)
    {
        pthread_join(s_mediaReceivePid, NULL);
        s_mediaReceivePid = -1;
    }
    (HI_VOID)HI_IPCMSG_DelService("HiMPP_MSG");
}

/******************************************************************************
* function : IPCMG Get connect Id
******************************************************************************/
HI_S32 SAMPLE_Media_MSG_GetSiId()
{
    return s_s32MediaMsgId;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */



