#pragma warning(disable : 4996)

#include <iostream>
#include <functional>

#include <windows.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool CaptureWindow(HWND hwnd, const std::function<void(const void* data, int width, int height)>& callback);
bool SaveAsPNG(const char* path, int w, int h, int src_stride, const void* data, bool flip_y);

LPCWSTR className = L"FLUTTER_RUNNER_WIN32_WINDOW";
LPCWSTR wndName = L"";


class DibSection {
    int Width = 0;
    int Height = 0;

    void* pData = nullptr;

    HBITMAP hBmp;
    HDC hDC;
    BITMAPINFO bmpInfo{};

    void Create(int width, int height) {
        bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
        bmpInfo.bmiHeader.biWidth = width;
        bmpInfo.bmiHeader.biHeight = height;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = width * height * 4;
    

        HWND hwnd = GetDesktopWindow();
        HDC hscreen = ::GetDC(hwnd);
        hDC = ::CreateCompatibleDC(hscreen);
        
        hBmp = ::CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, &pData, NULL, NULL);
    }
};


int main()
{
	HWND hwnd = FindWindowW(className, L"base");

	if (hwnd == 0) {
		printf("Window not found");
		return 1;
	}


	SetForegroundWindow(hwnd);

	CaptureWindow(hwnd, [](const void* data, int w, int h) {
            SaveAsPNG("test.png", w, h, w * 4, data, true);
        });
}


bool CaptureWindow(HWND hwnd, const std::function<void(const void* data, int width, int height)>& callback)
{
    RECT rect{};
    ::GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biSizeImage = width * height * 4;

    bool ret = false;
    HDC hscreen = ::GetDC(hwnd);
    HDC hdc = ::CreateCompatibleDC(hscreen);
    void* data = nullptr;
    if (HBITMAP hbmp = ::CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &data, NULL, NULL)) {
        
        ::SelectObject(hdc, hbmp);
        
        ::PrintWindow(hwnd, hdc, PW_RENDERFULLCONTENT);
        
        callback(data, width, height);
        
        ::DeleteObject(hbmp);
        
        ret = true;
    }
    ::DeleteDC(hdc);
    ::ReleaseDC(hwnd, hscreen);
    return ret;
}

bool SaveAsPNG(const char* path, int w, int h, int src_stride, const void* data, bool flip_y)
{
    std::vector<byte> buf(w * h * 4);
    int dst_stride = w * 4;
    auto src = (const byte*)data;
    auto dst = (byte*)buf.data();
    if (flip_y) {
        for (int i = 0; i < h; ++i) {
            auto s = src + (src_stride * (h - i - 1));
            auto d = dst + (dst_stride * i);
            for (int j = 0; j < w; ++j) {
                d[0] = s[2];
                d[1] = s[1];
                d[2] = s[0];
                d[3] = s[3];
                s += 4;
                d += 4;
            }
        }
    }
    else {
        for (int i = 0; i < h; ++i) {
            auto s = src + (src_stride * i);
            auto d = dst + (dst_stride * i);
            for (int j = 0; j < w; ++j) {
                d[0] = s[2];
                d[1] = s[1];
                d[2] = s[0];
                d[3] = s[3];
                s += 4;
                d += 4;
            }
        }
    }
    return stbi_write_png(path, w, h, 4, buf.data(), dst_stride);
}
