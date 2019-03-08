# Hisilicon mpp sample Makefile
include ../Makefile.param

SRCS := $(wildcard *.c) 
TARGET := $(SRCS:%.c=%)

MPI_LIBS += $(REL_LIB)/libtde.a

# compile linux or HuaweiLite
include $(PWD)/../$(OSTYPE).mak
