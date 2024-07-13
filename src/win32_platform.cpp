#include "platform.h"
#include "SAJ_lib.h"

//########################################################################
// WINDOWS PLATFORM
//########################################################################
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
//########################################################################
// WINDOWS GLOBALS
//########################################################################
static const char* W_MAIN_WINDOW_CLASS_NAME = "WMainWindow";
static HWND W_MAIN_WINDOW_HANDLE;

//########################################################################
// PLATFORM IMPLEMENTATIONS
//########################################################################
LRESULT CALLBACK windows_window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (msg)
    {
    case WM_CLOSE:
        running = false;
        break;
    case WM_KEYDOWN:
        break;
    default:
        result = DefWindowProcA(window, msg, wParam, lParam);
    }
    return result;
};


bool platform_create_window(int width, int height, char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASS wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = W_MAIN_WINDOW_CLASS_NAME; // unique id
    wc.lpfnWndProc = windows_window_callback; // callback for inputs in the window

    if(!RegisterClass(&wc))
    {
        return false;
    }

    int dwStyle = WS_OVERLAPPEDWINDOW;

    W_MAIN_WINDOW_HANDLE = CreateWindowExA(0, W_MAIN_WINDOW_CLASS_NAME, title, dwStyle,100,100, width, height, NULL,NULL, instance, NULL);
    if(W_MAIN_WINDOW_HANDLE == NULL)
    {
        return false;
    }

    ShowWindow(W_MAIN_WINDOW_HANDLE, SW_SHOW);
    return true;
}
void update_platform_window()
{
    MSG msg;
    while(PeekMessageA(&msg,W_MAIN_WINDOW_HANDLE, 0,0,PM_REMOVE ))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void* platform_load_gl_function(char* funcName)
{
    PROC proc = wglGetProcAddress("glCreateProgram");
    if(!proc)
    {
        //try loading from dll directly, static to ensure only once
        static HMODULE openglDLL = LoadLibraryA("opengl32.dll");
        proc = GetProcAddress(openglDLL, funcName);

        if(!proc)
        {
            SAJ_ASSERT(false, "Failed to load glCreateProgram")
            return nullptr;
        }
    }

    return (void*)proc;
}