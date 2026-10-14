
#ifndef IMAGE_OPERATIONS_H
#define IMAGE_OPERATIONS_H

#include "Image.h"

class ImageOperations
{
private:
    static bool sameColor(const Pixel& p1, const Pixel& p2)
    {
        return p1.r == p2.r && // basically for checking if two Pixels have the same color
            p1.g == p2.g &&
            p1.b == p2.b &&
            p1.a == p2.a;
    }

    static unsigned char clamp(int value)
    {
        if (value < 0)
        {
            return 0;
        }

        if (value > 255)
        {
            return 255;
        }

        return (unsigned char)value;
    }

public:
    // 1. CROP that is the basically for croping the image
    static bool crop(
        const Image& source,
        int x,
        int y,
        int cropWidth,
        int cropHeight,
        Image& output)
    {
        if (source.isEmpty() ||
            x < 0 || y < 0 ||
            cropWidth <= 0 || cropHeight <= 0)
        {
            return false;
        }

        if (x > source.width() ||
            y > source.height() ||
            cropWidth > source.width() - x ||
            cropHeight > source.height() - y)
        {
            return false;
        }

        Image result(cropWidth, cropHeight);

        if (result.isEmpty())
        {
            return false;
        }

        for (int row = 0; row < cropHeight; row++)
        {
            for (int col = 0; col < cropWidth; col++)
            {           // copying the cropped part
                result.at(col, row) =
                    source.at(x + col, y + row);
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // 2. ROTATE CLOCKWISE
    // Accepts 90, 180 or 270 degrees. these are the values that is going to be accepted
    // alse would return false
    static bool rotate(
        const Image& source,
        int degrees,
        Image& output)
    {
        if (source.isEmpty())
        {
            return false;
        }

        if (degrees != 90 &&
            degrees != 180 &&
            degrees != 270)
        {
            return false;
        }

        int newWidth = source.width();
        int newHeight = source.height();

        if (degrees == 90 || degrees == 270)
        {
            newWidth = source.height();
            newHeight = source.width();
        }

        Image result(newWidth, newHeight);

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < source.height(); y++)
        {
            for (int x = 0; x < source.width(); x++)
            {
                if (degrees == 90)
                {
                    result.at(source.height() - 1 - y, x) =
                        source.at(x, y);
                }
                else if (degrees == 180)
                {
                    result.at(
                        source.width() - 1 - x,
                        source.height() - 1 - y
                    ) = source.at(x, y);
                }
                else
                {
                    result.at(y, source.width() - 1 - x) =
                        source.at(x, y);
                }
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // flp the image horizonally 
    static bool flipHorizontal(
        const Image& source,
        Image& output)
    {
        if (source.isEmpty())
        {
            return false;
        }

        Image result(source.width(), source.height());

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < source.height(); y++)
        {
            for (int x = 0; x < source.width(); x++)
            {
                result.at(source.width() - 1 - x, y) =
                    source.at(x, y);
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // flip the image vrticaly
    static bool flipVertical(
        const Image& source,
        Image& output)
    {
        if (source.isEmpty())
        {
            return false;
        }

        Image result(source.width(), source.height());

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < source.height(); y++)
        {
            for (int x = 0; x < source.width(); x++)
            {
                result.at(x, source.height() - 1 - y) =
                    source.at(x, y);
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // 5. BRIGHTNESS
    // Positive amount brightens; negative amount darkens.
    static bool changeBrightness(
        const Image& source,
        int amount,
        Image& output)
    {
        if (source.isEmpty())
        {
            return false;
        }

        Image result(source);

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < result.height(); y++)
        {
            for (int x = 0; x < result.width(); x++)
            {
                Pixel& p = result.at(x, y);

                p.r = clamp((int)p.r + amount);
                p.g = clamp((int)p.g + amount);
                p.b = clamp((int)p.b + amount);
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // 6. GRAYSCALE
    static bool grayscale(
        const Image& source,
        Image& output)
    {
        if (source.isEmpty())
        {
            return false;
        }

        Image result(source);

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < result.height(); y++)
        {
            for (int x = 0; x < result.width(); x++)
            {
                Pixel& p = result.at(x, y);

                unsigned char average =
                    (unsigned char)(
                        ((int)p.r + (int)p.g + (int)p.b) / 3
                        );

                p.r = average;
                p.g = average;
                p.b = average;
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // 7. RESIZE USING NEAREST-NEIGHBOR
    static bool resize(
        const Image& source,
        int newWidth,
        int newHeight,
        Image& output)
    {
        if (source.isEmpty() ||
            newWidth <= 0 || newHeight <= 0)
        {
            return false;
        }

        Image result(newWidth, newHeight);

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < newHeight; y++)
        {
            for (int x = 0; x < newWidth; x++)
            {
                int sourceX = (int)(
                    (long long)x * source.width() / newWidth
                    );

                int sourceY = (int)(
                    (long long)y * source.height() / newHeight
                    );

                result.at(x, y) = source.at(sourceX, sourceY);
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // 8. BOX BLUR
    // Radius 1 means a 3x3 neighborhood.
    static bool blur(
        const Image& source,
        int radius,
        Image& output)
    {
        if (source.isEmpty() || radius < 0)
        {
            return false;
        }

        if (radius == 0)
        {
            Image result(source);

            if (result.isEmpty())
            {
                return false;
            }

            output = result;
            return !output.isEmpty();
        }

        // Limit the radius to avoid integer overflow.
        if (radius > source.width())
        {
            radius = source.width();
        }

        if (radius > source.height())
        {
            radius = source.height();
        }

        Image result(source.width(), source.height());

        if (result.isEmpty())
        {
            return false;
        }

        for (int y = 0; y < source.height(); y++)
        {
            for (int x = 0; x < source.width(); x++)
            {
                long long red = 0;
                long long green = 0;
                long long blue = 0;
                long long alpha = 0;
                long long count = 0;

                int startX = x - radius;
                int endX = x + radius;
                int startY = y - radius;
                int endY = y + radius;

                if (startX < 0) startX = 0;
                if (startY < 0) startY = 0;

                if (endX >= source.width())
                    endX = source.width() - 1;

                if (endY >= source.height())
                    endY = source.height() - 1;

                for (int ny = startY; ny <= endY; ny++)
                {
                    for (int nx = startX; nx <= endX; nx++)
                    {
                        const Pixel& p = source.at(nx, ny);

                        red += p.r;
                        green += p.g;
                        blue += p.b;
                        alpha += p.a;
                        count++;
                    }
                }

                result.at(x, y) = Pixel(
                    (unsigned char)(red / count),
                    (unsigned char)(green / count),
                    (unsigned char)(blue / count),
                    (unsigned char)(alpha / count)
                );
            }
        }

        output = result;
        return !output.isEmpty();
    }

    // 9. BUCKET FILL USING AN ARRAY-BASED STACK
    // Replaces the connected region matching the starting pixel.
    static bool bucketFill(
        Image& image,
        int startX,
        int startY,
        const Pixel& replacement)
    {
        if (image.isEmpty() ||
            startX < 0 || startX >= image.width() ||
            startY < 0 || startY >= image.height())
        {
            return false;
        }

        Pixel target = image.at(startX, startY);

        if (sameColor(target, replacement))
        {
            return true;
        }

        long long total = image.pixelCount();

        int* stack = new int[total];

        long long top = 0;

        stack[top] = startY * image.width() + startX;
        top++;

        // Mark when pushed, so a pixel is pushed only once.
        image.at(startX, startY) = replacement;

        while (top > 0)
        {
            top--;

            int index = stack[top];
            int x = index % image.width();
            int y = index / image.width();

            // Left neighbor
            if (x > 0 &&
                sameColor(image.at(x - 1, y), target))
            {
                image.at(x - 1, y) = replacement;
                stack[top] = y * image.width() + x - 1;
                top++;
            }

            // Right neighbor
            if (x < image.width() - 1 &&
                sameColor(image.at(x + 1, y), target))
            {
                image.at(x + 1, y) = replacement;
                stack[top] = y * image.width() + x + 1;
                top++;
            }

            // Upper neighbor
            if (y > 0 &&
                sameColor(image.at(x, y - 1), target))
            {
                image.at(x, y - 1) = replacement;
                stack[top] = (y - 1) * image.width() + x;
                top++;
            }

            // Lower neighbor
            if (y < image.height() - 1 &&
                sameColor(image.at(x, y + 1), target))
            {
                image.at(x, y + 1) = replacement;
                stack[top] = (y + 1) * image.width() + x;
                top++;
            }
        }

        delete[] stack;
        return true;
    }
};

#endif
