
include ../Makefile.param

LIBS +=$(REL_LIB)/libpciv.a $(REL_LIB)/libmpi.a $(REL_LIB)/libhdmi.a $(REL_LIB)/libsecurec.a

# target source
SRC  := $(wildcard *.c)  
OBJ  := $(SRC:%.c=%.o)

TARGET := $(OBJ:%.o=%)
SYS_INC := -I$(MPP_PATH)/component/pci/include
#CFLAGS += -Wall -O2
#CFLAGS += -lpthread -lm 

host:
	$(CC) -o sample_pciv_host sample_pciv_host.c pciv_msg.c pciv_trans.c $(COMMON_DIR)/*.c $(INC_FLAGS) $(CFLAGS) $(LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS) $(SYS_INC)


slave:
	$(CC) -o sample_pciv_slave sample_pciv_slave.c pciv_msg.c pciv_trans.c $(COMMON_DIR)/*.c $(INC_FLAGS) $(CFLAGS) $(LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS) $(SYS_INC)

clean:
	@rm sample_pciv_host sample_pciv_slave *.o -fr

cleanstream:
	@rm -f *.h264
	@rm -f *.h265
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4

