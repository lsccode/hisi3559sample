#ifndef __SAMPLE_MEDIA_VENC_H__
#define __SAMPLE_MEDIA_VENC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*
*VENC CMD
*/
typedef enum hiSAMPLE_MSG_VENC_CMD_E
{
    SAMPLE_MEDIA_CMD_VENC_SENDFRAME = 0,

    SAMPLE_MEDIA_CMD_VENC_BUTT
} SAMPLE_MEDIA_VENC_CMD_E;

/******************************************************************************
* function : Venc message process
******************************************************************************/
HI_S32 SAMPLE_VENC_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_MEDIA_VENC_H__ */
