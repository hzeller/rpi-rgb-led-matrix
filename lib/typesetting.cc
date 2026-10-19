// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#include "graphics.h"
#include "utf8-internal.h"
#include "textwrap.h"

#include <vector>
#include <locale>
#include <stdio.h>

extern const uint32_t kUnicodeReplacementCodepoint;

using namespace textwrap;

namespace rgb_matrix {

namespace { // Entities within this block are only accessible within this .cpp file

// Helper class to determine word separators for text wrapping.
// Remembers the previous codepoint to allow for context-sensitive separation.
class CodepointChecker
{
private:
  uint32_t _previous_codepoint = 0;
public:
  // Returns true if the current codepoint is considered a separator.
  // This is used to determine word boundaries for wrapping.
  bool is_separator(uint32_t codepoint) {
    bool is_separator = false;

    if (codepoint == ' ') {
      is_separator = true;
    }

    if (_previous_codepoint == '-') {
      // Treats a character after a hyphen as a separator to allow hyphenation.
      // This enables line breaks after hyphens, similar to common typesetting.
      is_separator = true;
    }

    _previous_codepoint = codepoint;
    return is_separator;
  };
};

// Returns the width of a Unicode codepoint in the given font.
// Falls back to the replacement character if the codepoint is not present in the font.
int GetCharacterWidth(const Font &font, uint32_t unicode_codepoint) {
  static const uint32_t kUnicodeReplacementCharacterCodepoint = 0xFFFD;
  int width = font.CharacterWidth(unicode_codepoint);
  if (width == -1) {
    // If the character does not exist, font.DrawGlyph() will draw the
    // replacement character, making this width crucial
    width = font.CharacterWidth(kUnicodeReplacementCharacterCodepoint);
    if (width == -1) {
      // If there is no replacement character in the font
      width = 0;
    }
  }
  return width;
}

// Splits the input UTF-8 text into fragments (words) for optimal line breaking.
// Each fragment contains its width and whitespace width, which are used for wrapping.
// The kerning_offset is added between characters to account for spacing.
vector<Fragment> GetWords(const Font &font, int line_width,
                          const char *utf8_text, int kerning_offset) {
  vector<Fragment> fragments;
  int word_width = 0;
  CodepointChecker checker;

  while (true) {
    const uint32_t cp = utf8_next_codepoint(utf8_text);
    const int c_width = GetCharacterWidth(font, cp) + kerning_offset;

    // Check if adding the next character would cause the word to exceed the line width.
    const bool brake_word = word_width + c_width > line_width;

    if (checker.is_separator(cp) || brake_word || cp == '\0') {
      if (word_width == 0 && not fragments.empty()) {
        // No new word, just a space. Therefore, only increase the whitespace
        // width of the last fragment if there already is one stored.
        fragments.back().whitespace_width += c_width;
      } else {
        // Store word as a new fragment
        fragments.emplace_back(Fragment {
          .width = (double)word_width,
          .whitespace_width = (double)(cp == ' ' ? c_width : 0),
          .penalty_width = 0
        });
      }
      // Start a new word
      word_width = cp == ' ' ? 0 : c_width;
    } else {
      // Continue accumulating width for the current word.
      word_width += c_width;
    }
    if (cp == '\0') {
      break;
    }
  }

  return fragments;
}

// Draws the wrapped text line by line onto the canvas.
// Uses the line break information from the optimal-fit algorithm to position words.
// Returns the total height of the drawn text block.
int DrawTextLines(Canvas *c, const Font &font, int x, int y, int line_width,
                  const Color &color,const Color *background_color,
                  const char *utf8_text, int kerning_offset, int leading,
                  vector<size_t> &lines) {
  const int start_x = x;
  const int start_y = y;
  size_t word_index = 0;
  int word_width = 0;
  size_t line_index = 1;     // First break before second line
  y += font.height();        // Minimum height: one line
  CodepointChecker checker;

  while (*utf8_text) {
    uint32_t cp = utf8_next_codepoint(utf8_text);
    int c_width = GetCharacterWidth(font, cp) + kerning_offset;

    // Check if adding the next character would cause the word to exceed the line width.
    const bool brake_word = word_width + c_width > line_width;

    if (checker.is_separator(cp) || brake_word) {
      if (word_width == 0 && word_index != 0) {
        // No new word, just a space which does not increase the word width.
      } else {
        // New word, check for line break
        word_index++;
        if (word_index == lines[line_index]) {
          // Move to the next line if the current word index matches the break point.
          line_index++;
          y += font.height() + leading;
          x = start_x;
          if (cp == ' ') {
            // Don't draw spaces at the beginning of a line.
            // Therefore search for the first non-space character
            while (cp == ' ') cp = utf8_next_codepoint(utf8_text);
            checker.is_separator(cp); // Save this codepoint as previous codepoint for the next check
            c_width = GetCharacterWidth(font, cp) + kerning_offset;
          }
        }
        // Start a new word
        word_width = cp == ' ' ? 0 : c_width;
      }
    } else {
      // Continue accumulating width for the current word.
      word_width += c_width;
    }
    if (c != nullptr) {
      // Draw the glyph and advance the x position.
      x += font.DrawGlyph(c, x, y, color, background_color, cp);
      x += kerning_offset;
    }
  }

  return y - start_y;
}

} // namespace

// Draw text, a standard NUL-terminated C-string encoded in UTF-8, with
// optimal line wrappin.
//
// This function wraps the input text to fit within the specified line width,
// using an optimal-fit algorithm to minimize raggedness and gaps at the ends
// of lines. It supports kerning and custom line spacing (leading).
//
// The function first splits the text into fragments (words), then determines
// optimal line breaks, and finally draws each line onto the canvas.
//
// Returns the height of the drawn paragraph in pixels
// (line height times number of lines).
int DrawTextWrapped(Canvas *c, const Font &font, int x, int y, int line_width,
                    const Color &color, const Color *background_color,
                    const char *utf8_text, int kerning_offset = 0, int leading = 0) {
  // Extend line width by kerning_offset as this is inserted after each character,
  // but is only visible between characters and not at the end of the line.
  // In addition, there is usually a pixel gap between characters, which is
  // also not visible at the end of the line.
  line_width += kerning_offset + 1;

  // Split the text into fragments (words) for optimal wrapping.
  auto words = GetWords(font, line_width, utf8_text, kerning_offset);
  
  Penalties penalties;
  penalties.nline_penalty = 0;
  penalties.short_last_line_penalty = 0;
  // Compute optimal line breaks using the textwrap algorithm.
  auto lines = wrap_optimal_fit(words, line_width, penalties);

  // Draw the lines onto the canvas and return the total height.
  int height = DrawTextLines(c, font, x, y, line_width, color, background_color,
                             utf8_text, kerning_offset, leading, lines);

  return height;
}

} // namespace rgb_matrix
