/**
 * @file      hi_product_scene_initosource.c
 * @brief     ini2source implementation
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2017/12/13
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "hi_scenetool_initosource.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/** product param */
//static HI_PARAM_CFG_S s_stPARAMCfg;


FILE * fpHeader = HI_NULL;
FILE * fpSoucre = HI_NULL;
FILE * fpMak = HI_NULL;
HI_BOOL g_bLogOn = 1;

static SCENETOOL_MODULE_STATE_S s_stSceneState;

#define SCENETOOL_MAX_FILESIZE      (128)

#ifndef HI_PARAM_SCENEMODE_CNT
#define HI_PARAM_SCENEMODE_CNT     (11)
#endif

HI_S32 SCENETOOL_WriteHeaderFileBegin(HI_U32 u32Count)
{
    if (0 != fseek(fpHeader, 0L,SEEK_SET))
    {
    	printf("Seek file error!\n");
        fclose(fpHeader);
		return HI_FAILURE;
    }

    /**write start of header*/
    fprintf(fpHeader, "#ifndef _HI_SCENE_AUTOGENERATE_H_\n");
    fprintf(fpHeader, "#define _HI_SCENE_AUTOGENERATE_H_\n\n");

    fprintf(fpHeader, "#include \"hi_common.h\"\n");
    fprintf(fpHeader, "#include \"hi_comm_dis.h\"\n");
    fprintf(fpHeader, "#include \"hi_comm_isp.h\"\n");
    fprintf(fpHeader, "#include \"hi_comm_vi.h\"\n");
    fprintf(fpHeader, "#include \"hi_comm_vpss.h\"\n");
    fprintf(fpHeader, "#include \"hi_comm_venc.h\"\n");
    fprintf(fpHeader, "#include \"hi_comm_hdr.h\"\n");

    fprintf(fpHeader, "#ifdef __cplusplus\n");
    fprintf(fpHeader, "#if __cplusplus\n");
    fprintf(fpHeader, "extern \"C\" {\n");
    fprintf(fpHeader, "#endif\n#endif\n\n");

    /**this value can be set by hdini's module num*/
    fprintf(fpHeader, "#define HI_SCENE_PIPETYPE_NUM                    (10)\n");
    fprintf(fpHeader, "#define HI_SCENE_AE_EXPOSURE_MAX_COUNT           (8)\n");
	fprintf(fpHeader, "#define HI_SCENE_SHADING_EXPOSURE_MAX_COUNT      (10)\n");
    fprintf(fpHeader, "#define HI_SCENE_GAMMA_EXPOSURE_MAX_COUNT        (10)\n");
    fprintf(fpHeader, "#define HI_SCENE_DRC_ISO_MAX_COUNT               (7)\n");
    fprintf(fpHeader, "#define HI_SCENE_DRC_RATIO_MAX_COUNT             (6)\n");
    fprintf(fpHeader, "#define HI_SCENE_FALSECOLOR_EXPOSURE_MAX_COUNT   (10)\n");
    fprintf(fpHeader, "#define HI_SCENE_DEMOSAIC_EXPOSURE_MAX_COUNT     (8)\n");
    fprintf(fpHeader, "#define HI_SCENE_DIS_ISO_MAX_COUNT               (3)\n");
    fprintf(fpHeader, "#define HI_SCENE_LDCI_EXPOSURE_MAX_COUNT         (6)\n");
    fprintf(fpHeader, "#define HI_SCENE_FSWDR_ISO_MAX_COUNT             (3)\n");
    fprintf(fpHeader, "#define HI_SCENE_DEHAZE_EXPOSURE_MAX_COUNT       (5)\n");
    fprintf(fpHeader, "#define HI_SCENE_THREEDNR_MAX_COUNT              (12)\n");
    fprintf(fpHeader, "#define HI_SCENE_HDR_MAX_COUNT                   (10)\n");


    fprintf(fpHeader, "typedef struct hiSCENE_MODULE_STATE_S\n{\n");
    fprintf(fpHeader, "    HI_BOOL bStaticAE;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticAERoute;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticAERouteEx;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticGlobalCac;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticWdrExposure;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticAWB;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticAWBEx;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticDrc;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticDehaze;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticLdci;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticStatistics;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticThreeDNR;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticSaturation;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticCCM;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticNr;\n");
	fprintf(fpHeader, "    HI_BOOL bStaticShading;\n");
    fprintf(fpHeader, "    HI_BOOL bStaticCSC;\n");
    fprintf(fpHeader, "    HI_BOOL bDyanamicGamma;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicShading;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicLdci;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicAE;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicDehaze;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicFSWDR;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicHDRTM;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicHDROETF;\n");
    fprintf(fpHeader, "    HI_BOOL bDynamicHDRDRC;\n");
    fprintf(fpHeader, "    HI_BOOL bDyanamicFalseColor;\n");
    fprintf(fpHeader, "    HI_BOOL bDyanamicDemosaic;\n");
    fprintf(fpHeader, "    HI_BOOL bDyanamicDIS;\n");
    fprintf(fpHeader, "    HI_BOOL bThreadDrc;\n");
    fprintf(fpHeader, "} HI_SCENE_MODULE_STATE_S;\n\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_WriteHeaderFileEnd(HI_VOID)
{
    /**pipe param*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_PIPE_PARAM_S\n{\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_AE_S stStaticAe;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_AEROUTE_S stStaticAeRoute;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_AEROUTEEX_S stStaticAeRouteEx;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_GLOBALCAC_S stStaticGlobalCac;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_WDREXPOSURE_S stStaticWdrExposure;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_DRC_S stStaticDrc;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_DEHAZE_S stStaticDehaze;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_LDCI_S stStaticLdci;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_STATISTICSCFG_S stStaticStatistics;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_THREEDNR_S stStaticThreeDNR;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_AWB_S stStaticAwb;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_AWBEX_S stStaticAwbEx;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_CCM_S stStaticCcm;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_CSC_S stStaticCsc;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_NR_S stStaticNr;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_SHADING_S stStaticShading;\n");
    fprintf(fpHeader, "    HI_SCENE_STATIC_SATURATION_S stStaticSaturation;\n");

    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_AE_S stDynamicAe;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_SHADING_S stDynamicShading;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_LDCI_S stDynamicLDCI;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_DEHAZE_S stDynamicDehaze;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_FSWDR_S stDynamicFSWDR;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_HDROETF_S stDynamicHDROETF; \n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_HDRTM_S stDynamicHDRTm;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_GAMMA_S stDynamicGamma;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_HDRDRC_S stDynamicHDRDRC;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_FALSECOLOR_S stDynamicFalsecolor;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_DEMOSAIC_S stDynamicDemosaic;\n");
    fprintf(fpHeader, "    HI_SCENE_DYNAMIC_DIS_S stDynamicDis;\n");
    fprintf(fpHeader, "    HI_SCENE_THREAD_DRC_S stThreadDrc;\n");
    fprintf(fpHeader, "} HI_SCENE_PIPE_PARAM_S;\n\n");


    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticAE_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticAERoute_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticAERouteEX_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticGlobalCac_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticWDRExposure_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticDRC_Autogenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticDeHaze_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticLDCI_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticAWB_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticAWBEX_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticSaturation_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticCCM_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticCSC_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticNr_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticShading_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticStatisticsCfg_AutoGenerare(VI_PIPE ViPipe, HI_U8 u8Index, HI_BOOL bMetryFixed);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetStaticThreeDNR_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index);\n\n");

    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicPhotoGamma_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicVideoGamma_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicAE_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicShading_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64ISO, HI_U64 u64LastISO, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicLDCI_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicFalseColor_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicDehaze_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicFsWdr_AutoGenerate(VI_PIPE ViPipe, HI_U32 u32ISO, HI_U32 u32LastISO, HI_U8 u8Index, HI_U32 u32ActRation);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicDemosaic_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicDIS_AutoGenerate(VI_PIPE ViPipe, VI_CHN ViChn, HI_U32 u32Iso, HI_U32 u32LastIso, HI_U8 u8Index, HI_BOOL bEnable);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicHDROTEF_AutoGenerate(VI_PIPE ViPipe, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32HDRBrightRatio, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicHDRTM_AutoGenerate(VI_PIPE ViPipe, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32HDRBrightRatio, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetDynamicHDRDRC_AutoGenerate(VI_PIPE ViPipe, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32HDRBrightRatio, HI_U8 u8Index);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetThreadDRC_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ActRation, HI_U32 u32Iso);\n\n");

    fprintf(fpHeader, "HI_S32 HI_SCENE_GetModuleState_AutoGenerate(HI_SCENE_MODULE_STATE_S* pstModuleState);\n\n");

    fprintf(fpHeader, "HI_S32 HI_SCENE_SetPipeParam_AutoGenerate(const HI_SCENE_PIPE_PARAM_S* pstScenePipeParam, HI_U32 u32Num);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_SetPause_AutoGenerate(HI_BOOL bPause);\n\n");
    fprintf(fpHeader, "HI_S32 HI_SCENE_GetPipeAERoute_AutoGenerate(HI_S32 s32PipeIndex, HI_U8 au8AEWeight[][AE_ZONE_COLUMN]);\n\n");

    fprintf(fpHeader, "\n\n#ifdef __cplusplus\n");
    fprintf(fpHeader, "#if __cplusplus\n");
    fprintf(fpHeader, "}\n");
    fprintf(fpHeader, "#endif\n#endif\n\n#endif");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_WriteSourceFileBegin(HI_VOID)
{
    if (0 != fseek(fpSoucre, 0L,SEEK_SET))
    {
    	printf("Seek file error!\n");
        fclose(fpSoucre);
		return HI_FAILURE;
    }

    fprintf(fpSoucre, "#include \"hi_scene_autogenerate.h\"\n");
    fprintf(fpSoucre, "#include <unistd.h>\n");
    fprintf(fpSoucre, "#include <string.h>\n");
    fprintf(fpSoucre, "#include \"mpi_awb.h\"\n");
    fprintf(fpSoucre, "#include \"mpi_ae.h\"\n");
    fprintf(fpSoucre, "#include \"mpi_isp.h\"\n");
    fprintf(fpSoucre, "#include \"mpi_vi.h\"\n");
    fprintf(fpSoucre, "#include \"mpi_hdr.h\"\n");

    fprintf(fpSoucre, "#ifdef __cplusplus\n");
    fprintf(fpSoucre, "#if __cplusplus\n");
    fprintf(fpSoucre, "extern \"C\" {\n");
    fprintf(fpSoucre, "#endif\n#endif\n\n");

    fprintf(fpSoucre, "#define SCENE_MAX(a, b) (((a) < (b)) ? (b) : (a))\n\n");
    fprintf(fpSoucre, "#define SCENE_DIV_0TO1(a)  ((0 == (a)) ? 1 : (a))\n\n");
    fprintf(fpSoucre, "#define SCENE_SHADING_TRIGMODE_L2H            (1)\n\n");
    fprintf(fpSoucre, "#define SCENE_SHADING_TRIGMODE_H2L            (2)\n\n");
    fprintf(fpSoucre, "#define SCENE_MAX_ISP_PIPE_NUM                (6)\n\n");

    fprintf(fpSoucre, "HI_BOOL g_bISPPause;\n");
    fprintf(fpSoucre, "HI_SCENE_PIPE_PARAM_S g_astScenePipeParam[HI_SCENE_PIPETYPE_NUM];\n\n");

    fprintf(fpSoucre, "#define CHECK_SCENE_PAUSE()\\\n");
    fprintf(fpSoucre, "    do{\\\n");
    fprintf(fpSoucre, "        if (HI_TRUE == g_bISPPause)\\\n");
    fprintf(fpSoucre, "        {\\\n");
    fprintf(fpSoucre, "            return HI_SUCCESS;\\\n");
    fprintf(fpSoucre, "        }\\\n");
    fprintf(fpSoucre, "    }while(0);\\\n\n");

    fprintf(fpSoucre, "#define CHECK_SCENE_RET(s32Ret)\\\n");
    fprintf(fpSoucre, "    do{\\\n");
    fprintf(fpSoucre, "        if (HI_SUCCESS != s32Ret)\\\n");
    fprintf(fpSoucre, "        {\\\n");
    fprintf(fpSoucre, "            printf(\"Failed at %%s: LINE: %%d with %%#x!\", __FUNCTION__, __LINE__, s32Ret);\\\n");
    fprintf(fpSoucre, "        }\\\n");
    fprintf(fpSoucre, "    }while(0);\\\n\n");

    fprintf(fpSoucre, "#define CHECK_SCENE_NULLPTR(ptr)\\\n");
    fprintf(fpSoucre, "    do{\\\n");
    fprintf(fpSoucre, "        if (NULL == ptr)\\\n");
    fprintf(fpSoucre, "        {\\\n");
    fprintf(fpSoucre, "            printf(\"NullPtr at %%s: LINE: %%d!\", __FUNCTION__, __LINE__);\\\n");
    fprintf(fpSoucre, "            return HI_FAILURE;\\\n");
    fprintf(fpSoucre, "        }\\\n");
    fprintf(fpSoucre, "    }while(0);\\\n\n");

    fprintf(fpSoucre, "#define FREE_SCENE_PTR(ptr)\\\n");
    fprintf(fpSoucre, "    do{\\\n");
    fprintf(fpSoucre, "        if (NULL != ptr)\\\n");
    fprintf(fpSoucre, "        {\\\n");
    fprintf(fpSoucre, "            free(ptr);\\\n");
    fprintf(fpSoucre, "            ptr = NULL;\\\n");
    fprintf(fpSoucre, "        }\\\n");
    fprintf(fpSoucre, "    }while(0);\\\n\n");


    fprintf(fpSoucre, "static  HI_U32 SCENE_GetLevelLtoH(HI_U64 u64Value, HI_U32 u32Count, HI_U64 *pu64Thresh)\n");
    fprintf(fpSoucre, "{\n");
    fprintf(fpSoucre, "    HI_U32 u32Level = 0;\n\n");
    fprintf(fpSoucre, "    for (u32Level = 0; u32Level < u32Count; u32Level++)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        if(u64Value <= pu64Thresh[u32Level])\n");
    fprintf(fpSoucre, "        {\n");
    fprintf(fpSoucre, "            break;\n");
    fprintf(fpSoucre, "        }\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    if (u32Level == u32Count)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        u32Level = u32Count - 1;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    return u32Level;\n");
    fprintf(fpSoucre, "}\n");

    fprintf(fpSoucre, "static  HI_U32 SCENE_GetLevelHtoL(HI_U64 u64Value, HI_U32 u32Count, HI_U64 *pu64Thresh)\n");
    fprintf(fpSoucre, "{\n");
    fprintf(fpSoucre, "    HI_U32 u32Level = 0;\n\n");
    fprintf(fpSoucre, "    for (u32Level = u32Count; u32Level > 0; u32Level--)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        if(u64Value > pu64Thresh[u32Level - 1])\n");
    fprintf(fpSoucre, "        {\n");
    fprintf(fpSoucre, "            break;\n");
    fprintf(fpSoucre, "        }\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    if (u32Level > 0)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        u32Level = u32Level - 1;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    return u32Level;\n");
    fprintf(fpSoucre, "}\n\n");

	fprintf(fpSoucre, "\nstatic HI_U32 SCENE_GetLevelLtoH_U32(HI_U32 u32Value, HI_U32 u32Count, HI_U32 *pu32Thresh)\n");
	fprintf(fpSoucre, "{\n");
	fprintf(fpSoucre, "	HI_U32 u32Level = 0;\n\n");
	fprintf(fpSoucre, "	for (u32Level = 0; u32Level < u32Count; u32Level++)\n");
	fprintf(fpSoucre, "	{\n");
	fprintf(fpSoucre, "		if(u32Value <= pu32Thresh[u32Level])\n");
	fprintf(fpSoucre, "		{\n");
	fprintf(fpSoucre, "			break;\n");
	fprintf(fpSoucre, "		}\n");
	fprintf(fpSoucre, "	}\n");
	fprintf(fpSoucre, "	if (u32Level == u32Count)\n");
	fprintf(fpSoucre, "	{\n");
	fprintf(fpSoucre, "		u32Level = u32Count - 1;\n");
	fprintf(fpSoucre, "	}\n\n");
	fprintf(fpSoucre, "	return u32Level;\n");
	fprintf(fpSoucre, "}\n");

    fprintf(fpSoucre, "static HI_U32 SCENE_Interpulate(HI_U32 u32Mid,HI_U32 u32Left, HI_U32 u32LValue, HI_U32 u32Right, HI_U32 u32RValue)\n");
    fprintf(fpSoucre, "{\n");
    fprintf(fpSoucre, "    HI_U32 u32Value = 0;\n");
    fprintf(fpSoucre, "    HI_U32 k = 0;\n\n");
    fprintf(fpSoucre, "    if (u32Mid <= u32Left)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        u32Value = u32LValue;\n");
    fprintf(fpSoucre, "        return u32Value;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    if (u32Mid >= u32Right)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        u32Value = u32RValue;\n");
    fprintf(fpSoucre, "        return u32Value;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    k = (u32Right - u32Left);\n");
    fprintf(fpSoucre, "    u32Value = (((u32Right - u32Mid) * u32LValue + (u32Mid - u32Left) * u32RValue + (k >> 1))/ k);\n");
    fprintf(fpSoucre, "    return u32Value;\n");
    fprintf(fpSoucre, "}\n\n");


    fprintf(fpSoucre, "static HI_U32 SCENE_TimeFilter(HI_U32 u32Para0,HI_U32 u32Para1, HI_U32 u32TimeCnt, HI_U32 u32Index)\n");
    fprintf(fpSoucre, "{\n");
    fprintf(fpSoucre, "    HI_U64 u64Temp = 0;\n");
    fprintf(fpSoucre, "    HI_U32 u32Value = 0;\n\n");
    fprintf(fpSoucre, "    if (u32Para0 > u32Para1)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        u64Temp = (HI_U64)(u32Para0 - u32Para1) << 8;\n");
    fprintf(fpSoucre, "        u64Temp = (u64Temp * (u32Index + 1)) /SCENE_DIV_0TO1(u32TimeCnt) >> 8;\n");
    fprintf(fpSoucre, "        u32Value = u32Para0 - (HI_U32)u64Temp;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    else\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        u64Temp = (HI_U64)(u32Para1 - u32Para0) << 8;\n");
    fprintf(fpSoucre, "        u64Temp = (u64Temp * (u32Index + 1)) /SCENE_DIV_0TO1(u32TimeCnt) >> 8;\n");
    fprintf(fpSoucre, "        u32Value = u32Para0 + (HI_U32)u64Temp;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    return u32Value;\n");
    fprintf(fpSoucre, "}\n\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_WriteSourceFileEnd(HI_VOID)
{
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_GetModuleState_AutoGenerate(HI_SCENE_MODULE_STATE_S* pstModuleState)\n{\n");
    fprintf(fpSoucre, "    if (HI_NULL == pstModuleState)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        printf(\"null pointer\");\n");
    fprintf(fpSoucre, "        return HI_FAILURE;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    pstModuleState->bStaticAE = %d;\n", s_stSceneState.bStaticAE);
    fprintf(fpSoucre, "    pstModuleState->bStaticAERoute = %d;\n", s_stSceneState.bStaticAERoute);
    fprintf(fpSoucre, "    pstModuleState->bStaticAERouteEx = %d;\n", s_stSceneState.bStaticAERouteEx);
    fprintf(fpSoucre, "    pstModuleState->bStaticGlobalCac = %d;\n", s_stSceneState.bStaticGlobalCac);
    fprintf(fpSoucre, "    pstModuleState->bStaticWdrExposure = %d;\n", s_stSceneState.bStaticWdrExposure);
    fprintf(fpSoucre, "    pstModuleState->bStaticDrc = %d;\n", s_stSceneState.bStaticDrc);
    fprintf(fpSoucre, "    pstModuleState->bStaticDehaze = %d;\n", s_stSceneState.bStaticDehaze);
    fprintf(fpSoucre, "    pstModuleState->bStaticLdci = %d;\n", s_stSceneState.bStaticLdci);
    fprintf(fpSoucre, "    pstModuleState->bStaticAWB = %d;\n", s_stSceneState.bStaticAWB);
    fprintf(fpSoucre, "    pstModuleState->bStaticAWBEx = %d;\n", s_stSceneState.bStaticAWBEx);
    fprintf(fpSoucre, "    pstModuleState->bStaticSaturation = %d;\n", s_stSceneState.bStaticSaturation);
    fprintf(fpSoucre, "    pstModuleState->bStaticCCM = %d;\n", s_stSceneState.bStaticCCM);
    fprintf(fpSoucre, "    pstModuleState->bStaticThreeDNR = %d;\n", s_stSceneState.bStaticThreeDnr);
    fprintf(fpSoucre, "    pstModuleState->bStaticNr = %d;\n", s_stSceneState.bStaticNr);
    fprintf(fpSoucre, "    pstModuleState->bStaticShading = %d;\n", s_stSceneState.bStaticShading);
    fprintf(fpSoucre, "    pstModuleState->bStaticCSC = %d;\n", s_stSceneState.bStaticCSC);
    fprintf(fpSoucre, "    pstModuleState->bStaticStatistics = %d;\n", s_stSceneState.bStaticStatistics);
    fprintf(fpSoucre, "    pstModuleState->bDyanamicGamma = %d;\n", s_stSceneState.bDynamicGamma);
    fprintf(fpSoucre, "    pstModuleState->bDynamicAE = %d;\n", s_stSceneState.bDynamicAE);
    fprintf(fpSoucre, "    pstModuleState->bDyanamicFalseColor = %d;\n", s_stSceneState.bDynamicFalseColor);
    fprintf(fpSoucre, "    pstModuleState->bDyanamicDemosaic = %d;\n", s_stSceneState.bDynamicDemosaic);
    fprintf(fpSoucre, "    pstModuleState->bDynamicShading = %d;\n", s_stSceneState.bDynamicShading);
    fprintf(fpSoucre, "    pstModuleState->bDynamicDehaze = %d;\n", s_stSceneState.bDynamicDehaze);
    fprintf(fpSoucre, "    pstModuleState->bDynamicFSWDR = %d;\n", s_stSceneState.bDynamicFsWdr);
    fprintf(fpSoucre, "    pstModuleState->bDynamicHDRTM = %d;\n", s_stSceneState.bDynamicHdrTM);
    fprintf(fpSoucre, "    pstModuleState->bDynamicHDROETF = %d;\n", s_stSceneState.bDynamicHdrOTEF);
    fprintf(fpSoucre, "    pstModuleState->bDynamicHDRDRC = %d;\n", s_stSceneState.bDynamicHdrDrc);
    fprintf(fpSoucre, "    pstModuleState->bDynamicLdci = %d;\n", s_stSceneState.bDynamicLDCI);
    fprintf(fpSoucre, "    pstModuleState->bThreadDrc = %d;\n", s_stSceneState.bThreadDrc);
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetPipeParam_AutoGenerate(const HI_SCENE_PIPE_PARAM_S* pstScenePipeParam, HI_U32 u32Num)\n{\n");
    fprintf(fpSoucre, "    if (HI_NULL == pstScenePipeParam)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        printf(\"null pointer\");\n");
    fprintf(fpSoucre, "        return HI_FAILURE;\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    memcpy(g_astScenePipeParam,pstScenePipeParam,sizeof(HI_SCENE_PIPE_PARAM_S) * u32Num);\n\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetPause_AutoGenerate(HI_BOOL bPause)\n{\n");
    fprintf(fpSoucre, "    g_bISPPause = bPause;\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_GetPipeAERoute_AutoGenerate(HI_S32 s32PipeIndex, HI_U8 au8AEWeight[][AE_ZONE_COLUMN])\n{\n");
    fprintf(fpSoucre, "    HI_S32 i, j = 0;\n");
    fprintf(fpSoucre, "    for (i = 0; i < AE_ZONE_ROW; i++)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        for (j = 0; j < AE_ZONE_COLUMN; j++)\n");
    fprintf(fpSoucre, "        {\n");
    fprintf(fpSoucre, "            au8AEWeight[i][j] = g_astScenePipeParam[s32PipeIndex].stStaticStatistics.au8AEWeight[i][j];\n");
    fprintf(fpSoucre, "        }\n");
    fprintf(fpSoucre, "    }\n\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    fprintf(fpSoucre, "\n\n#ifdef __cplusplus\n");
    fprintf(fpSoucre, "#if __cplusplus\n");
    fprintf(fpSoucre, "}\n");
    fprintf(fpSoucre, "#endif\n#endif\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticAE(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticAE header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_AE_S\n{\n");

    /**start of StaticAE source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticAE_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_EXPOSURE_ATTR_S stExposureAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetExposureAttr(ViPipe, &stExposureAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AERouteExValid");/*RouteExValid*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_EXVALID (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
		fprintf(fpHeader, "    HI_BOOL bAERouteExValid;\n");

        fprintf(fpSoucre, "    stExposureAttr.bAERouteExValid = g_astScenePipeParam[u8Index].stStaticAe.bAERouteExValid;\n");
        free(pszString);
        pszString = HI_NULL;
    }


    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AERunInterval");/*RunInterval*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_INTERVAL (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8AERunInterval;\n");

        fprintf(fpSoucre, "    stExposureAttr.u8AERunInterval = g_astScenePipeParam[u8Index].stStaticAe.u8AERunInterval;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoExpTimeMax");/*AutoExpTimeMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_EXPTIMEMAX (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32AutoExpTimeMax;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.stExpTimeRange.u32Max = g_astScenePipeParam[u8Index].stStaticAe.u32AutoExpTimeMax;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoISPDGainMax");/*AutoISPDGainMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_ISPDGAINMAX (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32AutoISPDGainMax;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.stISPDGainRange.u32Max = g_astScenePipeParam[u8Index].stStaticAe.u32AutoISPDGainMax;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoSysGainMax");/*AutoSysGainMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_SYSGAINMAX (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32AutoSysGainMax;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.stSysGainRange.u32Max = g_astScenePipeParam[u8Index].stStaticAe.u32AutoSysGainMax;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoSpeed");/*AutoSpeed*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_SPEED (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8AutoSpeed;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.u8Speed = g_astScenePipeParam[u8Index].stStaticAe.u8AutoSpeed;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoTolerance");/*AutoTolerance*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_TOLERANCE (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8AutoTolerance;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.u8Tolerance = g_astScenePipeParam[u8Index].stStaticAe.u8AutoTolerance;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoHistRatioSlope");/*AutoHistRatioSlope*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_HISTRATIOSLOPE (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoHistRatioSlope;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.u16HistRatioSlope = g_astScenePipeParam[u8Index].stStaticAe.u16AutoHistRatioSlope;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoMaxHistOffset");/*AutoMaxHistOffset*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_MAXHISTOFFSET (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8AutoMaxHistOffset;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.u8MaxHistOffset = g_astScenePipeParam[u8Index].stStaticAe.u8AutoMaxHistOffset;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoBlackDelayFrame");/*AutoBlackDelayFrame*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_BLACKDELAYFRAME (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoBlackDelayFrame;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = g_astScenePipeParam[u8Index].stStaticAe.u16AutoBlackDelayFrame;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ae:AutoWhiteDelayFrame");/*AutoWhiteDelayFrame*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AE_WHITEDELAYFRAME (1)\n");
        s_stSceneState.bStaticAE = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoWhiteDelayFrame;\n");

        fprintf(fpSoucre, "    stExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = g_astScenePipeParam[u8Index].stStaticAe.u16AutoWhiteDelayFrame;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticAE header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_AE_S;\n");
    /**end of StaticAE source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetExposureAttr(ViPipe, &stExposureAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticAERoute(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticAERoute header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_AEROUTE_S\n{\n");

    /**start of StaticAERoute source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticAERoute_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_AE_ROUTE_S stAeRoute;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetAERouteAttr(ViPipe, &stAeRoute);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");


    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aeroute:u32TotalNum");/*TotalNum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTE_TOTALNUM (1)\n");
        s_stSceneState.bStaticAERoute = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");

        fprintf(fpSoucre, "    stAeRoute.u32TotalNum = g_astScenePipeParam[u8Index].stStaticAeRoute.u32TotalNum;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aeroute:RouteIntTime");/*RouteIntTime*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTE_INTTIME (1)\n");
        s_stSceneState.bStaticAERoute = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32IntTime[ISP_AE_ROUTE_MAX_NODES];\n");

        fprintf(fpSoucre, "    for (i = 0; i < g_astScenePipeParam[u8Index].stStaticAeRoute.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAeRoute.astRouteNode[i].u32IntTime = g_astScenePipeParam[u8Index].stStaticAeRoute.au32IntTime[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aeroute:RouteSysGain");/*RouteSysGain*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTE_SYSGAIN (1)\n");
        s_stSceneState.bStaticAERoute = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32SysGain[ISP_AE_ROUTE_MAX_NODES];\n");

        fprintf(fpSoucre, "    for (i = 0; i < g_astScenePipeParam[u8Index].stStaticAeRoute.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAeRoute.astRouteNode[i].u32SysGain = g_astScenePipeParam[u8Index].stStaticAeRoute.au32SysGain[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticAERoute header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_AEROUTE_S;\n");
    /**end of StaticAERoute source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetAERouteAttr(ViPipe, &stAeRoute);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    printf(\"i is %%d.\", i);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticAWB(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticAWB header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_AWB_S\n{\n");

    /**start of StaticAWB source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticAWB_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_WB_ATTR_S stWbAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetWBAttr(ViPipe, &stWbAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoStaticWb");/*AutoStaticWb*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_STATICWB (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoStaticWB[4];\n");

        fprintf(fpSoucre, "    for (i = 0; i < 4; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWbAttr.stAuto.au16StaticWB[i] = g_astScenePipeParam[u8Index].stStaticAwb.au16AutoStaticWB[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCurvePara");/*AutoCurvePara*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_CURVEPARA (1)\n");
        s_stSceneState.bStaticAWB= HI_TRUE;
        fprintf(fpHeader, "    HI_S32 as32AutoCurvePara[6];\n");

        fprintf(fpSoucre, "    for (i = 0; i < 6; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWbAttr.stAuto.as32CurvePara[i] = g_astScenePipeParam[u8Index].stStaticAwb.as32AutoCurvePara[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoSpeed");/*AutoSpeed*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_SPEED (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoSpeed;\n");

        fprintf(fpSoucre, "    stWbAttr.stAuto.u16Speed = g_astScenePipeParam[u8Index].stStaticAwb.u16AutoSpeed;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoLowColorTemp");/*AutoLowColorTemp*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_LOWCOLORTEMP (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoLowColorTemp;\n");

        fprintf(fpSoucre, "    stWbAttr.stAuto.u16LowColorTemp = g_astScenePipeParam[u8Index].stStaticAwb.u16AutoLowColorTemp;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCrMax");/*AutoCrMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_CRMAX (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoCrMax[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWbAttr.stAuto.stCbCrTrack.au16CrMax[i] = g_astScenePipeParam[u8Index].stStaticAwb.au16AutoCrMax[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCrMin");/*AutoCrMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_CRMIN (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoCrMin[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWbAttr.stAuto.stCbCrTrack.au16CrMin[i] = g_astScenePipeParam[u8Index].stStaticAwb.au16AutoCrMin[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCbMax");/*AutoCbMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_CBMAX (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoCbMax[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWbAttr.stAuto.stCbCrTrack.au16CbMax[i] = g_astScenePipeParam[u8Index].stStaticAwb.au16AutoCbMax[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awb:AutoCbMin");/*AutoCbMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWB_CBMIN (1)\n");
        s_stSceneState.bStaticAWB = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoCbMin[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWbAttr.stAuto.stCbCrTrack.au16CbMin[i] = g_astScenePipeParam[u8Index].stStaticAwb.au16AutoCbMin[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticAWB header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_AWB_S;\n");
    /**end of StaticAWB source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetWBAttr(ViPipe, &stWbAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    printf(\"i is %%d.\", i);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticAWBEX(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticAWBEX header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_AWBEX_S\n{\n");

    /**start of StaticAWBEX source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticAWBEX_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_AWB_ATTR_EX_S stAwbAttrEx;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetAWBAttrEx(ViPipe, &stAwbAttrEx);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:Tolerance");/*Tolerance*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
   	    fprintf(fpMak, "#define CFG_STA_AWBEX_TOLERANCE (1)\n");
        s_stSceneState.bStaticAWBEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8Tolerance;\n");

        fprintf(fpSoucre, "    stAwbAttrEx.u8Tolerance = g_astScenePipeParam[u8Index].stStaticAwbEx.u8Tolerance;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:OutThresh");/*OutThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWBEX_OUTTHRESH (1)\n");
        s_stSceneState.bStaticAWBEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32OutThresh;\n");

        fprintf(fpSoucre, "    stAwbAttrEx.stInOrOut.u32OutThresh = g_astScenePipeParam[u8Index].stStaticAwbEx.u32OutThresh;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:LowStop");/*LowStop*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWBEX_LOWSTOP (1)\n");
        s_stSceneState.bStaticAWBEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16LowStop;\n");

        fprintf(fpSoucre, "    stAwbAttrEx.stInOrOut.u16LowStop = g_astScenePipeParam[u8Index].stStaticAwbEx.u16LowStop;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:HighStart");/*HighStart*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWBEX_HIGHSTART (1)\n");
        s_stSceneState.bStaticAWBEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16HighStart;\n");

        fprintf(fpSoucre, "    stAwbAttrEx.stInOrOut.u16HighStart = g_astScenePipeParam[u8Index].stStaticAwbEx.u16HighStart;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:HighStop");/*HighStop*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AWBEX_HIGHSTOP (1)\n");
        s_stSceneState.bStaticAWBEx= HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16HighStop;\n");

        fprintf(fpSoucre, "    stAwbAttrEx.stInOrOut.u16HighStop = g_astScenePipeParam[u8Index].stStaticAwbEx.u16HighStop;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:MultiLightSourceEn");/*MultiLightSourceEn*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {

		fprintf(fpMak, "#define CFG_STA_AWBEX_MULTILIGHTSOURCEEN (1)\n");
        s_stSceneState.bStaticAWBEx = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bMultiLightSourceEn;\n");

        fprintf(fpSoucre, "    stAwbAttrEx.bMultiLightSourceEn = g_astScenePipeParam[u8Index].stStaticAwbEx.bMultiLightSourceEn;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_awbex:MultiCTWt");/*MultiCTWt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
		fprintf(fpMak, "#define CFG_STA_AWBEX_MULTICTWT (1)\n");
        s_stSceneState.bStaticAWBEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16MultiCTWt[8];\n");

        fprintf(fpSoucre, "    for (i = 0; i < 8; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAwbAttrEx.au16MultiCTWt[i] = g_astScenePipeParam[u8Index].stStaticAwbEx.au16MultiCTWt[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticAWBEX header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_AWBEX_S;\n");
    /**end of StaticAWBEX source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetAWBAttrEx(ViPipe, &stAwbAttrEx);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    printf(\"i is %%d.\", i);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}


HI_S32 SCENETOOL_StaticAERouteEX(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticAERouteEX header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_AEROUTEEX_S\n{\n");

    /**start of StaticAERouteEX  source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticAERouteEX_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_AE_ROUTE_EX_S stAeRouteEx;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetAERouteAttrEx(ViPipe, &stAeRouteEx);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aerouteex:TotalNum");/*Tolerance*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTEEX_TOTALNUM (1)\n");
        s_stSceneState.bStaticAERouteEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");

        fprintf(fpSoucre, "    stAeRouteEx.u32TotalNum = g_astScenePipeParam[u8Index].stStaticAeRouteEx.u32TotalNum;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXIntTime");/*RouteEXIntTime*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTEEX_ROUTEEXINTTIME (1)\n");
        s_stSceneState.bStaticAERouteEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32IntTime[ISP_AE_ROUTE_EX_MAX_NODES];\n");

        fprintf(fpSoucre, "    for (i = 0; i < stAeRouteEx.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAeRouteEx.astRouteExNode[i].u32IntTime = g_astScenePipeParam[u8Index].stStaticAeRouteEx.au32IntTime[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXAGain");/*RouteEXAGain*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTEEX_ROUTEEXAGAIN (1)\n");
        s_stSceneState.bStaticAERouteEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32Again[ISP_AE_ROUTE_EX_MAX_NODES];\n");

        fprintf(fpSoucre, "    for (i = 0; i < stAeRouteEx.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAeRouteEx.astRouteExNode[i].u32Again= g_astScenePipeParam[u8Index].stStaticAeRouteEx.au32Again[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXDGain");/*RouteEXDGain*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTEEX_ROUTEEXDGAIN (1)\n");
        s_stSceneState.bStaticAERouteEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32Dgain[ISP_AE_ROUTE_EX_MAX_NODES];\n");

        fprintf(fpSoucre, "    for (i = 0; i < stAeRouteEx.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAeRouteEx.astRouteExNode[i].u32Dgain= g_astScenePipeParam[u8Index].stStaticAeRouteEx.au32Dgain[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_aerouteex:RouteEXISPDGain");/*RouteEXISPDGain*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_AEROUTEEX_ROUTEEXISPDGAIN (1)\n");
        s_stSceneState.bStaticAERouteEx = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32IspDgain[ISP_AE_ROUTE_EX_MAX_NODES];\n");

        fprintf(fpSoucre, "    for (i = 0; i < stAeRouteEx.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stAeRouteEx.astRouteExNode[i].u32IspDgain= g_astScenePipeParam[u8Index].stStaticAeRouteEx.au32IspDgain[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticAERouteEX header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_AEROUTEEX_S;\n");
    /**end of StaticAERouteEX source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetAERouteAttrEx(ViPipe, &stAeRouteEx);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticGlobalCac(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticGlobalCac header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_GLOBALCAC_S\n{\n");

    /**start of StaticGlobalCac  source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticGlobalCac_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_GLOBAL_CAC_ATTR_S stGlobalCac;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetGlobalCacAttr(ViPipe, &stGlobalCac);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:GlobalCacEnable");/*GlobalCacEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_ENABLE (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stGlobalCac.bEnable = g_astScenePipeParam[u8Index].stStaticGlobalCac.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:VerCoordinate");/*VerCoordinate*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_VERCOORDINATE (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16VerCoordinate;\n");

        fprintf(fpSoucre, "    stGlobalCac.u16VerCoordinate = g_astScenePipeParam[u8Index].stStaticGlobalCac.u16VerCoordinate;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:HorCoordinate");/*HorCoordinate*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_HORCOORDINATE (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16HorCoordinate;\n");

        fprintf(fpSoucre, "    stGlobalCac.u16HorCoordinate = g_astScenePipeParam[u8Index].stStaticGlobalCac.u16HorCoordinate;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:ParamRedA");/*ParamRedA*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_PARAMREDA (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_S16 s16ParamRedA;\n");

        fprintf(fpSoucre, "    stGlobalCac.s16ParamRedA = g_astScenePipeParam[u8Index].stStaticGlobalCac.s16ParamRedA;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:ParamRedB");/*ParamRedB*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_PARAMREDB (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_S16 s16ParamRedB;\n");

        fprintf(fpSoucre, "    stGlobalCac.s16ParamRedB = g_astScenePipeParam[u8Index].stStaticGlobalCac.s16ParamRedB;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:ParamRedC");/*ParamRedC*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_PARAMREDC (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_S16 s16ParamRedC;\n");

        fprintf(fpSoucre, "    stGlobalCac.s16ParamRedC = g_astScenePipeParam[u8Index].stStaticGlobalCac.s16ParamRedC;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:ParamBlueA");/*ParamBlueA*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_PARAMBLUEA (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_S16 s16ParamBlueA;\n");

        fprintf(fpSoucre, "    stGlobalCac.s16ParamBlueA = g_astScenePipeParam[u8Index].stStaticGlobalCac.s16ParamBlueA;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:ParamBlueB");/*ParamBlueB*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_PARAMBLUEB (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_S16 s16ParamBlueB;\n");

        fprintf(fpSoucre, "    stGlobalCac.s16ParamBlueB = g_astScenePipeParam[u8Index].stStaticGlobalCac.s16ParamBlueB;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:ParamBlueC");/*ParamBlueC*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_PARAMBLUEC (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_S16 s16ParamBlueC;\n");

        fprintf(fpSoucre, "    stGlobalCac.s16ParamBlueC = g_astScenePipeParam[u8Index].stStaticGlobalCac.s16ParamBlueC;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:VerNormShift");/*VerNormShift*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_VERNORMSHIFT (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8VerNormShift;\n");

        fprintf(fpSoucre, "    stGlobalCac.u8VerNormShift = g_astScenePipeParam[u8Index].stStaticGlobalCac.u8VerNormShift;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:VerNormFactor");/*VerNormFactor*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_VERNORMFACTOR (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8VerNormFactor;\n");

        fprintf(fpSoucre, "    stGlobalCac.u8VerNormFactor = g_astScenePipeParam[u8Index].stStaticGlobalCac.u8VerNormFactor;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:HorNormShift");/*HorNormShift*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_HORNORMSHIFT (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8HorNormShift;\n");

        fprintf(fpSoucre, "    stGlobalCac.u8HorNormShift = g_astScenePipeParam[u8Index].stStaticGlobalCac.u8HorNormShift;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:HorNormFactor");/*HorNormFactor*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_HORNORMFACTOR (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8HorNormFactor;\n");

        fprintf(fpSoucre, "    stGlobalCac.u8HorNormFactor = g_astScenePipeParam[u8Index].stStaticGlobalCac.u8HorNormFactor;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_globalcac:CorVarThr");/*CorVarThr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_GLOBALCAC_CORVARTHR (1)\n");
        s_stSceneState.bStaticGlobalCac = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16CorVarThr;\n");

        fprintf(fpSoucre, "    stGlobalCac.u16CorVarThr = g_astScenePipeParam[u8Index].stStaticGlobalCac.u16CorVarThr;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticGlobalCac header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_GLOBALCAC_S;\n");
    /**end of StaticGlobalCac source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetGlobalCacAttr(ViPipe, &stGlobalCac);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticWDRExposure(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticWDRExposure header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_WDREXPOSURE_S\n{\n");

    /**start of StaticWDRExposure  source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticWDRExposure_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_WDR_EXPOSURE_ATTR_S stWdrExposureAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetWDRExposureAttr(ViPipe, &stWdrExposureAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatioType");/*ExpRatioType*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_WDREXPOSURE_EXPRATIOTYPE (1)\n");
        s_stSceneState.bStaticWdrExposure = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8ExpRatioType;\n");

        fprintf(fpSoucre, "    stWdrExposureAttr.enExpRatioType = (ISP_OP_TYPE_E)g_astScenePipeParam[u8Index].stStaticWdrExposure.u8ExpRatioType;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatioMax");/*ExpRatioMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_WDREXPOSURE_EXPRATIOMAX (1)\n");
        s_stSceneState.bStaticWdrExposure = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32ExpRatioMax;\n");

        fprintf(fpSoucre, "    stWdrExposureAttr.u32ExpRatioMax = g_astScenePipeParam[u8Index].stStaticWdrExposure.u32ExpRatioMax;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatioMin");/*ExpRatioMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_WDREXPOSURE_EXPRATIOMIN (1)\n");
        s_stSceneState.bStaticWdrExposure = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32ExpRatioMin;\n");

        fprintf(fpSoucre, "    stWdrExposureAttr.u32ExpRatioMin = g_astScenePipeParam[u8Index].stStaticWdrExposure.u32ExpRatioMin;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_wdrexposure:ExpRatio");/*ExpRatio*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_WDREXPOSURE_EXPRATIO (1)\n");
        s_stSceneState.bStaticWdrExposure = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32ExpRatio[EXP_RATIO_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < EXP_RATIO_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stWdrExposureAttr.au32ExpRatio[i] = g_astScenePipeParam[u8Index].stStaticWdrExposure.au32ExpRatio[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticWDRExposure header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_WDREXPOSURE_S;\n");
    /**end of StaticWDRExposure source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetWDRExposureAttr(ViPipe, &stWdrExposureAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticDRC(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticDRC header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_DRC_S\n{\n");

    /**start of StaticDRC  source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticDRC_Autogenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_DRC_ATTR_S stDrcAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetDRCAttr(ViPipe, &stDrcAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_BENABLE (1)\n");
        s_stSceneState.bStaticDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stDrcAttr.bEnable = g_astScenePipeParam[u8Index].stStaticDrc.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:CurveSelect");/*CurveSelect*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_CURVESELECT (1)\n");
        s_stSceneState.bStaticDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8CurveSelect;\n");

        fprintf(fpSoucre, "    stDrcAttr.enCurveSelect = (ISP_DRC_CURVE_SELECT_E)g_astScenePipeParam[u8Index].stStaticDrc.u8CurveSelect;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:DRCOpType");/*DRCOpType*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_DRCOPTYPE (1)\n");
        s_stSceneState.bStaticDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8DRCOpType;;\n");

        fprintf(fpSoucre, "    stDrcAttr.enOpType = (ISP_OP_TYPE_E)g_astScenePipeParam[u8Index].stStaticDrc.u8DRCOpType;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:DRCAutoStr");/*DRCAutoStr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_DRCAUTOSTR (1)\n");
        s_stSceneState.bStaticDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoStrength;\n");

        fprintf(fpSoucre, "    stDrcAttr.stAuto.u16Strength = g_astScenePipeParam[u8Index].stStaticDrc.u16AutoStrength;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:DRCAutoStrMin");/*DRCAutoStrMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_DRCAUTOSTRMIN (1)\n");
        s_stSceneState.bStaticDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoStrengthMin;\n");

        fprintf(fpSoucre, "    stDrcAttr.stAuto.u16StrengthMin = g_astScenePipeParam[u8Index].stStaticDrc.u16AutoStrengthMin;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:DRCAutoStrMax");/*DRCAutoStrMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_DRCAUTOSTRMAX (1)\n");
        s_stSceneState.bStaticDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoStrengthMax;\n");

        fprintf(fpSoucre, "    stDrcAttr.stAuto.u16StrengthMax = g_astScenePipeParam[u8Index].stStaticDrc.u16AutoStrengthMax;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_drc:DRCToneMappingValue");/*DRCToneMappingValue*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DRC_DRCTONEMAPPINGVALUE (1)\n");
        s_stSceneState.bStaticWdrExposure = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16ToneMappingValue[HI_ISP_DRC_TM_NODE_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < HI_ISP_DRC_TM_NODE_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stDrcAttr.au16ToneMappingValue[i] = g_astScenePipeParam[u8Index].stStaticDrc.au16ToneMappingValue[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticDRC header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_DRC_S;\n");
    /**end of StaticDRC source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stDrcAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticLDCI(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticLDCI header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_LDCI_S\n{\n");

    /**start of StaticLDCI source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticLDCI_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_LDCI_ATTR_S stLDCIAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetLDCIAttr(ViPipe,&stLDCIAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_BENABLE (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stLDCIAttr.bEnable = g_astScenePipeParam[u8Index].stStaticLdci.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:LDCIOpType");/*LDCIOpType*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_LDCIOPTYPE (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8LDCIOpType;\n");

        fprintf(fpSoucre, "    stLDCIAttr.enOpType= (ISP_OP_TYPE_E)g_astScenePipeParam[u8Index].stStaticLdci.u8LDCIOpType;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:LDCIGaussLPFSigma");/*LDCIGaussLPFSigma*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_LDCIGAUSSLPFSIGMA (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8GaussLPFSigma;\n");

        fprintf(fpSoucre, "    stLDCIAttr.u8GaussLPFSigma = g_astScenePipeParam[u8Index].stStaticLdci.u8GaussLPFSigma;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:ManualBlcCtrl");/*ManualBlcCtrl*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_MANUALBLCCTRL (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16ManualBlcCtrl;\n");

        fprintf(fpSoucre, "    stLDCIAttr.stManual.u16BlcCtrl = g_astScenePipeParam[u8Index].stStaticLdci.u16ManualBlcCtrl;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:ManualHePosSigma");/*ManualHePosSigma*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_STA_LDCI_MANUALHEPOSSIGMA (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8ManualHePosSigma;\n");

        fprintf(fpSoucre, "    stLDCIAttr.stManual.stHeWgt.stHePosWgt.u8Sigma = g_astScenePipeParam[u8Index].stStaticLdci.u8ManualHePosSigma;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:ManualHePosMean");/*ManualHePosMean*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_MANUALHEPOSMEAN (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8ManualHePosMean;\n");

        fprintf(fpSoucre, "    stLDCIAttr.stManual.stHeWgt.stHePosWgt.u8Mean = g_astScenePipeParam[u8Index].stStaticLdci.u8ManualHePosMean;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:ManualHeNegSigma");/*ManualHeNegSigma*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_MANUALHENEGSIGMA (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8ManualHeNegSigma;\n");

        fprintf(fpSoucre, "    stLDCIAttr.stManual.stHeWgt.stHeNegWgt.u8Sigma = g_astScenePipeParam[u8Index].stStaticLdci.u8ManualHeNegSigma;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:ManualHeNegWgt");/*ManualHeNegWgt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_MANUALHENEGWGT (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8ManualHeNegWgt;\n");

        fprintf(fpSoucre, "    stLDCIAttr.stManual.stHeWgt.stHeNegWgt.u8Wgt = g_astScenePipeParam[u8Index].stStaticLdci.u8ManualHeNegWgt;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ldci:ManualHeNegMean");/*ManualHeNegMean */
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_LDCI_MANUALHENEGMEAN (1)\n");
        s_stSceneState.bStaticLdci = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8ManualHeNegMean;\n");

        fprintf(fpSoucre, "    stLDCIAttr.stManual.stHeWgt.stHeNegWgt.u8Mean = g_astScenePipeParam[u8Index].stStaticLdci.u8ManualHeNegMean;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticLDCI header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_LDCI_S;\n");
    /**end of StaticLDCI source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetLDCIAttr(ViPipe,&stLDCIAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticDehaze(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticDehaze header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_DEHAZE_S\n{\n");

    /**start of StaticDehaze source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticDeHaze_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_DEHAZE_ATTR_S stDehazeAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetDehazeAttr(ViPipe, &stDehazeAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_dehaze:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DEHAZE_BENABLE (1)\n");
        s_stSceneState.bStaticDehaze = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stDehazeAttr.bEnable = g_astScenePipeParam[u8Index].stStaticDehaze.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_dehaze:DehazeOpType");/*DehazeOpType*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DEHAZE_DEHAZEOPTYPE (1)\n");
        s_stSceneState.bStaticDehaze = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8DehazeOpType;\n");

        fprintf(fpSoucre, "    stDehazeAttr.enOpType = (ISP_OP_TYPE_E)g_astScenePipeParam[u8Index].stStaticDehaze.u8DehazeOpType;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_dehaze:bDehazeUserLutEnable");/*bDehazeUserLutEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DEHAZE_BDEHAZEUSERLUTENABLE (1)\n");
        s_stSceneState.bStaticDehaze = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bUserLutEnable;\n");

        fprintf(fpSoucre, "    stDehazeAttr.bUserLutEnable = g_astScenePipeParam[u8Index].stStaticDehaze.bUserLutEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_dehaze:DehazeLut");/*bDehazeUserLutEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_DEHAZE_DEHAZELUT (1)\n");
        s_stSceneState.bStaticDehaze = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8DehazeLut[256];\n");

        fprintf(fpSoucre, "    for (i = 0; i < 256; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stDehazeAttr.au8DehazeLut[i] = g_astScenePipeParam[u8Index].stStaticDehaze.au8DehazeLut[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticDehaze header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_DEHAZE_S;\n");
    /**end of StaticDehaze source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetDehazeAttr(ViPipe, &stDehazeAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}


HI_S32 SCENETOOL_StaticStatistics(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticStatistics header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_STATISTICSCFG_S\n{\n");

    /**start of StaticStatistics source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticStatisticsCfg_AutoGenerare(VI_PIPE ViPipe, HI_U8 u8Index, HI_BOOL bMetryFixed)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i, j = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_STATISTICS_CFG_S stStatisticsCfg;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetStatisticsConfig(ViPipe, &stStatisticsCfg);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_statistics:ExpWeight_0");/*ExpWeight_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_STATISTICS_EXPWEIGHT (1)\n");
        s_stSceneState.bStaticStatistics = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8AEWeight[AE_ZONE_ROW][AE_ZONE_COLUMN];\n");

        fprintf(fpSoucre, "    if (HI_FALSE == bMetryFixed)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        for (i = 0; i < AE_ZONE_ROW; i++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            for (j = 0; j < AE_ZONE_COLUMN; j++)\n");
        fprintf(fpSoucre, "            {\n");
        fprintf(fpSoucre, "                stStatisticsCfg.stAECfg.au8Weight[i][j] = g_astScenePipeParam[u8Index].stStaticStatistics.au8AEWeight[i][j];\n");
        fprintf(fpSoucre, "            }\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticStatistics header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_STATISTICSCFG_S;\n");
    /**end of StaticStatistics source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetStatisticsConfig(ViPipe, &stStatisticsCfg);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticThreeDNR(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_threednr:3DnrParam_0");/*ThreeDNRCount*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_THREEDNR_3DNRPARAM (1)\n");

		fprintf(fpHeader, "\ntypedef struct\n{\n");
        fprintf(fpHeader, "    HI_S32 IES, IESS;\n");
        fprintf(fpHeader, "    HI_S32 IEDZ;\n");
        fprintf(fpHeader, "} tV59aSceIEy;\n");

        fprintf(fpHeader, "\ntypedef struct\n{\n");
        fprintf(fpHeader, "    HI_S32  SBF, STR, STHp, SFT, kPro;\n");
        fprintf(fpHeader, "    HI_S32  STH[3], SBS[3], SDS[3];\n");
        fprintf(fpHeader, "} tV59aSceSFy;\n");

        fprintf(fpHeader, "\ntypedef struct\n{\n");
        fprintf(fpHeader, "    HI_S32  MATH,  MATE,  MATW;\n");
        fprintf(fpHeader, "    HI_S32  MASW,  MABW,  MAXN, _rB_;\n");
        fprintf(fpHeader, "} tV59aSceMDy;\n");

        fprintf(fpHeader, "\ntypedef struct\n{\n");
        fprintf(fpHeader, "    HI_S32  TFR[4];\n");
        fprintf(fpHeader, "    HI_S32  TDZ, TDX;\n");
        fprintf(fpHeader, "    HI_S32  TFS, _rb_;\n");
        fprintf(fpHeader, "} tV59aSceTFy;\n");

        fprintf(fpHeader, "\ntypedef struct\n{\n");
        fprintf(fpHeader, "    HI_S32  SFC, TFC;\n");
        fprintf(fpHeader, "    HI_S32  CSFS, CSFk;\n");
        fprintf(fpHeader, "    HI_S32  CTFS, CIIR;\n");
        fprintf(fpHeader, "    HI_S32  CTFR;\n");
        fprintf(fpHeader, "} tV59aSceNRc;\n");

        fprintf(fpHeader, "\ntypedef struct hiSCENE_3DNR_S\n{\n");
        fprintf(fpHeader, "    tV59aSceIEy  IEy;\n");
        fprintf(fpHeader, "    tV59aSceSFy  SFy[5];\n");
        fprintf(fpHeader, "    tV59aSceMDy  MDy[2];\n");
        fprintf(fpHeader, "    tV59aSceTFy  TFy[2];\n");
        fprintf(fpHeader, "    HI_S32  HdgType,   BriType;\n");
        fprintf(fpHeader, "    HI_S32  HdgMode,   kTab2;\n");
        fprintf(fpHeader, "    HI_S32  HdgWnd,    kTab3;\n");
        fprintf(fpHeader, "    HI_S32  HdgSFR,    nOut;\n");
        fprintf(fpHeader, "    HI_S32  HdgIES,    _rb_,   nRef;\n");

        fprintf(fpHeader, "    HI_S32  SFRi  [ 4], SFRk [ 4];\n");
        fprintf(fpHeader, "    HI_S32  SBSk2 [32], SBSk3[32];\n");
        fprintf(fpHeader, "    HI_S32  SDSk2 [32], SDSk3[32];\n");
        fprintf(fpHeader, "    HI_S32  BriThr[16];\n");
        fprintf(fpHeader, "    tV59aSceNRc  NRc;\n");
        fprintf(fpHeader, "} HI_SCENE_3DNR_S;\n");

    }

    /**start of StaticThreeDNR header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_THREEDNR_S\n{\n");

    /**start of StaticThreeDNR source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticThreeDNR_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i, j, k = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    VI_PIPE_NRX_PARAM_S stNrx;\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_threednr:ThreeDNRCount");/*ThreeDNRCount*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_THREEDNR_THREEDNRCOUNT (1)\n");
        s_stSceneState.bStaticThreeDnr = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32ThreeDNRCount;\n");

//        fprintf(fpSoucre, "    stNrx.stNRXParamV1.stNRXAutoV1.u32ParamNum = g_astScenePipeParam[u8Index].stStaticThreeDNR.u32ThreeDNRCount;\n");
		fprintf(fpSoucre, "    stNrx.stNRXParamV1.stNRXAutoV1.u32ParamNum = 12;\n");
		free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_threednr:IsoThresh");/*IsoThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_THREEDNR_ISOTHRESH (1)\n");
        s_stSceneState.bStaticThreeDnr = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32ThreeDNRIso[HI_SCENE_THREEDNR_MAX_COUNT];\n");

//        fprintf(fpSoucre, "    stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO = (HI_U32*)malloc(sizeof(HI_U32) * g_astScenePipeParam[u8Index].stStaticThreeDNR.u32ThreeDNRCount);\n");
		fprintf(fpSoucre, "    stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO = (HI_U32*)malloc(sizeof(HI_U32) * 12);\n");
		fprintf(fpSoucre, "    CHECK_SCENE_NULLPTR(stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_threednr:3DnrParam_0");/*3DnrParam_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_THREEDNR_3DNRPARAM (1)\n");
        s_stSceneState.bStaticThreeDnr = HI_TRUE;
        fprintf(fpHeader, "    HI_SCENE_3DNR_S astThreeDNRValue[HI_SCENE_THREEDNR_MAX_COUNT];\n");

        fprintf(fpSoucre, "    stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1 = (VI_PIPE_NRX_PARAM_V1_S*)malloc(sizeof(VI_PIPE_NRX_PARAM_V1_S) * 12);\n");
        fprintf(fpSoucre, "    CHECK_SCENE_NULLPTR(stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1);\n\n");

		fprintf(fpSoucre, "    stNrx.enNRVersion = VI_NR_V1;\n");
		fprintf(fpSoucre, "    stNrx.stNRXParamV1.enOptMode = OPERATION_MODE_AUTO;\n\n");

		fprintf(fpSoucre, "    s32Ret = HI_MPI_VI_GetPipeNRXParam(ViPipe, &stNrx);\n");
		fprintf(fpSoucre, "    if (HI_SUCCESS != s32Ret)\n");
		fprintf(fpSoucre, "    {\n");
		fprintf(fpSoucre, "    	printf(\"HI_MPI_VI_GetPipeNRXParam failed.\");\n");
		fprintf(fpSoucre, "    	FREE_SCENE_PTR(stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO);\n");
		fprintf(fpSoucre, "    	FREE_SCENE_PTR(stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1);\n");
		fprintf(fpSoucre, "    	return HI_FAILURE;\n");
		fprintf(fpSoucre, "    }\n\n");

		fprintf(fpSoucre, "    memcpy(stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO, g_astScenePipeParam[u8Index].stStaticThreeDNR.au32ThreeDNRIso,\n");
        fprintf(fpSoucre, "                      				sizeof(HI_U32) * g_astScenePipeParam[u8Index].stStaticThreeDNR.u32ThreeDNRCount);\n\n");

        fprintf(fpSoucre, "    for (i = 0; i < stNrx.stNRXParamV1.stNRXAutoV1.u32ParamNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].IEy.IES = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].IEy.IES;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].IEy.IESS = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].IEy.IESS;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].IEy.IEDZ = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].IEy.IEDZ;\n");
        fprintf(fpSoucre, "        for (j = 0; j < 5; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].SBF = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].SBF;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].STR = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].STR;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].STHp = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].STHp;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].SFT = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].SFT;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].kPro = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].kPro;\n");
        fprintf(fpSoucre, "            for (k = 0; k < 3; k++)\n");
        fprintf(fpSoucre, "            {\n");
        fprintf(fpSoucre, "                stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].STH[k] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].STH[k];\n");
        fprintf(fpSoucre, "                stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].SBS[k] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].SBS[k];\n");
        fprintf(fpSoucre, "                stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFy[j].SDS[k] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFy[j].SDS[k];\n");
        fprintf(fpSoucre, "            }\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        for (j = 0; j < 2; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].MDy[j].MATH = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].MDy[j].MATH;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].MDy[j].MATE = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].MDy[j].MATE;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].MDy[j].MATW = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].MDy[j].MATW;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].MDy[j].MASW = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].MDy[j].MASW;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].MDy[j].MABW = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].MDy[j].MABW;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].MDy[j].MAXN = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].MDy[j].MAXN;\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        for (j = 0; j < 2; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            for (k = 0; k < 4; k++)\n");
        fprintf(fpSoucre, "            {\n");
        fprintf(fpSoucre, "                stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].TFy[j].TFR[k] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].TFy[j].TFR[k];\n");
        fprintf(fpSoucre, "            }\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].TFy[j].TDZ = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].TFy[j].TDZ;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].TFy[j].TDX = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].TFy[j].TDX;\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].TFy[j].TFS = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].TFy[j].TFS;\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].HdgType = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].HdgType;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].BriType = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].BriType;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].HdgMode = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].HdgMode;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].kTab2 = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].kTab2;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].HdgWnd = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].HdgWnd;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].kTab3 = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].kTab3;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].HdgSFR = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].HdgSFR;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].nOut = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].nOut;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].HdgIES = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].HdgIES;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].nRef = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].nRef;\n");
        fprintf(fpSoucre, "        for (j = 0; j < 4; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFRi[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFRi[j];\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SFRk[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SFRk[j];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        for (j = 0; j < 32; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SBSk2[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SBSk2[j];\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SBSk3[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SBSk3[j];\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SDSk2[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SDSk2[j];\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].SDSk3[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].SDSk3[j];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        for (j = 0; j < 16; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].BriThr[j] = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].BriThr[j];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.SFC = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.SFC;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.TFC = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.TFC;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.CSFS = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.CSFS;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.CSFk = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.CSFk;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.CTFS = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.CTFS;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.CIIR = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.CIIR;\n");
        fprintf(fpSoucre, "        stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1[i].NRc.CTFR = g_astScenePipeParam[u8Index].stStaticThreeDNR.astThreeDNRValue[i].NRc.CTFR;\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticThreeDNR header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_THREEDNR_S;\n");
    /**end of StaticThreeDNR source, constant*/
    fprintf(fpSoucre, "    stNrx.enNRVersion = VI_NR_V1;\n");
    fprintf(fpSoucre, "    stNrx.stNRXParamV1.enOptMode = OPERATION_MODE_AUTO;\n");
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_VI_SetPipeNRXParam(ViPipe, &stNrx);\n");
    fprintf(fpSoucre, "    if (HI_SUCCESS != s32Ret)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        printf(\"HI_MPI_VI_SetPipeNRXParam failed.\");\n");
    fprintf(fpSoucre, "        FREE_SCENE_PTR(stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO);\n");
    fprintf(fpSoucre, "        FREE_SCENE_PTR(stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1);\n");
    fprintf(fpSoucre, "        return HI_FAILURE;\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    FREE_SCENE_PTR(stNrx.stNRXParamV1.stNRXAutoV1.pau32ISO);\n");
    fprintf(fpSoucre, "    FREE_SCENE_PTR(stNrx.stNRXParamV1.stNRXAutoV1.pastNRXParamV1);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}


HI_S32 SCENETOOL_StaticSaturation(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticSaturation header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_SATURATION_S\n{\n");

    /**start of StaticSaturation source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticSaturation_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_SATURATION_ATTR_S stSaturationAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetSaturationAttr(ViPipe, &stSaturationAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");


    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_saturation:AutoSat");/*AutoSat*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_STATURATION_AUTOSAT (1)\n");
        s_stSceneState.bStaticSaturation = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8AutoSat[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stSaturationAttr.stAuto.au8Sat[i] = g_astScenePipeParam[u8Index].stStaticSaturation.au8AutoSat[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticSaturation header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_SATURATION_S;\n");
    /**end of StaticSaturation source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetSaturationAttr(ViPipe, &stSaturationAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    printf(\"i is %%d.\", i);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

/**Gamma has two funcitons*/
HI_S32 SCENETOOL_StaticCCM(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticCCM header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_CCM_S\n{\n");

    /**start of StaticCCM source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticCCM_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 j = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_COLORMATRIX_ATTR_S stColormatrixAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetCCMAttr(ViPipe, &stColormatrixAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");


    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ccm:u32TotalNum");/*AutoColorTemp*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CCM_TOTALNUM (1)\n");
        s_stSceneState.bStaticCCM = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ccm:AutoColorTemp");/*AutoColorTemp*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CCM_AUTOCOLORTEMP (1)\n");
        s_stSceneState.bStaticCCM = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoColorTemp[CCM_MATRIX_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < g_astScenePipeParam[u8Index].stStaticCcm.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stColormatrixAttr.stAuto.astCCMTab[i].u16ColorTemp = g_astScenePipeParam[u8Index].stStaticCcm.au16AutoColorTemp[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_ccm:AutoCCMTable_0");/*AutoCCMTable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CCM_AUTOCCMTABLE (1)\n");
        s_stSceneState.bStaticCCM = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoCCM[CCM_MATRIX_NUM][CCM_MATRIX_SIZE];\n");

        fprintf(fpSoucre, "    for (i = 0; i < g_astScenePipeParam[u8Index].stStaticCcm.u32TotalNum; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        for (j = 0; j < CCM_MATRIX_SIZE; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stColormatrixAttr.stAuto.astCCMTab[i].au16CCM[j] = g_astScenePipeParam[u8Index].stStaticCcm.au16AutoCCM[i][j];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticCCM header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_CCM_S;\n");
    /**end of StaticCCM source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetCCMAttr(ViPipe, &stColormatrixAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    printf(\"i is %%d and j is %%d.\", i, j);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticCSC(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticCSC header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_CSC_S\n{\n");

    /**start of StaticCSC source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticCSC_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_CSC_ATTR_S stCscAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetCSCAttr(ViPipe, &stCscAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_csc:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CSC_ENABLE (1)\n");
        s_stSceneState.bStaticCSC = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stCscAttr.bEnable = g_astScenePipeParam[u8Index].stStaticCsc.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_csc:Hue");/*Hue*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CSC_HUE (1)\n");
        s_stSceneState.bStaticCSC = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8Hue;\n");

        fprintf(fpSoucre, "    stCscAttr.u8Hue = g_astScenePipeParam[u8Index].stStaticCsc.u8Hue;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_csc:Luma");/*Luma*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CSC_LUMA (1)\n");
        s_stSceneState.bStaticCSC = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8Luma;\n");

        fprintf(fpSoucre, "    stCscAttr.u8Luma= g_astScenePipeParam[u8Index].stStaticCsc.u8Luma;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_csc:Contrast");/*Contrast*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CSC_CONTRAST (1)\n");
        s_stSceneState.bStaticCSC = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8Contr;\n");

        fprintf(fpSoucre, "    stCscAttr.u8Contr = g_astScenePipeParam[u8Index].stStaticCsc.u8Contr;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_csc:Saturation");/*Saturation*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CSC_SATURATION (1)\n");
        s_stSceneState.bStaticCSC = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 u8Satu;\n");

        fprintf(fpSoucre, "    stCscAttr.u8Satu = g_astScenePipeParam[u8Index].stStaticCsc.u8Satu;\n");
        free(pszString);
        pszString = HI_NULL;
    }

	snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_csc:ColorGamut");/*ColorGamut*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_CSC_COLORGAMUT (1)\n");
        s_stSceneState.bStaticCSC = HI_TRUE;
        fprintf(fpHeader, "    COLOR_GAMUT_E enColorGamut;\n");

        fprintf(fpSoucre, "    stCscAttr.enColorGamut = g_astScenePipeParam[u8Index].stStaticCsc.enColorGamut;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticCSC header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_CSC_S;\n");
    /**end of StaticCSC source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetCSCAttr(ViPipe, &stCscAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

#if 0
HI_S32 SCENETOOL_StaticSharpen(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of StaticSharpen header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_SHARPEN_S\n{\n");

    /**start of StaticSharpen source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticSharpen_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 i, j = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_SHARPEN_ATTR_S stSharpenAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetIspSharpenAttr(ViPipe, &stSharpenAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticSharpen= HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stSharpenAttr.bEnable = g_astScenePipeParam[u8Index].stStaticSharpen.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:LumaWgt_A");/*LumaWgt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticSharpen = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8LumaWgt[ISP_SHARPEN_LUMA_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_ISP_SHARPEN_GAIN_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stSharpenAttr.au8LumaWgt[i] = g_astScenePipeParam[u8Index].stStaticSharpen.au8LumaWgt[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureStr_0");/*AutoTextureStr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticCCM = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoTextureStr[ISP_ISP_SHARPEN_GAIN_NUM][ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_ISP_SHARPEN_GAIN_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        for (j = 0; j < ISP_AUTO_ISO_STRENGTH_NUM; j++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stSharpenAttr.stAuto.au16TextureStr[i][j] = g_astScenePipeParam[u8Index].stStaticSharpen.au16AutoTextureStr[i][j];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoOverShoot");/*AutoOverShoot*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticSharpen = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8AutoOverShoot[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stSharpenAttr.stAuto.au8OverShoot[i] = g_astScenePipeParam[u8Index].stStaticSharpen.au8AutoOverShoot[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoUnderShoot");/*AutoUnderShoot*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticSharpen = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8AutoUnderShoot[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stSharpenAttr.stAuto.au8UnderShoot[i] = g_astScenePipeParam[u8Index].stStaticSharpen.au8AutoUnderShoot[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoTextureFreq");/*AutoTextureFreq*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticSharpen = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoTextureFreq[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stSharpenAttr.stAuto.au16TextureFreq[i] = g_astScenePipeParam[u8Index].stStaticSharpen.au16AutoTextureFreq[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_sharpen:AutoEdgeFreq");/*AutoEdgeFreq*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bStaticSharpen = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16AutoEdgeFreq[ISP_AUTO_ISO_STRENGTH_NUM];\n");

        fprintf(fpSoucre, "    for (i = 0; i < ISP_AUTO_ISO_STRENGTH_NUM; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stSharpenAttr.stAuto.au16EdgeFreq[i] = g_astScenePipeParam[u8Index].stStaticSharpen.au16AutoEdgeFreq[i];\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticSharpen header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_SHARPEN_S;\n");
    /**end of StaticSharpen source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetIspSharpenAttr(ViPipe, &stSharpenAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}
#endif
HI_S32 SCENETOOL_StaticNr(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};


    /**start of StaticNr header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_NR_S\n{\n");

    /**start of StaticNr source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticNr_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_NR_ATTR_S stNrAttr;\n\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetNRAttr(ViPipe, &stNrAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_nr:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_STA_NR_ENABLE (1)\n");
        s_stSceneState.bStaticNr= HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stNrAttr.bEnable = g_astScenePipeParam[u8Index].stStaticNr.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of StaticNr header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_NR_S;\n");
    /**end of StaticNr source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetNRAttr(ViPipe, &stNrAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_StaticShading(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of Shading header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_STATIC_SHADING_S\n{\n");

    /**start of Shading source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetStaticShading_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_SHADING_ATTR_S stShadingAttr;\n\n");

    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViPipe, &stShadingAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "static_shading:bEnable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

    	fprintf(fpMak, "#define CFG_STA_SHARDING_ENABLE (1)\n");
		s_stSceneState.bStaticShading = HI_TRUE;
        fprintf(fpSoucre, "    stShadingAttr.bEnable = g_astScenePipeParam[u8Index].stStaticShading.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of Shading header, constant*/
    fprintf(fpHeader, "} HI_SCENE_STATIC_SHADING_S;\n");
    /**end of Shading source, constant*/
    fprintf(fpSoucre, "\n    s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViPipe, &stShadingAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicGammaHeader(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicGamma header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_GAMMA_S\n{\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Interval");/*Interval*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_GAMMA_INTERVAL (1)\n");
        fprintf(fpHeader, "    HI_U32 u32InterVal;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:TotalNum");/*TotalNum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_GAMMA_TOTALNUM (1)\n");
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThreshLtoH");/*gammaExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_GAMMA_EXPTHRESHLTOH (1)\n");
        fprintf(fpHeader, "    HI_U64 au64ExpThreshLtoH[HI_SCENE_GAMMA_EXPOSURE_MAX_COUNT];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThreshHtoL");/*gammaExpThreshHtoL*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_GAMMA_EXPTHRESHHTOL (1)\n");
        fprintf(fpHeader, "    HI_U64 au64ExpThreshHtoL[HI_SCENE_GAMMA_EXPOSURE_MAX_COUNT];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThresh");/*GammaExpThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_GAMMA_EXPTHRESH (1)\n");
        fprintf(fpHeader, "    HI_U32 au32GammaExpThresh[HI_SCENE_GAMMA_EXPOSURE_COUNT];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Table_0");/*Table_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_GAMMA_TABLE (1)\n");
        s_stSceneState.bDynamicGamma= HI_TRUE;
        fprintf(fpHeader, "    HI_U16 au16Table[HI_SCENE_GAMMA_EXPOSURE_MAX_COUNT][GAMMA_NODE_NUM];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicGamma header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_GAMMA_S;\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicGammaPhotoSource(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicPhotoGamma source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicPhotoGamma_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_U32 i = 0;\n");
    fprintf(fpSoucre, "    HI_U32 u32ExpLevel = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    ISP_GAMMA_ATTR_S stIspGammaAttr;\n\n");
    fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetGammaAttr(ViPipe, &stIspGammaAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");
    fprintf(fpSoucre, "        stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThresh");/*GammaExpThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpSoucre, "        u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicGamma.u32TotalNum, g_astScenePipeParam[u8Index].stDynamicGamma.au32GammaExpThresh);\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Table_0");/*Table_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bDynamicGamma= HI_TRUE;
        fprintf(fpSoucre, "        for (i = 0; i < GAMMA_NODE_NUM; i++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stIspGammaAttr.u16Table[i] = g_astScenePipeParam[u8Index].stDynamicGamma.au16Table[u32ExpLevel][i];\n");
        fprintf(fpSoucre, "        }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicPhotoGamma source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetGammaAttr(ViPipe, &stIspGammaAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicGammaVideoSource(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszStringH = NULL;
    HI_CHAR *pszStringL = NULL;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicVideoGamma source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicVideoGamma_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_U32 i, j = 0;\n");
    fprintf(fpSoucre, "    HI_U32 u32ExpLevel = 0;\n");
    fprintf(fpSoucre, "    static HI_U32 u32LastExpLevel;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    ISP_GAMMA_ATTR_S stIspGammaAttr;\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThreshLtoH");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringH);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:gammaExpThreshHtoL");/*ExpThreshHtoL*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringL);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:TotalNum");/*TotalNum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);

    if (HI_NULL != pszStringH && HI_NULL != pszStringL && HI_NULL != pszString)
    {
        fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        if (u64Exposure > u64LastExposure)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicGamma.u32TotalNum, g_astScenePipeParam[u8Index].stDynamicGamma.au64ExpThreshLtoH);\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        else\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            u32ExpLevel = SCENE_GetLevelHtoL(u64Exposure, g_astScenePipeParam[u8Index].stDynamicGamma.u32TotalNum, g_astScenePipeParam[u8Index].stDynamicGamma.au64ExpThreshHtoL);\n");
        fprintf(fpSoucre, "        }\n");
        free(pszString);
        free(pszStringH);
        free(pszStringL);
        pszString = HI_NULL;
        pszStringH = HI_NULL;
        pszStringL = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_gamma:Table_0");/*Table_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bDynamicGamma= HI_TRUE;
        fprintf(fpSoucre, "        for (i = 0; i < g_astScenePipeParam[u8Index].stDynamicGamma.u32InterVal; i++)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            CHECK_SCENE_PAUSE();\n");
        fprintf(fpSoucre, "            s32Ret = HI_MPI_ISP_GetGammaAttr(ViPipe, &stIspGammaAttr);\n");
        fprintf(fpSoucre, "            CHECK_SCENE_RET(s32Ret);\n\n");
        fprintf(fpSoucre, "            for (j = 0; j < GAMMA_NODE_NUM; j++)\n");
        fprintf(fpSoucre, "            {\n");
        fprintf(fpSoucre, "                    stIspGammaAttr.u16Table[j] = SCENE_TimeFilter(g_astScenePipeParam[u8Index].stDynamicGamma.au16Table[u32LastExpLevel][j],\n");
        fprintf(fpSoucre, "                                                             g_astScenePipeParam[u8Index].stDynamicGamma.au16Table[u32ExpLevel][j],\n");
        fprintf(fpSoucre, "                                                             g_astScenePipeParam[u8Index].stDynamicGamma.u32InterVal, i);\n");
        fprintf(fpSoucre, "            }\n");

        fprintf(fpSoucre, "            stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;\n");
        fprintf(fpSoucre, "            CHECK_SCENE_PAUSE();\n");
        fprintf(fpSoucre, "            s32Ret = HI_MPI_ISP_SetGammaAttr(ViPipe, &stIspGammaAttr);\n");
        fprintf(fpSoucre, "            CHECK_SCENE_RET(s32Ret);\n");
        fprintf(fpSoucre, "            usleep(30000);\n");
        fprintf(fpSoucre, "        }\n");
		fprintf(fpSoucre, "        u32LastExpLevel = u32ExpLevel;\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicVideoGamma source, constant*/
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicGamma(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SCENETOOL_DynamicGammaHeader(pszIniModule);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_SCENETOOL_DynamicGammaHeader failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicGammaPhotoSource(pszIniModule);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_SCENETOOL_DynamicGammaHeader failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicGammaVideoSource(pszIniModule);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_SCENETOOL_DynamicGammaHeader failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicAE(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR *pszStringTwo = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicAE header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_AE_S\n{\n");

    /**start of DynamicAE source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicAE_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_U32 u32ExpLevel = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        ISP_EXPOSURE_ATTR_S stExposureAttr;\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetExposureAttr(ViPipe, &stExposureAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");


    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ae:aeExpCount");/*ExpLtoHThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
     snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ae:aeExpLtoHThresh");/*aeExpLtoHThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
     snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ae:aeExpHtoLThresh");/*aeExpHtoLThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringTwo);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne && HI_NULL != pszStringTwo)
    {
        fprintf(fpMak, "#define CFG_DYM_AE_EXPCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_AE_EXPTHRESHLTOH (1)\n");
        fprintf(fpMak, "#define CFG_DYM_AE_EXPTHRESHHTOL (1)\n");

        fprintf(fpHeader, "    HI_U8 u8AEExposureCnt;\n");
        fprintf(fpHeader, "    HI_U64 au64ExpLtoHThresh[HI_SCENE_AE_EXPOSURE_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U64 au64ExpHtoLThresh[HI_SCENE_AE_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        if (u64Exposure > u64LastExposure)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt, g_astScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh);\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        else\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            u32ExpLevel = SCENE_GetLevelHtoL(u64Exposure, g_astScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt, g_astScenePipeParam[u8Index].stDynamicAe.au64ExpHtoLThresh);\n");
        fprintf(fpSoucre, "        }\n");

        free(pszString);
        free(pszStringOne);
        free(pszStringTwo);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
        pszStringTwo = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ae:AutoCompesation");/*AutoCompensation*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bDynamicAE= HI_TRUE;
        fprintf(fpMak, "#define CFG_DYM_AE_COMPENSATION (1)\n");
        fprintf(fpHeader, "    HI_U8 au8AutoCompensation[HI_SCENE_AE_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        if (u32ExpLevel == g_astScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt - 1)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stExposureAttr.stAuto.u8Compensation = g_astScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        else\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stExposureAttr.stAuto.u8Compensation = SCENE_Interpulate(u64Exposure,\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel],\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel],\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel + 1],\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au8AutoCompensation[u32ExpLevel + 1]);\n\n");
        fprintf(fpSoucre, "        }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ae:AutoHistOffset");/*AutoHistOffset*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_AE_AUTOHISTOFFSET (1)\n");
        s_stSceneState.bDynamicAE= HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8AutoMaxHistOffset[HI_SCENE_AE_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        if (u32ExpLevel == g_astScenePipeParam[u8Index].stDynamicAe.u8AEExposureCnt - 1)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stExposureAttr.stAuto.u8MaxHistOffset = g_astScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        else\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stExposureAttr.stAuto.u8MaxHistOffset = SCENE_Interpulate(u64Exposure,\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel],\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel],\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au64ExpLtoHThresh[u32ExpLevel + 1],\n");
        fprintf(fpSoucre, "                                                               g_astScenePipeParam[u8Index].stDynamicAe.au8AutoMaxHistOffset[u32ExpLevel + 1]);\n\n");
        fprintf(fpSoucre, "        }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicAE header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_AE_S;\n");
    /**end of DynamicAE source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetExposureAttr(ViPipe, &stExposureAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicShading(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR *pszStringTwo = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};


    /**start of DynamicShading header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_SHADING_S\n{\n");

    /**start of DynamicShading source, constant*/
	fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicShading_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");
	fprintf(fpSoucre, "	HI_U32 u32ExpLevel = 0;\n");
	fprintf(fpSoucre, "	HI_S32 s32Ret = HI_SUCCESS;\n\n");

	fprintf(fpSoucre, "	ISP_SHADING_ATTR_S stShadingAttr;\n\n");

	fprintf(fpSoucre, "	if (u64Exposure != u64LastExposure)\n");
	fprintf(fpSoucre, "	{\n");
	fprintf(fpSoucre, "		CHECK_SCENE_PAUSE();\n");
	fprintf(fpSoucre, "		s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViPipe,&stShadingAttr);\n");
	fprintf(fpSoucre, "		CHECK_SCENE_RET(s32Ret);\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ExpThreshCnt");/*ExpThreshCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
	snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ExpThreshLtoH");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
	snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_shading:ManualStrength");/*ManualStrength*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringTwo);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne && HI_NULL != pszStringTwo)
   	{
   		s_stSceneState.bDynamicShading = HI_TRUE;
        fprintf(fpMak, "#define CFG_DYM_SHADING_EXPTHRESHCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_SHADING_EXPTHRESHLTOH (1)\n");
		fprintf(fpMak, "#define CFG_DYM_SHADING_MANUALSTRENGTH (1)\n");

        fprintf(fpHeader, "    HI_U32 u32ExpThreshCnt;\n");
        fprintf(fpHeader, "    HI_U64 au64ExpThreshLtoH[HI_SCENE_SHADING_EXPOSURE_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U16 au16ManualStrength[HI_SCENE_SHADING_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "\n	u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt, g_astScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH);\n");
        fprintf(fpSoucre, "	if (u32ExpLevel == 0 || u32ExpLevel == g_astScenePipeParam[u8Index].stDynamicShading.u32ExpThreshCnt - 1)\n");
        fprintf(fpSoucre, "	{\n");
        fprintf(fpSoucre, "	    stShadingAttr.u16MeshStr = g_astScenePipeParam[u8Index].stDynamicShading.au16ManualStrength[u32ExpLevel];\n");
        fprintf(fpSoucre, "	}\n");
        fprintf(fpSoucre, "	else\n");
        fprintf(fpSoucre, "	{\n");
        fprintf(fpSoucre, "	    stShadingAttr.u16MeshStr = SCENE_Interpulate(u64Exposure,\n");
        fprintf(fpSoucre, "	                     g_astScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u32ExpLevel - 1],\n");
        fprintf(fpSoucre, "	                     g_astScenePipeParam[u8Index].stDynamicShading.au16ManualStrength[u32ExpLevel - 1],\n");
        fprintf(fpSoucre, "	                     g_astScenePipeParam[u8Index].stDynamicShading.au64ExpThreshLtoH[u32ExpLevel],\n");
        fprintf(fpSoucre, "	                     g_astScenePipeParam[u8Index].stDynamicShading.au16ManualStrength[u32ExpLevel]);\n");
        fprintf(fpSoucre, "	}\n");
        fprintf(fpSoucre, "	CHECK_SCENE_PAUSE();\n");
        fprintf(fpSoucre, "	s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViPipe,&stShadingAttr);\n");
        fprintf(fpSoucre, "	CHECK_SCENE_RET(s32Ret);\n");

		free(pszString);
        free(pszStringOne);
        free(pszStringTwo);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
        pszStringTwo = HI_NULL;
	}

    /**end of DynamicShading header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_SHADING_S;\n");
    /**end of DynamicShading source, constant*/
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicLDCI(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR *pszStringTwo = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicLDCI header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_LDCI_S\n{\n");

    /**start of DynamicLDCI source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicLDCI_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");

    fprintf(fpSoucre, "    HI_U32 u32ExpLevel = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    ISP_LDCI_ATTR_S stLDCIAttr;\n\n");

    fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetLDCIAttr(ViPipe,&stLDCIAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ldci:EnableCount");/*EnableCount*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ldci:EnableExpThreshLtoH");/*EnableExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ldci:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringTwo);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne && HI_NULL != pszStringTwo)
    {
        fprintf(fpMak, "#define CFG_DYM_LDCI_ENABLECNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_LDCI_ENABLEEXPTHRESHLTOH (1)\n");
        fprintf(fpMak, "#define CFG_DYM_LDCI_ENABLE (1)\n");
        s_stSceneState.bDynamicLDCI = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32EnableCnt;\n");
        fprintf(fpHeader, "    HI_U8 au8Enable[HI_SCENE_LDCI_EXPOSURE_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U64 au64EnableExpThreshLtoH[HI_SCENE_LDCI_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicLDCI.u32EnableCnt, g_astScenePipeParam[u8Index].stDynamicLDCI.au64EnableExpThreshLtoH);\n");
        fprintf(fpSoucre, "        stLDCIAttr.bEnable = g_astScenePipeParam[u8Index].stDynamicLDCI.au8Enable[u32ExpLevel];\n");
        free(pszString);
        free(pszStringOne);
        free(pszStringTwo);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
        pszStringTwo = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ldci:ExpThreshCnt");/*ExpThreshCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ldci:ExpThreshLtoH");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_ldci:ManualLDCIHePosWgt");/*ManualLDCIHePosWgt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringTwo);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne && HI_NULL != pszStringTwo)
    {
        fprintf(fpMak, "#define CFG_DYM_LDCI_EXPTHRESHCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_LDCI_XPTHRESHLTOH (1)\n");
        fprintf(fpMak, "#define CFG_DYM_LDCI_MANUALHEPOSWGT (1)\n");

        s_stSceneState.bDynamicLDCI = HI_TRUE;

        free(pszString);
        free(pszStringOne);
        free(pszStringTwo);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
        pszStringTwo = HI_NULL;
    }

    /**end of DynamicLDCI header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_LDCI_S;\n");
    /**end of DynamicLDCI source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetLDCIAttr(ViPipe,&stLDCIAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicDehaze(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR *pszStringTwo = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicDehaze header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_DEHAZE_S\n{\n");

    /**start of DynamicDehaze source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicDehaze_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");

    fprintf(fpSoucre, "    HI_U32 u32ExpLevel = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    ISP_DEHAZE_ATTR_S stDehazeAttr;\n\n");

    fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetDehazeAttr(ViPipe,&stDehazeAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:ExpThreshCnt");/*ExpThreshCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:ExpThreshLtoH");/*ExpThreshLtoH*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_dehaze:ManualDehazeStr");/*ManualDehazeStr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringTwo);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne && HI_NULL != pszStringTwo)
    {
        fprintf(fpMak, "#define CFG_DYM_DEHAZE_EXPTHRESHCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_DEHAZE_EXPTHRESHLTOH (1)\n");
        fprintf(fpMak, "#define CFG_DYM_DEHAZE_MANUALDEHAZESTR (1)\n");
        s_stSceneState.bDynamicDehaze = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 u32ExpThreshCnt;\n");
        fprintf(fpHeader, "    HI_U64 au64ExpThreshLtoH[HI_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U8 au8ManualStrength[HI_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt, g_astScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH);\n");
        fprintf(fpSoucre, "        if (u32ExpLevel == 0 || u32ExpLevel == g_astScenePipeParam[u8Index].stDynamicDehaze.u32ExpThreshCnt - 1)\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stDehazeAttr.stManual.u8strength = g_astScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel];\n");
        fprintf(fpSoucre, "        }\n");
        fprintf(fpSoucre, "        else\n");
        fprintf(fpSoucre, "        {\n");
        fprintf(fpSoucre, "            stDehazeAttr.stManual.u8strength = SCENE_Interpulate(u64Exposure,\n");
        fprintf(fpSoucre, "                             g_astScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH[u32ExpLevel - 1],\n");
        fprintf(fpSoucre, "                             g_astScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel - 1],\n");
        fprintf(fpSoucre, "                             g_astScenePipeParam[u8Index].stDynamicDehaze.au64ExpThreshLtoH[u32ExpLevel],\n");
        fprintf(fpSoucre, "                             g_astScenePipeParam[u8Index].stDynamicDehaze.au8ManualStrength[u32ExpLevel]);\n");
        fprintf(fpSoucre, "        }\n");

        free(pszString);
        free(pszStringOne);
        free(pszStringTwo);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
        pszStringTwo = HI_NULL;
    }

    /**end of DynamicDehaze header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_DEHAZE_S;\n");
    /**end of DynamicDehaze source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetDehazeAttr(ViPipe,&stDehazeAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicFsWdr(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicFsWdr header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_FSWDR_S\n{\n");

    /**start of DynamicFsWdr source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicFsWdr_AutoGenerate(VI_PIPE ViPipe, HI_U32 u32ISO, HI_U32 u32LastISO, HI_U8 u8Index, HI_U32 u32ActRation)\n{\n");

    fprintf(fpSoucre, "    HI_U32 u32IsoLevel = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    ISP_WDR_FS_ATTR_S stFSWDRAttr;\n\n");

    fprintf(fpSoucre, "    if (u32ISO != u32LastISO)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetFSWDRAttr(ViPipe,&stFSWDRAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:IsoCnt");/*IsoCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_FSWDR_ISOCNT (1)\n");
        fprintf(fpHeader, "    HI_U32 u32IsoCnt;\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:ExpRation");/*ExpRation*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_FSWDR_EXPRATION (1)\n");
        fprintf(fpHeader, "    HI_U32 au32ExpRation[HI_SCENE_FSWDR_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "        u32IsoLevel = SCENE_GetLevelLtoH_U32(u32ActRation, g_astScenePipeParam[u8Index].stDynamicFSWDR.u32IsoCnt, g_astScenePipeParam[u8Index].stDynamicFSWDR.au32ExpRation);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:MotionComp");/*MotionComp*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_FSWDR_MOTIONCOMP (1)\n");
        s_stSceneState.bDynamicFsWdr = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8MotionComp[HI_SCENE_FSWDR_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "        stFSWDRAttr.stWDRCombine.bMotionComp = g_astScenePipeParam[u8Index].stDynamicFSWDR.au8MotionComp[u32IsoLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:ISOLtoHThresh");/*ISOLtoHThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_FSWDR_ISOTHRESHLTOH (1)\n");
        fprintf(fpHeader, "    HI_U32 au32ISOLtoHThresh[HI_SCENE_FSWDR_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "        u32IsoLevel = SCENE_GetLevelLtoH_U32(u32ISO, g_astScenePipeParam[u8Index].stDynamicFSWDR.u32IsoCnt, g_astScenePipeParam[u8Index].stDynamicFSWDR.au32ISOLtoHThresh);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:BnrStr");/*BnrStr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    { fprintf(fpMak, "#define CFG_DYM_FSWDR_BNRSTR (1)\n");
        s_stSceneState.bDynamicFsWdr = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8BnrStr[HI_SCENE_FSWDR_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "        stFSWDRAttr.stBnr.u8BnrStr = g_astScenePipeParam[u8Index].stDynamicFSWDR.au8BnrStr[u32IsoLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_fswdr:BnrMode");/*BnrMode*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_FSWDR_BNRMODE (1)\n");
        s_stSceneState.bDynamicFsWdr = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8BnrMode[HI_SCENE_FSWDR_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "        stFSWDRAttr.stBnr.enBnrMode = (ISP_BNR_MODE_E)g_astScenePipeParam[u8Index].stDynamicFSWDR.au8BnrMode[u32IsoLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicFsWdr header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_FSWDR_S;\n");
    /**end of DynamicFsWdr source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetFSWDRAttr(ViPipe,&stFSWDRAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicHdrOETF(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicHdrOTEF header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_HDROEFT_S\n{\n");

    /**start of DynamicHdrOTEF source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicHDROTEF_AutoGenerate(VI_PIPE ViPipe, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32HDRBrightRatio, HI_U8 u8Index)\n{\n");

    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    HDR_OETF_PARAM_S stOETFAttr;\n\n");
    fprintf(fpSoucre, "    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_HDR_GetOETFParam(ViPipe, VpssGrp,VpssChn, &stOETFAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdroetf:HDRCount");/*HDRCount*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdroetf:BrightRatioThresh");/*BrightRatioThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
        fprintf(fpMak, "#define CFG_DYM_HDROTEF_HDRCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_HDROTEF_BRIGHTRATIONTHRESH (1)\n");
        fprintf(fpHeader, "    HI_U32 u32HDRCount;\n");
        fprintf(fpHeader, "    HI_U32 au32BrightRatioThresh[HI_SCENE_HDR_MAX_COUNT];\n");


        fprintf(fpSoucre, "    HI_U32 u32BrightRatioLevel = 0;\n");
        fprintf(fpSoucre, "    HI_U32 u32LastBrightRatioLevel = 0;\n");
        fprintf(fpSoucre, "    u32BrightRatioLevel = SCENE_GetLevelLtoH_U32(u32HDRBrightRatio, g_astScenePipeParam[u8Index].stDynamicHDROETF.u32HDRCount, g_astScenePipeParam[u8Index].stDynamicHDROETF.au32BrightRatioThresh);\n");
        fprintf(fpSoucre, "    if (u32BrightRatioLevel > 0)\n");
        fprintf(fpSoucre, "   {\n");
        fprintf(fpSoucre, "        u32LastBrightRatioLevel = u32BrightRatioLevel - 1;\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    if (u32HDRBrightRatio >= g_astScenePipeParam[u8Index].stDynamicHDROETF.au32BrightRatioThresh[g_astScenePipeParam[u8Index].stDynamicHDROETF.u32HDRCount - 1])\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u32LastBrightRatioLevel = u32BrightRatioLevel;\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdroetf:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDROTEF_ENABLE (1)\n");
        s_stSceneState.bDynamicHdrOTEF = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stOETFAttr.bEnable = g_astScenePipeParam[u8Index].stDynamicHDROETF.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdroetf:MaxLum");/*MaxLum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDROTEF_MAXLUM (1)\n");
        s_stSceneState.bDynamicHdrOTEF = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32MaxLum[HI_SCENE_HDR_MAX_COUNT];\n");

        fprintf(fpSoucre, "    stOETFAttr.u32MaxLum = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32MaxLum[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32MaxLum[u32BrightRatioLevel]);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdroetf:CurLum");/*CurLum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDROTEF_CURLUM (1)\n");
        s_stSceneState.bDynamicHdrOTEF = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32CurLum[HI_SCENE_HDR_MAX_COUNT];\n");

        fprintf(fpSoucre, "    stOETFAttr.u32CurLum = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32CurLum[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                   g_astScenePipeParam[u8Index].stDynamicHDROETF.au32CurLum[u32BrightRatioLevel]);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicHdrOTEF header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_HDROETF_S;\n");
    /**end of DynamicHdrOTEF source, constant*/
    fprintf(fpSoucre, "\n    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_HDR_SetOETFParam(ViPipe, VpssGrp,VpssChn, &stOETFAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicHdrTM(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicHdrTM header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_HDRTM_S\n{\n");

    /**start of DynamicHdrTM source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicHDRTM_AutoGenerate(VI_PIPE ViPipe, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32HDRBrightRatio, HI_U8 u8Index)\n{\n");


    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    HDR_TM_PARAM_S stTMAttr;\n\n");
    fprintf(fpSoucre, "    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_HDR_GetTMParam(ViPipe, VpssGrp,VpssChn, &stTMAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrtm:HDRCount");/*HDRCount*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrtm:BrightRatioThresh");/*BrightRatioThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
        fprintf(fpMak, "#define CFG_DYM_HDRTM_HDRCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_HDRTM_BRIGHTRATIOTHRESH (1)\n");

        fprintf(fpHeader, "    HI_U32 u32HDRCount;\n");
        fprintf(fpHeader, "    HI_U32 au32BrightRatioThresh[HI_SCENE_HDR_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U32 u32BrightRatioLevel = 0;\n");
        fprintf(fpSoucre, "    HI_U32 u32LastBrightRatioLevel = 0;\n");
        fprintf(fpSoucre, "    u32BrightRatioLevel = SCENE_GetLevelLtoH_U32(u32HDRBrightRatio, g_astScenePipeParam[u8Index].stDynamicHDRTm.u32HDRCount, g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh);\n");
        fprintf(fpSoucre, "    if (u32BrightRatioLevel > 0)\n");
        fprintf(fpSoucre, "   {\n");
        fprintf(fpSoucre, "        u32LastBrightRatioLevel = u32BrightRatioLevel - 1;\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    if (u32HDRBrightRatio >= g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[g_astScenePipeParam[u8Index].stDynamicHDRTm.u32HDRCount - 1])\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u32LastBrightRatioLevel = u32BrightRatioLevel;\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrtm:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDRTM_ENABLE (1)\n");
        s_stSceneState.bDynamicHdrTM = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stTMAttr.bEnable = g_astScenePipeParam[u8Index].stDynamicHDRTm.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrtm:TMCubicX_0");/*TMCubicX_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDRTM_TMCUBIC (1)\n");
        s_stSceneState.bDynamicHdrTM = HI_TRUE;
        fprintf(fpHeader, "    HDR_TM_CUBIC_ATTR_S astCubicPoint[5][HI_SCENE_HDR_MAX_COUNT];\n");


        fprintf(fpSoucre, "    HI_S32 i = 0;\n");
        fprintf(fpSoucre, "    for (i = 0; i < 5; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stTMAttr.astCubicPoint[i].u16X = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.astCubicPoint[i][u32LastBrightRatioLevel].u16X,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.astCubicPoint[i][u32BrightRatioLevel].u16X);\n");
        fprintf(fpSoucre, "        stTMAttr.astCubicPoint[i].u16Y = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.astCubicPoint[i][u32LastBrightRatioLevel].u16Y,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.astCubicPoint[i][u32BrightRatioLevel].u16Y);\n");
        fprintf(fpSoucre, "        stTMAttr.astCubicPoint[i].u16Slope = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.astCubicPoint[i][u32LastBrightRatioLevel].u16Slope,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRTm.astCubicPoint[i][u32BrightRatioLevel].u16Slope);\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicHdrTM header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_HDRTM_S;\n");
    /**end of DynamicHdrTM source, constant*/
    fprintf(fpSoucre, "\n    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_HDR_SetTMParam(ViPipe, VpssGrp,VpssChn, &stTMAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicHdrDRC(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicHdrDRC header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_HDRDRC_S\n{\n");

    /**start of DynamicHdrDRC source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicHDRDRC_AutoGenerate(VI_PIPE ViPipe, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32HDRBrightRatio, HI_U8 u8Index)\n{\n");

    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_DRC_ATTR_S stIspDrcAttr;\n\n");
    fprintf(fpSoucre, "    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetDRCAttr(ViPipe, &stIspDrcAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrdrc:HDRCount");/*HDRCount*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrdrc:BrightRatioThresh");/*BrightRatioThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
        fprintf(fpMak, "#define CFG_DYM_HDRDRC_HDRCNT (1)\n");
        fprintf(fpMak, "#define CFG_DYM_HDRDRC_BRIGHTRATIOTHRESH (1)\n");

        fprintf(fpHeader, "    HI_U32 u32HDRCount;\n");
        fprintf(fpHeader, "    HI_U32 au32BrightRatioThresh[HI_SCENE_HDR_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U32 u32BrightRatioLevel = 0;\n");
        fprintf(fpSoucre, "    HI_U32 u32LastBrightRatioLevel = 0;\n");

        fprintf(fpSoucre, "    u32BrightRatioLevel = SCENE_GetLevelLtoH_U32(u32HDRBrightRatio, g_astScenePipeParam[u8Index].stDynamicHDRDRC.u32HDRCount, g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh);\n");
        fprintf(fpSoucre, "    if (u32BrightRatioLevel > 0)\n");
        fprintf(fpSoucre, "   {\n");
        fprintf(fpSoucre, "        u32LastBrightRatioLevel = u32BrightRatioLevel - 1;\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    if (u32HDRBrightRatio >= g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[g_astScenePipeParam[u8Index].stDynamicHDRDRC.u32HDRCount - 1])\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u32LastBrightRatioLevel = u32BrightRatioLevel;\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrdrc:bEnable");/*bEnable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDRDRC_ENABLE (1)\n");
        s_stSceneState.bDynamicHdrDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        fprintf(fpSoucre, "    stIspDrcAttr.bEnable = g_astScenePipeParam[u8Index].stDynamicHDRDRC.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_hdrdrc:DRCCubicX_0");/*DRCCubicX_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_HDRDRC_DRCCUBIC (1)\n");
        s_stSceneState.bDynamicHdrDrc = HI_TRUE;
        fprintf(fpHeader, "    HDR_TM_CUBIC_ATTR_S astCubicPoint[5][HI_SCENE_HDR_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_S32 i = 0;\n");
        fprintf(fpSoucre, "    for (i = 0; i < 5; i++)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        stIspDrcAttr.astCubicPoint[i].u16X = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.astCubicPoint[i][u32LastBrightRatioLevel].u16X,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.astCubicPoint[i][u32BrightRatioLevel].u16X);\n");
        fprintf(fpSoucre, "        stIspDrcAttr.astCubicPoint[i].u16Y = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.astCubicPoint[i][u32LastBrightRatioLevel].u16Y,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.astCubicPoint[i][u32BrightRatioLevel].u16Y);\n");
        fprintf(fpSoucre, "        stIspDrcAttr.astCubicPoint[i].u16Slope = SCENE_Interpulate(u32HDRBrightRatio,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[u32LastBrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.astCubicPoint[i][u32LastBrightRatioLevel].u16Slope,\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.au32BrightRatioThresh[u32BrightRatioLevel],\n");
        fprintf(fpSoucre, "                                                      g_astScenePipeParam[u8Index].stDynamicHDRDRC.astCubicPoint[i][u32BrightRatioLevel].u16Slope);\n");
        fprintf(fpSoucre, "    }\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicHdrDRC header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_HDRDRC_S;\n");
    /**end of DynamicHdrDRC source, constant*/
    fprintf(fpSoucre, "\n    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stIspDrcAttr);\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe, &stIspDrcAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicFalseColor(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicFalseColor header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_FALSECOLOR_S\n{\n");

    /**start of DynamicFalseColor source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicFalseColor_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");

    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_ANTIFALSECOLOR_ATTR_S stAntiFalsecolorAttr;\n\n");
    fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetAntiFalseColorAttr(ViPipe, &stAntiFalsecolorAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_falsecolor:FalsecolorExpThresh");/*FalsecolorExpThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_falsecolor:TotalNum");/*TotalNum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
        fprintf(fpMak, "#define CFG_DYM_FALSECOLOR_TOTALNUM (1)\n");
        fprintf(fpMak, "#define CFG_DYM_FALSECOLOR_EXPTHRESH (1)\n");
        fprintf(fpHeader, "    HI_U32 au32FalsecolorExpThresh[HI_SCENE_FALSECOLOR_EXPOSURE_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");

        fprintf(fpSoucre, "        HI_U32 u32ExpLevel = 0;\n");
        fprintf(fpSoucre, "        u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicFalsecolor.u32TotalNum, g_astScenePipeParam[u8Index].stDynamicFalsecolor.au32FalsecolorExpThresh);\n");

        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_falsecolor:ManualStrength");/*ManualStrength*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_FALSECOLOR_MANUALSTRENGTH (1)\n");
        s_stSceneState.bDynamicFalseColor = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8ManualStrength[HI_SCENE_FALSECOLOR_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        stAntiFalsecolorAttr.stManual.u8AntiFalseColorStrength = g_astScenePipeParam[u8Index].stDynamicFalsecolor.au8ManualStrength[u32ExpLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicFalseColor header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_FALSECOLOR_S;\n");
    /**end of DynamicFalseColor source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetAntiFalseColorAttr(ViPipe, &stAntiFalsecolorAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicDemosaic(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicDemosaic header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_DEMOSAIC_S\n{\n");

    /**start of DynamicDemosaic source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicDemosaic_AutoGenerate(VI_PIPE ViPipe, HI_U64 u64Exposure, HI_U64 u64LastExposure, HI_U8 u8Index)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    ISP_DEMOSAIC_ATTR_S stDemosaicAttr;\n\n");
    fprintf(fpSoucre, "    if (u64Exposure != u64LastExposure)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_GetDemosaicAttr(ViPipe, &stDemosaicAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_demosaic:DemosaicExpThresh");/*DemosaicExpThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
     snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_demosaic:TotalNum");/*TotalNum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
    	fprintf(fpMak, "#define CFG_DYM_DEMOSAIC_DEMOSAICEXPTHRESH (1)\n");
    	fprintf(fpMak, "#define CFG_DYM_DEMOSAIC_TOTALNUM (1)\n");
        fprintf(fpHeader, "    HI_U32 au32DemosaicExpThresh[HI_SCENE_DEMOSAIC_EXPOSURE_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");

        fprintf(fpSoucre, "        HI_U32 u32ExpLevel = 0;\n");
        fprintf(fpSoucre, "        u32ExpLevel = SCENE_GetLevelLtoH(u64Exposure, g_astScenePipeParam[u8Index].stDynamicDemosaic.u32TotalNum, g_astScenePipeParam[u8Index].stDynamicDemosaic.au32DemosaicExpThresh);\n");

        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_demosaic:ManualAntiAliasThr");/*ManualAntiAliasThr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_DEMOSAIC_MANUALANTIALIASTHR (1)\n");
        s_stSceneState.bDynamicDemosaic = HI_TRUE;
        fprintf(fpHeader, "    HI_U16  au16ManualAntiAliasThr[HI_SCENE_DEMOSAIC_EXPOSURE_MAX_COUNT];\n");

        fprintf(fpSoucre, "        stDemosaicAttr.stManual.u16AntiAliasThr = g_astScenePipeParam[u8Index].stDynamicDemosaic.au16ManualAntiAliasThr[u32ExpLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicDemosaic header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_DEMOSAIC_S;\n");
    /**end of DynamicDemosaic source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetDemosaicAttr(ViPipe, &stDemosaicAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_DynamicDIS(const HI_CHAR *pszIniModule)
{

    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of DynamicDIS header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_DYNAMIC_DIS_S\n{\n");

    /**start of DynamicDIS source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetDynamicDIS_AutoGenerate(VI_PIPE ViPipe, VI_CHN ViChn, HI_U32 u32Iso, HI_U32 u32LastIso, HI_U8 u8Index, HI_BOOL bEnable)\n{\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");
    fprintf(fpSoucre, "    DIS_ATTR_S stDisAttr;\n\n");
    fprintf(fpSoucre, "    if (HI_TRUE != bEnable)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        return HI_SUCCESS;\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    if (u32Iso != u32LastIso)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_VI_GetChnDISAttr(ViPipe, ViChn,&stDisAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_dis:DISIsoThresh");/*DISIsoThresh*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_dis:TotalNum");/*TotalNum*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
    	fprintf(fpMak, "#define CFG_DYM_DIS_DISISOTHRESH (1)\n");
    	fprintf(fpMak, "#define CFG_DYM_DIS_TOTALNUM (1)\n");
        fprintf(fpHeader, "    HI_U32 au32DISIsoThresh[HI_SCENE_DIS_ISO_MAX_COUNT];\n");
        fprintf(fpHeader, "    HI_U32 u32TotalNum;\n");

        fprintf(fpSoucre, "        HI_U32 u32IsoLevel = 0;\n");
        fprintf(fpSoucre, "        u32IsoLevel = SCENE_GetLevelLtoH_U32(u32Iso, g_astScenePipeParam[u8Index].stDynamicDis.u32TotalNum, g_astScenePipeParam[u8Index].stDynamicDis.au32DISIsoThresh);\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "dynamic_dis:MovingSubjectLevel");/*MovingSubjectLevel*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_DIS_MOVINGSUBJECTLEVEL (1)\n");
        s_stSceneState.bDynamicDIS = HI_TRUE;
        fprintf(fpHeader, "    HI_U32 au32MovingSubjectLevel[HI_SCENE_DIS_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "        stDisAttr.u32MovingSubjectLevel = g_astScenePipeParam[u8Index].stDynamicDis.au32MovingSubjectLevel[u32IsoLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    /**end of DynamicDIS header, constant*/
    fprintf(fpHeader, "} HI_SCENE_DYNAMIC_DIS_S;\n");
    /**end of DynamicDIS source, constant*/
    fprintf(fpSoucre, "\n        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_VI_SetChnDISAttr(ViPipe, ViChn,&stDisAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");

    return HI_SUCCESS;
}

HI_S32 SCENETOOL_ThreadDRC(const HI_CHAR *pszIniModule)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR *pszString = NULL;
    HI_CHAR *pszStringOne = NULL;
    HI_CHAR aszIniNodeName[SCENETOOL_INIPARAM_NODE_NAME_LEN] = {0,};

    /**start of ThreadDRC header, constant*/
    fprintf(fpHeader, "\ntypedef struct hiSCENE_THREAD_DRC_S\n{\n");

    /**start of ThreadDRC source, constant*/
    fprintf(fpSoucre, "\nHI_S32 HI_SCENE_SetThreadDRC_AutoGenerate(VI_PIPE ViPipe, HI_U8 u8Index, HI_U32 u32ActRation, HI_U32 u32Iso)\n{\n");
    fprintf(fpSoucre, "    HI_U32 u32IsoLevel = 0;\n");
    fprintf(fpSoucre, "    HI_U32 u32RatioLevel = 0;\n");
    fprintf(fpSoucre, "    HI_U32 i = 0;\n");
    fprintf(fpSoucre, "    HI_S32 s32Ret = HI_SUCCESS;\n\n");

    fprintf(fpSoucre, "    ISP_PUB_ATTR_S stPubAttr;\n");
    fprintf(fpSoucre, "    ISP_DRC_ATTR_S stIspDrcAttr;\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RationCnt");/*RationCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_RATIONCNT (1)\n");
        fprintf(fpHeader, "    HI_U32 u32RationCount;\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RationLevel");/*RationLevel*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        fprintf(fpMak, "#define CFG_DYM_THREADDRC_RATIONLEVE (1)\n");
        fprintf(fpHeader, "    HI_U32 au32RationLevel[HI_SCENE_DRC_RATIO_MAX_COUNT];\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:IsoCnt");/*IsoCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_ISOCNT (1)\n");
        fprintf(fpHeader, "    HI_U32 u32ISOCount;\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:IsoLevel");/*IsoLevel*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_ISOLEVEL (1)\n");
        fprintf(fpHeader, "    HI_U32 au32ISOLevel[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Interval");/*Interval*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_INTERVAL (1)\n");
        fprintf(fpHeader, "    HI_U32 u32Interval;\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_ENABLE (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_BOOL bEnable;\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingBrightMax");/*LocalMixingBrightMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
   		 fprintf(fpMak, "#define CFG_DYM_THREADDRC_LOCALMIXINGBRIGHTMAX (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8LocalMixingBrightMax[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8LocalMixingBrightMax = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingBrightMin");/*LocalMixingBrightMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_LOCALMIXINGBRIGHTMIN (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8LocalMixingBrightMin[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8LocalMixingBrightMin = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingDarkMax");/*LocalMixingDarkMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_LOCALMIXINGDARKMAX (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8LocalMixingDarkMax[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8LocalMixingDarkMax = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingDarkMin");/*LocalMixingDarkMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	 fprintf(fpMak, "#define CFG_DYM_THREADDRC_LOCALMIXINGDARKMIN (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8LocalMixingDarkMin[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8LocalMixingDarkMin = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:DetailAdjustFactor");/*DetailAdjustFactor*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_DETAILADJUSTFACTOR (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_S8 as8DetailAdjustFactor[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_S8 s8DetailAdjustFactor = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:SpatialFltCoef");/*SpatialFltCoef*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_DSPATIALFLTCOEF (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8SpatialFltCoef[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8SpatialFltCoef = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RangeFltCoef");/*RangeFltCoef*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_RANGEFLTCOEF (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8RangeFltCoef[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8RangeFltCoef = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:GradRevMax");/*GradRevMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_GRADREVMAX (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8GradRevMax[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8GradRevMax = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:GradRevThr");/*GradRevThr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_GRADREVTHR (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8GradRevThr[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8GradRevThr = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Compress");/*Compress*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_COMPRESS (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8Compress[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8Compress = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Stretch");/*Stretch*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {

		fprintf(fpMak, "#define CFG_DYM_THREADDRC_STRETCH (1)\n");
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U8 au8Stretch[HI_SCENE_DRC_ISO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U8 u8Stretch = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:AutoStrength_0");/*AutoStrength_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
    	fprintf(fpMak, "#define CFG_DYM_THREADDRC_AUTOSTRENGTH (1)\n");
		s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpHeader, "    HI_U16 u16AutoStrength[HI_SCENE_DRC_ISO_MAX_COUNT][HI_SCENE_DRC_RATIO_MAX_COUNT];\n");

        fprintf(fpSoucre, "    HI_U16 u16Strength = 0;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    fprintf(fpSoucre, "    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetPubAttr(ViPipe, &stPubAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");
    fprintf(fpSoucre, "    if (WDR_MODE_NONE == stPubAttr.enWDRMode)\n");
    fprintf(fpSoucre, "    {\n");
    fprintf(fpSoucre, "        return HI_SUCCESS;\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetDRCAttr(ViPipe, &stIspDrcAttr);\n");
    fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RationCnt");/*RationCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RationLevel");/*RationLevel*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
        fprintf(fpSoucre, "    u32RatioLevel = SCENE_GetLevelLtoH_U32(u32ActRation, g_astScenePipeParam[u8Index].stThreadDrc.u32RationCount, g_astScenePipeParam[u8Index].stThreadDrc.au32RationLevel);\n");
        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RationCnt");/*IsoCnt*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RationLevel");/*IsoLevel*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszStringOne);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString && HI_NULL != pszStringOne)
    {
        fprintf(fpSoucre, "    u32IsoLevel = SCENE_GetLevelLtoH_U32(u32Iso, g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount, g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel);\n");
        free(pszString);
        free(pszStringOne);
        pszString = HI_NULL;
        pszStringOne = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingBrightMax");/*LocalMixingBrightMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingBrightMax = g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingBrightMax[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingBrightMax = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingBrightMax[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingBrightMax[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingBrightMin");/*LocalMixingBrightMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingBrightMin = g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingBrightMin[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingBrightMin = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingBrightMin[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingBrightMin[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingDarkMax");/*LocalMixingDarkMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingDarkMax = g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingDarkMax[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingDarkMax = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingDarkMax[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingDarkMax[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingDarkMin");/*LocalMixingDarkMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingDarkMin = g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingDarkMin[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8LocalMixingDarkMin = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingDarkMin[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8LocalMixingDarkMin[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:DetailAdjustFactor");/*DetailAdjustFactor*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        s8DetailAdjustFactor = g_astScenePipeParam[u8Index].stThreadDrc.as8DetailAdjustFactor[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        s8DetailAdjustFactor = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.as8DetailAdjustFactor[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.as8DetailAdjustFactor[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:SpatialFltCoef");/*SpatialFltCoef*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8SpatialFltCoef = g_astScenePipeParam[u8Index].stThreadDrc.au8SpatialFltCoef[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8SpatialFltCoef = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8SpatialFltCoef[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8SpatialFltCoef[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RangeFltCoef");/*RangeFltCoef*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8RangeFltCoef = g_astScenePipeParam[u8Index].stThreadDrc.au8RangeFltCoef[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8RangeFltCoef = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8RangeFltCoef[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8RangeFltCoef[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:GradRevMax");/*GradRevMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8GradRevMax = g_astScenePipeParam[u8Index].stThreadDrc.au8GradRevMax[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8GradRevMax = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8GradRevMax[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8GradRevMax[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:GradRevThr");/*GradRevThr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8GradRevThr = g_astScenePipeParam[u8Index].stThreadDrc.au8GradRevThr[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8GradRevThr = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8GradRevThr[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8GradRevThr[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Compress");/*Compress*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8Compress = g_astScenePipeParam[u8Index].stThreadDrc.au8Compress[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8Compress = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8Compress[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8Compress[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    ISP_WDR_EXPOSURE_ATTR_S stWDRExpAttr;\n");
        fprintf(fpSoucre, "    CHECK_SCENE_PAUSE();\n");
        fprintf(fpSoucre, "    s32Ret = HI_MPI_ISP_GetWDRExposureAttr(ViPipe, &stWDRExpAttr);\n");
        fprintf(fpSoucre, "    CHECK_SCENE_RET(s32Ret);\n\n");
		fprintf(fpSoucre, "    #if 0\n");
        fprintf(fpSoucre, "    if (u32Iso < 400 && u32ActRation > stWDRExpAttr.u32ExpRatioMin)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8Compress = u8Compress + (HI_U8)(stWDRExpAttr.u32ExpRatioMax/ SCENE_DIV_0TO1(u32ActRation) * 4);\n");
        fprintf(fpSoucre, "    }\n");
		fprintf(fpSoucre, "    #endif\n\n");

        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Stretch");/*Stretch*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    if (u32IsoLevel == 0 || u32IsoLevel == g_astScenePipeParam[u8Index].stThreadDrc.u32ISOCount - 1)\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8Stretch = g_astScenePipeParam[u8Index].stThreadDrc.au8Stretch[u32IsoLevel];\n");
        fprintf(fpSoucre, "    }\n");
        fprintf(fpSoucre, "    else\n");
        fprintf(fpSoucre, "    {\n");
        fprintf(fpSoucre, "        u8Stretch = SCENE_Interpulate(u32Iso,g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8Stretch[u32IsoLevel],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au32ISOLevel[u32IsoLevel + 1],\n");
        fprintf(fpSoucre, "                                                   g_astScenePipeParam[u8Index].stThreadDrc.au8Stretch[u32IsoLevel + 1]);\n");
        fprintf(fpSoucre, "    }\n\n");

        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:AutoStrength_0");/*AutoStrength_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    u16Strength = g_astScenePipeParam[u8Index].stThreadDrc.u16AutoStrength[u32IsoLevel][u32RatioLevel];\n");
        free(pszString);
        pszString = HI_NULL;
    }

    fprintf(fpSoucre, "    stIspDrcAttr.u8DetailDarkStep = 0;\n");
    fprintf(fpSoucre, "    //stIspDrcAttr.enCurveSelect = DRC_CURVE_ASYMMETRY;\n");

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Enable");/*Enable*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "    stIspDrcAttr.bEnable = g_astScenePipeParam[u8Index].stThreadDrc.bEnable;\n");
        free(pszString);
        pszString = HI_NULL;
    }

    fprintf(fpSoucre, "    for (i = 0; i < g_astScenePipeParam[u8Index].stThreadDrc.u32Interval; i++)\n");
    fprintf(fpSoucre, "    {\n");



    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingBrightMax");/*LocalMixingBrightMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;

        fprintf(fpSoucre, "        stIspDrcAttr.u8LocalMixingBrightMax = SCENE_TimeFilter(stIspDrcAttr.u8LocalMixingBrightMax,\n");
        fprintf(fpSoucre, "                                               u8LocalMixingBrightMax, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingBrightMin");/*LocalMixingBrightMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.u8LocalMixingBrightMin = SCENE_TimeFilter(stIspDrcAttr.u8LocalMixingBrightMin,\n");
        fprintf(fpSoucre, "                                               u8LocalMixingBrightMin, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingDarkMax");/*LocalMixingDarkMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.u8LocalMixingDarkMax = SCENE_TimeFilter(stIspDrcAttr.u8LocalMixingDarkMax,\n");
        fprintf(fpSoucre, "                                               u8LocalMixingDarkMax, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:LocalMixingDarkMin");/*LocalMixingDarkMin*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.u8LocalMixingDarkMin = SCENE_TimeFilter(stIspDrcAttr.u8LocalMixingDarkMin,\n");
        fprintf(fpSoucre, "                                               u8LocalMixingDarkMin, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:DetailAdjustFactor");/*DetailAdjustFactor*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.s8DetailAdjustFactor = SCENE_TimeFilter(stIspDrcAttr.s8DetailAdjustFactor,\n");
        fprintf(fpSoucre, "                                               s8DetailAdjustFactor, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:SpatialFltCoef");/*SpatialFltCoef*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
       fprintf(fpSoucre, "        stIspDrcAttr.u8SpatialFltCoef = SCENE_TimeFilter(stIspDrcAttr.u8SpatialFltCoef,\n");
        fprintf(fpSoucre, "                                               u8SpatialFltCoef, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:RangeFltCoef");/*RangeFltCoef*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.u8RangeFltCoef = SCENE_TimeFilter(stIspDrcAttr.u8RangeFltCoef,\n");
        fprintf(fpSoucre, "                                               u8RangeFltCoef, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:GradRevMax");/*GradRevMax*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.u8GradRevMax = SCENE_TimeFilter(stIspDrcAttr.u8GradRevMax,\n");
        fprintf(fpSoucre, "                                               u8GradRevMax, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:GradRevThr");/*GradRevThr*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.u8GradRevThr = SCENE_TimeFilter(stIspDrcAttr.u8GradRevThr,\n");
        fprintf(fpSoucre, "                                               u8GradRevThr, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Compress");/*Compress*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.stAsymmetryCurve.u8Compress = SCENE_TimeFilter(stIspDrcAttr.stAsymmetryCurve.u8Compress,\n");
        fprintf(fpSoucre, "                                               u8Compress, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }
    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:Stretch");/*Stretch*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.stAsymmetryCurve.u8Stretch = SCENE_TimeFilter(stIspDrcAttr.stAsymmetryCurve.u8Stretch,\n");
        fprintf(fpSoucre, "                                               u8Stretch, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    snprintf(aszIniNodeName, SCENETOOL_INIPARAM_NODE_NAME_LEN, "thread_drc:AutoStrength_0");/*AutoStrength_0*/
    s32Ret = HI_CONFACCESS_GetString(SCENETOOL_INIPARAM, pszIniModule, aszIniNodeName, NULL, &pszString);
    SCENETOOL_INIPARAM_CHECK_LOAD_RESULT(s32Ret, aszIniNodeName);
    if (HI_NULL != pszString)
    {
        s_stSceneState.bThreadDrc = HI_TRUE;
        fprintf(fpSoucre, "        stIspDrcAttr.stAuto.u16Strength = SCENE_TimeFilter(stIspDrcAttr.stAuto.u16Strength,\n");
        fprintf(fpSoucre, "                                               u16Strength, g_astScenePipeParam[u8Index].stThreadDrc.u32Interval, i);\n");
        free(pszString);
        pszString = HI_NULL;
    }

    fprintf(fpHeader, "    } HI_SCENE_THREAD_DRC_S;\n");

    fprintf(fpSoucre, "        CHECK_SCENE_PAUSE();\n");
    fprintf(fpSoucre, "        s32Ret = HI_MPI_ISP_SetDRCAttr(ViPipe,&stIspDrcAttr);\n");
    fprintf(fpSoucre, "        CHECK_SCENE_RET(s32Ret);\n");
    fprintf(fpSoucre, "        usleep(80000);\n");
    fprintf(fpSoucre, "    }\n");
    fprintf(fpSoucre, "    return HI_SUCCESS;\n}\n");
    return HI_SUCCESS;
}

HI_S32 SCENETOOL_AutoGenerateParam(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR aszIniModuleName[SCENETOOL_INIPARAM_MODULE_NAME_LEN] = {0,};

    snprintf(aszIniModuleName, SCENETOOL_INIPARAM_MODULE_NAME_LEN, "%s", SCENETOOL_INIPARAM_SCENETOOL_COMM);

    s32Ret = SCENETOOL_StaticAE(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticAE failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticAERoute(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticAERoute failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticAWB(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticAWB failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticAWBEX(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticAWBEX failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticAERouteEX(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticAERouteEX failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticGlobalCac(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticGlobalCac failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticWDRExposure(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticWDRExposure failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticDRC(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticDRC failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticLDCI(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticLDCI failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticDehaze(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticDehaze failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticThreeDNR(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticThreeDNR failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticStatistics(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticStatistics failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticSaturation(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticSaturation failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticCCM(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticCCM failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticCSC(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticCSC failed\n");
        return HI_FAILURE;
    }

    #if 0
    s32Ret = SCENETOOL_StaticSharpen(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticSharpen failed\n");
        return HI_FAILURE;
    }
    #endif

    s32Ret = SCENETOOL_StaticNr(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticNr failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_StaticShading(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_StaticShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicGamma(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicGamma failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicAE(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicAE failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicShading(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicShading failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicLDCI(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicLDCI failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicDehaze(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicDehaze failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicFsWdr(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicFsWdr failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicHdrOETF(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicHdrOETF failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicHdrTM(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicHdrTM failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicHdrDRC(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicHdrDRC failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicFalseColor(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicFalseColor failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicDemosaic(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicDemosaic failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_DynamicDIS(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_DynamicDIS failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_ThreadDRC(aszIniModuleName);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_ThreadDRC failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 SCENETOOL_CreateFile(HI_U32 u32Count, HI_CHAR* pcPath)
{
    HI_S32 s32Ret = HI_SUCCESS;


    HI_CHAR acSrcpath[SCENETOOL_MAX_FILESIZE]= {0};
    HI_CHAR acHeadpath[SCENETOOL_MAX_FILESIZE]= {0};
    HI_CHAR acMakpath[SCENETOOL_MAX_FILESIZE]= {0};

    snprintf(acHeadpath, SCENETOOL_MAX_FILESIZE, "%s/%s", pcPath, "hi_scene_autogenerate.h");
    printf("The acHeadpath is %s.\n",acHeadpath);

    snprintf(acSrcpath, SCENETOOL_MAX_FILESIZE, "%s/%s", pcPath, "hi_scene_autogenerate.c");
    printf("The acSrcpath is %s.\n",acSrcpath);

    snprintf(acMakpath, SCENETOOL_MAX_FILESIZE, "%s/%s", pcPath, "hi_scene_paramtype_autogenerate.h");
    printf("The acMakpath is %s.\n",acMakpath);



    fpHeader = fopen(acHeadpath, "w+b");
    if(!fpHeader)
    {
        MLOGE("open [%s] failed\n", acHeadpath);
        return HI_FAILURE;
    }

    fpSoucre = fopen(acSrcpath, "w+b");
    if(!fpSoucre)
    {
        MLOGE("open [%s] failed\n", acSrcpath);
        return HI_FAILURE;
    }

	fpMak = fopen(acMakpath, "w+b");
    if(!fpSoucre)
    {
        MLOGE("open [%s] failed\n", acMakpath);
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_WriteHeaderFileBegin(u32Count);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_WriteHeaderFileBegin failed\n");
        return HI_FAILURE;
    }


    s32Ret = SCENETOOL_WriteSourceFileBegin();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_WriteSourceFileBegin failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_AutoGenerateParam();

    if(HI_SUCCESS != s32Ret)
        {
            MLOGE("SCENETOOL_AutoGenerateParam failed\n");
            return HI_FAILURE;

    }

    s32Ret = SCENETOOL_WriteHeaderFileEnd();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_WriteHeaderFileEnd failed\n");
        return HI_FAILURE;
    }

    s32Ret = SCENETOOL_WriteSourceFileEnd();
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("SCENETOOL_WriteSourceFileEnd failed\n");
        return HI_FAILURE;
    }

    fclose(fpHeader);
    fpHeader = HI_NULL;

    fclose(fpSoucre);
    fpSoucre = HI_NULL;

	fclose(fpMak);
    fpMak = HI_NULL;

    return HI_SUCCESS;
}



HI_S32 main(int argc, char **argv)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ModuleNum = 0;
	HI_CHAR * inipath = NULL;
    HI_CHAR * srcpath = NULL;
    HI_CHAR acInipath[SCENETOOL_MAX_FILESIZE]= {0};

	if(argc < 3)
	{
		printf("Usage : ./exe inidir sourcedir\n");
		return 0;
	}

    inipath = argv[1];
    srcpath = argv[2];
    if (strlen(inipath) >= SCENETOOL_MAX_FILESIZE ||
        strlen(srcpath) >= SCENETOOL_MAX_FILESIZE )
    {
        printf("The path is too long.\n");
        return 0;
    }

    snprintf(acInipath, SCENETOOL_MAX_FILESIZE, "%s/%s", inipath, "config_cfgaccess_hd.ini");
    printf("The iniPath is %s.\n",acInipath);

    /* Load Product Ini Configure */
    s32Ret = HI_CONFACCESS_Init(SCENETOOL_INIPARAM, acInipath, &u32ModuleNum);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("load ini [%s] failed [%08x]\n", inipath, s32Ret);
        return HI_FAILURE;
    }
	inipath = NULL;

    s32Ret = SCENETOOL_CreateFile(u32ModuleNum, srcpath);
    if(HI_SUCCESS != s32Ret)
    {
        MLOGE("Create File failed!\n");
        return HI_FAILURE;
    }
    srcpath = NULL;

    MLOGD("file create end\n");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

