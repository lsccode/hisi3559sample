# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c)
SRCS += $(wildcard ./sample/*.c)
SRCS += $(wildcard ./common/*.c)

CFLAGS += -I$(PWD)/common/
CFLAGS += -I$(PWD)/sample/
TARGET := sample_dpu_main

# compile linux or HuaweiLite
include $(PWD)/../$(OSTYPE).mak
