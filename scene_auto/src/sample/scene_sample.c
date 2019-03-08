#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <limits.h>

#include "hi_type.h"
#include "hi_scene.h"
#include "hi_scene_loadparam.h"
#include "hi_scenecomm_log.h"
#include "sample_comm.h"

HI_VOID SAMPLE_SCENE_HandleSig(HI_S32 signo)
{
    HI_S32 s32ret;

    if (SIGINT == signo || SIGTERM == signo)
    {
        s32ret = HI_SCENE_Deinit();
        if (s32ret != 0)
        {
            printf("HI_SCENE_DeInit failed\n");
            exit(-1);
        }
    }
    exit(-1);
}

HI_S32 SAMPLE_SCENE_GetPathName(HI_CHAR* processdir, HI_CHAR* processname)
{
	HI_CHAR* path_end;

	path_end = strrchr(processdir, '/');
	if(NULL == path_end)
	{
		printf("processdir error\n");
		return HI_FAILURE;
	}
	++path_end;

	strcpy(processname, path_end);
	*path_end = '\0';
		
	return HI_SUCCESS;
}

#ifdef __HuaweiLite__
HI_S32 app_main()
#else
HI_S32 main(HI_S32 argc, HI_CHAR* argv[])
#endif
//HI_S32 sample_exe(HI_S32 s32SensorType)
{
    HI_S32 s32ret = HI_SUCCESS;
	HI_S32 s32choice = -1;
	HI_CHAR aszinput[10];
	HI_CHAR aszpath[1024];
	HI_CHAR aszdirname[128];
    HI_SCENE_PARAM_S stSceneParam;
    HI_SCENE_VIDEO_MODE_S stVideoMode;

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_SCENE_HandleSig);
    signal(SIGTERM, SAMPLE_SCENE_HandleSig);
#else

#endif

	getcwd(aszpath, sizeof(aszpath));
	SAMPLE_SCENE_GetPathName(aszpath, aszdirname);
	printf("dirname is %s \n", aszdirname);

    printf("init success\n");

    while (1)
    {
        printf("1.scene start\n");
        printf("2.scene pause and reset media route\n");
        printf("3.scene resume and set a new media type\n");
		printf("4.exit the sample\n");

        s32choice = -1;
		fgets(aszinput, 10, stdin);
		sscanf(aszinput, "%d", &s32choice);

        switch (s32choice)
        {
            case 1:
                printf("When We Start SceneAuto, we neen to set video Pipe mode\n");
                printf("Please input videomode count, without which we couldn't work  effectively.\n");
                s32choice = -1;
            	fgets(aszinput, 10, stdin);
            	sscanf(aszinput, "%d", &s32choice);
                printf("videomode type has already been input.\n");
				s32ret = HI_SCENE_CreateParam(aszdirname, &stSceneParam, &stVideoMode);
                if (HI_SUCCESS != s32ret)
                {
                    printf("SCENETOOL_CreateParam failed\n");
                    return HI_FAILURE;
                }
                s32ret = HI_SCENE_Init(&stSceneParam);
                if (HI_SUCCESS != s32ret)
                {
                    printf("HI_SCENE_Init failed\n");
                    return HI_FAILURE;
                }

                s32ret = HI_SCENE_SetSceneMode(&(stVideoMode.astVideoMode[s32choice]));
                if (HI_SUCCESS != s32ret)
                {
                    printf("HI_SRDK_SCENEAUTO_Start failed\n");
                    return HI_FAILURE;
                }
                MLOGD("The sceneauto is started.");
                break;
            case 2:
                s32ret = HI_SCENE_Pause(HI_TRUE);
                if (HI_SUCCESS != s32ret)
                {
                    printf("HI_SCENE_Pause failed\n");
                    return HI_FAILURE;
                }
                MLOGD("The sceneauto is pause.");
                break;
            case 3:

                s32choice = -1;
            	fgets(aszinput, 10, stdin);
            	sscanf(aszinput, "%d", &s32choice);
                printf("videomode type has already been input.\n");

                s32ret = HI_SCENE_SetSceneMode(&(stVideoMode.astVideoMode[s32choice]));
                if (HI_SUCCESS != s32ret)
                {
                    printf("HI_SRDK_SCENEAUTO_Start failed\n");
                    return HI_FAILURE;
                }

                s32ret = HI_SCENE_Pause(HI_FALSE);
                if (HI_SUCCESS != s32ret)
                {
                    printf("HI_SCENE_Resume failed\n");
                    return HI_FAILURE;
                }
                MLOGD("The sceneauto is resume.");
                break;
            case 4:
                s32ret = HI_SCENE_Deinit();
                if (HI_SUCCESS != s32ret)
                {
                    printf("HI_SCENE_Deinit failed\n");
                    return HI_FAILURE;
                }
                MLOGD("The scene sample is end.");
                exit(-1);
            default:
                printf("unknown input\n");
                break;
        }

        sleep(1);
    }
    return HI_SUCCESS;
}
