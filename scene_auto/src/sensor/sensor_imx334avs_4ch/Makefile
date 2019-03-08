# Hisilicon Hi35xx sample Makefile

include ../../../../Makefile.param

CC   := $(CROSS)gcc
AR   := $(CROSS)ar

INC_FLAGS :=-I./../../../include
INC_FLAGS +=-I./../scene
INC_FLAGS +=-I./../../../tools/configaccess/include
INC_FLAGS +=-I./../../../tools/iniparser/include
INC_FLAGS +=-I./../../../../common
INC_FLAGS +=-I$(REL_DIR)/include
INC_FLAGS +=-I./src
INC_FLAGS +=-I./../../core

SRC := $(wildcard ./../../../tools/configaccess/src/*.c)
SRC += $(wildcard ./../../../tools/iniparser/src/*.c)
SRC += $(wildcard ./src/*.c)
SRC += ./../../core/hi_scene.c  
SRC += ./../../sample/scene_sample.c
SRC += ./../../sample/scene_loadparam.c

CFLAGS := -Wall -c -g $(INC_FLAGS) -D$(HIARCH) -fPIC
LD_CFLAGS += $(LIBS_LD_CFLAGS)

# target source
OBJ := $(SRC:%.c=%.o)

#LIBA := libsample_scene.a
LIBS := sample_scene
LIBDIR := $(shell basename $(shell pwd))

.PHONY : clean $(TARGET) all

#all: $(LIBS) $(LIBA)
$(LIBS): $(OBJ) $(SRC)
	@echo -e "\e[0;32;1m--Compiling 'libscene_sensor'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), DBG=$(HIGDB)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@@echo -e "\e[0m"
	
	#@$(CC) $(LD_CFLAGS) -shared -o $(LIBS) $(OBJ) > /dev/null $(MPI_LIBS) $(SENSOR_LIBS) $(AUDIO_LIBA) $(REL_LIB)/libsecurec.a 
	@$(CC) $(LD_CFLAGS) -lpthread -lm -o $(LIBS) $(OBJ) > /dev/null $(MPI_LIBS) $(SENSOR_LIBS) $(AUDIO_LIBA) $(REL_LIB)/libsecurec.a
	mv ${LIBS} $(shell pwd)/../../../out/sensor_bin/$(LIBDIR)

#$(LIBA): $(OBJ) 
#	@$(AR) -rsv $(LIBA)  $(OBJ) > /dev/null 
#	mv ${LIBA} $(shell pwd)/../../../out/sensor_lib/$(LIBDIR)
	
$(OBJ): %.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)
	@rm -f ./../../../out/sensor_bin/$(LIBDIR)/*
