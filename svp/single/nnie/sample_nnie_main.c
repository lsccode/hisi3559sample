#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_nnie_main.h"

static char **s_ppChCmdArgv = NULL;
/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
void SAMPLE_SVP_HandleSig(int s32Signo)
{
    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
        switch (*s_ppChCmdArgv[1])
        {
            case '0':
            {
               SAMPLE_SVP_NNIE_Rfcn_HandleSig();
            }
            break;
            default :
            {
            }
            break;
        }

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif
/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_SVP_Usage(char* pchPrgName)
{
    printf("Usage : %s <index> \n", pchPrgName);
    printf("index:\n");
    printf("\t 0) RFCN(VI->VPSS->NNIE->VGS->VO).\n");
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
        SAMPLE_SVP_Usage(argv[0]);
        return HI_FAILURE;
    }
    s_ppChCmdArgv = argv;
#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_SVP_HandleSig);
    signal(SIGTERM, SAMPLE_SVP_HandleSig);
#endif

    switch (*argv[1])
    {
        case '0':
        {
            SAMPLE_SVP_NNIE_Rfcn();
        }
        break;
        default :
        {
            SAMPLE_SVP_Usage(argv[0]);
        }
        break;
    }

    return s32Ret;
}



