#ifndef __SAMPLE_COMM_NNIE_H__
#define __SAMPLE_COMM_NNIE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include "sample_comm_ive.h"

#define SAMPLE_SVP_NNIE_MAX_CLASS_NUM             30
#define SAMPLE_SVP_NNIE_MAX_ROI_NUM_OF_CLASS      50

/*Array rect info*/
typedef struct hiSAMPLE_SVP_NNIE_RECT_ARRAY_S
 {
     HI_U32 u32ClsNum;
     HI_U32 u32TotalNum;
     HI_U32 au32RoiNum[SAMPLE_SVP_NNIE_MAX_CLASS_NUM];
     SAMPLE_IVE_RECT_S astRect[SAMPLE_SVP_NNIE_MAX_CLASS_NUM][SAMPLE_SVP_NNIE_MAX_ROI_NUM_OF_CLASS];
 } SAMPLE_SVP_NNIE_RECT_ARRAY_S;

typedef struct hiSAMPLE_NNIE_DRAW_RECT_MSG_S
{
    VIDEO_FRAME_INFO_S stFrameInfo;
    SAMPLE_SVP_NNIE_RECT_ARRAY_S *pstRegion;
}SAMPLE_NNIE_DRAW_RECT_MSG_S;

/*****************************************************************************
*   Prototype    : SAMPLE_COMM_SVP_NNIE_FillRect
*   Description  : Draw rect
*   Input        : VIDEO_FRAME_INFO_S             *pstFrmInfo   Frame info
* 		            SAMPLE_SVP_NNIE_RECT_ARRAY_S  *pstRect       Rect
*                  HI_U32                         u32Color      Color
*
*
*   Output       :
*   Return Value : HI_S32
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-03-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_COMM_SVP_NNIE_FillRect(VIDEO_FRAME_INFO_S *pstFrmInfo, SAMPLE_SVP_NNIE_RECT_ARRAY_S* pstRect, HI_U32 u32Color);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_COMM_NNIE_H__ */
