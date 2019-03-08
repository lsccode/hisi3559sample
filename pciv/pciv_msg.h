
#ifndef __KERNEL__
#include <sys/time.h>
#endif

#include "hi_common.h"
#include "hi_type.h"
#include "hi_comm_pciv.h"


#ifndef __PCIV_MSG_H__
#define __PCIV_MSG_H__


#define PCIV_MSG_BASE_PORT          100     /* we use msg port above this value */
//#define PCIV_MSG_MAX_PORT_NUM       260     /* max msg port count, also you can change it */
#define PCIV_MSG_MAX_PORT_NUM       262     /* max msg port count, also you can change it */
#define PCIV_MSG_MAX_PORT           ((PCIV_MSG_BASE_PORT)+(PCIV_MSG_MAX_PORT_NUM))

#define PCIV_MSGPORT_COMM_CMD       PCIV_MSG_BASE_PORT  /* common msg port, used for general command */

#define SAMPLE_PCIV_MSG_HEADLEN     sizeof(SAMPLE_PCIV_MSGHEAD_S)
#define SAMPLE_PCIV_MSG_MAXLEN      1036

typedef struct hiSAMPLE_PCIV_MSGHEAD_S
{
    HI_U32                  u32Target;  /* The target PCI Chip ID. follow PCIV definition */
    HI_U32                  u32MsgType; /* Message type that you defined */
    HI_U32                  u32MsgLen;  /* Length of message BODY (exclude Header) */
    HI_S32                  s32RetVal;  /* Return value from target chip */
} SAMPLE_PCIV_MSGHEAD_S;

typedef struct hiSAMPLE_PCIV_MSG_S
{
    SAMPLE_PCIV_MSGHEAD_S   stMsgHead;
    HI_U8                   cMsgBody[SAMPLE_PCIV_MSG_MAXLEN];
} SAMPLE_PCIV_MSG_S;

HI_S32 PCIV_WaitConnect(HI_S32 s32Tgt);

HI_S32 PCIV_AllocMsgPort(HI_S32 *ps32MsgPort);
HI_S32 PCIV_OpenMsgPort(HI_S32 s32TgtId, HI_S32 s32Port);
HI_S32 PCIV_CloseMsgPort(HI_S32 s32TgtId, HI_S32 s32Port);
HI_S32 PCIV_SendMsg(HI_S32 s32TgtId, HI_S32 s32Port, SAMPLE_PCIV_MSG_S *pMsg);
HI_S32 PCIV_ReadMsg(HI_S32 s32TgtId, HI_S32 s32Port, SAMPLE_PCIV_MSG_S *pMsg);


#endif

