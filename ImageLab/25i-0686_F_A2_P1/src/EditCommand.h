#ifndef EDIT_COMMAND_H
#define EDIT_COMMAND_H

#include <string>
#include "Image.h"

class EditCommand {
public:
	string operationName;
	string description;
	int param1;
	int param2;
	int param3;
	int param4;
	Pixel color;
	Image previousImage;
	string fileName;          
	string previousFileName;  

	EditCommand() {
		operationName = "";
		description = "";
		param1 = 0;
		param2 = 0;
		param3 = 0;
		param4 = 0;

	}
};

#endif