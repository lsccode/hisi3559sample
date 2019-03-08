#include <stdio.h>
#include <sys/prctl.h>
#include <pthread.h>

#include "hi_comm_ipcmsg.h"
#include "hi_ipcmsg.h"

HI_S32 g_MCmsgId;
static pthread_t s_mediaReceivePid = -1;

static void Media_Client_Msg_Proc(HI_S32 s32Id, HI_IPCMSG_MESSAGE_S* msg)
{
    printf("receive msg: %s\n", (HI_CHAR *)msg->pBody);
}

static void* Media_Client_Receive_thread(void *arg)
{
    HI_S32 *pId = (HI_S32*)arg;

    prctl(PR_SET_NAME, (unsigned long) "Hi_pTMsgRec", 0, 0, 0);
    printf("id:%d Run...\n", *pId);
    HI_IPCMSG_Run(*pId);
    printf("after Run...\n");
    return HI_NULL;
}

HI_S32 Media_Msg_Init(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_IPCMSG_CONNECT_S stConnectAttr;

    stConnectAttr.u32RemoteId = 1;
    stConnectAttr.u32Port = 0;
    stConnectAttr.u32Priority = 0;
    s32Ret = HI_IPCMSG_AddService("HiMPP_MSG", &stConnectAttr);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_AddService fail\n");
        return s32Ret;
    }
printf("func:%s, line:%d\n", __FUNCTION__, __LINE__);
    s32Ret = HI_IPCMSG_Connect(&g_MCmsgId,"HiMPP_MSG", Media_Client_Msg_Proc);
    if(HI_SUCCESS != s32Ret)
    {
        printf("HI_IPCMSG_Connect fail\n");
        goto EXIT;
    }
printf("func:%s, line:%d\n", __FUNCTION__, __LINE__);

    s32Ret = pthread_create(&s_mediaReceivePid, NULL, Media_Client_Receive_thread, &g_MCmsgId);
    if(HI_SUCCESS != s32Ret)
    {
        printf("Media_Client_Receive_thread create fail\n");
        return s32Ret;
    }

    return HI_SUCCESS;

EXIT1:
    HI_IPCMSG_Disconnect(g_MCmsgId);

EXIT:
    HI_IPCMSG_DelService("HiMPP_MSG");
    return s32Ret;
}

HI_S32 Media_Msg_Deinit(void)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_IPCMSG_Disconnect(g_MCmsgId);

    if(s_mediaReceivePid != -1)
    {
        pthread_join(s_mediaReceivePid, NULL);
    }

    HI_IPCMSG_DelService("HiMPP_MSG");

    return s32Ret;
}



