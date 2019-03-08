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
#include <pthread.h>
#include <sys/prctl.h>
#include <math.h>

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "sample_comm.h"
#include "sample_comm_svp.h"
#include "sample_nnie_main.h"
#include "sample_comm_ive.h"
#include "sample_media_server.h"

/*rfcn para*/
static SAMPLE_IVE_SWITCH_S s_stRfcnSwitch = {HI_FALSE,HI_FALSE};
static SAMPLE_VI_CONFIG_S s_stViConfig = {0};

/******************************************************************************
* function : Rfcn
******************************************************************************/
void SAMPLE_SVP_NNIE_Rfcn(void)
{
    PIC_SIZE_E enSize = PIC_CIF;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_Media_MSG_Init();
    SAMPLE_SVP_CHECK_EXPR_RET_VOID(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_Media_MSG_Init failed!\n",s32Ret);

    s_stRfcnSwitch.bVenc = HI_FALSE;
    s_stRfcnSwitch.bVo   = HI_TRUE;
    s32Ret = SAMPLE_COMM_IVE_StartViVpssVencVo(&s_stViConfig,&s_stRfcnSwitch,&enSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),SAMPLE_COMM_IVE_StartViVpssVencVo failed!\n",s32Ret);
    }
    else
    {
        SAMPLE_PAUSE();
        SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stRfcnSwitch);
    }

    SAMPLE_Media_MSG_DeInit();

    return ;
}

/******************************************************************************
* function : rfcn sample signal handle
******************************************************************************/
void SAMPLE_SVP_NNIE_Rfcn_HandleSig(void)
{
    SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stRfcnSwitch);
    SAMPLE_Media_MSG_DeInit();
}

