#include <stdio.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"
#include "sample_msg_comm_define.h"

HI_S32 g_MSmsgId;

extern HI_S32 MSG_PHOTO_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg);
extern HI_S32 MSG_SNAP_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg);
extern HI_S32 MSG_VPSS_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg);
extern HI_S32 MSG_VENC_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg);
extern HI_S32 MSG_VB_PROC(HI_S32 s32MsgId, HI_IPCMSG_MESSAGE_S *pstMsg);

static void Media_Server_Msg_Proc(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* stMsg)
{
    HI_U32 u32ModID=stMsg->u32Module;

    switch(u32ModID)
    {
        case SAMPLE_MOD_PHOTO:
            MSG_PHOTO_PROC(s32Id, stMsg);
            break;
        case SAMPLE_MOD_VB:
            MSG_VB_PROC(s32Id, stMsg);
            break;
        case SAMPLE_MOD_SNAP:
            MSG_SNAP_PROC(s32Id, stMsg);
            break;
        case SAMPLE_MOD_VPSS:
            MSG_VPSS_PROC(s32Id, stMsg);
            break;
        case SAMPLE_MOD_VENC:
            MSG_VENC_PROC(s32Id, stMsg);
            break;
        default:
            printf("receive u32ModID:%d cmd:%d error.\n", u32ModID, stMsg->u32CMD);
        break;
    }
}

static void* Media_Server_Receive_thread(void *arg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    prctl(PR_SET_NAME, (unsigned long) "Hi_pTMsgRec", 0, 0, 0);
    do{
        if(HI_TRUE == HI_IPCMSG_IsConnected(g_MSmsgId))
        {
            printf("id:%d Run...\n", g_MSmsgId);
            HI_IPCMSG_Run(g_MSmsgId);
            printf("after Run...\n");
        }
        else
        {
            HI_IPCMSG_Disconnect(g_MSmsgId);

            s32Ret = HI_IPCMSG_Connect(&g_MSmsgId,"HiMPP_MSG", Media_Server_Msg_Proc);
            if(HI_SUCCESS != s32Ret)
            {
                printf("HI_IPCMSG_Connect fail\n");
                return NULL;
            }
        }
    }while(1);

    return HI_NULL;
}

HI_S32 Media_Msg_Init(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_CONNECT_S stConnectAttr;
    pthread_t rcv_threadid;

    stConnectAttr.u32RemoteId = 0;
    stConnectAttr.u32Port = 0;
    stConnectAttr.u32Priority = 0;
    s32Ret = HI_IPCMSG_AddService("HiMPP_MSG", &stConnectAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_AddService fail\n");
        return s32Ret;
    }
printf("func:%s, line:%d\n", __FUNCTION__, __LINE__);

    s32Ret = HI_IPCMSG_Connect(&g_MSmsgId,"HiMPP_MSG", Media_Server_Msg_Proc);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_Connect fail\n");
        goto EXIT;
    }
printf("func:%s, line:%d\n", __FUNCTION__, __LINE__);

    pthread_attr_t pthAttr;
    pthread_attr_init(&pthAttr);
    pthread_attr_setdetachstate(&pthAttr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&pthAttr, 0x10000);
    s32Ret = pthread_create(&rcv_threadid, &pthAttr, Media_Server_Receive_thread, &g_MSmsgId);
    if(HI_SUCCESS != s32Ret)
    {
        printf("Media_Server_Receive_thread create fail\n");
        goto EXIT1;
    }
    pthread_attr_destroy(&pthAttr);

    return HI_SUCCESS;

EXIT1:
    HI_IPCMSG_Disconnect(g_MSmsgId);

EXIT:
    HI_IPCMSG_DelService("HiMPP_MSG");
    return s32Ret;
}

HI_S32 Media_Msg_Deinit(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    printf("Media_Msg_Deinit ...\n");

    s32Ret = HI_IPCMSG_Disconnect(g_MSmsgId);

    HI_IPCMSG_DelService("HiMPP_MSG");

    return s32Ret;
}


