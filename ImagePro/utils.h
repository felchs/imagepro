#pragma once

#include <iostream>
#include <vector>

#include <windows.h>

namespace imagepro
{
    //
    // Structure to hold bitmap pixels and dimensions
    //
    struct ClipboardImage
    {
        std::vector<unsigned char> pixels; // BGRA
        int width = 0;
        int height = 0;
        bool valid = false;
    };

    class ClipBoard
    {
    public:
        static void ConvertBGRAToRGBA(const std::vector<unsigned char>& src, std::vector<unsigned char>& dst)
        {
            size_t pixelCount = src.size() / 4;
            dst.resize(src.size());

            for (size_t i = 0; i < pixelCount; ++i)
            {
                dst[i * 4 + 0] = src[i * 4 + 2]; // R
                dst[i * 4 + 1] = src[i * 4 + 1]; // G
                dst[i * 4 + 2] = src[i * 4 + 0]; // B
                dst[i * 4 + 3] = src[i * 4 + 3]; // A
            }
        }

        // Reads DIB (device-independent bitmap) image from clipboard and returns pixel data (BGRA)
        static ClipboardImage GetImageFromClipboard()
        {
            ClipboardImage result;

            if (!OpenClipboard(nullptr))
            {
                std::cerr << "Failed to open clipboard\n";
                return result;
            }

            // Check for DIB format (more flexible than CF_BITMAP)
            HANDLE hDIB = GetClipboardData(CF_DIB);
            if (!hDIB)
            {
                std::cerr << "No DIB image in clipboard\n";
                CloseClipboard();
                return result;
            }

            void* pDIB = GlobalLock(hDIB);
            if (!pDIB)
            {
                std::cerr << "Failed to lock clipboard data\n";
                CloseClipboard();
                return result;
            }

            BITMAPINFOHEADER* bih = reinterpret_cast<BITMAPINFOHEADER*>(pDIB);

            int width = bih->biWidth;
            int height = bih->biHeight;
            int bitCount = bih->biBitCount;

            if (bitCount != 32)
            {
                std::cerr << "Unsupported bit count: " << bitCount << " (expected 32)\n";
                GlobalUnlock(hDIB);
                CloseClipboard();
                return result;
            }

            // Pointer to pixel data (immediately after BITMAPINFOHEADER + color masks if any)
            unsigned char* pPixels = reinterpret_cast<unsigned char*>(pDIB) + bih->biSize;

            // Bitmap DIB is stored bottom-up, so flip vertically
            int rowSize = ((width * bitCount + 31) / 32) * 4; // bytes per row (DWORD aligned)

            std::vector<unsigned char> pixels(width * height * 4);

            for (int y = 0; y < height; ++y)
            {
                unsigned char* srcRow = pPixels + (height - 1 - y) * rowSize;
                unsigned char* dstRow = &pixels[y * width * 4];
                memcpy(dstRow, srcRow, width * 4);
            }

            GlobalUnlock(hDIB);
            CloseClipboard();

            result.pixels = std::move(pixels);
            result.width = width;
            result.height = height;
            result.valid = true;
            return result;
        }
    };
   
}