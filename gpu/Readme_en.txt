How to use GPU:

1 Driver
Hi3559A GPU driver consists of two parts, kernel space driver and user space driver.

1.1 Kernel space driver
Kernel space driver depends on Linux kernel. It is open source and is released in the form of source code. User need to compile it before using kernel space driver.
The source code for kernel space driver is in $(SDK)/mpp/component/gpu/kernel. 
Please make sure that the "KDIR" in Makefile is equal to the Linux kernel path, then use "make" command to compile it.
hi_dbe.ko and mali_kbase.ko will be compiled and put in $(SDK)/mpp/component/gpu/release/ko. You can insert these modules using the commmand "./loadgpu -i".

1.2 User space driver
User space driver is released in the form of dynamic library binary. It is not open source.
You can find the libs in $(SDK)/mpp/component/gpu/release/lib.
When you want to use these libs, you must make sure that the libs can be found by the system. This could done by using the following command:
export LD_LIBRARY_PATH="/xxxx/mpp/component/gpu/release/lib:$LD_LIBRARY_PATH"


2 Samples

2.1 OpenGL ES
GPU uses framebuffer to display on graphics layer. So it has a dependency on framebuffer.
Please note that, the GPU sample does not open framebuffer device by itselt. User may need to execute sample_hifb using ARGB8888 to open the device.
This could be done by using the command:
./sample_hifb 0 0 0
after you see the display, use Ctrl+Z to put it in background.

2.1.1 es20_eglCreateWindowSurface
simple example of on-screen draw with GPU.

2.1.2 es20_eglCreateWindowSurface
simple example of off-screen draw with GPU.

2.2 pano2view
pano2view is a module using GPU to do anti-distortion correction on VR videos.
The module does not have a dependency on framebuffer module.