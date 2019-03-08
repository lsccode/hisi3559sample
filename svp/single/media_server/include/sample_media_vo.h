#ifndef __SAMPLE_MEDIA_SERVER_VO_H__
#define __SAMPLE_MEDIA_SERVER_VO_H__

/*
*VO CMD
*/
typedef enum hiSAMPLE_MSG_VO_CMD_E
{
    SAMPLE_MEDIA_CMD_VO_SENDFRAME = 0,

    SAMPLE_MEDIA_CMD_VO_BUTT
} SAMPLE_MEDIA_VO_CMD_E;

/******************************************************************************
* function : VO message process
******************************************************************************/
HI_S32 SAMPLE_VO_MSG_PROC(HI_IPCMSG_MESSAGE_S *pstMsg);

#endif
