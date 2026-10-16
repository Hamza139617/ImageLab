#ifndef IMAGE_EDITOR_H
#define IMAGE_EDITOR_H

#include <string>
#include "EditCommand.h"
#include "Image.h"
#include "ImageOperations.h"
#include "queueType.h"
#include "stackType.h"
#include "Job.h"

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
	queueType<Job> jobQueue;
	int nextJobId;

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

	void performUndo() {

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

	//  redo one step (used by the menu and by a redo job)
	void performRedo() {

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

	//  puts a job at the rear of the queue, a filled queue is going to be rejecting the job
	bool addJob(Job& job) {

		if (jobQueue.isFullQueue()) {
			cout << endl << "Job queue is full, process the pending jobs first" << endl;
			return false;
		}

		job.jobId = nextJobId;
		nextJobId++;
		jobQueue.addQueue(job);

		cout << endl << "Job #" << job.jobId << " added : " << job.description << endl;
		return true;
	}

	//  true boolean value iff if at least one job in the queue is not cancelled
	bool hasPendingJobs() {

		for (int i = 0; i < jobQueue.size(); i++) {
			if (!jobQueue.getAt(i)->cancelled)
				return true;
		}

		return false;
	}

	//  runs one job, an edit is recorded in the undo stack here (when it is executed, not when it is added)
	void executeJob(const Job& job) {

		cout << endl << "Processing job #" << job.jobId << " : " << job.description << endl;

		if (job.operationName == "undo") {
			performUndo();
			return;
		}

		if (job.operationName == "redo") {
			performRedo();
			return;
		}

		if (job.operationName == "save") {
			// saves into the file that is currently loaded (the project's working file)
			if (currImage.save(prevImgName))
				cout << "Job #" << job.jobId << " finished : image saved" << endl;
			else
				cout << "Job #" << job.jobId << " failed : image save failed" << endl;	// the job # is going to be showing the job id for debugging purpose s
			return;
		}

		if (job.operationName == "export") {
			if (currImage.save(job.fileName))
				cout << "Job #" << job.jobId << " finished : image exported" << endl;
			else
				cout << "Job #" << job.jobId << " failed : image export failed" << endl;
			return;
		}

		// resize, blur and bucket fill change the image
		EditCommand cmd;
		cmd.operationName = job.operationName;
		cmd.description = job.description;
		cmd.param1 = job.param1;
		cmd.param2 = job.param2;
		cmd.param3 = job.param3;
		cmd.param4 = job.param4;
		cmd.color = job.color;

		Image latest;

		if (applyCommand(cmd, currImage, latest)) {
			recordEdit(cmd, latest);
			cout << "Job #" << job.jobId << " finished" << endl;
		}
		else {
			cout << "Job #" << job.jobId << " failed : invalid parameters for the current image" << endl;
		}
	}

	//  taking the job at the front and runs it, cancelled jobs are skipped
	bool processNextJob() {

		while (!jobQueue.isEmptyQueue()) {

			Job job = jobQueue.front();
			jobQueue.deleteQueue();

			if (job.cancelled) {
				cout << endl << "Skipped cancelled job #" << job.jobId << endl;
				continue;
			}

			executeJob(job);
			return true;
		}

		cout << endl << "No pending jobs" << endl;
		return false;
	}

public:

	imageEditor(int jobQueueSize = 10) : jobQueue(jobQueueSize) {   //  the queue capacity can be set
		editorOpened = true;
		imageLoaded = false;
		option = -1;
		nextJobId = 1;   // ADDED

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
			cout << "15. List Pending Jobs.\n 16. Process Next Job.\n 17. Process all jobs. \n 18. Canel Job. \n 19 Clear All Jobs. \n 20. Set Job Queue Capacity. \n";
			cout << "21. Data Structure Inspector .\n";

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

					//  the resize is added as a job, it rns when the job is processed
					Job job;
					job.operationName = "resize";
					job.param1 = newWidth;
					job.param2 = newHeight;
					job.description = "resize (" + to_string(newWidth) + " x " + to_string(newHeight) + ")";
					addJob(job);

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

					//  added it as a job
					Job job;
					job.operationName = "blur";
					job.param1 = radius;
					job.description = "blur (radius " + to_string(radius) + ")";
					addJob(job);

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

					int mode;
					cout << endl << "Enter 1 for stack based fill or 2 for queue based fill ";
					cin >> mode;

					if (mode != 1 && mode != 2) {
						cout << endl << "Invalid fill type" << endl;
						break;
					}

					// added it as a job
					Job job;
					job.operationName = "bucketFill";
					job.param1 = x;
					job.param2 = y;
					job.param3 = mode;
					job.color = Pixel((unsigned char)red, (unsigned char)green, (unsigned char)blue);
					job.description = string("bucket fill (") + (mode == 1 ? "stack" : "queue") + ") at (" + to_string(x) + "," + to_string(y) + ")";
					addJob(job);

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				  break;

			case 10: {

				if (imageLoaded) {

					//  added as a job, the image is saved when the job is processed
					Job job;
					job.operationName = "save";
					job.description = "save image";
					addJob(job);

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

					//  added as a job
					// the queued ones were added as a job because it will help us in differentiation of the pending ones and processed ones and queed ones
					Job job;
					job.operationName = "export";
					job.fileName = exportName;
					job.description = "export to " + exportName;
					addJob(job);

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

			case 13: {   // undo

				if (imageLoaded) {

					//  with pending jobs the undo request waits in the queue
					if (hasPendingJobs()) {
						Job job;
						job.operationName = "undo";
						job.description = "undo request";
						addJob(job);
					}
					else {
						performUndo();
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				   break;

			case 14: {   // redo

				if (imageLoaded) {

					//  with pending jobs the redo request waits in the queue
					if (hasPendingJobs()) {
						Job job;
						job.operationName = "redo";
						job.description = "redo request";
						addJob(job);
					}
					else {
						performRedo();
					}

				}
				else {
					cout << endl << "Load an image first" << endl;
				}
			}
				   break;

			case 15: {   // list the pending jobs without removing them

				if (jobQueue.isEmptyQueue()) {
					cout << endl << "No pending jobs" << endl;
				}
				else {
					cout << endl << "Pending jobs (front to rear) :" << endl;

					for (int i = 0; i < jobQueue.size(); i++) {
						const Job* job = jobQueue.peekAt(i);

						cout << i + 1 << ". Job #" << job->jobId << " : " << job->description;
						if (job->cancelled)
							cout << " (cancelled)";
						cout << endl;
					}
				}
			}
				   break;

			case 16: {   // process the next job

				processNextJob();
			}
				   break;

			case 17: {   // process all pending jobs

				if (jobQueue.isEmptyQueue()) {
					cout << endl << "No pending jobs" << endl;
				}
				else {
					while (!jobQueue.isEmptyQueue())
						processNextJob();

					cout << endl << "All pending jobs processed" << endl;
				}
			}
				   break;

			case 18: {   // cancl a job, it is only marked, the queue is not shifted

				int id;
				cout << endl << "Enter the id of the job to cancel ";
				cin >> id;

				bool found = false;

				for (int i = 0; i < jobQueue.size() && !found; i++) {
					Job* job = jobQueue.getAt(i);

					if (job->jobId == id) {
						found = true;

						if (job->cancelled) {
							cout << endl << "Job #" << id << " is already cancelled" << endl;
						}
						else {
							job->cancelled = true;
							cout << endl << "Job #" << id << " cancelled" << endl;
						}
					}
				}

				if (!found)
					cout << endl << "No pending job with that id" << endl;
			}
				   break;

			case 19: {   // for cleareance of al the pending jobs

				int removed = jobQueue.size();
				jobQueue.initializeQueue();
				cout << endl << "All pending jobs cleared (" << removed << " removed)" << endl;
			}
				   break;

			case 20: {   //  change the queue capacity, only possible when the queue is empty

				if (!jobQueue.isEmptyQueue()) {
					cout << endl << "Cannot change capacity while jobs are pending" << endl;
				}
				else {
					int newSize;
					cout << endl << "Enter the new job queue capacity ";
					cin >> newSize;

					if (newSize < 1 || newSize > 100000) {
						cout << endl << "Invalid capacity" << endl;
					}
					else {
						jobQueue = queueType<Job>(newSize);
						cout << endl << "Job queue capacity set to " << newSize << endl;
					}
				}
			}
				   break;

			case 21: {
				
				cout << "Undo stack size : " << undoStack.size() << endl;
				cout << "Redo stack size : " << redoStack.size() << endl;
				cout << "Current queue size : " << jobQueue.capacity() << endl;
				cout << "Max recorded queue size : " << jobQueue.getMaxRecordedSize() << endl;
				cout << "Undo stack push count : " << undoStack.getPushCount() << endl;
				cout << "Undo stack pop count : " << undoStack.getPopCount() << endl;
				cout << "Redo stack push count : " << redoStack.getPushCount() << endl;
				cout << "Redo stack pop count : " << redoStack.getPopCount() << endl;
				cout << "Queue addQueue count : " << jobQueue.getAddCount() << endl;
				cout << "Queue deleteQueue count : " << jobQueue.getDeleteCount() << endl;
				cout << "Undo stack content : \n";

				// now for the turn of undo stack contents
				for (int i = 0; i < undoStack.size(); i++) {
					const EditCommand* temp = undoStack.peekAt(i);
					cout << "Operation Name : " << temp->operationName << endl;
				}

				cout << "Redo stack content : \n ";

				for (int i = 0; i < redoStack.size(); i++) {
					const EditCommand* temp = redoStack.peekAt(i);
					cout << "Operation Name : " << temp->operationName << endl;
				}

				cout << "Job queue content : " << endl;

				for (int i = 0; i < jobQueue.size(); i++) {
					 Job* temp = jobQueue.getAt(i);
					 cout << "Operation Name : " << temp->operationName << endl;
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