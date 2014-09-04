// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Utility base class for continuously updating the matrix.
#ifndef RPI_THREADED_MATRIX_MANIPULATOR_H
#define RPI_THREADED_MATRIX_MANIPULATOR_H

#include "thread.h"

//
// Typically, your programs will crate a matrix and then updating the image
// in a loop. If you want to do stuff in parallel, then this utility class
// helps you doing that. Also a demo for how to use the Thread class.
//
// Extend it, then just implement Run(). Example:
/*
  class MyCrazyDemo : public ThreadedMatrixManipulator {
  public:
    MyCrazyDemo(RGBMatrix *m) : ThreadedMatrixManipulator(m) {}
    virtual void Run() {
      unsigned char c;
      while (running_) {
          // Calculate the next frame.
          c++;
          for (int x = 0; x < matrix_->width(); ++x) {
            for (int y = 0; y < matrix_->height(); ++y) {
              matrix_->SetPixel(x, y, c, c, c);
            }
          }
          usleep(15 * 1000);
      }
    }
  };

  // Later, in your main method.
  RGBMatrix matrix(&gpio);
  MyCrazyDemo *demo = new MyCrazyDemo(&matrix);
  demo->Start();   // Start doing things.
  // This now runs in the background, you can do other things here,
  // e.g. aquiring new data or simply wait.
  demo->Stop();
  delete demo;
*/
class ThreadedMatrixManipulator : public Thread {
public:
  ThreadedMatrixManipulator(RGBMatrix *m) : running_(true), matrix_(m) {}
  virtual ~ThreadedMatrixManipulator() {  Stop(); }

  // Stop the thread at the next possible time Run() checks the running_ flag.
  void Stop() { running_ = false; }

  // Implement this and run while running_ is true. In other words: return from
  // this method once running_ becomes false.
  virtual void Run() = 0;

protected:
  volatile bool running_;  // TODO: use mutex, but this is good enough for now.
  RGBMatrix *const matrix_;
};

#endif  // RPI_THREADED_MATRIX_MANIPULATOR_H
