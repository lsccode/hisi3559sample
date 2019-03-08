# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

.PHONY: lib_clean

#TARGET := $(SRCS:%.c=%)
SUBDIRS := ./src/sensor/sensor*

#TARGET := sample_scene

all:	
	for SENSOR in $(SUBDIRS);do\
		make -C $$SENSOR;done

clean: lib_clean

lib_clean:
	for SENSOR in $(SUBDIRS);do\
		make clean -C $$SENSOR;done

# compile linux or HuaweiLite
include $(PWD)/../$(OSTYPE).mak
