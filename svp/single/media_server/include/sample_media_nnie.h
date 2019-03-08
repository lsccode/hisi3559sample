#ifndef __SAMPLE_MEDIA_NNIE_H__
#define __SAMPLE_MEDIA_NNIE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*
*NNIE CMD
*/
typedef enum hiSAMPLE_MEDIA_NNIE_CMD_E
{
    SAMPLE_MEDIA_CMD_NNIE_DRAW_RECT_PROC = 0,

    SAMPLE_MEIDA_CMD_NNIE_BUTT
} SAMPLE_MEDIA_NNIE_CMD_E;

/******************************************************************************
* function : IVE message process
******************************************************************************/
HI_S32 SAMPLE_NNIE_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_MSG_NNIE_H__ */


