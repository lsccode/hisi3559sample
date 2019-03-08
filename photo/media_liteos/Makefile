# Hisilicon Hi35xx sample Makefile
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../../Makefile.param
     include $(PARAM_FILE)
endif

COMMON_DIR = $(PWD)/../../common
AUDIO_ADP_DIR = $(PWD)/../../audio/adp

include ../../Makefile.param

SRCS := $(wildcard *.c)
SRCS += $(wildcard ../media_msg/mediaserver/*.c)

TARGET := sample_photo

INC_FLAGS := -I$(REL_INC)
INC_FLAGS += -I$(SDK_PATH)/hisyslink/ipcmsg/include
INC_FLAGS += -I../media_msg/include
CFLAGS += $(INC_FLAGS)

CFLAGS += -Wno-strict-prototypes -Wno-unused-function -Wno-unused-variable

# compile linux or HuaweiLite
include $(PWD)/../../liteos.mak

