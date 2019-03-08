#ifndef __SAMPLE_SNAP_API__
#define __SAMPLE_SNAP_API__
#include "hi_common.h"
#include "hi_comm_snap.h"

HI_S32 SAMPLE_SNAP_COMM_Trigger(VI_PIPE ViPipe);
HI_S32 SAMPLE_SNAP_COMM_SetAttr(VI_PIPE ViPipe, SNAP_ATTR_S *pstSnapAttr);
HI_S32 SAMPLE_SNAP_COMM_Enable(VI_PIPE ViPipe);
HI_S32 SAMPLE_SNAP_COMM_Disable(VI_PIPE ViPipe);

HI_S32 SAMPLE_SNAP_COMM_GetBNRRaw(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, HI_S32 s32MilliSec);
HI_S32 SAMPLE_SNAP_COMM_ReleaseBNRRaw(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame);


#endif

