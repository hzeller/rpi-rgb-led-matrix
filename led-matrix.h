// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Controlling a 32x32 RGB matrix via GPIO.

#ifndef RPI_RGBMATRIX_H
#define RPI_RGBMATRIX_H

#include <stdint.h>
#include "gpio.h"

// The RGB matrix provides the framebuffer and the facilities to constantly
// update the LED matrix.
class RGBMatrix {
public:
  RGBMatrix(GPIO *io);
  ~RGBMatrix();

  void ClearScreen();
  void FillScreen(uint8_t red, uint8_t green, uint8_t blue);

  inline int width() const { return kColumns; }
  inline int height() const { return kDisplayRows; }
  void SetPixel(uint8_t x, uint8_t y,
                uint8_t red, uint8_t green, uint8_t blue);

private:
  class UpdateThread;
  friend class UpdateThread;

  // Updates the screen, connected to the GPIO pins, once.
  // Call this in a continous loop in some realtime
  // thread.
  void UpdateScreen();

  GPIO *const io_;
  UpdateThread *updater_;

  // Configuration settings.
  enum {
    // Displays typically come in (rows x columns) = 16x32 or 32x32
    // configuration. Set number of rows in your display here, so 16 or 32.
    kDisplayRows = 32,

    // You can chain the output of one board to the input of the next board.
    // That increases the total number of columns you can display. With only
    // one board, this is 1.
    kChainedBoards = 1,

    // Maximum PWM resolution. The number of gray-values you can display
    // per color is 2^kPWMBits, so 4 -> 16 gray values (= 16*16*16 = 4096 colors)
    // Higher values up to 7 are possible, but things get sluggish with slower
    // refresh rate.
    kPWMBits = 4,

    // Usually, the display is divided in two parts, so the 16x32 display are
    // actually 2x8 rows that are filled in parallel; the 32x32 board are 2x16
    // rows. This is usually given in the datasheet as 1:8 or 1:16 multiplexing.
    // These 'double rows' are what this is.
    //
    // Sometimes, 16x32 boards actually might not have 2x8, but 1x16, which
    // means 1:16 multiplexing for that as well. In that case, remove the '/ 2'.
    // Rarely needed.
    kDoubleRows = kDisplayRows / 2,  // Calculated constant.

    // Calculated constants that you probably don't want to change.
    kColumns = kChainedBoards * 32,
    kRowMask = kDoubleRows - 1
  };

  union IoBits {
    struct {
      // These reflect the GPIO mapping.
      unsigned int unused1 : 2;  // 0..1
      unsigned int output_enable : 1;  // 2
      unsigned int clock  : 1;   // 3
      unsigned int strobe : 1;   // 4
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
};
#endif  // RPI_RGBMATRIX_H
