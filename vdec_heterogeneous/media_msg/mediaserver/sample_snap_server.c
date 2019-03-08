#include "sample_comm.h"
#include "mpi_snap.h"


HI_S32 SAMPLE_SNAP_COMM_Trigger(VI_PIPE ViPipe)
{
    return HI_MPI_SNAP_TriggerPipe(ViPipe);
}


HI_S32 SAMPLE_SNAP_COMM_SetAttr(VI_PIPE ViPipe, SNAP_ATTR_S *pstSnapAttr)
{
    return HI_MPI_SNAP_SetPipeAttr(ViPipe, pstSnapAttr);
}


HI_S32 SAMPLE_SNAP_COMM_Enable(VI_PIPE ViPipe)
{
    return HI_MPI_SNAP_EnablePipe(ViPipe);
}

HI_S32 SAMPLE_SNAP_COMM_Disable(VI_PIPE ViPipe)
{
    return HI_MPI_SNAP_DisablePipe(ViPipe);
}

HI_S32 SAMPLE_SNAP_COMM_GetBNRRaw(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame, HI_S32 s32MilliSec)
{
    return HI_MPI_SNAP_GetBNRRaw(ViPipe, pstVideoFrame, s32MilliSec);
}

HI_S32 SAMPLE_SNAP_COMM_ReleaseBNRRaw(VI_PIPE ViPipe, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
    return HI_MPI_SNAP_ReleaseBNRRaw(ViPipe, pstVideoFrame);
}

