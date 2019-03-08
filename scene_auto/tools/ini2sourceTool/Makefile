include $(PARAM_FILE)
# Compiler flags to enable all warnings & debug info
CC 	= gcc
INICFG_PATH := ./inicfg
SCENEMODE_CNT = $(shell grep -nr "config_product_scene_*" $(INICFG_PATH)/config_cfgaccess_hd.ini |wc -l)

CFLAGS = -Wall -g  -DHI_PARAM_SCENEMODE_CNT=$(SCENEMODE_CNT) -O2 # -static #-Werror
# -std=c++0x

INCLUDE = -I./include
INCLUDE += -I./../../include
INCLUDE += -I./../configaccess/include
INCLUDE += -I./../iniparser/include
INCLUDE += -I./../../../../../out/hi3559av100/linux/multi-core/include

LIBS = -lpthread -lrt
#LIB4G = ./lib/libfile-avcodec.a

# C source code files that are required

CSRC = ./src/hi_scenetool_initosource.c
CSRC += ./../configaccess/src/hi_confaccess.c
CSRC += ./../iniparser/src/dictionary.c
CSRC += ./../iniparser/src/iniparser.c

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@


TARGETS = ini2source

all: ${TARGETS}


$(TARGETS): ${CSRC}
	$(CC) ${CFLAGS} ${INCLUDE} -o $@ $^ ${LIBS}
	mv ${TARGETS} ../../

clean:
	@rm -f *.o *~ ${TARGETS}
