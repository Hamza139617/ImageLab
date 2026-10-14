
#include "ImageOperations.h"
#include <iostream>

using namespace std;

int main()
{
    Image original;
    Image result;

    if (!original.load("25i-0686_F_A2_P1/images/input.png"))
    {
        cout << "Image loading failed." << endl;
        return 1;
    }

    cout << "Width: " << original.width() << endl;
    cout << "Height: " << original.height() << endl;

    if (ImageOperations::grayscale(original, result))
    {
        if (result.save("25i-0686_F_A2_P1/images/grayscale.png"))
        {
            cout << "Grayscale image saved." << endl;
        }
    }

    if (ImageOperations::rotate(original, 90, result))
    {
        result.save("25i-0686_F_A2_P1/images/rotate.png");
    }

    if (ImageOperations::resize(original, 200, 200, result))
    {
        result.save("25i-0686_F_A2_P1/images/resize.png");
    }

    return 0;
}
