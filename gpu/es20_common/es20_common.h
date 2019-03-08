#ifndef __ES20_COMMON_H__
#define __ES20_COMMON_H__

#include <stdio.h>

#define MALI_FPRINT(fmt, arg...) fprintf(stderr, fmt, ##arg)

#define ASSERT_EQUAL(value, expected) if(value!=expected) \
    fprintf(stderr, "func = %s, line = %d, value = %d\n", __FUNCTION__, __LINE__, (int)value);

#define ASSERT_NOT_EQUAL(value, expected) if(value==expected) \
    fprintf(stderr,"func = %s, line = %d, value = %d\n", __FUNCTION__, __LINE__, (int)value);

typedef struct hiMMZ_BUF_S
{
    char bufname[20];  /** Strings of an MMZ buffer name **/
    HI_U64  phyaddr;                /** Physical address of an MMZ buffer **/
    void  *viraddr;           /** User-state virtual address of an MMZ buffer **/
    unsigned int  bufsize;                /** Size of an MMZ buffer **/
}HI_MMZ_BUF_S;

/** Enum for display formats */
typedef enum display_format
{
    DISPLAY_FORMAT_INVALID,
    DISPLAY_FORMAT_RGB565,
    DISPLAY_FORMAT_ARGB8888,
    DISPLAY_FORMAT_ARGB4444,
    DISPLAY_FORMAT_ARGB1555,
    DISPLAY_FORMAT_ARGB8885, /* Bogus format, for testing */
    DISPLAY_FORMAT_ARGB8880, /* Bogus format, for testing */
    DISPLAY_FORMAT_ARGB8808, /* Bogus format, for testing */
    DISPLAY_FORMAT_ARGB8088, /* Bogus format, for testing */
    DISPLAY_FORMAT_ARGB0888, /* Bogus format, for testing */
    DISPLAY_FORMAT_ARGB0000, /* Bogus format, for testing */
    DISPLAY_FORMAT_RGB560, /* Bogus format, for testing */
    DISPLAY_FORMAT_RGB505, /* Bogus format, for testing */
    DISPLAY_FORMAT_RGB065, /* Bogus format, for testing */
    DISPLAY_FORMAT_RGB000, /* Bogus format, for testing */
    DISPLAY_FORMAT_L8,
    DISPLAY_FORMAT_AL88,
    DISPLAY_FORMAT_A8,
    DISPLAY_FORMAT_COUNT
}
display_format;

#ifdef FRAMEBUFFER_DEVICE_OPEN
int  HI_COMMON_OpenFramebufferDevice(int width, int height, display_format display_fmt);
void HI_COMMON_CloseFramebufferDevice(int fd);
#endif

/*EGL API Function*/
NativeDisplayType HI_COMMON_CreateNativeDisplay(void);
NativeWindowType HI_COMMON_CreateNativeWindow(int width,int height);
NativePixmapType HI_COMMON_CreateNativePixmap(unsigned int phyaddr, int width,int height, uint64_t pixmap_format);
void HI_COMMON_DestroyNativeWindow(NativeWindowType native_window);
void HI_COMMON_DestroyNativeDisplay(NativeDisplayType native_display);
void HI_COMMON_DestroyNativePixmap(NativePixmapType native_pixmap);

/*OpenGL ES API Function*/
unsigned int HI_COMMON_CreateShader(const char* shader_name, unsigned int shader_type);
unsigned int HI_COMMON_CreateProgram(unsigned int vshader_id, unsigned int fshader_id);
unsigned int HI_COMMON_CreateTexture(unsigned int tex_num, unsigned int tex_target, int tex_wrap_mode, int tex_filter);
unsigned int HI_COMMON_CreateBuffer(unsigned int buffer_type, GLsizeiptr size, const void *data);
unsigned int HI_COMMON_CreateFBO(unsigned int texture_id, unsigned int tex_target, unsigned int attachment);

void HI_COMMON_DestroyShader(unsigned int shader_id);
void HI_COMMON_DestroyProgram(unsigned int program_id);
void HI_COMMON_DestroyTexture(unsigned int texture_id);
void HI_COMMON_DestroyBuffer(unsigned int buffer_id);
void HI_COMMON_DestroyFBO(unsigned int fbo_id);

/*Other Function*/
int HI_COMMON_WrapDmaBufFD(unsigned int physical_address, unsigned int size);
void* HI_COMMON_LoadFile(const char *f_path, unsigned long skip, unsigned long f_size, const char *f_type);
void HI_COMMON_ModifyVirtualMemory(void **vir_addr, unsigned int color, unsigned long size);
void HI_COMMON_PostMemoryToFramebuffer(void *virtual_addr, int memsize);

HI_MMZ_BUF_S * HI_COMMON_CreateMMZMemory(unsigned int size);
void HI_COMMON_DestroyMMZMemory(HI_MMZ_BUF_S *pBuf);

#endif
