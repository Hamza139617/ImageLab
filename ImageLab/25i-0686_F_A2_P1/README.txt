ImageLab - Phase 1 Core Image Operations
=======================================

Build environment
-----------------
- Windows 10/11
- Visual Studio with the Desktop development with C++ workload
- Windows SDK (GDI+ / gdiplus.lib)
- C++20, as configured in ImageLab.vcxproj

Build
-----
1. Open ImageLab.slnx in Visual Studio.
2. Select the ImageLab project and choose x64 / Debug.
3. Build > Build Solution.
4. Run the ImageLab project.

GDI+ is used only to decode/encode image files. The pixel transformations
(crop, rotate, flip, brightness, grayscale, resize, blur and bucket fill)
are implemented manually in ImageOperations.h.

Supported image formats
-----------------------
Input and output: PNG, JPG/JPEG and BMP.

Menu workflow
-------------
1. Load an image.
2. Run instant operations (crop, rotate, flip, brightness, grayscale).
3. Submit queued operations (resize, blur, bucket fill, save project, export).
4. Use List Pending Jobs, Process Next, or Process All to run queued work.
5. Use Undo / Redo and the Data Structure Inspector to inspect state.

The project working file is ImageLab_project.png in the program's working
directory. Export filenames should include .png, .jpg/.jpeg or .bmp.

Important notes
---------------
- The processing queue has a fixed capacity of 100 by default. When full,
  new jobs are rejected with a message.
- Undo/redo use full image snapshots, which is allowed for Phase 1.
- This implementation is not yet the complete assignment submission:
  the standalone PASS/FAIL data-structure tests, formal report, and Phase 2
  memory-aware history / priority scheduling / batching still need to be done.
- Compile and run in Visual Studio and test using your own sample images
  before submitting. This source has not been built in this environment.
