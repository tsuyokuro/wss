#pragma warning(disable : 4996)

#include <iostream>
#include <functional>

#include <windows.h>
#include <winbase.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool SaveAsPNG(const char* path, int w, int h, int src_stride, const void* data, bool flip_y);

LPCWSTR className = L"FLUTTER_RUNNER_WIN32_WINDOW";
LPCWSTR wndName = L"base";


class DibSection {
public:
    int Width = 0;
    int Height = 0;

    void* pData = nullptr;

    HBITMAP hBmp;
    HDC hDC;
    BITMAPINFO bmpInfo{};


    ~DibSection() {
        ::DeleteObject(hBmp);
        ::DeleteDC(hDC);
    }

    void Create(int width, int height) {
        bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
        bmpInfo.bmiHeader.biWidth = width;
        bmpInfo.bmiHeader.biHeight = height;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 32;
        bmpInfo.bmiHeader.biCompression = BI_RGB;
        bmpInfo.bmiHeader.biSizeImage = width * height * 4;
    
        Width = width;
        Height = height;

        HWND hwnd = GetDesktopWindow();
        HDC hscreen = ::GetDC(hwnd);

        hDC = ::CreateCompatibleDC(hscreen);
        
        ReleaseDC(hwnd, hscreen);

        hBmp = ::CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, &pData, NULL, NULL);
    }

    void SelectObject() {
        ::SelectObject(hDC, hBmp);
    }
};


int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Error: No file name\n");
        return 1;
    }

	HWND hwnd = FindWindowW(className, wndName);

	if (hwnd == 0) {
		printf("Error: Window not found\n");
		return 1;
    }


	SetForegroundWindow(hwnd);

    RECT rect{};
    ::GetWindowRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;


    int trimL = 8;
    int trimR = 8;
    int trimT = 0;
    int trimB = 8;

    int trimW = width - (trimL + trimR);
    int trimH = height - (trimT + trimB);


    printf("Window found. width:%d, height:%d\n", trimW, trimH);


    DibSection* pDibSection = new DibSection();
    pDibSection->Create(width, height);
    pDibSection->SelectObject();


    BOOL ret = ::PrintWindow(hwnd, pDibSection->hDC, PW_RENDERFULLCONTENT);


    DibSection* pDibSection2 = new DibSection();
    pDibSection2->Create(trimW, trimH);
    pDibSection2->SelectObject();

    BitBlt(
        pDibSection2->hDC,
        0, 0,
        pDibSection2->Width, pDibSection2->Height,
        pDibSection->hDC,
        trimL, trimT, SRCCOPY);

    int w = pDibSection2->Width;
    int h = pDibSection2->Height;
    void* data = pDibSection2->pData;

    std::string path(argv[1]);
    path += ".png";

    bool writen = SaveAsPNG(path.c_str(), w, h, w * 4, data, true);

    if (writen) {
        printf("Save as PNG success. %s\n", path.c_str());
    }
    else {
        printf("Save as PNG failed. %s\n", path.c_str());
    }

    delete pDibSection;
    delete pDibSection2;
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
