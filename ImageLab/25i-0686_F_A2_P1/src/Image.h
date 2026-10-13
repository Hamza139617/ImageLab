
#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <ctime>

#define STB_IMAGE_STATIC
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;

class Pixel
{
public:
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    Pixel()
    {
        r = 0;
        g = 0;
        b = 0;
        a = 255;
    }

    Pixel(unsigned char red, unsigned char green,
        unsigned char blue, unsigned char alpha = 255)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
};

class Image
{
private:
    int imgWidth;
    int imgHeight;
    Pixel* pixels;

    void copyPixels(const Image& other)
    {
        imgWidth = 0;
        imgHeight = 0;
        pixels = NULL;

        if (other.isEmpty())
        {
            return;
        }

        long long total =
            (long long)other.imgWidth * other.imgHeight;

        Pixel* temp = new Pixel[total];

        for (long long i = 0; i < total; i++)
        {
            temp[i] = other.pixels[i];
        }

        pixels = temp;
        imgWidth = other.imgWidth;
        imgHeight = other.imgHeight;
    }

public:
    Image()
    {
        imgWidth = 0;
        imgHeight = 0;
        pixels = NULL;
    }

    Image(int width, int height)
    {
        imgWidth = 0;
        imgHeight = 0;
        pixels = NULL;

        if (width <= 0 || height <= 0)
        {
            return;
        }

        long long total = (long long)width * height;

        pixels = new Pixel[total];

        imgWidth = width;
        imgHeight = height;
    }

    Image(const Image& other)
    {
        imgWidth = 0;
        imgHeight = 0;
        pixels = NULL;

        copyPixels(other);
    }

    Image& operator=(const Image& other)
    {
        if (this == &other)
        {
            return *this;
        }

        Image temp(other);

        if (!other.isEmpty() && temp.isEmpty())
        {
            return *this;
        }

        Pixel* oldPixels = pixels;

        pixels = temp.pixels;
        imgWidth = temp.imgWidth;
        imgHeight = temp.imgHeight;

        temp.pixels = oldPixels;
        temp.imgWidth = 0;
        temp.imgHeight = 0;

        return *this;
    }

    ~Image()
    {
        delete[] pixels;
    }

    int width() const
    {
        return imgWidth;
    }

    int height() const
    {
        return imgHeight;
    }

    long long pixelCount() const
    {
        return (long long)imgWidth * imgHeight;
    }

    bool isEmpty() const
    {
        return pixels == NULL ||
            imgWidth <= 0 ||
            imgHeight <= 0;
    }

    Pixel& at(int x, int y)
    {
        return pixels[(long long)y * imgWidth + x];
    }

    const Pixel& at(int x, int y) const
    {
        return pixels[(long long)y * imgWidth + x];
    }

    bool load(const string& filename)
    {
        int width;
        int height;
        int channels;

        unsigned char* data = stbi_load(
            filename.c_str(),
            &width,
            &height,
            &channels,
            4
        );

        if (data == NULL)
        {
            cout << "Image loading failed: "
                << stbi_failure_reason() << endl;

            return false;
        }

        if (width <= 0 || height <= 0)
        {
            stbi_image_free(data);
            return false;
        }

        long long total = (long long)width * height;

        Pixel* temp = new Pixel[total];

        for (long long i = 0; i < total; i++)
        {
            temp[i].r = data[i * 4];
            temp[i].g = data[i * 4 + 1];
            temp[i].b = data[i * 4 + 2];
            temp[i].a = data[i * 4 + 3];
        }

        stbi_image_free(data);

        delete[] pixels;

        pixels = temp;
        imgWidth = width;
        imgHeight = height;

        return true;
    }

    bool save(const string& filename) const
    {
        if (isEmpty())
        {
            return false;
        }

        long long total = pixelCount();

        unsigned char* data =
            new unsigned char[total * 4];

        for (long long i = 0; i < total; i++)
        {
            data[i * 4] = pixels[i].r;
            data[i * 4 + 1] = pixels[i].g;
            data[i * 4 + 2] = pixels[i].b;
            data[i * 4 + 3] = pixels[i].a;
        }

        int result = 0;

        int dotPosition = -1;

        for (int i = (int)filename.length() - 1; i >= 0; i--)
        {
            if (filename[i] == '.')
            {
                dotPosition = i;
                break;
            }

            if (filename[i] == '/' || filename[i] == '\\')
            {
                break;
            }
        }

        string extension = "";

        if (dotPosition != -1)
        {
            extension = filename.substr(dotPosition);
        }

        for (int i = 0; i < (int)extension.length(); i++)
        {
            if (extension[i] >= 'A' && extension[i] <= 'Z')
            {
                extension[i] =
                    (char)(extension[i] - 'A' + 'a');
            }
        }

        if (extension == ".png")
        {
            result = stbi_write_png(
                filename.c_str(),
                imgWidth,
                imgHeight,
                4,
                data,
                imgWidth * 4
            );
        }
        else if (extension == ".jpg" ||
            extension == ".jpeg")
        {
            result = stbi_write_jpg(
                filename.c_str(),
                imgWidth,
                imgHeight,
                4,
                data,
                95
            );
        }

        delete[] data;

        return result != 0;
    }
};

#endif
