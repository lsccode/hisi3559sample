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
#include <semaphore.h>
#include <pthread.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vgs.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vgs.h"
#include "mpi_vi.h"
#include "mpi_vo.h"

#include "sample_comm_ive.h"
#include "sample_comm.h"

static SAMPLE_IVE_SWITCH_S s_stBgModelSwitch = {HI_FALSE,HI_FALSE};
static SAMPLE_VI_CONFIG_S s_stViConfig = {0};

/******************************************************************************
* function : show BgModel sample
******************************************************************************/
HI_VOID SAMPLE_IVE_BgModel(HI_CHAR chEncode, HI_CHAR chVo)
{
    HI_S32 s32Ret;
    PIC_SIZE_E enPicSize = PIC_CIF;

    s_stBgModelSwitch.bVenc = '1' == chEncode ? HI_TRUE : HI_FALSE;
    s_stBgModelSwitch.bVo   = '1' == chVo ? HI_TRUE : HI_FALSE;
    s32Ret = SAMPLE_COMM_IVE_StartViVpssVencVo(&s_stViConfig,&s_stBgModelSwitch,&enPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),SAMPLE_COMM_IVE_StartViVpssVencVo failed!\n",s32Ret);
    }
    else
    {
        SAMPLE_PAUSE();
        SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stBgModelSwitch);
    }

    s_stBgModelSwitch.bVenc = HI_FALSE;
    s_stBgModelSwitch.bVo   = HI_FALSE;
}
/******************************************************************************
* function : BgModel sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_BgModel_HandleSig(HI_VOID)
{
    SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stBgModelSwitch);
    s_stBgModelSwitch.bVenc = HI_FALSE;
    s_stBgModelSwitch.bVo   = HI_FALSE;
}

