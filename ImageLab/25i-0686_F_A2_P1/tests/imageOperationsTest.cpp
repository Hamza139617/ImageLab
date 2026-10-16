
#include "../src/ImageOperations.h"
#include <iostream>

using namespace std;

int main()
{
    Image original;
    Image result;

    int testFailed = 9;
    int testPassed = 9;

    if (!original.load("25i-0686_F_A2_P1/images/bucket.png"))
    {
        cout << "Image loading failed." << endl;
        return 1;
    }

    testFailed--;

    cout << "Width: " << original.width() << endl;
    cout << "Height: " << original.height() << endl;

    if (ImageOperations::grayscale(original, result))
    {
        if (result.save("25i-0686_F_A2_P1/images/grayscale.png"))
        {
            cout << "Grayscale image saved." << endl;
            testFailed--;
        }
    }

    if (ImageOperations::rotate(original, 90, result))
    {
        result.save("25i-0686_F_A2_P1/images/rotate.png");
        testFailed--;
    }

    if (ImageOperations::resize(original, 200, 200, result))
    {
        result.save("25i-0686_F_A2_P1/images/resize.png");
        testFailed--;
    }

    if (ImageOperations::blur(original, 20, result)) {
        result.save("25i-0686_F_A2_P1/images/blur.png");
        testFailed--;
    }

    if (ImageOperations::changeBrightness(original, 20, result)) {
        result.save("25i-0686_F_A2_P1/images/brightness.png");
        testFailed--;
    }

    if (ImageOperations::flipHorizontal(original, result)) {
        result.save("25i-0686_F_A2_P1/images/fhorizontal.png");
        testFailed--;
    }

    if (ImageOperations::flipVertical(original, result)) {
        result.save("25i-0686_F_A2_P1/images/fvertical.png");
        testFailed--;
    }

    if (ImageOperations::crop(original, 20, 20, 20, 20, result)) {
        result.save("25i-0686_F_A2_P1/images/crop.png");
        cout << "Passed";
        testFailed--;
    }

    cout << endl << "===================================================" << endl;
    cout << "Test Passed : " << testPassed - testFailed << endl;
    cout << "Test Failed : " << testFailed << endl;


    return 0;
}
