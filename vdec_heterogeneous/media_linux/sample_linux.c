#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "sample_vdec_api.h"
#include "sample_common_vdec.h"
#include "sample_msg_api.h"
#include "mpi_sys.h"
#include "mpi_vb.h"




#define SAMPLE_STREAM_PATH "./source_file"


/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_VDEC_Usage(char* sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0)SAMPLE_H265_VDEC_VPSS_VO\n");

    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_VDEC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        HI_MPI_SYS_Exit();
        HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
        HI_MPI_VB_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}


HI_S32 SAMPLE_H265_VDEC_VPSS_VO(void)
{
    HI_S32 i, s32Ret = HI_FAILURE;
    HI_S32 s32VdecChnNum = 16;
    HI_U64 u64PubPhyAddr;
    HI_VOID* pPubVirAddr;
    HI_U64 u64DesPhyAddr;
    VENC_CHN VeChn = 0;
    HI_U32 u32ISO = 0;
    VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];
    pthread_t   VdecThread[VDEC_MAX_CHN_NUM];

    s32Ret = HI_MPI_SYS_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

    s32Ret = Media_Msg_Init();
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Media_Msg_Init failed!\n");
        goto EXIT1;
    }

    s32Ret = SAMPLE_COMM_VDEC_DatafifoInit(s32VdecChnNum);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Media_Msg_Init failed!\n");
        goto EXIT2;
    }

    //start video
    s32Ret = SAMPLE_VDEC_StartVideo(s32VdecChnNum);
    if(HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Media_Msg_Init failed!\n");
        goto EXIT3;
    }

    /************************************************
    step8:  send stream to VDEC
    *************************************************/
    for(i=0; i<s32VdecChnNum; i++)
    {
        snprintf(stVdecSend[i].cFileName, sizeof(stVdecSend[i].cFileName), "1080P_8bit.h265");
        snprintf(stVdecSend[i].cFilePath, sizeof(stVdecSend[i].cFilePath), "%s", SAMPLE_STREAM_PATH);
        stVdecSend[i].enType          = PT_H265;
        stVdecSend[i].s32StreamMode   = VIDEO_MODE_FRAME;
        stVdecSend[i].s32ChnId        = i;
        stVdecSend[i].s32IntervalTime = 1000;
        stVdecSend[i].u64PtsInit      = 0;
        stVdecSend[i].u64PtsIncrease  = 0;
        stVdecSend[i].eThreadCtrl     = THREAD_CTRL_START;
        stVdecSend[i].bCircleSend     = HI_TRUE;
        stVdecSend[i].s32MilliSec     = 0;
        stVdecSend[i].s32MinBufSize   = (1920 * 1080 * 3)>>1;
    }
    SAMPLE_COMM_VDEC_StartSendStream(s32VdecChnNum, &stVdecSend[0], &VdecThread[0]);

    SAMPLE_COMM_VDEC_CmdCtrl(s32VdecChnNum, &stVdecSend[0], &VdecThread[0]);

EXIT5:
    SAMPLE_COMM_VDEC_StopSendStream(s32VdecChnNum, &stVdecSend[0], &VdecThread[0]);

EXIT4:
    SAMPLE_VDEC_StopVideo(s32VdecChnNum);

EXIT3:
    SAMPLE_COMM_VDEC_DatafifoDeinit(s32VdecChnNum);

EXIT2:
    Media_Msg_Deinit();

EXIT1:
    HI_MPI_SYS_Exit();

    return s32Ret;
}



#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 s32Index = 0;

    if (argc < 2)
    {
        SAMPLE_VDEC_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_VDEC_HandleSig);
    signal(SIGTERM, SAMPLE_VDEC_HandleSig);
#endif


    s32Index = atoi(argv[1]);
    switch (s32Index)
    {
        case 0:
            s32Ret = SAMPLE_H265_VDEC_VPSS_VO();
            break;

        default:
            SAMPLE_PRT("the index %d is invaild!\n",s32Index);
            SAMPLE_VDEC_Usage(argv[0]);
            return HI_FAILURE;
    }

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("sample exit success!\n");
    }
    else
    {
        SAMPLE_PRT("sample exit abnormally!\n");
    }

    return (s32Ret);
}



