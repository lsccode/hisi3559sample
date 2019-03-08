#ifndef __SAMPLE_MEDIA_NNIE_H__
#define __SAMPLE_MEDIA_NNIE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*
*IVE CMD
*/
typedef enum hiSAMPLE_MEDIA_NNIE_CMD_E
{
    SAMPLE_MEDIA_CMD_NNIE_DRAW_RECT_PROC = 0,

    SAMPLE_MEIDA_CMD_NNIE_BUTT
} SAMPLE_MEDIA_NNIE_CMD_E;

/******************************************************************************
* function : NNIE message process
******************************************************************************/
HI_S32 SAMPLE_NNIE_MsgProcess(HI_BOOL bVo, HI_BOOL bEncode,
        SAMPLE_NNIE_DRAW_RECT_MSG_S *pstDrawRectMsg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_MSG_IVE_H__ */

