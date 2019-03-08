# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c)

TARGET := $(SRCS:%.c=%)

SRCS += $(wildcard $(PWD)/adp/*.c)

# compile linux or HuaweiLite
include $(PWD)/../$(OSTYPE).mak
