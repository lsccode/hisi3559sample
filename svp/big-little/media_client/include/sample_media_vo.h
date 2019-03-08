#ifndef __SAMPLE_MEDIA_SERVER_VO_H__
#define __SAMPLE_MEDIA_SERVER_VO_H__

/*
*VO CMD
*/
typedef enum hiSAMPLE_MEDIA_VO_CMD_E
{
    SAMPLE_MEDIA_CMD_VO_SENDFRAME = 0,

    SAMPLE_MEDIA_CMD_VO_BUTT
} SAMPLE_MEDIA_VO_CMD_E;

/******************************************************************************
* function : VO Send frame
******************************************************************************/
HI_S32 SAMPLE_VO_SendFrame(HI_S32 VoLayer, HI_S32 VoChn, \
    VIDEO_FRAME_INFO_S *pstVFrame,HI_S32 s32MilliSec);

#endif
