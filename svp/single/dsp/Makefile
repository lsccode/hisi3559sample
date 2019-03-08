# Hisilicon Hi35xx sample Makefile
include ../Makefile.param

CFLAGS += -I$(PWD)/sample

SRCS += $(wildcard *.c)
SRCS += $(wildcard ./sample/*.c)
TARGET := sample_dsp_main

# compile linux or HuaweiLite
include $(PWD)/../../../$(OSTYPE).mak
