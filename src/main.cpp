#define APIENTRY
//to include other function definition like glEnable
#define GL_GLEXT_PROTOTYPES

#include "SAJ_lib.h"
#include "input.h"
#include "platform.h"

//avoid re6importing windows and defining mean max
//opengl core
#include "glcorearb.h"

#ifdef _WIN32
#include "win32_platform.cpp"
#endif

#include "gl_renderer.cpp"

int main()
{
    BumpAllocator transientStorage = make_bump_allocator(MB(50));
    input.screenSizeX = 1200;
    input.screenSizeY = 720;
    platform_create_window(input.screenSizeX,input.screenSizeY, "hello world");

    gl_init(&transientStorage);
    while (running)
    {
        update_platform_window();
        gl_render();
        platform_swap_buffers();
    }
    return 0;
}