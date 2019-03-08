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
#include "sample_dpu_main.h"


static char **s_ppChCmdArgv = NULL;
/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
void SAMPLE_DPU_HandleSig(int s32Signo)
{
    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
		switch (*s_ppChCmdArgv[1])
	    {
            case '0':
            {
                SAMPLE_DPU_VI_VPSS_RECT_MATCH_HandleSig();
                break;
            }
            case '1':
            {
                SAMPLE_DPU_FILE_RECT_MATCH_HandleSig();
                break;
            } 
            default :
            {
                break;
            }
	    }

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_DPU_Usage(char* pchPrgName)
{
    printf("Usage : %s <index> \n", pchPrgName);
    printf("index:\n");
    printf("\t 0) VI->VPSS->RECT->MATCH.\n");
    printf("\t 1) FILE->RECT->MATCH.\n");
}

/******************************************************************************
* function : ive sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int s32Ret = HI_SUCCESS;

    if (argc < 2)
    {
        SAMPLE_DPU_Usage(argv[0]);
        return HI_FAILURE;
    }
	s_ppChCmdArgv = argv;
#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_DPU_HandleSig);
    signal(SIGTERM, SAMPLE_DPU_HandleSig);
#endif

    switch (*argv[1])
    {
        case '0':
        {
            s32Ret = SAMPLE_DPU_VI_VPSS_RECT_MATCH();
             break;
        }
        case '1':
        {
            s32Ret = SAMPLE_DPU_FILE_RECT_MATCH();
             break;
        }
        default :
        {
            SAMPLE_DPU_Usage(argv[0]);
            break;
        }
    }

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }
    
    return s32Ret;
}


