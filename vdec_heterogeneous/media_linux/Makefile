ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../../Makefile.param
     include $(PARAM_FILE)
endif

SRCS := $(wildcard *.c)
SRCS += $(wildcard ./common/*.c)
SRCS += $(wildcard ../media_msg/mediaclient/*.c)

TARGET := sample_vdec
INC_FLAGS := -I$(REL_INC)
INC_FLAGS += -I$(SDK_PATH)/hisyslink/ipcmsg/include
INC_FLAGS += -I$(SDK_PATH)/hisyslink/datafifo/include
INC_FLAGS += -I../media_msg/include
INC_FLAGS += -I./common


MPI_LIBS := $(wildcard $(REL_LIB)/*.a)
MPI_LIBS += $(SDK_PATH)/hisyslink/ipcmsg/out/linux/big-little/libipcmsg_big-little.a
MPI_LIBS += $(SDK_PATH)/hisyslink/datafifo/out/linux/big-little/libdatafifo_big-little.a

OBJS  := $(SRCS:%.c=%.o)
CFLAGS += $(INC_FLAGS)
CFLAGS += -lstdc++

.PHONY : clean all

all: $(TARGET)
	cp -f $(TARGET) ../

$(TARGET):$(OBJS)
	@$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ -Wl,--start-group $(MPI_LIBS) -Wl,--end-group


clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
	@rm -f ../$(TARGET)