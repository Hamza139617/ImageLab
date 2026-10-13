#ifndef IMAGELAB_EDITOR_H
#define IMAGELAB_EDITOR_H

#include "ImageOperations.h"
#include <iostream>
#include <string>

enum class JobType {
    Resize,
    Blur,
    BucketFillStack,
    BucketFillQueue,
    SaveProject,
    Export
};

struct ProcessingJob {
    int id;
    JobType type;
    int first;
    int second;
    int third;
    int fourth;
    Pixel colour;
    std::string fileName;
    bool cancelled;

    ProcessingJob()
        : id(0), type(JobType::Resize), first(0), second(0), third(0),
          fourth(0), colour(), fileName(), cancelled(false) {
    }
};

inline const char* jobTypeName(JobType type) {
    switch (type) {
    case JobType::Resize: return "Resize";
    case JobType::Blur: return "Blur";
    case JobType::BucketFillStack: return "Bucket Fill (Stack)";
    case JobType::BucketFillQueue: return "Bucket Fill (Queue)";
    case JobType::SaveProject: return "Save Project";
    case JobType::Export: return "Export";
    default: return "Unknown";
    }
}

class ImageLabEditor {
private:
    Image image;
    stackType<Image> undoStack;
    stackType<Image> redoStack;
    queueType<ProcessingJob> jobs;
    int nextJobId;
    std::string projectFileName;

    void clearRedo() {
        redoStack.initializeStack();
    }

    bool applyJob(const ProcessingJob& job) {
        switch (job.type) {
        case JobType::Resize:
            return ImageOperations::resize(image, job.first, job.second);
        case JobType::Blur:
            return ImageOperations::blur(image, job.first);
        case JobType::BucketFillStack:
            return ImageOperations::bucketFillStack(image, job.first, job.second, job.colour);
        case JobType::BucketFillQueue:
            return ImageOperations::bucketFillQueue(image, job.first, job.second, job.colour);
        case JobType::SaveProject:
            return image.save(projectFileName);
        case JobType::Export:
            return image.save(job.fileName);
        default:
            return false;
        }
    }

    bool isImageChanging(JobType type) const {
        return type == JobType::Resize || type == JobType::Blur ||
               type == JobType::BucketFillStack || type == JobType::BucketFillQueue;
    }

public:
    explicit ImageLabEditor(int queueCapacity = 100,
                            const std::string& projectPath = "ImageLab_project.png")
        : image(), undoStack(), redoStack(),
          jobs(queueCapacity > 0 ? queueCapacity : 100),
          nextJobId(1), projectFileName(projectPath) {
    }

    bool hasImage() const {
        return !image.isEmpty();
    }

    const Image& currentImage() const {
        return image;
    }

    bool loadImage(const std::string& fileName) {
        Image loaded;
        if (!loaded.load(fileName))
            return false;

        image = loaded;
        undoStack.initializeStack();
        redoStack.initializeStack();
        std::cout << "Loaded " << image.width() << " x " << image.height() << " image.\n";
        return true;
    }

    bool crop(int x, int y, int width, int height) {
        if (image.isEmpty()) {
            std::cout << "Load an image first.\n";
            return false;
        }
        Image before = image;
        if (!ImageOperations::crop(image, x, y, width, height))
            return false;
        undoStack.push(before);
        clearRedo();
        return true;
    }

    bool rotate(int degrees) {
        if (image.isEmpty()) {
            std::cout << "Load an image first.\n";
            return false;
        }
        Image before = image;
        if (!ImageOperations::rotate(image, degrees))
            return false;
        undoStack.push(before);
        clearRedo();
        return true;
    }

    bool flip(bool horizontal) {
        if (image.isEmpty()) {
            std::cout << "Load an image first.\n";
            return false;
        }
        Image before = image;
        if (!ImageOperations::flip(image, horizontal))
            return false;
        undoStack.push(before);
        clearRedo();
        return true;
    }

    bool changeBrightness(int adjustment) {
        if (image.isEmpty()) {
            std::cout << "Load an image first.\n";
            return false;
        }
        Image before = image;
        if (!ImageOperations::brightness(image, adjustment))
            return false;
        undoStack.push(before);
        clearRedo();
        return true;
    }

    bool convertToGrayscale() {
        if (image.isEmpty()) {
            std::cout << "Load an image first.\n";
            return false;
        }
        Image before = image;
        if (!ImageOperations::grayscale(image))
            return false;
        undoStack.push(before);
        clearRedo();
        return true;
    }

    bool undo() {
        if (undoStack.isEmptyStack()) {
            std::cout << "Nothing to undo.\n";
            return false;
        }
        redoStack.push(image);
        image = undoStack.top();
        undoStack.pop();
        return true;
    }

    bool redo() {
        if (redoStack.isEmptyStack()) {
            std::cout << "Nothing to redo.\n";
            return false;
        }
        undoStack.push(image);
        image = redoStack.top();
        redoStack.pop();
        return true;
    }

    bool submitResize(int width, int height) {
        ProcessingJob job;
        job.type = JobType::Resize;
        job.first = width;
        job.second = height;
        return enqueue(job);
    }

    bool submitBlur(int radius) {
        ProcessingJob job;
        job.type = JobType::Blur;
        job.first = radius;
        return enqueue(job);
    }

    bool submitBucketFill(int x, int y, const Pixel& colour, bool useQueue) {
        ProcessingJob job;
        job.type = useQueue ? JobType::BucketFillQueue : JobType::BucketFillStack;
        job.first = x;
        job.second = y;
        job.colour = colour;
        return enqueue(job);
    }

    bool submitSaveProject() {
        ProcessingJob job;
        job.type = JobType::SaveProject;
        return enqueue(job);
    }

    bool submitExport(const std::string& fileName) {
        if (fileName.empty()) {
            std::cout << "Filename cannot be empty.\n";
            return false;
        }
        ProcessingJob job;
        job.type = JobType::Export;
        job.fileName = fileName;
        return enqueue(job);
    }

    bool enqueue(ProcessingJob job) {
        if (!hasImage() && job.type != JobType::SaveProject && job.type != JobType::Export) {
            std::cout << "Load an image before submitting image operations.\n";
            return false;
        }
        if (jobs.isFullQueue()) {
            std::cout << "Processing queue is full. Process or cancel a job first.\n";
            return false;
        }
        job.id = nextJobId++;
        job.cancelled = false;
        jobs.addQueue(job);
        std::cout << "Added job #" << job.id << ": " << jobTypeName(job.type) << '\n';
        return true;
    }

    void listJobs() const {
        if (jobs.isEmptyQueue()) {
            std::cout << "No pending jobs.\n";
            return;
        }
        std::cout << "\nPending jobs (front to rear):\n";
        for (int i = 0; i < jobs.size(); ++i) {
            const ProcessingJob* job = jobs.peekAt(i);
            if (job != nullptr)
                std::cout << "  #" << job->id << "  " << jobTypeName(job->type)
                          << (job->cancelled ? " [CANCELLED]" : "") << '\n';
        }
    }

    bool cancelJob(int id) {
        for (int i = 0; i < jobs.size(); ++i) {
            ProcessingJob* current = jobs.peekAt(i);
            if (current != nullptr && current->id == id && !current->cancelled) {
                // Keep the job in the circular queue and mark it. It will be
                // skipped when it reaches the front, without shifting elements.
                current->cancelled = true;
                std::cout << "Marked job #" << id << " as cancelled.\n";
                return true;
            }
        }

        std::cout << "No pending job with ID #" << id << ".\n";
        return false;
    }

    void clearJobs() {
        jobs.initializeQueue();
        std::cout << "Pending jobs cleared.\n";
    }

    bool processNextJob() {
        if (jobs.isEmptyQueue()) {
            std::cout << "No pending jobs to process.\n";
            return false;
        }

        ProcessingJob job = jobs.front();
        jobs.deleteQueue();

        if (job.cancelled) {
            std::cout << "Skipped cancelled job #" << job.id << ".\n";
            return true;
        }

        Image before;
        const bool changesImage = isImageChanging(job.type);
        if (changesImage)
            before = image;

        const bool success = applyJob(job);
        if (success && changesImage) {
            undoStack.push(before);
            clearRedo();
        }

        std::cout << (success ? "Completed " : "Failed ")
                  << "job #" << job.id << ": " << jobTypeName(job.type) << ".\n";
        return success;
    }

    void processAllJobs() {
        while (!jobs.isEmptyQueue())
            processNextJob();
    }

    void inspect() const {
        std::cout << "\n========== DATA STRUCTURE INSPECTOR ==========\n";
        std::cout << "Image: ";
        if (image.isEmpty())
            std::cout << "none loaded\n";
        else
            std::cout << image.width() << " x " << image.height()
                      << " (" << image.pixelCount() << " pixels)\n";

        std::cout << "Undo stack size: " << undoStack.size() << '\n';
        std::cout << "Undo snapshots (top first): ";
        if (undoStack.isEmptyStack()) std::cout << "(empty)";
        for (int i = 0; i < undoStack.size(); ++i) {
            const Image* snapshot = undoStack.peekAt(i);
            if (snapshot) std::cout << snapshot->width() << 'x' << snapshot->height() << ' ';
        }
        std::cout << "\nRedo stack size: " << redoStack.size() << '\n';
        std::cout << "Redo snapshots (top first): ";
        if (redoStack.isEmptyStack()) std::cout << "(empty)";
        for (int i = 0; i < redoStack.size(); ++i) {
            const Image* snapshot = redoStack.peekAt(i);
            if (snapshot) std::cout << snapshot->width() << 'x' << snapshot->height() << ' ';
        }

        std::cout << "\nProcessing queue size/capacity: "
                  << jobs.size() << '/' << jobs.capacity() << '\n';
        std::cout << "Queue adds/deletes: " << jobs.getAddCount()
                  << '/' << jobs.getDeleteCount() << '\n';
        std::cout << "Maximum recorded queue size: " << jobs.getMaxRecordedSize() << '\n';
        listJobs();
        std::cout << "==============================================\n";
    }
};

#endif
