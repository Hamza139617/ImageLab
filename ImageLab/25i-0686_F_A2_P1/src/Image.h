#ifndef IMAGELAB_IMAGE_H
#define IMAGELAB_IMAGE_H

// GDI+ is used only for decoding/encoding image files. All transformations
// are implemented manually on the Pixel array below.
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <iostream>
#include <string>
#include <limits>
#include <new>
#include <cctype>

struct Pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    Pixel(unsigned char red = 0, unsigned char green = 0,
          unsigned char blue = 0, unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {
    }

    bool operator==(const Pixel& other) const {
        return r == other.r && g == other.g &&
               b == other.b && a == other.a;
    }

    bool operator!=(const Pixel& other) const {
        return !(*this == other);
    }
};

class GdiPlusRuntime {
private:
    ULONG_PTR token;
    Gdiplus::Status startupStatus;

public:
    GdiPlusRuntime() : token(0), startupStatus(Gdiplus::GenericError) {
        Gdiplus::GdiplusStartupInput input;
        startupStatus = Gdiplus::GdiplusStartup(&token, &input, nullptr);
    }

    ~GdiPlusRuntime() {
        if (startupStatus == Gdiplus::Ok)
            Gdiplus::GdiplusShutdown(token);
    }

    bool isReady() const {
        return startupStatus == Gdiplus::Ok;
    }
};

inline GdiPlusRuntime& getGdiPlusRuntime() {
    static GdiPlusRuntime runtime;
    return runtime;
}

inline std::wstring imagePathToWide(const std::string& path) {
    // This conversion supports ordinary Windows paths in the active code page.
    // Use a Unicode-aware conversion if paths containing non-ASCII characters
    // must be supported.
    return std::wstring(path.begin(), path.end());
}

inline bool getImageEncoderClsid(const WCHAR* mimeType, CLSID* clsid) {
    UINT encoderCount = 0;
    UINT bytesNeeded = 0;

    if (Gdiplus::GetImageEncodersSize(&encoderCount, &bytesNeeded) != Gdiplus::Ok ||
        bytesNeeded == 0) {
        return false;
    }

    BYTE* buffer = new (std::nothrow) BYTE[bytesNeeded];
    if (buffer == nullptr)
        return false;

    Gdiplus::ImageCodecInfo* codecs =
        reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer);

    bool found = false;
    if (Gdiplus::GetImageEncoders(encoderCount, bytesNeeded, codecs) == Gdiplus::Ok) {
        for (UINT i = 0; i < encoderCount; ++i) {
            if (wcscmp(codecs[i].MimeType, mimeType) == 0) {
                *clsid = codecs[i].Clsid;
                found = true;
                break;
            }
        }
    }

    delete[] buffer;
    return found;
}

class Image {
private:
    int imageWidth;
    int imageHeight;
    Pixel* pixels;

    bool validDimensions(int width, int height) const {
        return width > 0 && height > 0 &&
               width <= (std::numeric_limits<int>::max)() / height;
    }

public:
    Image() : imageWidth(0), imageHeight(0), pixels(nullptr) {
    }

    Image(const Image& other)
        : imageWidth(0), imageHeight(0), pixels(nullptr) {
        if (!other.isEmpty())
            create(other.imageWidth, other.imageHeight, Pixel());
        const int total = imageWidth * imageHeight;
        for (int i = 0; i < total; ++i)
            pixels[i] = other.pixels[i];
    }

    Image& operator=(const Image& other) {
        if (this == &other)
            return *this;

        if (other.isEmpty()) {
            clear();
            return *this;
        }

        Image copy;
        if (!copy.create(other.imageWidth, other.imageHeight, Pixel()))
            return *this;

        const int total = other.imageWidth * other.imageHeight;
        for (int i = 0; i < total; ++i)
            copy.pixels[i] = other.pixels[i];

        swap(copy);
        return *this;
    }

    ~Image() {
        delete[] pixels;
    }

    void swap(Image& other) {
        int tempWidth = imageWidth;
        imageWidth = other.imageWidth;
        other.imageWidth = tempWidth;

        int tempHeight = imageHeight;
        imageHeight = other.imageHeight;
        other.imageHeight = tempHeight;

        Pixel* tempPixels = pixels;
        pixels = other.pixels;
        other.pixels = tempPixels;
    }

    bool create(int width, int height, const Pixel& fill = Pixel()) {
        if (!validDimensions(width, height))
            return false;

        Pixel* newPixels = new (std::nothrow) Pixel[width * height];
        if (newPixels == nullptr)
            return false;

        const int total = width * height;
        for (int i = 0; i < total; ++i)
            newPixels[i] = fill;

        delete[] pixels;
        pixels = newPixels;
        imageWidth = width;
        imageHeight = height;
        return true;
    }

    void clear() {
        delete[] pixels;
        pixels = nullptr;
        imageWidth = 0;
        imageHeight = 0;
    }

    bool isEmpty() const {
        return pixels == nullptr || imageWidth <= 0 || imageHeight <= 0;
    }

    int width() const {
        return imageWidth;
    }

    int height() const {
        return imageHeight;
    }

    int pixelCount() const {
        return isEmpty() ? 0 : imageWidth * imageHeight;
    }

    Pixel* data() {
        return pixels;
    }

    const Pixel* data() const {
        return pixels;
    }

    Pixel& at(int x, int y) {
        return pixels[y * imageWidth + x];
    }

    const Pixel& at(int x, int y) const {
        return pixels[y * imageWidth + x];
    }

    bool contains(int x, int y) const {
        return !isEmpty() && x >= 0 && y >= 0 &&
               x < imageWidth && y < imageHeight;
    }

    // Load the file into our own RGBA pixel array.
    bool load(const std::string& fileName) {
        if (!getGdiPlusRuntime().isReady()) {
            std::cerr << "GDI+ could not be initialized.\n";
            return false;
        }

        const std::wstring widePath = imagePathToWide(fileName);
        Gdiplus::Bitmap bitmap(widePath.c_str());

        if (bitmap.GetLastStatus() != Gdiplus::Ok) {
            std::cerr << "Could not load image: " << fileName << '\n';
            return false;
        }

        const UINT sourceWidth = bitmap.GetWidth();
        const UINT sourceHeight = bitmap.GetHeight();

        if (sourceWidth == 0 || sourceHeight == 0 ||
            sourceWidth > static_cast<UINT>((std::numeric_limits<int>::max)()) ||
            sourceHeight > static_cast<UINT>((std::numeric_limits<int>::max)()) ||
            sourceWidth > static_cast<UINT>((std::numeric_limits<int>::max)()) / sourceHeight) {
            std::cerr << "Image dimensions are invalid or too large.\n";
            return false;
        }

        const int width = static_cast<int>(sourceWidth);
        const int height = static_cast<int>(sourceHeight);

        Pixel* newPixels = new (std::nothrow) Pixel[width * height];
        if (newPixels == nullptr) {
            std::cerr << "Not enough memory to load this image.\n";
            return false;
        }

        Gdiplus::Rect rect(0, 0, width, height);
        Gdiplus::BitmapData bitmapData{};
        const Gdiplus::Status lockStatus = bitmap.LockBits(
            &rect, Gdiplus::ImageLockModeRead,
            Gdiplus::PixelFormat32bppARGB, &bitmapData);

        if (lockStatus != Gdiplus::Ok) {
            delete[] newPixels;
            std::cerr << "Could not read the image pixels.\n";
            return false;
        }

        for (int y = 0; y < height; ++y) {
            const BYTE* row = static_cast<const BYTE*>(bitmapData.Scan0) +
                              y * bitmapData.Stride;

            for (int x = 0; x < width; ++x) {
                const int offset = x * 4;
                Pixel& p = newPixels[y * width + x];
                p.b = row[offset + 0];
                p.g = row[offset + 1];
                p.r = row[offset + 2];
                p.a = row[offset + 3];
            }
        }

        bitmap.UnlockBits(&bitmapData);

        delete[] pixels;
        pixels = newPixels;
        imageWidth = width;
        imageHeight = height;
        return true;
    }

    // Save the pixel array as PNG, JPG/JPEG, or BMP. Encoding is delegated
    // to GDI+; no image transformation is delegated to the library.
    bool save(const std::string& fileName) const {
        if (isEmpty()) {
            std::cerr << "There is no image to save.\n";
            return false;
        }

        if (!getGdiPlusRuntime().isReady()) {
            std::cerr << "GDI+ could not be initialized.\n";
            return false;
        }

        std::string extension;
        const std::string::size_type dot = fileName.find_last_of('.');
        if (dot != std::string::npos)
            extension = fileName.substr(dot);

        for (char& ch : extension)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        const WCHAR* mimeType = nullptr;
        if (extension == ".png")
            mimeType = L"image/png";
        else if (extension == ".jpg" || extension == ".jpeg")
            mimeType = L"image/jpeg";
        else if (extension == ".bmp")
            mimeType = L"image/bmp";
        else {
            std::cerr << "Unsupported output extension. Use PNG, JPG/JPEG, or BMP.\n";
            return false;
        }

        CLSID encoderClsid{};
        if (!getImageEncoderClsid(mimeType, &encoderClsid)) {
            std::cerr << "Could not find an encoder for this file type.\n";
            return false;
        }

        Gdiplus::Bitmap output(imageWidth, imageHeight, Gdiplus::PixelFormat32bppARGB);
        if (output.GetLastStatus() != Gdiplus::Ok) {
            std::cerr << "Could not create the output image.\n";
            return false;
        }

        Gdiplus::Rect rect(0, 0, imageWidth, imageHeight);
        Gdiplus::BitmapData bitmapData{};
        if (output.LockBits(&rect, Gdiplus::ImageLockModeWrite,
                            Gdiplus::PixelFormat32bppARGB, &bitmapData) != Gdiplus::Ok) {
            std::cerr << "Could not prepare the output pixels.\n";
            return false;
        }

        for (int y = 0; y < imageHeight; ++y) {
            BYTE* row = static_cast<BYTE*>(bitmapData.Scan0) +
                        y * bitmapData.Stride;

            for (int x = 0; x < imageWidth; ++x) {
                const Pixel& p = at(x, y);
                const int offset = x * 4;
                row[offset + 0] = p.b;
                row[offset + 1] = p.g;
                row[offset + 2] = p.r;
                row[offset + 3] = p.a;
            }
        }

        output.UnlockBits(&bitmapData);

        const std::wstring widePath = imagePathToWide(fileName);
        if (output.Save(widePath.c_str(), &encoderClsid, nullptr) != Gdiplus::Ok) {
            std::cerr << "Could not save image: " << fileName << '\n';
            return false;
        }

        return true;
    }
};

#endif
