#ifndef __SAMPLE_MEDIA_VENC_H__
#define __SAMPLE_MEDIA_VENC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef enum hiSAMPLE_MSG_VENC_CMD_E
{   
    SAMPLE_MEDIA_CMD_VENC_SENDFRAME = 0,

} SAMPLE_MEDIA_VENC_CMD_E;

/******************************************************************************
* function : VENC Send frame
******************************************************************************/
HI_S32 SAMPLE_VENC_SendFrame(HI_S32 VencChn, VIDEO_FRAME_INFO_S *pstVFrame,HI_S32 s32MilliSec);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_MEDIA_VENC_H__ */
