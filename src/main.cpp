#include "SAJ_lib.h"
#include "platform.h"

#define APIENTRY
#include "glcorearb.h"

#ifdef _WIN32
#include "win32_platform.cpp"
#endif

#include "gl_renderer.h"

int main()
{
    platform_create_window(1200,900, "hello world");

    while (running)
    {
        update_platform_window();
        SAJ_TRACE("green");
        SAJ_DEBUG("cyan");
        SAJ_WARN("yellow");
        SAJ_ERROR("red");
        SAJ_ASSERT(false,"assertion not hit!");
    }
    return 0;
}