#ifndef __SAMPLE_MEDIA_SERVER_VPSS_H__
#define __SAMPLE_MEDIA_SERVER_VPSS_H__

/*
* VPSS CMD
*/
typedef enum hiSAMPLE_MEDIA_VPSS_CMD_E
{
    SAMPLE_MEDIA_CMD_VPSS_GETCHNFRAME     = 0,
    SAMPLE_MEDIA_CMD_VPSS_RELEASECHNFRAME = 0x1,

    SAMPLE_MEDIA_CMD_VPSS_BUTT

} SAMPLE_MEDIA_VPSS_CMD_E;

/******************************************************************************
* function : VPSS message process
******************************************************************************/
HI_S32 SAMPLE_VPSS_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg);

#endif
