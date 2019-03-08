#include "sample_comm.h"
#include "mpi_vpss.h"

HI_S32 SAMPLE_VPSS_COMM_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S* pstVideoFrame, HI_S32 s32MilliSecs)
{
    return HI_MPI_VPSS_GetChnFrame(VpssGrp, VpssChn, pstVideoFrame, s32MilliSecs);
}

HI_S32 SAMPLE_VPSS_COMM_ReleaseChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S* pstVideoFrame)
{
    return HI_MPI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, pstVideoFrame);
}



