// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "graphics.h"
#include "utf8-internal.h"

namespace rgb_matrix {
int DrawText(Canvas *c, const Font &font,
             int x, int y, const Color &color,
             const char *utf8_text) {
    const int start_x = x;
    while (*utf8_text) {
        uint8_t cp = utf8_next_codepoint(utf8_text);
        x += font.DrawGlyph(c, x, y, color, cp);
    }
    return x - start_x;
}
}
