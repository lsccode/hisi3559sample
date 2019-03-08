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
#include "hi_comm_video.h"
#include "hi_comm_sys.h"

#include "sample_comm_ive.h"
#include "sample_comm_svp.h"
#include "sample_dsp_main.h"
#include "sample_media_server.h"

static SAMPLE_VI_CONFIG_S s_stViConfig = {0};
static SAMPLE_IVE_SWITCH_S s_stDspSwitch = {HI_FALSE,HI_FALSE};

static HI_VOID SAMPLE_SVP_DSP_DilateCore(HI_VOID)
{
    HI_S32 s32Ret;
    PIC_SIZE_E enSize = PIC_CIF;

    s_stDspSwitch.bVo   = HI_TRUE;
    s_stDspSwitch.bVenc = HI_FALSE;

    s32Ret = SAMPLE_Media_MSG_Init();
    SAMPLE_CHECK_EXPR_RET_VOID(HI_SUCCESS != s32Ret,"Error(%#x),SAMPLE_Media_MSG_Init failed!\n",s32Ret);

    s32Ret = SAMPLE_COMM_IVE_StartViVpssVencVo(&s_stViConfig,&s_stDspSwitch,&enSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Error(%#x),SAMPLE_COMM_IVE_StartViVpssVencVo failed!\n",s32Ret);
    }
    else
    {
        SAMPLE_PAUSE();
        SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stDspSwitch);
    }

    SAMPLE_Media_MSG_DeInit();
}

/*
*Dilate sample
*/
HI_VOID SAMPLE_SVP_DSP_Dilate(HI_VOID)
{
    SAMPLE_SVP_DSP_DilateCore();
}

/*
*Dilate single handle
*/
HI_VOID SAMPLE_SVP_DSP_DilateHandleSig(HI_VOID)
{
    SAMPLE_COMM_IVE_StopViVpssVencVo(&s_stViConfig,&s_stDspSwitch);
}


