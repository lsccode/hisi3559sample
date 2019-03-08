1. update the file: mpp/out/linux/single/ko/load3559av100es
   It need to insmod tlv_320aic31's driver.
    insmod extdrv/hi_tlv320aic31.ko

2. modify the makefile parameter: mpp/sample/Makefile.param. Set ACODEC_TYPE to  ACODEC_TYPE_TLV320AIC31.
   It means use the external codec tlv_320aic31 sample code.
    ################ select audio codec type for your sample ################
    #ACODEC_TYPE ?= ACODEC_TYPE_INNER
    #external acodec
    ACODEC_TYPE ?= ACODEC_TYPE_TLV320AIC31

3. Rebuild the sample and get the sample_audio.
