#ifndef __SAMPLE_MEDIA_CLIENT_H__
#define __SAMPLE_MEDIA_CLIENT_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/******************************************************************************
* function : IPCMG Init
******************************************************************************/
HI_S32 SAMPLE_Media_MSG_Init(void);

/******************************************************************************
* function : IPCMG Deinit
******************************************************************************/
HI_VOID SAMPLE_Media_MSG_DeInit(void);

/******************************************************************************
* function : IPCMG Get connect Id
******************************************************************************/
HI_S32 SAMPLE_Media_MSG_GetSiId(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SAMPLE_MSG_CLIENT_H__ */
