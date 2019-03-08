#ifndef __SAMPLE_MEDIA_SERVER_VPSS_H__
#define __SAMPLE_MEDIA_SERVER_VPSS_H__

typedef enum hiSAMPLE_MEDIA_VPSS_CMD_E
{   
    SAMPLE_MEDIA_CMD_VPSS_GETCHNFRAME = 0,
    SAMPLE_MEDIA_CMD_VPSS_RELEASECHNFRAME,

} SAMPLE_MEDIA_VPSS_CMD_E;

/******************************************************************************
* function : VPSS Get frame
******************************************************************************/
HI_S32 SAMPLE_VPSS_GetChnFrame(HI_S32 VpssGrp, HI_S32 VpssChn, \
             VIDEO_FRAME_INFO_S *pstFrameInfo, HI_S32 s32MilliSec);

/******************************************************************************
* function : VPSS release frame
******************************************************************************/
HI_S32 SAMPLE_VPSS_ReleaseChnFrame(HI_S32 VpssGrp, HI_S32 VpssChn, \
             VIDEO_FRAME_INFO_S *pstFrameInfo);

#endif 
