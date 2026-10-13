#include "ImageLabEditor.h"

#include <iostream>
#include <string>
#include <limits>

using namespace std;

void showMenu() {
    cout << "\n=============== ImageLab ===============\n"
         << " 1. Load image\n"
         << " 2. Crop (instant)\n"
         << " 3. Rotate 90 / 180 / 270 (instant)\n"
         << " 4. Flip horizontal / vertical (instant)\n"
         << " 5. Adjust brightness (instant)\n"
         << " 6. Convert to grayscale (instant)\n"
         << " 7. Queue resize\n"
         << " 8. Queue blur\n"
         << " 9. Queue bucket fill\n"
         << "10. Queue save project\n"
         << "11. Queue export\n"
         << "12. List pending jobs\n"
         << "13. Process next job\n"
         << "14. Process all jobs\n"
         << "15. Cancel a pending job\n"
         << "16. Clear pending jobs\n"
         << "17. Undo\n"
         << "18. Redo\n"
         << "19. Data Structure Inspector\n"
         << " 0. Exit\n"
         << "Choose: ";
}

bool readPixel(Pixel& colour) {
    int red, green, blue;
    cout << "Enter replacement colour as R G B (each 0-255): ";
    if (!(cin >> red >> green >> blue))
        return false;

    if (red < 0 || red > 255 || green < 0 || green > 255 ||
        blue < 0 || blue > 255) {
        cout << "RGB values must be between 0 and 255.\n";
        return false;
    }

    colour = Pixel(static_cast<unsigned char>(red),
                   static_cast<unsigned char>(green),
                   static_cast<unsigned char>(blue), 255);
    return true;
}

int main() {
    ImageLabEditor editor(100, "ImageLab_project.png");
    int choice = -1;

    cout << "ImageLab - manual pixel-array image editor\n";
    cout << "Supported files: PNG, JPG/JPEG, BMP\n";

    while (choice != 0) {
        showMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore((numeric_limits<streamsize>::max)(), '\n');
            cout << "Please enter a menu number.\n";
            continue;
        }

        int x, y, width, height, value;
        string fileName;
        Pixel colour;

        switch (choice) {
        case 1:
            cout << "Image filename: ";
            cin >> ws;
            getline(cin, fileName);
            editor.loadImage(fileName);
            break;

        case 2:
            cout << "Crop start x y, then crop width height: ";
            if (cin >> x >> y >> width >> height)
                editor.crop(x, y, width, height);
            else
                cout << "Invalid crop input.\n";
            break;

        case 3:
            cout << "Rotation angle (90, 180, 270): ";
            if (cin >> value)
                editor.rotate(value);
            else
                cout << "Invalid angle.\n";
            break;

        case 4:
            cout << "Flip direction (1 = horizontal, 2 = vertical): ";
            if (cin >> value && (value == 1 || value == 2))
                editor.flip(value == 1);
            else
                cout << "Choose 1 or 2.\n";
            break;

        case 5:
            cout << "Brightness adjustment (e.g. 30 or -30): ";
            if (cin >> value)
                editor.changeBrightness(value);
            else
                cout << "Invalid adjustment.\n";
            break;

        case 6:
            editor.convertToGrayscale();
            break;

        case 7:
            cout << "Target width and height: ";
            if (cin >> width >> height)
                editor.submitResize(width, height);
            else
                cout << "Invalid dimensions.\n";
            break;

        case 8:
            cout << "Blur radius (1-20): ";
            if (cin >> value)
                editor.submitBlur(value);
            else
                cout << "Invalid blur radius.\n";
            break;

        case 9:
            cout << "Starting pixel x y: ";
            if (!(cin >> x >> y)) {
                cout << "Invalid coordinates.\n";
                break;
            }
            if (!readPixel(colour))
                break;
            cout << "Fill implementation (1 = custom stack, 2 = custom queue): ";
            if (cin >> value && (value == 1 || value == 2))
                editor.submitBucketFill(x, y, colour, value == 2);
            else
                cout << "Choose 1 or 2.\n";
            break;

        case 10:
            editor.submitSaveProject();
            break;

        case 11:
            cout << "Export filename (include .png, .jpg, or .bmp): ";
            cin >> ws;
            getline(cin, fileName);
            editor.submitExport(fileName);
            break;

        case 12:
            editor.listJobs();
            break;

        case 13:
            editor.processNextJob();
            break;

        case 14:
            editor.processAllJobs();
            break;

        case 15:
            cout << "Job ID to cancel: ";
            if (cin >> value)
                editor.cancelJob(value);
            else
                cout << "Invalid job ID.\n";
            break;

        case 16:
            editor.clearJobs();
            break;

        case 17:
            editor.undo();
            break;

        case 18:
            editor.redo();
            break;

        case 19:
            editor.inspect();
            break;

        case 0:
            cout << "Exiting ImageLab.\n";
            break;

        default:
            cout << "Unknown option. Choose 0-19.\n";
            break;
        }
    }

    return 0;
}
