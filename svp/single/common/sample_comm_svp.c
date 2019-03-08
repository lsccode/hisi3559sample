#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "sample_comm_svp.h"

static HI_BOOL s_bSampleSvpInit = HI_FALSE;
/*
*System init
*/
static HI_S32 SAMPLE_COMM_SVP_SysInit(HI_VOID)
{
    HI_S32 s32Ret = HI_FAILURE;
    VB_CONFIG_S struVbConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    memset(&struVbConf,0,sizeof(VB_CONFIG_S));

    struVbConf.u32MaxPoolCnt             = 2;
    struVbConf.astCommPool[1].u64BlkSize = 768*576*2;
    struVbConf.astCommPool[1].u32BlkCnt  = 1;

    s32Ret = HI_MPI_VB_SetConfig((const VB_CONFIG_S *)&struVbConf);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_SetConf failed!\n", s32Ret);

    s32Ret = HI_MPI_VB_Init();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_Init failed!\n", s32Ret);

    s32Ret = HI_MPI_SYS_Init();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_Init failed!\n", s32Ret);

    return s32Ret;
}
/*
*System exit
*/
static HI_S32 SAMPLE_COMM_SVP_SysExit(HI_VOID)
{
	HI_S32 s32Ret = HI_FAILURE;

	s32Ret = HI_MPI_SYS_Exit();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_Exit failed!\n", s32Ret);

	s32Ret = HI_MPI_VB_Exit();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_Exit failed!\n", s32Ret);

	return HI_SUCCESS;
}
/*
*System init
*/
HI_VOID SAMPLE_COMM_SVP_CheckSysInit(HI_VOID)
{
	if(HI_FALSE == s_bSampleSvpInit)
    {
        if (SAMPLE_COMM_SVP_SysInit())
        {
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_ERROR,"Svp mpi init failed!\n");
            exit(-1);
        }
        s_bSampleSvpInit = HI_TRUE;
    }

	SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_DEBUG,"Svp mpi init ok!\n");
}
/*
*System exit
*/
HI_VOID SAMPLE_COMM_SVP_CheckSysExit(HI_VOID)
{
    if ( s_bSampleSvpInit)
    {
        SAMPLE_COMM_SVP_SysExit();
        s_bSampleSvpInit = HI_FALSE;
    }

	SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_DEBUG,"Svp mpi exit ok!\n");
}

