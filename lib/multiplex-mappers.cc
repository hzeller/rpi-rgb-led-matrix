// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2017 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "multiplex-mappers-internal.h"

namespace rgb_matrix {
namespace internal {
// A Pixel Mapper maps physical pixels locations to the internal logical
// mapping in a panel or panel-assembly, which depends on the wiring.
class MultiplexMapperBase : public MultiplexMapper {
public:
  MultiplexMapperBase(const char *name, int stretch_factor)
    : name_(name), panel_stretch_factor_(stretch_factor) {}

  // This method is const, but we sneakily remember the original size
  // of the panels so that we can more easily quantize things.
  // So technically, we're stateful, but let's pretend we're not changing
  // state. In the context this is used, it is never accessed in multiple
  // threads.
  virtual void EditColsRows(int *cols, int *rows) const {
    panel_rows_ = *rows;
    panel_cols_ = *cols;

    *rows /= panel_stretch_factor_;
    *cols *= panel_stretch_factor_;
  }

  virtual bool GetSizeMapping(int matrix_width, int matrix_height,
                              int *visible_width, int *visible_height) const {
    // Matrix width has been altered. Alter it back.
    *visible_width = matrix_width / panel_stretch_factor_;
    *visible_height = matrix_height * panel_stretch_factor_;
    return true;
  }

  virtual const char *GetName() const { return name_; }

  // The MapVisibleToMatrix() as required by PanelMatrix here does
  virtual void MapVisibleToMatrix(int matrix_width, int matrix_height,
                                  int visible_x, int visible_y,
                                  int *matrix_x, int *matrix_y) const {
    const int chained_panel  = visible_x / panel_cols_;
    const int parallel_panel = visible_y / panel_rows_;

    const int within_panel_x = visible_x % panel_cols_;
    const int within_panel_y = visible_y % panel_rows_;

    int new_x, new_y;
    MapSinglePanel(within_panel_x, within_panel_y, &new_x, &new_y);
    *matrix_x = chained_panel  * panel_stretch_factor_*panel_cols_ + new_x;
    *matrix_y = parallel_panel * panel_rows_/panel_stretch_factor_ + new_y;
  }

  // Map the coordinates for a single panel. This is to be overridden in
  // derived classes.
  virtual void MapSinglePanel(int visible_x, int visible_y,
                              int *matrix_x, int *matrix_y) const = 0;
protected:
  const char *const name_;
  const int panel_stretch_factor_;

  mutable int panel_cols_;
  mutable int panel_rows_;
};


/* ========================================================================
 * Multiplexer implementations.
 *
 * Extend MultiplexMapperBase and implement MapSinglePanel. You only have
 * to worry about the mapping within a single panel, the overall panel
 * construction with chains and parallel is already taken care of.
 *
 * Don't forget to register the new multiplexer sin CreateMultiplexMapperList()
 * below. After that, the new mapper is available in the --led-multiplexing
 * option.
 */
class StripeMultiplexMapper : public MultiplexMapperBase {
public:
  StripeMultiplexMapper() : MultiplexMapperBase("Stripe", 2) {}

  void MapSinglePanel(int x, int y, int *matrix_x, int *matrix_y) const {
    const bool is_top_stripe = (y % (panel_rows_/2)) < panel_rows_/4;
    *matrix_x = is_top_stripe ? x + panel_cols_ : x;
    *matrix_y = ((y / (panel_rows_/2)) * (panel_rows_/4)
                 + y % (panel_rows_/4));
  }
};

class CheckeredMultiplexMapper : public MultiplexMapperBase {
public:
  CheckeredMultiplexMapper() : MultiplexMapperBase("Checkered", 2) {}

  void MapSinglePanel(int x, int y, int *matrix_x, int *matrix_y) const {
    const bool is_top_check = (y % (panel_rows_/2)) < panel_rows_/4;
    const bool is_left_check = (x < panel_cols_/2);
    if (is_top_check) {
      *matrix_x = is_left_check ? x+panel_cols_/2 : x+panel_cols_;
    } else {
      *matrix_x = is_left_check ? x : x + panel_cols_/2;
    }
    *matrix_y = ((y / (panel_rows_/2)) * (panel_rows_/4)
                 + y % (panel_rows_/4));
  }
};

class SpiralMultiplexMapper : public MultiplexMapperBase {
public:
  SpiralMultiplexMapper() : MultiplexMapperBase("Spiral", 2) {}

  void MapSinglePanel(int x, int y, int *matrix_x, int *matrix_y) const {
    const bool is_top_stripe = (y % (panel_rows_/2)) < panel_rows_/4;
    const int panel_quarter = panel_cols_/4;
    const int quarter = x / panel_quarter;
    const int offset = x % panel_quarter;
    *matrix_x = ((2*quarter*panel_quarter)
                 + (is_top_stripe
                    ? panel_quarter - 1 - offset
                    : panel_quarter + offset));
    *matrix_y = ((y / (panel_rows_/2)) * (panel_rows_/4)
                 + y % (panel_rows_/4));
  }
};

class ZStripeMultiplexMapper : public MultiplexMapperBase {
public:
  ZStripeMultiplexMapper(const char *name, int even_vblock_offset, int odd_vblock_offset)
  : MultiplexMapperBase(name, 2),
    even_vblock_offset_(even_vblock_offset),
    odd_vblock_offset_(odd_vblock_offset) {}

  void MapSinglePanel(int x, int y, int *matrix_x, int *matrix_y) const {
    static const int tile_width = 8;
    static const int tile_height = 4;

    const int vert_block_is_odd = ((y / tile_height) % 2);

    const int even_vblock_shift = (1 - vert_block_is_odd) * even_vblock_offset_;
    const int odd_vblock_shitf = vert_block_is_odd * odd_vblock_offset_;

    *matrix_x = x + ((x + even_vblock_shift) / tile_width) * tile_width + odd_vblock_shitf;
    *matrix_y = (y % tile_height) + tile_height * (y / (tile_height * 2));
  }

private:
  const int even_vblock_offset_;
  const int odd_vblock_offset_;
};

class CoremanMapper : public MultiplexMapperBase {
public:
  CoremanMapper() : MultiplexMapperBase("coreman", 2) {}

  void MapSinglePanel(int x, int y, int *matrix_x, int *matrix_y) const {
    const bool is_left_check = (x < panel_cols_/2);

    if((y <= 7) || ((y >= 16) && (y <= 23))){
      *matrix_x = ((x / (panel_cols_/2)) * panel_cols_) + (x % (panel_cols_/2));
      if ((y & (panel_rows_/4)) == 0) {
        *matrix_y = (y / (panel_rows_/2)) * (panel_rows_/4) + (y % (panel_rows_/4));
      }
    } else {
      *matrix_x = is_left_check ? x + panel_cols_/2 : x + panel_cols_;
      *matrix_y = (y / (panel_rows_/2)) * (panel_rows_/4) + y % (panel_rows_/4);
    }
  }
};

/*
 * Here is where the registration happens.
 * If you add an instance of the mapper here, it will automatically be
 * made available in the --led-multiplexing commandline option.
 */
static MuxMapperList *CreateMultiplexMapperList() {
  MuxMapperList *result = new MuxMapperList();

  // Here, register all multiplex mappers from above.
  result->push_back(new StripeMultiplexMapper());
  result->push_back(new CheckeredMultiplexMapper());
  result->push_back(new SpiralMultiplexMapper());
  result->push_back(new ZStripeMultiplexMapper("ZStripe", 0, 8));
  result->push_back(new ZStripeMultiplexMapper("ZnMirrorZStripe", 4, 4));
  result->push_back(new CoremanMapper());

  return result;
}

const MuxMapperList &GetRegisteredMultiplexMappers() {
  static const MuxMapperList *all_mappers = CreateMultiplexMapperList();
  return *all_mappers;
}
}  // namespace internal
}  // namespace rgb_matrix
