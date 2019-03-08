GPU使用须知：

1 驱动
Hi3559A GPU驱动包含两部分，内核态驱动与用户态驱动。

1.1 内核态驱动
内核态驱动依赖linux内核，是开源发布的。
内核态驱动源码包含在发布路径的mpp/component/gpu/kernel中，用户需要在路径下，修改Makefile，指定Makefile中的KDIR=$(用户的内核路径)，然后用make命令编译。
编译出的内核态ko在mpp/component/gpu/release/ko中，使用该目录下的脚本./loadgpu -i命令，即可加载。

1.2 用户态驱动
用户态驱动不开源，发布方式为二进制库文件，包含在发布路径的mpp/component/gpu/release/lib中。
用户使用GPU，须在单板上将该路径export到系统可以找到的位置，可以使用类似下面的命令
export LD_LIBRARY_PATH="/xxxx/mpp/component/gpu/release/lib:$LD_LIBRARY_PATH"


2 用例
GPU支持全特性OpenGLES与OpenCL接口标准，因此对应两种接口标准提供有两套用例。

2.1 OpenGLES
OpenGLES是图形相关的绘制接口，它依赖framebuffer
运行时，需要用户进入mpp/sample/gpu，使用make命令编译。
请注意，GPU的sample并不会自行打开framebuffer，因此运行GLES的sample时，需要用户在后台运行一个ARGB8888格式的sample_hifb。
可以进入mpp/sample/hifb，使用./sample_hifb 0 0 0命令，然后Ctrl+Z将sample_hifb放置到后台。

2.1.1 es20_eglCreateWindowSurface
简单的使用GPU绘制到屏幕上的sample。

2.1.2 es20_eglCreateWindowSurface
简单的使用GPU绘制到内存中的sample（离屏绘制）。

2.2 pano2view
pano2view是使用GPU对全景视频进行反畸变校正的模块。
运行pano2view的sample不需要依赖framebuffer。

