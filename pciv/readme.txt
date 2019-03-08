
编译主从片小系统：
到SDK发布包路径下的osdrv目录，查看read me按照上面所描述的方法，编译主从片的小系统。到osdrv/component/pcie_mcc/目录，按照说明文档上的方法编译pcie模块。注意，编译主片小系统后才能编译主片pcie模块，
编译从片小系统后才能编译从片pcie模块。然后将编译出的pcie的ko拷贝到mpp/out/linux/multi-core/ko/pcie目录下。此方法用于编译主从片单独启动的小系统。从启动时请按照说明文档编译从启动小系统。
小系统编译好之后，按照烧写系统方法烧写小系统。

编译pci 模块
1、到SDK发布包下的mpp/component/pciv路径下。输入make命令编译pciv模块。
   在有内核源码存在的情况下，可以编译pciv模块。

2、到SDK发布包下的mpp/sample/pciv路径下。
   按照文档所写，编译主片和从片的sample程序。
   注意在编译sample程序时，需要根据所选择的sensor类型，调整sample路径下的Makefile.param文件中的SENSOR0_TYPE的值。
   在sample/pciv目录下放置名称为3840x2160_8bit.h264的H264格式的码流文件和名称为mm2.bmp格式的位图文件。

加载ko：
1、将上面编译好的pcie的ko放到mpp/out/linux/multi-core/ko/pcie目录下。
2、到mpp/out/linux/multi-core/ko/目录下。
3、在主片上执行load3559av100_multicore脚本，在从片上执行load3559av100_multicore_slaver加载ko。
输入命令：./load3559av100_multicore -i -sensor0 （这项根据sensor类型选择）  
          ./load3559av100_multicore_slaver -i -sensor0 （这项根据sensor类型选择）
		  
执行sample文件：
到sample/pci目录下，从片运行sample_pciv_slave。主片运行sample_pciv_master。注意，要先运行从片的用例，在运行主片的用例。
运行是按照打印提示选择相应的用例，退出时在主片上输入enter键退出主从片。
		  
		  
  
		  
关于PCIV sample程序的介绍


PCIV 相关样例程序包含以下几部分：

1、PCIV MSG ：PCI业务层的消息通讯封装。基于MCC模块提供的ioctl接口，提供消息端口的打开关闭、消息发送、消息接收等接口。
	相关代码为pciv_msg.c、pciv_msg.h。

2、PCIV Trans：PCI业务层的数据传输封装。基于PCI DMA传输接口、PCI消息交互及一套基本的读写指针Buffer，实现业务层通用数据传输接口。
	可以用于主从片之间的任何类型的数据的传输。相关代码为pciv_trans.c、pciv_trans.h。

3、PCIV Sample：PCI业务层的接口使用样例。基于PCIV的MPI接口和以上两个模块提供的接口。
	相关代码为sample_pciv_host.c sample_pciv_slave.c、sample_pciv_comm.h。
	目前演示的业务主要有Hi3559A的级联：
	1）从片将实时预览图像通过PCI传输到主片并显示,同时把编码后的码流送完主片保存。
    2) 从片将解码出的图像传送到主片显示。
	3）演示示例。用于模拟主片向从片传输yuv数据。从片这边会打印接收buff的地址，通过命令查看内存可以查看数据是否正确传输。
	4）主片向从片传输码流，解码后将图像传回到主片显示。

编译方法：
主片：
    make host
从片：
    make slave
清除：
    make clean


PCIV Sample提供的仅仅是样例代码，部分功能可能实现不全面或有缺陷，请参考使用。		  
		  
		  
   
