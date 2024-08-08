#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "platform.h"
#include "SAJ_lib.h"
//########################################################################
// WINDOWS PLATFORM
//########################################################################
#include <Windows.h>

#include "wglext.h"
//########################################################################
// WINDOWS GLOBALS
//########################################################################
static const char* W_MAIN_WINDOW_CLASS_NAME = "WMainWindow";
static HWND W_MAIN_WINDOW_HANDLE;
static HDC W_DEVICE_CONTEXT_HANDLE;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT_ptr;
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
    case WM_SIZE:
    {
        RECT rect = {};
        GetClientRect(W_MAIN_WINDOW_HANDLE, &rect);
        input->ScreenSize.x = rect.right - rect.left;
        input->ScreenSize.y = rect.bottom - rect.top;
        break;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        bool isDown = (msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN) ;
        KeyCodeBinding keyCode = KeyCodeLookupTable[wParam];
        KeyState* key = &input->keys[keyCode];
        key->isPressed = !key->isPressed &&  !key->isDown && isDown;
        key->isReleased = !key->isReleased && key->isDown && !isDown;
        key->isDown = isDown;
        key->halfTransitionCount++;
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
        bool isDown = (msg == WM_LBUTTONDOWN) || (msg == WM_RBUTTONDOWN) || (msg == WM_MBUTTONDOWN);
        int mouseCode = (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? VK_LBUTTON:
                        (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) ? VK_MBUTTON: VK_RBUTTON;

        KeyCodeBinding keyCode = KeyCodeLookupTable[mouseCode];
        KeyState* key = &input->keys[keyCode];
        key->isPressed = !key->isPressed && !key->isDown && isDown;
        key->isReleased = !key->isReleased && key->isDown && !isDown;
        key->isDown = isDown;
        key->halfTransitionCount++;
        break;
    }
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
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs;

    //Fake window to init OpenGl first
    {

        W_MAIN_WINDOW_HANDLE = CreateWindowExA(0, W_MAIN_WINDOW_CLASS_NAME, title, dwStyle,100,100, width, height, NULL,NULL, instance, NULL);
        if(W_MAIN_WINDOW_HANDLE == NULL)
        {
            SAJ_ASSERT(false, "Failed to create Windows Window")
            return false;
        }

        HDC fakeDC = GetDC(W_MAIN_WINDOW_HANDLE);
        if(!fakeDC)
        {
            SAJ_ASSERT(false, "Failed to get HDC")
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);
        if(!pixelFormat)
        {
            SAJ_ASSERT(false, "Failed to create pixel format")
            return false;
        }

        if(!SetPixelFormat(fakeDC, pixelFormat, &pfd))
        {
            SAJ_ASSERT(false, "Failed to set pixel format")
            return false;
        }

        //Create Handle to fake OpenGl Rendering Context
        HGLRC fakeRC = wglCreateContext(fakeDC);
        if(!fakeRC)
        {
            SAJ_ASSERT(false, "Failed to create render context")
            return false;
        }

        if(!wglMakeCurrent(fakeDC, fakeRC))
        {
            SAJ_ASSERT(false, "Failed to make current")
            return false;
        }
        //set function for the real window
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) platform_load_gl_function("wglChoosePixelFormatARB");
        wglCreateContextAttribs = (PFNWGLCREATECONTEXTATTRIBSARBPROC) platform_load_gl_function("wglCreateContextAttribsARB");
        wglSwapIntervalEXT_ptr = (PFNWGLSWAPINTERVALEXTPROC) platform_load_gl_function("wglSwapIntervalEXT");

        if(!wglCreateContextAttribs || !wglChoosePixelFormatARB)
        {
            SAJ_ASSERT(false, "Failed to Create Context Attributes or Chosing pixel format ARB")
            return false;
        }

        // Clean up
        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(W_MAIN_WINDOW_HANDLE, fakeDC);
        DestroyWindow(W_MAIN_WINDOW_HANDLE);

    }

    // Create actual window
    {
        RECT borderRec = {};
        AdjustWindowRectEx(&borderRec, dwStyle, 0, 0);
        width += borderRec.right - borderRec.left;
        height += borderRec.bottom - borderRec.top;

        W_MAIN_WINDOW_HANDLE = CreateWindowExA(0, W_MAIN_WINDOW_CLASS_NAME, title, dwStyle,100,100, width, height, NULL,NULL, instance, NULL);
        if(W_MAIN_WINDOW_HANDLE == NULL)
        {
            SAJ_ASSERT(false, "Failed to create Windows Window")
            return false;
        }

        W_DEVICE_CONTEXT_HANDLE = GetDC(W_MAIN_WINDOW_HANDLE);
        if(!W_DEVICE_CONTEXT_HANDLE)
        {
            SAJ_ASSERT(false, "Failed to get HDC")
            return false;
        }

        const int pixelAttribs[] = 
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 24,
            0 //terminate by 0 or error is thrown
        };

        UINT numPixelFormats;
        int pixelFormat = 0;
        if(!wglChoosePixelFormatARB(W_DEVICE_CONTEXT_HANDLE, pixelAttribs, 0, 1, &pixelFormat, &numPixelFormats))
        {
            SAJ_ASSERT(false, "Failed to choose pixel format ARB")
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        DescribePixelFormat(W_DEVICE_CONTEXT_HANDLE, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if(!SetPixelFormat(W_DEVICE_CONTEXT_HANDLE, pixelFormat, &pfd))
        {
            SAJ_ASSERT(false, "Failed to SetPixelFormat")
            return false;
        }
        const int contextAttribs[] = 
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0
        };
        HGLRC rc = wglCreateContextAttribs(W_DEVICE_CONTEXT_HANDLE, 0, contextAttribs);
        if(!rc)
        {
            SAJ_ASSERT(false, "Failed to cwglCreateContextAttrib")
            return false;
        }
        if(!wglMakeCurrent(W_DEVICE_CONTEXT_HANDLE, rc))
        {
            SAJ_ASSERT(false, "Failed to wglMakeCurrent")
            return false;
        }
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


    {
        POINT point = {};
        GetCursorPos(&point);
        ScreenToClient(W_MAIN_WINDOW_HANDLE, &point);

        input->mousePosition.x = point.x;
        input->mousePosition.y = point.y;

        input->mousePositionWorld = screen_to_world(input->mousePosition);
    }
}

void platform_swap_buffers()
{
   SwapBuffers(W_DEVICE_CONTEXT_HANDLE); 
}

void  platform_set_vsync(bool isVsync)
{
    wglSwapIntervalEXT_ptr(isVsync);
}

void* platform_load_gl_function(char* funcName)
{
    PROC proc = wglGetProcAddress(funcName);
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

bool platform_free_dynamic_lib(void* dll)
{
    BOOL result = FreeLibrary((HMODULE) dll);
    SAJ_ASSERT(result, "Failed to free library")
    return (bool)result;
}
void* platform_load_dynamic_lib(char* dll)
{
    HMODULE result = LoadLibraryA(dll);
    SAJ_ASSERT(result, "Failed to load lib %s", dll)

    return result;
}

void* platform_load_dynamic_function(void* dll, char* func_name)
{
    FARPROC proc = GetProcAddress((HMODULE)dll,func_name);
    SAJ_ASSERT(proc, "Failed to load functions: %s from DLL", func_name)

    return (void*) proc;
}

void platform_fill_keycode_lookup_table()
{
    KeyCodeLookupTable[VK_LBUTTON] = KEY_MOUSE_LEFT;
    KeyCodeLookupTable[VK_RBUTTON] = KEY_MOUSE_RIGHT;
    KeyCodeLookupTable[VK_MBUTTON] = KEY_MOUSE_MIDDLE;

    KeyCodeLookupTable['A'] = KEY_A;
    KeyCodeLookupTable['B'] = KEY_B;
    KeyCodeLookupTable['C'] = KEY_C;
    KeyCodeLookupTable['D'] = KEY_D;
    KeyCodeLookupTable['E'] = KEY_E;
    KeyCodeLookupTable['F'] = KEY_F;
    KeyCodeLookupTable['G'] = KEY_G;
    KeyCodeLookupTable['H'] = KEY_H;
    KeyCodeLookupTable['I'] = KEY_I;
    KeyCodeLookupTable['J'] = KEY_J;
    KeyCodeLookupTable['K'] = KEY_K;
    KeyCodeLookupTable['L'] = KEY_L;
    KeyCodeLookupTable['M'] = KEY_M;
    KeyCodeLookupTable['N'] = KEY_N;
    KeyCodeLookupTable['O'] = KEY_O;
    KeyCodeLookupTable['P'] = KEY_P;
    KeyCodeLookupTable['Q'] = KEY_Q;
    KeyCodeLookupTable['R'] = KEY_R;
    KeyCodeLookupTable['S'] = KEY_S;
    KeyCodeLookupTable['T'] = KEY_T;
    KeyCodeLookupTable['U'] = KEY_U;
    KeyCodeLookupTable['V'] = KEY_V;
    KeyCodeLookupTable['W'] = KEY_W;
    KeyCodeLookupTable['X'] = KEY_X;
    KeyCodeLookupTable['Y'] = KEY_Y;
    KeyCodeLookupTable['Z'] = KEY_Z;
    KeyCodeLookupTable[VK_SPACE] = KEY_SPACE;
    KeyCodeLookupTable[VK_TAB] = KEY_TAB;
    KeyCodeLookupTable[VK_LSHIFT] = KEY_LSHIFT;
    KeyCodeLookupTable[VK_RSHIFT] = KEY_RSHIFT;
    KeyCodeLookupTable[VK_DOWN] = KEY_ARROW_DOWN;
    KeyCodeLookupTable[VK_LEFT] = KEY_ARROW_LEFT;
    KeyCodeLookupTable[VK_RIGHT] = KEY_ARROW_RIGHT;
    KeyCodeLookupTable[VK_UP] = KEY_ARROW_UP;
    KeyCodeLookupTable[VK_MENU] = KEY_ALT;
    KeyCodeLookupTable[VK_RETURN] = KEY_ENTER;

    KeyCodeLookupTable[VK_NUMPAD0] = KEY_0;
    KeyCodeLookupTable[VK_NUMPAD1] = KEY_1;
    KeyCodeLookupTable[VK_NUMPAD2] = KEY_2;
    KeyCodeLookupTable[VK_NUMPAD3] = KEY_3;
    KeyCodeLookupTable[VK_NUMPAD4] = KEY_4;
    KeyCodeLookupTable[VK_NUMPAD5] = KEY_5;
    KeyCodeLookupTable[VK_NUMPAD6] = KEY_6;
    KeyCodeLookupTable[VK_NUMPAD7] = KEY_7;
    KeyCodeLookupTable[VK_NUMPAD8] = KEY_8;
    KeyCodeLookupTable[VK_NUMPAD9] = KEY_9;


}