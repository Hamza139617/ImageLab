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


			cin >> option;

			switch (option) {
			case 1: {
				if (imageLoaded) {
					EditCommand cmd;
					cmd.previousImage = currImage;
					cmd.operationName = "load";
					cmd.description = prevImgName + " Image loaded";
					
					string name;
					cout << "Enter image relative path : ";
					cin >> name;

					bool result = currImage.load(prevImgName);

					if (result) {
					
						cout << endl << "Image loaded successfully" << endl;
						
						prevImgName = name;
						undoStack.push(cmd);  
						redoStack.initializeStack();
					}
					else { 
					
						cout << endl << "Image loaded not " << endl; 
						
					}

				}
			}
				break;

			case 2: {



			}
				  break;



			}

		}
	}


};

#endif