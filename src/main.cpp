




#include "SAJ_lib.h"

//########################################################################
// PLATFORM GLOBALS
//########################################################################
bool static running = true;


//########################################################################
// PLATFORM FUNCTIONS
//########################################################################
bool platform_create_window(int width, int height, char* title);
void update_platform_window();
//########################################################################
// WINDOWS PLATFORM
//########################################################################
#ifdef _WIN32
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

#endif

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