#ifndef __SAMPLE_MEDIA_IVE_H__
#define __SAMPLE_MEDIA_IVE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*
*IVE CMD
*/
typedef enum hiSAMPLE_MEDIA_IVE_CMD_E
{
    SAMPLE_MEDIA_CMD_IVE_DRAW_RECT_PROC = 0,

    SAMPLE_MEIDA_CMD_IVE_BUTT
} SAMPLE_MEDIA_IVE_CMD_E;

/******************************************************************************
* function : IVE message process
******************************************************************************/
HI_S32 SAMPLE_IVE_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_MSG_IVE_H__ */
