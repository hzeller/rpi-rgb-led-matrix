// -*- c++ -*-
// Controlling a 32x32 RGB matrix via GPIO.

#ifndef RPI_RGBMATRIX_H
#define RPI_RGBMATRIX_H

#include <stdint.h>
#include "gpio.h"

class RGBMatrix {
 public:
  RGBMatrix(GPIO *io);
  void ClearScreen();
  void FillScreen(uint8_t red, uint8_t green, uint8_t blue);
  void FillAll(uint8_t setze);

  void SetPixel(uint8_t x, uint8_t y,
                uint8_t red, uint8_t green, uint8_t blue);

  // Updates the screen once. Call this in a continous loop in some realtime
  // thread.
  void UpdateScreen();

  int columns() const { return kColumns; }

private:
  GPIO *const io_;

  enum {
    kDoubleRows = 16,     // Physical constant of the used board.
    kChainedBoards = 1,   // Number of boards that are daisy-chained.
    kColumns = kChainedBoards * 32,
    kPWMBits = 4          // maximum PWM resolution.
  };

  union IoBits {  //entspricht den GPIO-Nummern
    struct {
      unsigned int unused1 : 2;  // 0..1
      unsigned int output_enable : 1;  // 2
      unsigned int clock  : 1;   // 3
      unsigned int strobe : 1;   // 4 Bedeutet: Impuls
      unsigned int unused2 : 2;  // 5..6
      unsigned int row : 4;  // 7..10
      unsigned int unused3 : 6;  // 11..16
      unsigned int r1 : 1;   // 17
      unsigned int g1 : 1;   // 18
      unsigned int unused4 : 3;
      unsigned int b1 : 1;   // 22
      unsigned int r2 : 1;   // 23
      unsigned int g2 : 1;   // 24
      unsigned int b2 : 1;   // 25
    } bits;
    uint32_t raw;
    IoBits() : raw(0) {}
  };

  // A double row represents row n and n+16. The physical layout of the
  // 32x32 RGB is two sub-panels with 32 columns and 16 rows.
  struct DoubleRow {
    IoBits column[kColumns];  // only color bits are set
  };
  struct Screen {
    DoubleRow row[kDoubleRows];
  };

  Screen bitplane_[kPWMBits];

  uint8_t luminance_lut[255];
};

#endif  // RPI_RGBMATRIX_H
