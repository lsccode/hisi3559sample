#include "sample_comm.h"
#include "hi_comm_vb.h"

VB_BLK SAMPLE_VB_COMM_GetBlock(VB_POOL Pool, HI_U64 u64BlkSize, HI_CHAR *pcMmzName)
{
    return HI_MPI_VB_GetBlock(Pool, u64BlkSize, pcMmzName);
}

HI_S32 SAMPLE_VB_COMM_ReleaseBlock(VB_BLK Block)
{
    return HI_MPI_VB_ReleaseBlock(Block);
}

HI_U64 SAMPLE_VB_COMM_Handle2PhysAddr(VB_BLK Block)
{
    return HI_MPI_VB_Handle2PhysAddr(Block);
}


