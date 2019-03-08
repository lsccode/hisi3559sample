#===============================================================================
# rules
#===============================================================================
.PHONY: all clean

include ../Makefile.param

COMMON_SRC := ../es20_common/es20_common.c $(COMM_SRC)

INC_DIR := $(CFLAGS) -I$(SDK_PATH)/mpp/component/gpu/release/include \
			-I../es20_common/

LIB_DIR := -L$(REL_LIB) -L$(SDK_PATH)/mpp/component/gpu/release/lib

MACRO := -DEGL_EGLEXT_PROTOTYPES -DGL_GLEXT_PROTOTYPES \
         -DEGL_FBDEV -DEGL_API_FBDEV -DEGL_API_MIDGARD -DPLATFORM_MALI700

LIBS += $(CFLAGS) $(MPI_LIBS) $(SENSOR_LIBS) -lmali -lstdc++ -ldnvqe -lupvqe -lVoiceEngine -lsecurec

export  LIB_NAME COMMON_SRC INC_DIR LIB_DIR MACRO LIBS
all:
	$(AT)make -C es20_eglCreateWindowSurface
	$(AT)make -C es20_eglCreatePixmapSurface
	$(AT)make -C es20_pano2view

clean:
	$(AT)make -C es20_eglCreateWindowSurface clean
	$(AT)make -C es20_eglCreatePixmapSurface clean
	$(AT)make -C es20_pano2view clean


