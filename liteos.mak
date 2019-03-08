SDK_LIB_PATH := -L$(REL_LIB) -L$(REL_LIB)/extdrv

#SDK_LIB_PATH += -L$(SAMPLE_DIR)/../../tools/hi3559av100/

SDK_LIB := $(SDK_LIB_PATH) --start-group -lhi_osal -lmpi -ldpu_rect -ldpu_match -l$(HIARCH)_base -l$(HIARCH)_sys -lhi_user -lhdmi -l$(HIARCH)_isp -l$(HIARCH)_vi \
							-l$(HIARCH)_vo -l$(HIARCH)_vpss -l$(HIARCH)_vgs -l$(HIARCH)_gdc -lhi_mipi -lhi_mipi_tx \
							-l$(HIARCH)_chnl -l$(HIARCH)_rc -l$(HIARCH)_rgn -l$(HIARCH)_vedu \
							-l$(HIARCH)_venc -l$(HIARCH)_h265e -l$(HIARCH)_jpege -l$(HIARCH)_h264e -l$(HIARCH)_dpu_rect -l$(HIARCH)_dpu_match\
							-l$(HIARCH)_dis  -l$(HIARCH)_hdmi -lhifisheyecalibrate  -lhiavslut\
							-l_hidehaze -l_hidrc -l_hildci -l_hiawb -l_hiae -lisp  -lhi_sensor_i2c\
							-laacdec -laacenc -lupvqe -ldnvqe -lVoiceEngine -l$(HIARCH)_ai -l$(HIARCH)_ao -l$(HIARCH)_aio -l$(HIARCH)_aenc -l$(HIARCH)_adec -l$(HIARCH)_acodec \
							-l$(HIARCH)_avs -lhi_sil9022 \
							-l$(HIARCH)_vdec -l$(HIARCH)_jpegd  -l$(HIARCH)_vfmw \
							-lhi_ssp_sony -lhi_sensor_spi -lhi_pwm \
					#		-ltools \
							--end-group
							
SDK_LIB += $(SENSOR_LIBS)
SDK_LIB += $(wildcard $(SDK_PATH)/hisyslink/ipcmsg/out/liteos/single/*.a)

LITEOS_LIBDEPS = --start-group $(LITEOS_LIBDEP) --end-group $(LITEOS_TABLES_LDFLAGS)

LDFLAGS := $(LITEOS_LDFLAGS) --gc-sections

# target source
SRCS  += $(SAMPLE_DIR)/$(OSTYPE)/app_init.c
SRCS  += $(REL_DIR)/init/sdk_init.c
SRCS  += $(REL_DIR)/init/sdk_exit.c
OBJS  := $(SRCS:%.c=%.o)
OBJS += $(COMM_OBJ)


BIN := $(TARGET).bin
MAP := $(TARGET).map

.PHONY : clean all

all: $(BIN)

$(BIN):$(TARGET)
	@$(OBJCOPY) -O binary $(TARGET) $(BIN)

$(TARGET):$(OBJS)
	@$(LD) $(LDFLAGS) -Map=$(MAP) -o $(TARGET) $(OBJS) $(SDK_LIB) $(LITEOS_LIBDEPS) $(REL_LIB)/libsecurec.a
	@$(OBJDUMP) -d $(TARGET) > $(TARGET).asm

$(OBJS):%.o:%.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGET) $(BIN) $(MAP) *.asm
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
