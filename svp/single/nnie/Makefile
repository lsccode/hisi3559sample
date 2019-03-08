# Hisilicon Hi35xx sample Makefile
include $(PWD)/../Makefile.param

CFLAGS += -I$(PWD)/sample
CFLAGS += -O3

SRCS += $(wildcard *.c)
SRCS += $(wildcard ./sample/*.c)
TARGET := sample_nnie_main

# compile linux or HuaweiLite
include $(PWD)/../../../$(OSTYPE).mak
