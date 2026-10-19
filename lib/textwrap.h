// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// C++ Implementation of a Optimal-Fit Text Wrapping Algorithm
//
// The text wrapping algorithm in `wrap_optimal_fit` considers all
// possible break points and picks the breaks which minimizes the gaps
// at the end of each line. More precisely, the algorithm assigns a
// cost or penalty to each break point, determined by `cost = gap * gap`
// where `gap = target_width - line_width`. Shorter lines are thus
// penalized more heavily since they leave behind a larger gap.
//
// Searching through all possible combinations would normally be
// prohibitively slow. We therefore use a linear-time algorithm called
// SMAWK (see https://en.wikipedia.org/wiki/SMAWK_algorithm) to find the
// optimal break points.
//
// This code is a C++ port of the Rust implementation from
// https://github.com/mgeisler/textwrap/blob/master/src/wrap_algorithms/optimal_fit.rs
// by Martin Geisler released under the MIT License:
//
// Copyright (c) 2016 Martin Geisler
//
// The following MIT License applies to the relevant portions of this file:

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to 
// deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software. 
//
// This modified version is part of a project licensed under the GNU General
// Public License 

#include <vector>
#include <cstddef>

namespace textwrap {

using std::vector;
using std::size_t;

// Penalties for `wrap_optimal_fit`.
//
// This wrapping algorithm in `wrap_optimal_fit` considers the
// entire paragraph to find optimal line breaks. When wrapping text,
// "penalties" are assigned to line breaks based on the gaps left at
// the end of lines. The constructor assigning penalties that work
// well for monospace text.
//
// If you are wrapping proportional text, you are advised to assign
// your own penalties according to your font size. See the individual
// penalties below for details.
//
struct Penalties {
  // Per-line penalty. This is added for every line, which makes it
  // expensive to output more lines than the minimum required.
  size_t nline_penalty;

  // Per-character cost for lines that overflow the target line width.
  //
  // With a default value of 50², every single character costs as
  // much as leaving a gap of 50 characters behind. This is because
  // we assign as cost of `gap * gap` to a short line. When
  // wrapping monospace text, we can overflow the line by 1
  // character in extreme cases.
  //
  // This only happens if the overflowing word is 50 characters
  // long _and_ if the word overflows the line by exactly one
  // character. If it overflows by more than one character, the
  // overflow penalty will quickly outgrow the cost of the gap, as
  // seen above.
  size_t overflow_penalty;

  // When should a single word on the last line be considered
  // "too short"?
  //
  // If the last line of the text consists of a single word and if
  // this word is shorter than `1 / short_last_line_fraction` of
  // the line width, then the final line will be considered "short"
  // and `short_last_line_penalty` is added as an extra penalty.
  //
  // The effect of this is to avoid a final line consisting of a
  // single small word. For example, with a
  // `short_last_line_penalty` of 25 (the default), a gap of up to
  // 5 columns will be seen as more desirable than having a final
  // short line.
  size_t short_last_line_fraction;

  // Penalty for a last line with a single short word.
  //
  // Set this to zero if you do not want to penalize short last lines.
  size_t short_last_line_penalty;

  // Penalty for lines ending with a hyphen.
  size_t hyphen_penalty;

  // Default penalties for monospace text.
  //
  // The penalties here work well for monospace text. This is
  // because they expect the gaps at the end of lines to be roughly
  // in the range `0..100`. If the gaps are larger, the
  // `overflow_penalty` and `hyphen_penalty` become insignificant.
  Penalties() :
    nline_penalty(1000),
    overflow_penalty(50 * 50),
    short_last_line_fraction(4),
    short_last_line_penalty(25),
    hyphen_penalty(25) {
  };
};

// A (text) fragment denotes the unit to be wrapped into lines.
//
// Fragments represent an abstract _word_ plus the _whitespace_
// following the word. In case the word falls at the end of the line,
// the whitespace is dropped and a so-called _penalty_ is inserted
// instead (typically `"-"` if the word was hyphenated).
//
// For wrapping purposes, the precise content of the word, the
// whitespace, and the penalty is irrelevant. All we need to know is
// the displayed width of each part, which this struct provides.
struct Fragment {
    // Displayed width of word represented by this fragment.
    double width;

    // Displayed width of the whitespace that must follow the word
    // when the word is not at the end of a line.
    double whitespace_width;

    // Displayed width of the penalty that must be inserted if the
    // word falls at the end of a line.
    double penalty_width;
};

// Wrap abstract fragments into lines with an optimal-fit algorithm.
//
// The `target_width` gives the target line width for each line.
//
// The fragments must already have been split into the desired
// widths, this function will not (and cannot) attempt to split them
// further when arranging them into lines.
vector<size_t> wrap_optimal_fit(
    vector<Fragment> &fragments,
    double target_width,
    Penalties &penalties
);

}
