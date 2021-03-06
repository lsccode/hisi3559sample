
#include "hi_defines.h"

#define SAMPLE_SEND_MSG_TIMEOUT 900000


typedef enum hiSAMPLE_MOD_ID_E
{
    SAMPLE_MOD_PHOTO = 0,
    SAMPLE_MOD_VB,
    SAMPLE_MOD_SNAP,
    SAMPLE_MOD_VPSS,
    SAMPLE_MOD_VENC,
    SAMPLE_MOD_VDEC,

    SAMPLE_MOD_BUTT,
} SAMPLE_MOD_ID_E;

typedef enum hiSAMPLE_PHOTO_CMD_E
{
    SAMPLE_PHOTO_CMD_START_VIDEO = 0,
    SAMPLE_PHOTO_CMD_STOP_VIDEO,

    SAMPLE_PHOTO_CMD_BUTT,
} SAMPLE_PHOTO_CMD_E;

typedef enum hiSAMPLE_VB_CMD_E
{
    SAMPLE_VB_CMD_GET_BLOCK = 0,
    SAMPLE_VB_CMD_RELEASE_BLOCK,
    SAMPLE_VB_CMD_HANDLE2PHYSADDR,

    SAMPLE_VB_CMD_BUTT,
} SAMPLE_VB_CMD_E;

typedef enum hiSAMPLE_SNAP_CMD_E
{
    SAMPLE_SNAP_CMD_TRIGGER = 0,
    SAMPLE_SNAP_CMD_SET_ATTR,
    SAMPLE_SNAP_CMD_ENABLE,
    SAMPLE_SNAP_CMD_DISABLE,
    SAMPLE_SNAP_CMD_GET_BNRRAW,
    SAMPLE_SNAP_CMD_RELEASE_BNRRAW,

    SAMPLE_SNAP_CMD_BUTT,
} SAMPLE_SNAP_CMD_E;

typedef enum hiSAMPLE_VPSS_CMD_E
{
    SAMPLE_VPSS_CMD_GET_CHN_FRAME = 0,
    SAMPLE_VPSS_CMD_RELEASE_CHN_FRAME,

    SAMPLE_VPSS_CMD_BUTT,
} SAMPLE_VPSS_CMD_E;

typedef enum hiSAMPLE_VENC_CMD_E
{
    SAMPLE_VENC_CMD_SEND_FRAME = 0,
    SAMPLE_VENC_CMD_GET_STREAM,
    SAMPLE_VENC_CMD_RELEASE_STREAM,
    SAMPLE_VENC_CMD_GET_STREAM_BUFINFO,
    SAMPLE_VENC_CMD_GET_JPEG_PACKS,
    SAMPLE_VENC_CMD_BUTT,
} SAMPLE_VENC_CMD_E;

typedef enum hiSAMPLE_VDEC_CMD_E
{
    SAMPLE_VDEC_CMD_SEND_DATAFIFO_INFO,
    SAMPLE_VDEC_CMD_DATAFIFO_INIT,
    SAMPLE_VDEC_CMD_DATAFIFO_DEINIT,
    SAMPLE_VDEC_CMD_START_VIDEO,
    SAMPLE_VDEC_CMD_STOP_VIDEO,
    SAMPLE_VDEC_CMD_BUTT,
} SAMPLE_VDEC_CMD_E;



