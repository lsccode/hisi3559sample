#ifndef __SAMPLE_COMM_DPU_H__
#define __SAMPLE_COMM_DPU_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "mpi_dpu_rect.h"
#include "mpi_dpu_match.h"
#include "hi_comm_dpu_rect.h"
#include "hi_comm_dpu_match.h"


#define SAMPLE_DPU_ALIGN_16 (16)
#define SAMPLE_DPU_FRAME_NUM (1)
#define SAMPLE_DPU_DISP_NUM  (80)

#define SAMPLE_DPU_IN_WIDTH (1280)
#define SAMPLE_DPU_IN_HEIGHT (720)
#define SAMPLE_DPU_DST_WIDTH (1050)
#define SAMPLE_DPU_DST_HEIGHT (560)


#define SAMPLE_DPU_MMZ_FREE(phy,vir)\
do{\
    if ((0 != (phy)) && (0 != (vir)))\
    {\
         HI_MPI_SYS_MmzFree((phy),(HI_VOID *)(HI_UL)(vir));\
         (phy) = 0;\
         (vir) = 0;\
    }\
}while(0)

typedef struct hiSAMPLE_DPU_CONFIG_S
{
    DPU_RECT_GRP DpuRectGrp;
    DPU_RECT_GRP_ATTR_S stDpuRectGrpAttr;
    DPU_RECT_LUT_ID s32LutId[DPU_RECT_MAX_PIPE_NUM];
    DPU_RECT_MEM_INFO_S astDpuRectMemInfo[DPU_RECT_MAX_PIPE_NUM];
    VB_POOL astPipeVbPool[DPU_RECT_MAX_PIPE_NUM];
    FILE *apstPipeFile[DPU_RECT_MAX_PIPE_NUM];
    VB_BLK au32VbBlk[DPU_RECT_MAX_PIPE_NUM];
    
    DPU_MATCH_GRP DpuMatchGrp;
    DPU_MATCH_GRP_ATTR_S stDpuMatchGrpAttr;
    VIDEO_FRAME_INFO_S astDpuMatchSrcFrame[DPU_MATCH_MAX_PIPE_NUM];
    VIDEO_FRAME_INFO_S stDpuMatchDstFrame;
    FILE *pstMatchFile;
}SAMPLE_DPU_CONFIG_S;

/******************************************************************************
* function : calculate stride
******************************************************************************/
HI_U32 SAMPLE_COMM_DPU_CalcStride(HI_U32 u32Width, HI_U8 u8Align);
/******************************************************************************
* function : create dpu rect memory info
******************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_CreateMemInfo(DPU_RECT_MEM_INFO_S *pstMemInfo,HI_U32 u32Size);
/*****************************************************************************
* function : read frame from file
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_ReadOneFrame(FILE *pFp, HI_U8 *pu8Y, 
    HI_U32 u32Width, HI_U32 u32Height, HI_U32 u32Stride);
/*****************************************************************************
* function : get frame from file
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_GetVFrameFromFile(FILE *pFp, VB_POOL hPool, HI_U32 u32Width,
    HI_U32 u32Height,HI_U32 u32Stride, VIDEO_FRAME_INFO_S *pstVFrameInfo, VB_BLK VbBlk);
/*****************************************************************************
* function : load dpu rect lut.
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_LoadLut(HI_CHAR *pszFileName, 
             DPU_RECT_MEM_INFO_S *pstMemInfo, DPU_RECT_LUT_ID *ps32LutId);
/*****************************************************************************
* function : unload dpu rect lut.
*****************************************************************************/
HI_VOID SAMPLE_COMM_DPU_RECT_UnloadLut(DPU_RECT_MEM_INFO_S *pstMemInfo, DPU_RECT_LUT_ID *ps32LutId);
/*****************************************************************************
* function : start dpu rect grp.
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_Start(DPU_RECT_GRP DpuRectGrp,
             DPU_RECT_GRP_ATTR_S *pstGrpAttr, DPU_RECT_CHN_ATTR_S* pastChnAttr);
/*****************************************************************************
* function : stop dpu rect grp
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_Stop(DPU_RECT_GRP DpuRectGrp, DPU_RECT_MODE_E enRectMode);

/*****************************************************************************
* function : write frame to file.
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_WriteFrame2File(FILE *pFp, HI_U32 u32EleSize, VIDEO_FRAME_INFO_S *pstVFrameInfo);

/*****************************************************************************
* function : start dpu match grp.
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_MATCH_Start(DPU_MATCH_GRP DpuMatchGrp,
             DPU_MATCH_GRP_ATTR_S *pstGrpAttr, DPU_MATCH_CHN_ATTR_S* pstChnAttr);
/*****************************************************************************
* function : stop dpu match grp
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_MACTH_Stop(DPU_MATCH_GRP DpuMatchGrp);
/******************************************************************************
* function : create dpu match memory info
******************************************************************************/
HI_S32 SAMPLE_COMM_DPU_MATCH_CreateMemInfo(DPU_MATCH_MEM_INFO_S *pstMemInfo,HI_U32 u32Size);
/*****************************************************************************
* function : get dpu match assist buffer size.
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_MATCH_GetAssistBufSize(HI_U16 u16DispNum, HI_U32 u32DstHeight, HI_U32 *pu32Size);
/*****************************************************************************
* function : bind dpu rect to match
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_Bind_MATCH(DPU_RECT_GRP DpuRectGrp, DPU_MATCH_GRP DpuMatchGrp);
/*****************************************************************************
* function : unbind dpu rect to match
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_RECT_UnBind_MATCH(DPU_RECT_GRP DpuRectGrp, DPU_MATCH_GRP DpuMatchGrp);

/*****************************************************************************
* function : bind vpss to dpu rect
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_VPSS_Bind_RECT(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
        DPU_RECT_GRP DpuRectGrp, DPU_RECT_PIPE DpuRectPipe);
/*****************************************************************************
* function : unbind vpss to dpu rect
*****************************************************************************/
HI_S32 SAMPLE_COMM_DPU_VPSS_UnBind_RECT(VPSS_GRP VpssGrp, VPSS_CHN VpssChn,
        DPU_RECT_GRP DpuRectGrp, DPU_RECT_PIPE DpuRectPipe);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_COMM_DPU_H__ */
