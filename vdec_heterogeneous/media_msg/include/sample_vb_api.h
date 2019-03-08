#ifndef __SAMPLE_VB_API__
#define __SAMPLE_VB_API__

#include "hi_common.h"
#include "hi_comm_vb.h"

VB_BLK SAMPLE_VB_COMM_GetBlock(VB_POOL Pool, HI_U64 u64BlkSize, HI_CHAR *pcMmzName);
HI_S32 SAMPLE_VB_COMM_ReleaseBlock(VB_BLK Block);
HI_U64 SAMPLE_VB_COMM_Handle2PhysAddr(VB_BLK Block);


#endif

