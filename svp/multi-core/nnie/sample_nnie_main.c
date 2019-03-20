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

/******************************************************************************
* function : to process abnormal case
******************************************************************************/

#ifndef __HuaweiLite__
void SAMPLE_SVP_HandleSig(int s32Signo)
{
    int switchNum = atoi(s_ppChCmdArgv[1]);
    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
        switch (switchNum)
        {
            case 0:
            {
                SAMPLE_SVP_NNIE_Rfcn_HandleSig();
            }
            break;
            case 1:
            {
                SAMPLE_SVP_NNIE_Segnet_HandleSig();
            }
            break;
            case 2:
            {
                SAMPLE_SVP_NNIE_FasterRcnn_HandleSig();
            }
            break;
            case 3:
            {
                SAMPLE_SVP_NNIE_FasterRcnn_HandleSig();
            }
            break;
            case 4:
            {
                SAMPLE_SVP_NNIE_Cnn_HandleSig();
            }
            break;
            case 5:
            {
                SAMPLE_SVP_NNIE_Ssd_HandleSig0();
                SAMPLE_SVP_NNIE_Ssd_HandleSig1();
                SAMPLE_VENC_NNIE_HandleSig(s32Signo);
                //SAMPLE_SVP_NNIE_Ssd_HandleSigFoward0();
                //SAMPLE_SVP_NNIE_Ssd_HandleSigFoward1();
            }
            break;
            case 6:
            {
                SAMPLE_SVP_NNIE_Yolov1_HandleSig();
            }
            break;
            case 7:
            {
                SAMPLE_SVP_NNIE_Yolov2_HandleSig();
            }
            break;
            case 8:
            {
                SAMPLE_SVP_NNIE_Lstm_HandleSig();
            }
            break;
            case 9:
            {
                SAMPLE_SVP_NNIE_Mobilenet_Ssd_HandleSig();
            }
            break;
            case 10:
            {
                SAMPLE_SVP_NNIE_SsdCaffeProfiling_HandleSig();
            }
            break;
            case 11:
            {
                SAMPLE_SVP_NNIE_CaffeModelConv_HandleSig();
            }
            break;
            case 12:
            {
                SAMPLE_SVP_NNIE_DepthWiseConv_HandleSig();
            }
            break;
            case 13:
            {
                SAMPLE_SVP_NNIE_DepthWiseNormConv_HandleSig();
            }
            break;
            case 14:
            {
                SAMPLE_SVP_NNIE_PointWiseNormConv_HandleSig();
            }
            break;
            case 15:
            {
                SAMPLE_SVP_NNIE_SSDNormConv_HandleSig();
            }
            break;
            case 16:
            {
                SAMPLE_SVP_NNIE_SSDNormConv1X1_128x256x256_256_HandleSig();
            }
            break;
            case 17:
            {
                SAMPLE_SVP_NNIE_SSDNormConv1x1_128x256x256_128_HandleSig();
            }
            break;
            case 18:
            {
                SAMPLE_SVP_NNIE_SSDNormConv1x3_128x256x256_128_HandleSig();
            }
            break;
            case 19:
            {
                SAMPLE_SVP_NNIE_SSDNormConv1x5_128x256x256_128_HandleSig();
            }
            break;
            case 20:
            {
                SAMPLE_SVP_NNIE_SSDNormConv3x1_128x256x256_128_HandleSig();
            }
            break;
            case 21:
            {
                SAMPLE_SVP_NNIE_SSDNormConv3x3_128x256x256_128_HandleSig();
            }
            break;
            case 22:
            {
                SAMPLE_SVP_NNIE_SSDNormConv3x5_128x256x256_128_HandleSig();
            }
            break;
            case 23:
            {
                SAMPLE_SVP_NNIE_SSDNormConv5x1_128x256x256_128_HandleSig();
            }
            break;
            case 24:
            {
                SAMPLE_SVP_NNIE_SSDNormConv5x3_128x256x256_128_HandleSig();
            }
            break;
            case 25:
            {
                SAMPLE_SVP_NNIE_SSDNormConv5x5_128x256x256_128_HandleSig();
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
    printf("\t 0)  RFCN(VI->VPSS->NNIE->VGS->VO).\n");
    printf("\t 1)  Segnet(Read File).\n");
    printf("\t 2)  FasterRcnnAlexnet(Read File).\n");
    printf("\t 3)  FasterRcnnDoubleRoiPooling(Read File).\n");
    printf("\t 4)  Cnn(Read File).\n");
    printf("\t 5)  SSD(Read File).\n");
    printf("\t 6)  Yolov1(Read File).\n");
    printf("\t 7)  Yolov2(Read File).\n");
    printf("\t 8)  LSTM(Read File).\n");
    printf("\t 9)  mobilenet-ssd(Read File).\n");
    printf("\t 10) ssd caffe profiling(Read File).\n");
    printf("\t 11) ssd caffe conv (Read File).\n");
    printf("\t 12) mobilenet depthwise conv (Read File).\n");
    printf("\t 13) mobilenet depthwise norm conv (Read File).\n");
    printf("\t 14) mobilenet Pointwise norm conv (Read File).\n");
    printf("\t 15) ssd norm conv (Read File).\n");
    printf("\t 16) conv_1x1_128x256x256_256 \n");
    printf("\t 17) conv_1x1_128x256x256_128 \n");
    printf("\t 18) conv_1x3_128x256x256_128 \n");
    printf("\t 19) conv_1x5_128x256x256_128 \n");
    printf("\t 20) conv_3x1_128x256x256_128 \n");
    printf("\t 21) conv_3x3_128x256x256_128 \n");
    printf("\t 22) conv_3x5_128x256x256_128 \n");
    printf("\t 23) conv_5x1_128x256x256_128 \n");
    printf("\t 24) conv_5x3_128x256x256_128 \n");
    printf("\t 25) conv_5x5_128x256x256_128 \n");
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
    int switchNum = 0;

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

    switchNum = atoi(argv[1]);

    switch (switchNum)
    {
        case 0:
        {
            SAMPLE_SVP_NNIE_Rfcn();
        }
        break;
        case 1:
        {
            SAMPLE_SVP_NNIE_Segnet();
        }
        break;
        case 2:
        {
            SAMPLE_SVP_NNIE_FasterRcnn();
        }
        break;
        case 3:
        {
            SAMPLE_SVP_NNIE_FasterRcnn_DoubleRoiPooling();
        }
        break;
        case 4:
        {
            SAMPLE_SVP_NNIE_Cnn();
        }
        break;
        case 5:
        {
            pthread_t threadvenc;
            pthread_t thread0;
            //pthread_create(&thread0,NULL,SAMPLE_SVP_NNIE_Ssd0,NULL);
            //SAMPLE_SVP_NNIE_Ssd1(NULL);
            SAMPLE_SVP_NNIE_Ssd0(NULL);
            //pthread_create(&threadvenc,NULL,SAMPLE_VENC_4K120,NULL);
            //sleep(30);
            //pthread_create(&thread0,NULL,SAMPLE_SVP_NNIE_SsdForward0,NULL);
            //SAMPLE_SVP_NNIE_SsdForward1(NULL);
            //SAMPLE_SVP_NNIE_SsdForward0(NULL);
        }
        break;
        case 6:
        {
            SAMPLE_SVP_NNIE_Yolov1();
        }
        break;
        case 7:
        {
            SAMPLE_SVP_NNIE_Yolov2();
        }
        break;
        case 8:
        {
            SAMPLE_SVP_NNIE_Lstm();
        }
        break;
        case 9:
        {
            SAMPLE_SVP_NNIE_Mobilenet_Ssd(NULL);
        }
        break;
        case 10:
        {
            SAMPLE_SVP_NNIE_SsdCaffeProfiling(NULL);
        }
        break;
        case 11:
        {
            SAMPLE_SVP_NNIE_CaffeModelConv(NULL);
        }
        break;
        case 12:
        {
            SAMPLE_SVP_NNIE_DepthWiseConv(NULL);
        }
        break;
        case 13:
        {
            SAMPLE_SVP_NNIE_DepthWiseNormConv(NULL);
        }
        break;
        case 14:
        {
            SAMPLE_SVP_NNIE_PointWiseNormConv(NULL);
        }
        break;
        case 15:
        {
            SAMPLE_SVP_NNIE_SSDNormConv(NULL);
        }
        break;
        case 16:
        {
            SAMPLE_SVP_NNIE_SSDNormConv1X1_128x256x256_256(NULL);
        }
        break;
        case 17:
        {
            SAMPLE_SVP_NNIE_SSDNormConv1x1_128x256x256_128(NULL);
        }
        break;
        case 18:
        {
            SAMPLE_SVP_NNIE_SSDNormConv1x3_128x256x256_128(NULL);
        }
        break;
        case 19:
        {
            SAMPLE_SVP_NNIE_SSDNormConv1x5_128x256x256_128(NULL);
        }
        break;
        case 20:
        {
            SAMPLE_SVP_NNIE_SSDNormConv3x1_128x256x256_128(NULL);
        }
        break;
        case 21:
        {
            SAMPLE_SVP_NNIE_SSDNormConv3x3_128x256x256_128(NULL);
        }
        break;
        case 22:
        {
            SAMPLE_SVP_NNIE_SSDNormConv3x5_128x256x256_128(NULL);
        }
        break;
        case 23:
        {
            SAMPLE_SVP_NNIE_SSDNormConv5x1_128x256x256_128(NULL);
        }
        break;
        case 24:
        {
            SAMPLE_SVP_NNIE_SSDNormConv5x3_128x256x256_128(NULL);
        }
        break;
        case 25:
        {
            SAMPLE_SVP_NNIE_SSDNormConv5x5_128x256x256_128(NULL);
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



