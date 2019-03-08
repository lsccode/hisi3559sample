/**
 * @file      hi_product_scene_ini2bin.h
 * @brief     iniscene header
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/14
 * @version   1.0

 */

#ifndef __HI_PRODUCT_INISCENE_H__
#define __HI_PRODUCT_INISCENE_H__

#ifndef CFG_DEBUG_LOG_ON
#define CFG_DEBUG_LOG_ON
#endif

#include "hi_scenecomm.h"
#include "hi_confaccess.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/** \addtogroup     INIPARAM */
/** @{ */  /** <!-- [INIPARAM] */

/** product ini identification */
#define SCENETOOL_INIPARAM  "productini"

/** ini configure module: media_common */
#define SCENETOOL_INIPARAM_SCENETOOL_COMM "scene_param_0"


/** ini module name maximum length */
#define SCENETOOL_INIPARAM_MODULE_NAME_LEN (64)

/** ini node name maximum length */
#define SCENETOOL_INIPARAM_NODE_NAME_LEN  (128)


/** Load IniNode Result Check */
#define SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(ret, name) \
    do{ \
        if (HI_SUCCESS != ret){   \
            MLOGE(" Load [%s] failed\n", name); \
            return HI_FAILURE;  \
        }   \
    }while(0)

typedef struct tagSCENETOOL_MODULE_STATE_S
{
    HI_BOOL bStaticAE;
    HI_BOOL bStaticAERoute;
    HI_BOOL bStaticAERouteEx;
    HI_BOOL bStaticAWB;
    HI_BOOL bStaticAWBEx;
    HI_BOOL bStaticGlobalCac;
    HI_BOOL bStaticWdrExposure;
    HI_BOOL bStaticDrc;
    HI_BOOL bStaticDehaze;
    HI_BOOL bStaticLdci;
    HI_BOOL bStaticStatistics;
    HI_BOOL bStaticThreeDnr;
    HI_BOOL bStaticSaturation;
    HI_BOOL bStaticCCM;
    HI_BOOL bStaticNr;
	HI_BOOL bStaticShading;
    HI_BOOL bStaticCSC;
    HI_BOOL bStaticSharpen;

    HI_BOOL bDynamicGamma;
    HI_BOOL bDynamicAE;
    HI_BOOL bDynamicShading;
    HI_BOOL bDynamicLDCI;
    HI_BOOL bDynamicDehaze;
    HI_BOOL bDynamicFsWdr;
    HI_BOOL bDynamicHdrOTEF;
    HI_BOOL bDynamicHdrTM;
    HI_BOOL bDynamicHdrDrc;
    HI_BOOL bDynamicFalseColor;
    HI_BOOL bDynamicDemosaic;
    HI_BOOL bDynamicDCI;
    HI_BOOL bDynamicDIS;

    HI_BOOL bThreadDrc;
}SCENETOOL_MODULE_STATE_S;

/** @}*/  /** <!-- ==== INIPARAM End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_PRODUCT_INIPARAM_H__ */
