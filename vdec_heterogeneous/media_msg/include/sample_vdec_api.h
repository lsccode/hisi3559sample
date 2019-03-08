#ifndef __SAMPLE_VDEC_API__
#define __SAMPLE_VDEC_API__

#include "hi_comm_vdec.h"

typedef struct hiSAMPLE_BUF_S
{
    HI_U64 u64PhyAddr;
    HI_VOID *pVirAddr;
    HI_U32 u32Len;
} SAMPLE_BUF_S;


typedef struct hiSAMPLE_VDEC_STREAM_S
{
    VDEC_STREAM_S stStream;
    SAMPLE_BUF_S  stBufInfo;
    HI_S32        s32MilliSec;
    HI_S32        s32ChnId;
}SAMPLE_VDEC_STREAM_S;

HI_VOID SAMPLE_VDEC_SetReleaseStreamFlag(HI_S32 s32ChnId, HI_BOOL bFlag);
HI_BOOL SAMPLE_VDEC_GetReleaseStreamFlag(HI_S32 s32ChnId);
HI_S32 SAMPLE_VDEC_CreateDatafifoThread(HI_VOID);
HI_S32 SAMPLE_VDEC_DatafifoInit(HI_S32 VdecChn, HI_U64 *pu64Addr);
HI_VOID SAMPLE_VDEC_DatafifoDeinit(HI_S32 VdecChn);
HI_S32 SAMPLE_VDEC_StartVideo(HI_S32 s32ChnNum);
HI_S32 SAMPLE_VDEC_StopVideo(HI_S32 s32ChnNum);
HI_S32 SAMPLE_VDEC_MSG_DatafifoInit(HI_S32 s32ChnId, HI_U64 u64PhyAdr);
HI_S32 SAMPLE_VDEC_MSG_DatafifoDeinit(HI_S32 s32ChnId);
HI_S32 SAMPLE_VDEC_SendStreamByDatafifo(HI_S32 s32ChnId, SAMPLE_VDEC_STREAM_S *pstSampleStream);


#endif

