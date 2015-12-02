// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
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

#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// The little question-mark box "�" for unknown code.
static const uint32_t kUnicodeReplacementCodepoint = 0xFFFD;

// Bitmap for one row. This limits the number of available columns.
// Make wider if running into trouble.
typedef uint32_t rowbitmap_t;

namespace rgb_matrix {
struct Font::Glyph {
  int width, height;
  int y_offset;
  rowbitmap_t bitmap[0];  // contains 'height' elements.
};

Font::Font() : font_height_(-1), base_line_(0) {}
Font::~Font() {
  for (CodepointGlyphMap::iterator it = glyphs_.begin();
       it != glyphs_.end(); ++it) {
    free(it->second);
  }
}

// TODO: that might not be working for all input files yet.
bool Font::LoadFont(const char *path) {
  if (!path || !*path) return false;
  FILE *f = fopen(path, "r");
  if (f == NULL)
    return false;
  uint32_t codepoint;
  char buffer[1024];
  int dummy;
  Glyph tmp;
  Glyph *current_glyph = NULL;
  int row = 0;
  int x_offset = 0;
  int bitmap_shift = 0;
  while (fgets(buffer, sizeof(buffer), f)) {
    if (sscanf(buffer, "FONTBOUNDINGBOX %d %d %d %d",
               &dummy, &font_height_, &dummy, &base_line_) == 4) {
      base_line_ += font_height_;
    }
    else if (sscanf(buffer, "ENCODING %ud", &codepoint) == 1) {
      // parsed.
    }
    else if (sscanf(buffer, "BBX %d %d %d %d", &tmp.width, &tmp.height,
                    &x_offset, &tmp.y_offset) == 4) {
      current_glyph = (Glyph*) malloc(sizeof(Glyph)
                                      + tmp.height * sizeof(rowbitmap_t));
      *current_glyph = tmp;
      // We only get number of bytes large enough holding our width. We want
      // it always left-aligned.
      bitmap_shift =
        8 * (sizeof(rowbitmap_t) - ((current_glyph->width + 7) / 8)) + x_offset;
      row = -1;  // let's not start yet, wait for BITMAP
    }
    else if (strncmp(buffer, "BITMAP", strlen("BITMAP")) == 0) {
      row = 0;
    }
    else if (current_glyph && row >= 0 && row < current_glyph->height
             && (sscanf(buffer, "%x", &current_glyph->bitmap[row]) == 1)) {
      current_glyph->bitmap[row] <<= bitmap_shift;
      row++;
    }
    else if (strncmp(buffer, "ENDCHAR", strlen("ENDCHAR")) == 0) {
      if (current_glyph && row == current_glyph->height) {
        free(glyphs_[codepoint]);  // just in case there was one.
        glyphs_[codepoint] = current_glyph;
        current_glyph = NULL;
      }
    }
  }
  fclose(f);
  return true;
}

const Font::Glyph *Font::FindGlyph(uint32_t unicode_codepoint) const {
  CodepointGlyphMap::const_iterator found = glyphs_.find(unicode_codepoint);
  if (found == glyphs_.end())
    return NULL;
  return found->second;
}

int Font::CharacterWidth(uint32_t unicode_codepoint) const {
  const Glyph *g = FindGlyph(unicode_codepoint);
  return g ? g->width : -1;
}

int Font::DrawGlyph(Canvas *c, int x_pos, int y_pos,
                    const Color &color, const Color *bgcolor,
                    uint32_t unicode_codepoint) const {
  const Glyph *g = FindGlyph(unicode_codepoint);
  if (g == NULL) g = FindGlyph(kUnicodeReplacementCodepoint);
  if (g == NULL) return 0;
  y_pos = y_pos - g->height - g->y_offset;
  for (int y = 0; y < g->height; ++y) {
    const rowbitmap_t row = g->bitmap[y];
    rowbitmap_t x_mask = 0x80000000;
    for (int x = 0; x < g->width; ++x, x_mask >>= 1) {
      if (row & x_mask) {
        c->SetPixel(x_pos + x, y_pos + y, color.r, color.g, color.b);
      } else if (bgcolor) {
          c->SetPixel(x_pos + x, y_pos + y, bgcolor->r, bgcolor->g, bgcolor->b);
      }
    }
  }
  return g->width;
}

int Font::DrawGlyph(Canvas *c, int x_pos, int y_pos, const Color &color,
                    uint32_t unicode_codepoint) const {
  return DrawGlyph(c, x_pos, y_pos, color, NULL, unicode_codepoint);
}

}  // namespace rgb_matrix
