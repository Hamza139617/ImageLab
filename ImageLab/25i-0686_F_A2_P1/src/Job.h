#ifndef JOB_H
#define JOB_H

#include <string>
#include "Image.h"

class Job   // basic structure of the job
{// assisting the queue and ant the pending jobs
public:
    int jobId;
    string operationName;
    string description;
    int param1;
    int param2;
    int param3;
    int param4;
    Pixel color;
    string fileName;
    bool cancelled;

    Job()
    {
        jobId = 0;
        operationName = "";
        description = "";
        param1 = 0;
        param2 = 0;
        param3 = 0;
        param4 = 0;
        fileName = "";
        cancelled = false;
    }
};

#endif