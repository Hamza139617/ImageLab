#ifndef IMAGE_EDITOR_H
#define IMAGE_EDITOR_H

#include <string>
#include "EditCommand.h"
#include "Image.h"
#include "ImageOperations.h"
#include "queueType.h"
#include "stackType.h"

#include <iostream>
using namespace std;

class imageEditor {
private:
	Image currImage;
	stackType<EditCommand> undoStack;
	stackType<EditCommand> redoStack;
	string prevImgName;
	bool editorOpened;
	bool imageLoaded;
	int option;

	// repeadet steps so mad it a function
	void recordEdit(EditCommand& cmd, Image& latest) {
		cmd.previousImage = currImage;   // image before the operation (needed for undo)
		currImage = latest;
		undoStack.push(cmd);
		redoStack.initializeStack();     // a new operation clears the redo stack
	}

		// basically for the redo operation like when a spefic operation to be redod
	bool applyCommand(const EditCommand& cmd, const Image& source, Image& output) {

		if (cmd.operationName == "load") {
			Image loaded;
			if (!loaded.load(cmd.fileName)) return false;
			output = loaded;
			return true;
		}
		if (cmd.operationName == "crop")
			return ImageOperations::crop(source, cmd.param1, cmd.param2, cmd.param3, cmd.param4, output);
		if (cmd.operationName == "rotate")
			return ImageOperations::rotate(source, cmd.param1, output);
		if (cmd.operationName == "flipH")
			return ImageOperations::flipHorizontal(source, output);
		if (cmd.operationName == "flipV")
			return ImageOperations::flipVertical(source, output);
		if (cmd.operationName == "brightness")
			return ImageOperations::changeBrightness(source, cmd.param1, output);
		if (cmd.operationName == "grayscale")
			return ImageOperations::grayscale(source, output);
		if (cmd.operationName == "resize")
			return ImageOperations::resize(source, cmd.param1, cmd.param2, output);
		if (cmd.operationName == "blur")
			return ImageOperations::blur(source, cmd.param1, output);
		if (cmd.operationName == "bucketFill") {
			Image filled = source;
			bool done;
			if (cmd.param3 == 2)
				done = ImageOperations::bucketFillQueue(filled, cmd.param1, cmd.param2, cmd.color);
			else
				done = ImageOperations::bucketFillStack(filled, cmd.param1, cmd.param2, cmd.color);
			if (!done) return false;
			output = filled;
			return true;
		}

		return false;
	}

public:

	imageEditor() {
		editorOpened = true;
		imageLoaded = false;
		option = -1;

	}

	void showMenu() {

		while (editorOpened) {

			cout << endl << "========================================================================";
			cout << endl;

			cout << "Enter the operation you want to perform : ";

			cout << endl;

			cout << "1. Load Image.(Instant)\n2. Crop Image. (Instant)\n3. Rotate Image. (Instant)\n4. Flip Image.(Instant)\n5. Change Brightness.(Instant)\n";
			cout << "6. Apply Grayscale.(Instant)\n7. Resize Image.(Queued)\n8. Blur Image. (Queued)\n9. Bucket Fill (Queued)\n10. Save Image (Queued).\n11. Export Image. (Queued)\n12. Exit. (Instant)\n";
			cout << "13. Undo. (Instant)\n14. Redo. (Instant)\n";   

			//  if a non-number is typed, clear the error, otherwise the loop never stops
			if (!(cin >> option)) {
				cin.clear();
				cin.ignore(10000, '\n');
				option = -1;
			}

			switch (option) {
			case 1: {

				string name;                                  
				cout << "Enter image relative path : ";
				cin >> name;

				EditCommand cmd;
				cmd.previousImage = currImage;
				cmd.operationName = "load";
				cmd.fileName = name;                          
				cmd.previousFileName = prevImgName;           
				cmd.description = "load(" + name + ")";       

				bool result = currImage.load(name);            // basically taking the image info 

				if (result) {

					cout << endl << "Image loaded successfully" << endl;

					if (imageLoaded) {                        
						undoStack.push(cmd);
						redoStack.initializeStack();
					}

					imageLoaded = true;                      
					prevImgName = name;
				}
				else {

					cout << endl << "Image loaded not " << endl;

				}

			}
				  break;

			case 2: {

				if (imageLoaded) {

					int x, y, crpWidth, crpHeight;
					cout << endl << "Enter starting x , y , crop width and crop height ";
					cin >> x >> y >> crpWidth >> crpHeight;
					Image latest;
					bool result = ImageOperations::crop(currImage, x, y, crpWidth, crpHeight, latest);

					if (result) {
						cout << endl << "image cropped successfully ";
						EditCommand cmd;
						cmd.description = "cropped image";
						cmd.operationName = "crop";
						cmd.param1 = x;
						cmd.param2 = y;
						cmd.param3 = crpWidth;
						cmd.param4 = crpHeight;
						recordEdit(cmd, latest);             
					}
					else {
						cout << endl << "Invalid crop region" << endl;  
					}

				}
				else {
					cout << endl << "Load an image first" << endl;     
				}
			}
				  break;

			case 3: {

				if (imageLoaded) {

					int degrees;
					cout << endl << "Enter rotation angle (90, 180 or 270) ";
					cin >> degrees;
					Image latest;
					bool result = ImageOperations::rotate(currImage, degrees, latest);

					if (result) {
						cout << endl << "image rotated successfully ";
						EditCommand cmd;
						cmd.description = "rotated image";
						cmd.operationName = "rotate";
						cmd.param1 = degrees;
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Invalid angle" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 4: {

				if (imageLoaded) {

					int direction;
					cout << endl << "Enter 1 for horizontal flip or 2 for vertical flip ";
					cin >> direction;
					Image latest;		// this is the info section where info is going to be taking of the command 
					EditCommand cmd;	// and then application section
					bool result = false;

					if (direction == 1) {
						result = ImageOperations::flipHorizontal(currImage, latest);
						cmd.description = "flipped image horizontally";
						cmd.operationName = "flipH";
					}
					else if (direction == 2) {
						result = ImageOperations::flipVertical(currImage, latest);
						cmd.description = "flipped image vertically";
						cmd.operationName = "flipV";
					}

					if (result) {
						cout << endl << "image flipped successfully ";
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Invalid direction" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 5: {

				if (imageLoaded) {

					int amount;
					cout << endl << "Enter brightness value (positive or negative) ";
					cin >> amount;
					Image latest;
					bool result = ImageOperations::changeBrightness(currImage, amount, latest);

					if (result) {
						cout << endl << "brightness changed successfully ";
						EditCommand cmd;
						cmd.description = "changed brightness";
						cmd.operationName = "brightness";
						cmd.param1 = amount;
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Brightness change failed" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 6: {

				if (imageLoaded) {

					Image latest;
					bool result = ImageOperations::grayscale(currImage, latest);

					if (result) {
						cout << endl << "grayscale applied successfully ";
						EditCommand cmd;
						cmd.description = "applied grayscale";
						cmd.operationName = "grayscale";
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Grayscale failed" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 7: {

				
				if (imageLoaded) {

					int newWidth, newHeight;
					cout << endl << "Enter new width and new height ";
					cin >> newWidth >> newHeight;
					Image latest;
					bool result = ImageOperations::resize(currImage, newWidth, newHeight, latest);

					if (result) {
						cout << endl << "image resized successfully ";
						EditCommand cmd;
						cmd.description = "resized image";
						cmd.operationName = "resize";
						cmd.param1 = newWidth;
						cmd.param2 = newHeight;
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Invalid size" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 8: {

				
				if (imageLoaded) {

					int radius;
					cout << endl << "Enter blur radius ";
					cin >> radius;
					Image latest;
					bool result = ImageOperations::blur(currImage, radius, latest);

					if (result) {
						cout << endl << "image blurred successfully ";
						EditCommand cmd;
						cmd.description = "blurred image";
						cmd.operationName = "blur";
						cmd.param1 = radius;
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Invalid radius" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 9: {

			
				if (imageLoaded) {

					int x, y, red, green, blue;
					cout << endl << "Enter starting x , y and the replacement colour (red green blue, 0 to 255) ";
					cin >> x >> y >> red >> green >> blue;

					if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
						cout << endl << "Colour values must be between 0 and 255" << endl;
						break;
					}

					//Image latest = currImage;   // bucket fill changes the image it receives, so work on a copy
					//Pixel color((unsigned char)red, (unsigned char)green, (unsigned char)blue);
					//bool result = ImageOperations::bucketFill(latest, x, y, color);

					int mode;
					cout << endl << "pressing 1 for stack based fill or 2 for queue based fill ";
					cin >> mode;

					if (mode != 1 && mode != 2) {
						cout << endl << "Invalid fill type" << endl;
						break;
					}

					Image latest = currImage;
					Pixel color((unsigned char)red, (unsigned char)green, (unsigned char)blue);

					bool result;
					if (mode == 1)
						result = ImageOperations::bucketFillStack(latest, x, y, color);
					else
						result = ImageOperations::bucketFillQueue(latest, x, y, color);

					if (result) {
						cout << endl << "bucket fill applied successfully ";
						EditCommand cmd;
						cmd.description = (mode == 1) ?  "bucket fill (stack)" : "bucket fill (queue)";
						cmd.operationName = "bucketFill";
						cmd.param1 = x;
						cmd.param2 = y;
						cmd.param3 = mode; // also for storing the type of mode information which is going to be used for the undo
						cmd.color = color;
						recordEdit(cmd, latest);
					}
					else {
						cout << endl << "Invalid starting pixel" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 10: {

				
				if (imageLoaded) {

					// saves into the file that is currently loaded (the project's working file)
					if (currImage.save(prevImgName)) {
						cout << endl << "Image saved successfully" << endl;
					}
					else {
						cout << endl << "Image save failed" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				   break;

			case 11: {

			
				if (imageLoaded) {

					string exportName;
					cout << endl << "Enter new file name (.png or .jpg) ";
					cin >> exportName;

					if (currImage.save(exportName)) {
						cout << endl << "Image exported successfully" << endl;
					}
					else {
						cout << endl << "Image export failed" << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				   break;

			case 12: {
				editorOpened = false;
				cout << endl << "Exiting the editor" << endl;
			}
				   break;

			case 13: {   // undo part when selected then it will undo

				if (imageLoaded) {

					if (undoStack.isEmptyStack()) {
						cout << endl << "Nothing to undo" << endl;
					}
					else {
						EditCommand cmd = undoStack.top();
						undoStack.pop();

						currImage = cmd.previousImage;        
						if (cmd.operationName == "load")
							prevImgName = cmd.previousFileName;

						redoStack.push(cmd);
						cout << endl << "Undone : " << cmd.description << endl;
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				   break;

			case 14: {   

				if (imageLoaded) {

					if (redoStack.isEmptyStack()) {
						cout << endl << "Nothing to redo" << endl;
					}
					else {
						EditCommand cmd = redoStack.top();
						Image result;

						// currImage is the same as cmd.previousImage here, so the operation is applied again
						if (applyCommand(cmd, currImage, result)) {
							redoStack.pop();

							currImage = result;
							if (cmd.operationName == "load")
								prevImgName = cmd.fileName;

							undoStack.push(cmd);
							cout << endl << "Redone : " << cmd.description << endl;
						}
						else {
							cout << endl << "Redo failed" << endl;
						}
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				   break;

			default:   
				cout << endl << "Invalid option" << endl;
				break;

			}

		}
	}


};

#endif