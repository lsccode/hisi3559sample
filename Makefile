
all: linux lite linuxclean liteclean

linux:
	cp ../cfg.mak.multicore ../cfg.mak
	@cd vio;     make
	@cd vdec;    make
	@cd venc;    make
	@cd avs;     make
	@cd fisheye; make
	@cd dis;     make
	@cd hifb;    make
	@cd audio;   make
	@cd snap;    make
	@cd vgs;     make
	@cd tde;     make
	@cd mode_switch; make
	@cd scene_auto;  make
	@cd dpu;     make
	@cd svp/multi-core; make
	@cd gpu;     make
	@cd lsc_online_cali; make
	@cd awb_online_calibration; make
linuxclean:
	cp ../cfg.mak.multicore ../cfg.mak
	@cd vio;     make clean
	@cd vdec;    make clean
	@cd venc;    make clean
	@cd avs;     make clean
	@cd fisheye; make clean
	@cd dis;     make clean
	@cd hifb;    make clean
	@cd audio;   make clean
	@cd snap;    make clean
	@cd vgs;     make clean
	@cd tde;     make clean
	@cd mode_switch; make clean
	@cd scene_auto;  make clean
	@cd dpu;     make clean
	@cd svp/multi-core; make clean
	@cd gpu;     make clean
	@cd lsc_online_cali; make clean
	@cd awb_online_calibration; make clean

lite:
	cp ../cfg.mak.single ../cfg.mak
	@cd vio;     make
	@cd vdec;    make
	@cd venc;    make
	@cd avs;     make
	@cd fisheye; make
	@cd dis;     make
	@cd audio;   make
	@cd snap;    make
	@cd vgs;     make
	@cd dpu;     make
	@cd svp/single; make
	@cd mode_switch; make
	@cd lsc_online_cali; make
	@cd awb_online_calibration; make
	@cd svp/big-little; make
liteclean:
	cp ../cfg.mak.single ../cfg.mak
	@cd vio;     make clean
	@cd vdec;    make clean
	@cd venc;    make clean
	@cd avs;     make clean
	@cd fisheye; make clean
	@cd dis;     make clean
	@cd audio;   make clean
	@cd snap;    make clean
	@cd vgs;     make clean
	@cd dpu;     make clean
	@cd svp/single; make clean
	@cd mode_switch; make clean
	@cd lsc_online_cali; make clean
	@cd awb_online_calibration; make clean
	@cd svp/big-little; make clean

