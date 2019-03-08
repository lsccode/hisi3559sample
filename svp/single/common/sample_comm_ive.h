#ifndef __SAMPLE_COMM_IVE_H__
#define __SAMPLE_COMM_IVE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "hi_debug.h"

#include "sample_comm.h"


#define IVE_RECT_NUM   20

#define VPSS_CHN_NUM 2

typedef struct hiSAMPLE_IVE_RECT_S
{
    POINT_S astPoint[4];
} SAMPLE_IVE_RECT_S;

typedef struct hiSAMPLE_RECT_ARRAY_S
{
    HI_U16 u16Num;
    SAMPLE_IVE_RECT_S astRect[IVE_RECT_NUM];
} SAMPLE_RECT_ARRAY_S;

typedef struct hiIVE_LINEAR_DATA_S
{
    HI_S32 s32LinearNum;
    HI_S32 s32ThreshNum;
    POINT_S* pstLinearPoint;
} IVE_LINEAR_DATA_S;

typedef struct hiSAMPLE_IVE_DRAW_RECT_MSG_S
{
    VIDEO_FRAME_INFO_S stFrameInfo;
    SAMPLE_RECT_ARRAY_S stRegion;
} SAMPLE_IVE_DRAW_RECT_MSG_S;

typedef struct hiSAMPLE_IVE_SWITCH_S
{
   HI_BOOL bVenc;
   HI_BOOL bVo;
}SAMPLE_IVE_SWITCH_S;

#define IVE_CLOSE_FILE(fp)\
    do{\
        if (NULL != (fp))\
        {\
            fclose((fp));\
            (fp) = NULL;\
        }\
    }while(0)

#define SAMPLE_PAUSE()\
    do {\
        printf("---------------press Enter key to exit!---------------\n");\
        (void)getchar();\
    } while (0)
#define SAMPLE_CHECK_EXPR_RET(expr, ret, fmt...)\
do\
{\
    if(expr)\
    {\
        SAMPLE_PRT(fmt);\
        return (ret);\
    }\
}while(0)
#define SAMPLE_CHECK_EXPR_RET_VOID(expr, fmt...)\
do\
{\
    if(expr)\
    {\
        SAMPLE_PRT(fmt);\
    }\
}while(0)

#define SAMPLE_CHECK_EXPR_GOTO(expr, label, fmt...)\
do\
{\
    if(expr)\
    {\
        SAMPLE_PRT(fmt);\
        goto label;\
    }\
}while(0)
/******************************************************************************
* function : Mpi init
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_CheckIveMpiInit(HI_VOID);
/******************************************************************************
* function : Mpi exit
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_IveMpiExit(HI_VOID);

/******************************************************************************
* function : Call vgs to fill rect
******************************************************************************/
HI_S32 SAMPLE_COMM_VGS_FillRect(VIDEO_FRAME_INFO_S* pstFrmInfo, SAMPLE_RECT_ARRAY_S* pstRect, HI_U32 u32Color);

/******************************************************************************
* function :Calc stride
******************************************************************************/
HI_U16 SAMPLE_COMM_IVE_CalcStride(HI_U32 u32Width, HI_U8 u8Align);
/******************************************************************************
* function : Init Vb
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_VbInit(PIC_SIZE_E *paenSize,SIZE_S *pastSize,HI_U32 u32VpssChnNum);
/******************************************************************************
* function : Start Vpss
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartVpss(SIZE_S *pastSize,HI_U32 u32VpssChnNum);
/******************************************************************************
* function : Stop Vpss
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_StopVpss(HI_U32 u32VpssChnNum);
/******************************************************************************
* function : Start Vo
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartVo(HI_VOID);
/******************************************************************************
* function : Stop Vo
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_StopVo(HI_VOID);
/******************************************************************************
* function : Start Vi/Vpss/Venc/Vo
******************************************************************************/
HI_S32 SAMPLE_COMM_IVE_StartViVpssVencVo(SAMPLE_VI_CONFIG_S *pstViConfig,SAMPLE_IVE_SWITCH_S *pstSwitch,PIC_SIZE_E *penExtPicSize);
/******************************************************************************
* function : Stop Vi/Vpss/Venc/Vo
******************************************************************************/
HI_VOID SAMPLE_COMM_IVE_StopViVpssVencVo(SAMPLE_VI_CONFIG_S *pstViConfig,SAMPLE_IVE_SWITCH_S *pstSwitch);

#endif


