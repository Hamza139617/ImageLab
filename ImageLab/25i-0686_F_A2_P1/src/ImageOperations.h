#ifndef IMAGELAB_IMAGE_OPERATIONS_H
#define IMAGELAB_IMAGE_OPERATIONS_H

#include "Image.h"
#include "stackType.h"
#include "queueType.h"

#include <iostream>
#include <string>
#include <limits>

namespace ImageOperations {

inline unsigned char clampChannel(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<unsigned char>(value);
}

inline bool crop(Image& image, int x, int y, int cropWidth, int cropHeight) {
    if (image.isEmpty() || x < 0 || y < 0 || cropWidth <= 0 || cropHeight <= 0 ||
        x > image.width() - cropWidth || y > image.height() - cropHeight) {
        std::cerr << "Invalid crop rectangle.\n";
        return false;
    }

    Image result;
    if (!result.create(cropWidth, cropHeight))
        return false;

    for (int row = 0; row < cropHeight; ++row)
        for (int col = 0; col < cropWidth; ++col)
            result.at(col, row) = image.at(x + col, y + row);

    image = result;
    return true;
}

inline bool rotate(Image& image, int degrees) {
    if (image.isEmpty() || (degrees != 90 && degrees != 180 && degrees != 270)) {
        std::cerr << "Rotation must be 90, 180, or 270 degrees.\n";
        return false;
    }

    const int oldWidth = image.width();
    const int oldHeight = image.height();
    const int newWidth = (degrees == 180) ? oldWidth : oldHeight;
    const int newHeight = (degrees == 180) ? oldHeight : oldWidth;

    Image result;
    if (!result.create(newWidth, newHeight))
        return false;

    for (int y = 0; y < oldHeight; ++y) {
        for (int x = 0; x < oldWidth; ++x) {
            if (degrees == 90)
                result.at(oldHeight - 1 - y, x) = image.at(x, y);
            else if (degrees == 180)
                result.at(oldWidth - 1 - x, oldHeight - 1 - y) = image.at(x, y);
            else
                result.at(y, oldWidth - 1 - x) = image.at(x, y);
        }
    }

    image = result;
    return true;
}

inline bool flip(Image& image, bool horizontal) {
    if (image.isEmpty()) {
        std::cerr << "Load an image first.\n";
        return false;
    }

    const int width = image.width();
    const int height = image.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width / 2; ++x) {
            const int otherX = width - 1 - x;
            Pixel temp = image.at(x, y);
            image.at(x, y) = image.at(otherX, y);
            image.at(otherX, y) = temp;
        }
    }

    if (!horizontal) {
        for (int y = 0; y < height / 2; ++y) {
            const int otherY = height - 1 - y;
            for (int x = 0; x < width; ++x) {
                Pixel temp = image.at(x, y);
                image.at(x, y) = image.at(x, otherY);
                image.at(x, otherY) = temp;
            }
        }
    }

    return true;
}

inline bool brightness(Image& image, int adjustment) {
    if (image.isEmpty()) {
        std::cerr << "Load an image first.\n";
        return false;
    }

    const int total = image.pixelCount();
    for (int i = 0; i < total; ++i) {
        Pixel& p = image.data()[i];
        p.r = clampChannel(static_cast<int>(p.r) + adjustment);
        p.g = clampChannel(static_cast<int>(p.g) + adjustment);
        p.b = clampChannel(static_cast<int>(p.b) + adjustment);
    }
    return true;
}

inline bool grayscale(Image& image) {
    if (image.isEmpty()) {
        std::cerr << "Load an image first.\n";
        return false;
    }

    const int total = image.pixelCount();
    for (int i = 0; i < total; ++i) {
        Pixel& p = image.data()[i];
        const unsigned char gray = static_cast<unsigned char>(
            (static_cast<int>(p.r) + p.g + p.b) / 3);
        p.r = gray;
        p.g = gray;
        p.b = gray;
    }
    return true;
}

inline bool resize(Image& image, int newWidth, int newHeight) {
    if (image.isEmpty() || newWidth <= 0 || newHeight <= 0 ||
        newWidth > (std::numeric_limits<int>::max)() / newHeight) {
        std::cerr << "Invalid target dimensions.\n";
        return false;
    }

    Image result;
    if (!result.create(newWidth, newHeight))
        return false;

    const int oldWidth = image.width();
    const int oldHeight = image.height();

    for (int y = 0; y < newHeight; ++y) {
        int sourceY = static_cast<int>((static_cast<long long>(y) * oldHeight) / newHeight);
        if (sourceY >= oldHeight) sourceY = oldHeight - 1;

        for (int x = 0; x < newWidth; ++x) {
            int sourceX = static_cast<int>((static_cast<long long>(x) * oldWidth) / newWidth);
            if (sourceX >= oldWidth) sourceX = oldWidth - 1;
            result.at(x, y) = image.at(sourceX, sourceY);
        }
    }

    image = result;
    return true;
}

// A box blur. radius=1 means a 3x3 neighbourhood. Border pixels use only
// the neighbours that are actually inside the image.
inline bool blur(Image& image, int radius) {
    if (image.isEmpty() || radius < 1 || radius > 20) {
        std::cerr << "Blur radius must be between 1 and 20.\n";
        return false;
    }

    Image result;
    if (!result.create(image.width(), image.height()))
        return false;

    const int width = image.width();
    const int height = image.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            long long red = 0, green = 0, blue = 0, alpha = 0;
            int samples = 0;

            const int minY = (y - radius < 0) ? 0 : y - radius;
            const int maxY = (y + radius >= height) ? height - 1 : y + radius;
            const int minX = (x - radius < 0) ? 0 : x - radius;
            const int maxX = (x + radius >= width) ? width - 1 : x + radius;

            for (int ny = minY; ny <= maxY; ++ny) {
                for (int nx = minX; nx <= maxX; ++nx) {
                    const Pixel& p = image.at(nx, ny);
                    red += p.r;
                    green += p.g;
                    blue += p.b;
                    alpha += p.a;
                    ++samples;
                }
            }

            result.at(x, y) = Pixel(
                static_cast<unsigned char>(red / samples),
                static_cast<unsigned char>(green / samples),
                static_cast<unsigned char>(blue / samples),
                static_cast<unsigned char>(alpha / samples));
        }
    }

    image = result;
    return true;
}

// Iterative fill using the student's custom stack. The visited array ensures
// each pixel is added at most once; recursion is deliberately not used.
inline bool bucketFillStack(Image& image, int startX, int startY,
                            const Pixel& replacement) {
    if (!image.contains(startX, startY)) {
        std::cerr << "Bucket-fill coordinates are outside the image.\n";
        return false;
    }

    const Pixel original = image.at(startX, startY);
    if (original == replacement)
        return true;

    const int total = image.pixelCount();
    bool* visited = new (std::nothrow) bool[total];
    if (visited == nullptr) {
        std::cerr << "Not enough memory for bucket fill.\n";
        return false;
    }
    for (int i = 0; i < total; ++i) visited[i] = false;

    stackType<int> pending;
    pending.push(startY * image.width() + startX);

    while (!pending.isEmptyStack()) {
        const int index = pending.top();
        pending.pop();

        if (index < 0 || index >= total || visited[index])
            continue;

        visited[index] = true;
        const int x = index % image.width();
        const int y = index / image.width();

        if (image.at(x, y) != original)
            continue;

        image.at(x, y) = replacement;

        if (y > 0 && !visited[index - image.width()] &&
            image.at(x, y - 1) == original)
            pending.push(index - image.width());
        if (y + 1 < image.height() && !visited[index + image.width()] &&
            image.at(x, y + 1) == original)
            pending.push(index + image.width());
        if (x > 0 && !visited[index - 1] && image.at(x - 1, y) == original)
            pending.push(index - 1);
        if (x + 1 < image.width() && !visited[index + 1] &&
            image.at(x + 1, y) == original)
            pending.push(index + 1);
    }

    delete[] visited;
    return true;
}

// Iterative fill using the student's circular queue. The capacity equals the
// number of pixels; visited prevents duplicate enqueues from exceeding it.
inline bool bucketFillQueue(Image& image, int startX, int startY,
                            const Pixel& replacement) {
    if (!image.contains(startX, startY)) {
        std::cerr << "Bucket-fill coordinates are outside the image.\n";
        return false;
    }

    const Pixel original = image.at(startX, startY);
    if (original == replacement)
        return true;

    const int total = image.pixelCount();
    bool* visited = new (std::nothrow) bool[total];
    if (visited == nullptr) {
        std::cerr << "Not enough memory for bucket fill.\n";
        return false;
    }
    for (int i = 0; i < total; ++i) visited[i] = false;

    queueType<int> pending(total > 0 ? total : 1);
    const int start = startY * image.width() + startX;
    pending.addQueue(start);
    visited[start] = true;

    while (!pending.isEmptyQueue()) {
        const int index = pending.front();
        pending.deleteQueue();

        const int x = index % image.width();
        const int y = index / image.width();
        if (image.at(x, y) != original)
            continue;

        image.at(x, y) = replacement;

        const int neighbours[4] = {
            y > 0 ? index - image.width() : -1,
            y + 1 < image.height() ? index + image.width() : -1,
            x > 0 ? index - 1 : -1,
            x + 1 < image.width() ? index + 1 : -1
        };

        for (int i = 0; i < 4; ++i) {
            const int next = neighbours[i];
            if (next < 0 || next >= total || visited[next])
                continue;

            const int nx = next % image.width();
            const int ny = next / image.width();
            if (image.at(nx, ny) == original) {
                visited[next] = true;
                pending.addQueue(next);
            }
        }
    }

    delete[] visited;
    return true;
}

} // namespace ImageOperations

#endif
