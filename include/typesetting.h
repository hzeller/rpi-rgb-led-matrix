// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Very simple graphics library to do simple things.
//
// Might be useful to consider using Cairo instead and just have an interface
// between that and the Canvas. Well, this is a quick set of things to get
// started (and nicely self-contained).
#ifndef RPI_TYPESETTING_H
#define RPI_TYPESETTING_H

#include "canvas.h"

#include <stdint.h>
#include <stddef.h>

#include <map>

namespace rgb_matrix {

// Draw text, a standard NUL-terminated C-string encoded in UTF-8, with
// optimal line wrapping using an algorithm that minimizes
// raggedness and gaps at the ends of lines.
// 
// Draws the text with the given "font" at "x","y" (upper left corner) with
// "color". "color" must always be set (passed by reference).
// "background_color" is an optional pointer; if set to NULL, the background 
// remains transparent.
// 
// "kerning_offset" allows additional horizontal spacing between characters
// (can be negative).
// "leading" allows additional vertical spacing between lines (can be negative).
//
// Returns the height of the drawn paragraph in pixels
// (line height times number of lines).
int DrawTextWrapped(Canvas *c, const Font &font, int x, int y, int line_width,
                    const Color &color, const Color *background_color,
                    const char *utf8_text, int kerning_offset = 0, int leading = 0);

}  // namespace rgb_matrix

#endif  // RPI_TYPESETTING_H
