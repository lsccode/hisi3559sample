#!/bin/sh

SDK_PATH=../../..
OUT_PATH=$SDK_PATH/mpp/out

DSP_BIN_PATH=$OUT_PATH/liteos/dsp/dsp0/runtime/bin


function echo_yellow()
{
    echo -e "\033[33m $1 \033[0m"
}

function echo_green()
{
    echo -e "\033[32m $1 \033[0m"
}

function echo_red()
{
    echo -e "\033[31m $1 \033[0m"
}

function echo_blue()
{
    echo -e "\033[34m $1 \033[0m"
}

function report_error()
{
    echo_red "******* Error!! Shell exit for error *****"
    exit 1
}


build()
{
	#build liteos
	echo_green "Start build liteos ..."
	cp $SDK_PATH/mpp/cfg.mak.single $SDK_PATH/mpp/cfg.mak
	pushd media_liteos
	make || report_error
	popd

	#build big.LITTLE linux
	echo_green "Start build big.LITTLE linux ..."
	cp $SDK_PATH/mpp/cfg.mak.biglittle $SDK_PATH/mpp/cfg.mak
	cp $OUT_PATH/liteos/single/include/hi_comm_venc.h ./media_msg/include/
	cp $OUT_PATH/liteos/single/include/hi_comm_rc.h ./media_msg/include/
	cp $OUT_PATH/liteos/single/include/hi_comm_snap.h ./media_msg/include/
	cp $OUT_PATH/liteos/single/include/hi_comm_isp.h ./media_msg/include/
	cp $OUT_PATH/liteos/single/include/hi_isp_debug.h ./media_msg/include/
	cp $OUT_PATH/liteos/single/include/hi_isp_defines.h ./media_msg/include/
	pushd media_linux
	make || report_error
	popd
	rm ./media_msg/include/hi_*.h
	
	#create dsp bin
	echo_green "Start create dsp bin ..."
	mkdir -p ./dsp_bin/dsp0
	cp $DSP_BIN_PATH/* ./dsp_bin/dsp0
}

buildclean()
{
	#clean liteos
	echo_green "Start clean liteos ..."
	cp $SDK_PATH/mpp/cfg.mak.single $SDK_PATH/mpp/cfg.mak
	pushd media_liteos
	make clean || report_error
	popd
	
	#clean linux
	echo_green "Start clean linux ..."
	cp $SDK_PATH/mpp/cfg.mak.biglittle $SDK_PATH/mpp/cfg.mak
	pushd media_linux
	make clean || report_error
	popd
	
	#clean dsp bin
	echo_green "Start clean dsp bin ..."
	rm -fr ./dsp_bin/dsp0
}

usage()
{
	echo "Usage:  ./build.sh [-option]"
	echo "options:"
	echo "    -m                       make photo sample"
	echo "    -c                       make clean photo sample"
	echo "    -a                       make clean first, then make sample"
	echo "    -h                       help information"
	echo -e "for example: ./build.sh -a\n"
}

######################parse arg###################################
b_build=0
b_clean_build=0

for arg in $@
do
	case $arg in
		"-m")
			b_build=1;
			;;
		"-c")
			b_clean_build=1;
			;;
		"-a")			
			b_build=1;
			b_clean_build=1;
			;;
		"-h")
			usage;
			;;

	esac
done
#######################parse arg end########################

#######################Action###############################

if [ $# -lt 1 ]; then
    usage;
    exit 0;
fi


if [ $b_clean_build -eq 1 ]; then
	buildclean;
fi

if [ $b_build -eq 1 ]; then
	build;
fi

