#define DDGL_CORE

#include "ddglwindow.h"
#include "ddglopengl.h"
#include "opengl/ddglopengl_shader.h"
#include "opengl/ddglopengl_material.h"

#include "ddclmap.h"

#include <Windows.h>
#include <GL/glew.h>

#include <stdlib.h>
#include <stdio.h>

typedef struct tag_WindowExtend{
    HWND hwnd;
    HDC hdc;
}WindowExtend;

static ddcl_Map * _HWND_M = NULL;
static HGLRC _hglrc = NULL;

static LRESULT CALLBACK
win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void
swap_buffer_window(ddgl_Window * w){
    WindowExtend * extend = (WindowExtend *)w->extend;
    SwapBuffers(extend->hdc);
}

static void
free_window(ddgl_Window * w){
    WindowExtend * extend = (WindowExtend *)w->extend;
    DestroyWindow(extend->hwnd);
}

static void
make_current_window(ddgl_Window * w){
    if(w){
        WindowExtend * extend = (WindowExtend *)w->extend;
        wglMakeCurrent(extend->hdc, _hglrc);
    }else{
        wglMakeCurrent(NULL, NULL);
    }
}

static void
clear_window(ddgl_Window * w){
    WindowExtend * extend = (WindowExtend *)w->extend;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3b(100, 100, 100);
}

DDGLAPI int
ddgl_new_opengl_window(ddgl_Window * w){
    static int _class_registerd = 0;
    if(!_class_registerd){
        _class_registerd = 1;

        _HWND_M = ddcl_new_map(NULL, NULL);
        WNDCLASSEX wc;
        memset(&wc, 0, sizeof(WNDCLASSEX));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = win32_window_proc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = "ddgl";
        RegisterClassEx(&wc);
    }
    int x = 0;
    int y = 0;

    HWND hwnd;
    HDC hdc;
    GLuint pixelFormat;
    int width = w->width;
    int height = w->height;

    DWORD style = WS_MINIMIZEBOX ;
    DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    int sys_width = GetSystemMetrics(SM_CXSCREEN);
    int sys_height = GetSystemMetrics(SM_CYSCREEN);
    int nX = (sys_width - width) / 2;
    int nY = (sys_height - height) / 2;
    RECT rect = { x, y, width, height };
    AdjustWindowRectEx(&rect, style, 0, exStyle);
    hwnd = CreateWindowEx(exStyle,
        "ddgl", w->title, style,
        nX, nY, rect.right - rect.left, rect.bottom - rect.top,
        HWND_DESKTOP, 0, GetModuleHandleW(NULL), NULL);
    if (!hwnd)
        return 1;
    hdc = GetDC(hwnd);
    if (!hdc){
        DestroyWindow(hwnd);
        return 1;
    }
    PIXELFORMATDESCRIPTOR pfd = {         /**< 设置像素描述结构 */
        sizeof(PIXELFORMATDESCRIPTOR),    /**< 像素描述结构的大小 */
        1,                                /**< 版本号 */
        PFD_DRAW_TO_WINDOW |              /**< 缓存区的输出显示在一个窗口中 */
        PFD_SUPPORT_OPENGL |              /**< 缓存区支持OpenGL绘图 */
        PFD_STEREO |                      /**< 颜色缓存区是立体缓存 */
        PFD_DOUBLEBUFFER,                 /**< 颜色缓存区是双缓存 */
        PFD_TYPE_RGBA,                    /**< 使用RGBA颜色格式 */
        16,                               /**< 颜色缓存区中颜色值所占的位深 */
        0, 0, 0, 0, 0, 0,                 /**< 使用默认的颜色设置 */
        0,                                /**< 无Alpha缓存 */
        0,                                /**< 颜色缓存区中alpha成分的移位计数 */
        0,                                /**< 无累计缓存区 */
        0, 0, 0, 0,                       /**< 累计缓存区无移位 */
        32,                               /**< 32位深度缓存 */
        0,                                /**< 无蒙版缓存 */
        0,                                /**< 无辅助缓存区 */
        PFD_MAIN_PLANE,                   /**< 必须为PFD_MAIN_PLANE，设置为主绘图层 */
        0,                                /**< 表示OpenGL实现所支持的上层或下层平面的数量 */
        0, 0, 0                           /**< 过时，已不再使用 */
    };
    pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat){
        DestroyWindow(hwnd);
        return 1;
     }
    if (!SetPixelFormat(hdc, pixelFormat, &pfd)){
        DestroyWindow(hwnd);
        return 1;
    }
    if(!_hglrc){
        _hglrc = wglCreateContext(hdc);
        if (!_hglrc) {
            DestroyWindow(hwnd);
            return 1;
        }
        wglMakeCurrent(hdc, _hglrc);
        GLenum err = glewInit();
        if(err != GLEW_OK){
            printf("glewInit error: %s\n", glewGetErrorString(err));
        }
		ddgl_init_opengl_module(ddgl_conf());
    }
    WindowExtend * extend = (WindowExtend *)w->extend;
    extend->hwnd = hwnd;
    extend->hdc = hdc;
    w->free = free_window;
    w->swap_buffer = swap_buffer_window;
    w->make_current = make_current_window;
    w->clear = clear_window;
    ShowWindow(hwnd, SW_NORMAL);
    return 0;
}

static LRESULT CALLBACK
win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_CLOSE:
        printf("WM_CLOSE %d\n", uMsg);
        break;
    case WM_PAINT:
        printf("paint\n");
        break;
    case WM_SIZE:
        printf("WM_SIZE %d\n", uMsg);
        break;
    case WM_SIZING:
        break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        printf("WM_MOUSEMOVE %d\n", uMsg);
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
