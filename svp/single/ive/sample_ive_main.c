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

#include "sample_comm_ive.h"
#include "sample_ive_main.h"
#include "sample_media_server.h"

static char **s_ppChCmdArgv = NULL;

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
HI_VOID SAMPLE_IVE_HandleSig(HI_S32 s32Signo)
{
    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
        switch (*s_ppChCmdArgv[1])
        {
            case '0':
                {
                    SAMPLE_IVE_BgModel_HandleSig();
                }
                break;
             case '1':
                {
                    SAMPLE_IVE_Gmm_HandleSig();
                }
                break;
            case '2':
                {
                    SAMPLE_IVE_Od_HandleSig();
                }
                break;
            case '3':
                {
                    SAMPLE_IVE_Md_HandleSig();
                }
                break;
            default :
                {
                }
                break;
        }

        SAMPLE_Media_MSG_DeInit();

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif
/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_IVE_Usage(HI_CHAR* pchPrgName)
{
    printf("Usage : %s <index> [encode] [vo]\n", pchPrgName);
    printf("index:\n");
    printf("\t 0)BgModel,<encode>:0, not encode;1,encode.<vo>:0,not call vo;1,call vo.(VI->VPSS->IVE->VGS->[VENC_H264]->[VO_HDMI]).\n");
    printf("\t 1)Gmm,<encode>:0, not encode;1,encode.<vo>:0,not call vo;1,call vo.(VI->VPSS->IVE->VGS->[VENC_H264]->[VO_HDMI]).\n");
    printf("\t 2)Occlusion detected.(VI->VPSS->IVE->VO_HDMI).\n");
    printf("\t 3)Motion detected.(VI->VPSS->IVE->VGS->VO_HDMI).\n");
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
    HI_S32 s32Ret = HI_SUCCESS;

    if (argc < 2)
    {
        SAMPLE_IVE_Usage(argv[0]);
        return HI_FAILURE;
    }
    s_ppChCmdArgv = argv;
#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_IVE_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_HandleSig);
#endif

    s32Ret = SAMPLE_Media_MSG_Init();
    SAMPLE_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,"Error(%#x),SAMPLE_Media_MSG_Init failed!\n",s32Ret);

    switch (*argv[1])
    {
        case '0':
            {
                if ((argc < 4) || (('0' != *argv[2]) && ('1' != *argv[2])) || (('0' != *argv[3]) && ('1' != *argv[3])))
                {
                    SAMPLE_IVE_Usage(argv[0]);
                    return HI_FAILURE;
                }
                SAMPLE_IVE_BgModel(*argv[2], *argv[3]);
            }
            break;
        case '1':
            {
                if ((argc < 4) || (('0' != *argv[2]) && ('1' != *argv[2])) || (('0' != *argv[3]) && ('1' != *argv[3])))
                {
                    SAMPLE_IVE_Usage(argv[0]);
                    return HI_FAILURE;
                }
                SAMPLE_IVE_Gmm(*argv[2], *argv[3]);
            }
            break;
        case '2':
            {
                SAMPLE_IVE_Od();
            }
            break;
        case '3':
            {
                SAMPLE_IVE_Md();
            }
            break;
        default :
        {
            SAMPLE_IVE_Usage(argv[0]);
        }
        break;
    }

   SAMPLE_Media_MSG_DeInit();

   return s32Ret;
}



