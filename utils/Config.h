// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Matrix configuration parsing class declaration.
// Author: Tony DiCola
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#include "GridTransformer.h"
#include "led-matrix.h"

class Config {
public:
  Config(rgb_matrix::RGBMatrix::Options *options,
         const std::string& filename);

  // Attribute accessors:
  int getDisplayWidth() const {
    return (_display_width < 0)
      ? getPanelWidth() * getChainLength()
      : _display_width;
  }
  int getDisplayHeight() const {
    return (_display_height < 0)
      ? getPanelHeight() * getParallelCount()
      : _display_height;
  }
  int getPanelWidth() const {
    return (_panel_width) < 0 ? 32 : _panel_width;
  }
  int getPanelHeight() const {
    return _moptions->rows;
  }
  int getChainLength() const {
    return _chain_length;
  }
  int getParallelCount() const {
    return _moptions->parallel;
  }
  bool hasTransformer() const { return !_panels.empty(); }
  GridTransformer getGridTransformer() const {
    return GridTransformer(getDisplayWidth(), getDisplayHeight(),
                           getPanelWidth(), getPanelHeight(),
                           getChainLength(), _panels);
  }

private:
  rgb_matrix::RGBMatrix::Options* const _moptions;
  int _display_width,
      _display_height,
      _panel_width,
      _chain_length;
  std::vector<GridTransformer::Panel> _panels;
};

#endif
