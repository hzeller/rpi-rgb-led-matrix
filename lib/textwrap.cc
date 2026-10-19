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
#include <algorithm>
#include <cmath>

#include "textwrap.h"
#include "smawk.h"

namespace textwrap {

using std::isinf;

// Wrap abstract fragments into lines with an optimal-fit algorithm.
//
// The `target_width` gives the target line width for each line.
//
// The fragments must already have been split into the desired
// widths, this function will not (and cannot) attempt to split them
// further when arranging them into lines.
//
// # Optimal-Fit Algorithm
//
// The algorithm considers all possible break points and picks the
// breaks which minimizes the gaps at the end of each line. More
// precisely, the algorithm assigns a cost or penalty to each break
// point, determined by `cost = gap * gap` where `gap = target_width -
// line_width`. Shorter lines are thus penalized more heavily since
// they leave behind a larger gap.
//
// We can illustrate this with the text “To be, or not to be: that is
// the question”. We will be wrapping it in a narrow column with room
// for only 10 characters. If another word exceeds the line width, it
// is wrapped to the next line.:
//
// ```text
// "To be, or"   1² =  1
// "not to be:"  0² =  0
// "that is"     3² =  9
// "the"         7² = 49
// "question"    2² =  4
// ```
//
// We see that line four with “the” leaves a gap of 7 columns, which
// gives it a penalty of 49. The sum of the penalties is 63.
//
// There are 10 words, which means that there are `2_u32.pow(9)` or
// 512 different ways to typeset it. We can compute the sum of the
// penalties for each possible line break and search for the one with
// the lowest sum:
//
// ```text
// "To be,"     4² = 16
// "or not to"  1² =  1
// "be: that"   2² =  4
// "is the"     4² = 16
// "question"   2² =  4
// ```
//
// The sum of the penalties is 41, which is better than what the
// result of the simple wrapping in the example shown above.
//
// Searching through all possible combinations would normally be
// prohibitively slow. However, it turns out that the problem can be
// formulated as the task of finding column minima in a cost matrix.
// This matrix has a special form (totally monotone) which lets us
// use a [linear-time algorithm called
// SMAWK](https://en.wikipedia.org/wiki/SMAWK_algorithm) to find the
// optimal break points.
//
// This means that the time complexity remains O(_n_) where _n_ is
// the number of words.
//
// The optimization of per-line costs over the entire paragraph is
// inspired by the line breaking algorithm used in TeX, as described
// in the 1981 article [_Breaking Paragraphs into
// Lines_](http://www.eprg.org/G53DOC/pdfs/knuth-plass-breaking.pdf)
// by Knuth and Plass. The implementation here is based on [Rust code
// by Martin
// Geisler](https://github.com/mgeisler/textwrap/blob/master/src/wrap_algorithms/optimal_fit.rs).
//
// # Return Value
//
// This function returns a vector with the indexes of the first
// fragment of each line. The first fragment in the first line has
// the index 0, so the first index returned is always 0.
//
// # Errors
//
// In case of an overflow during the cost computation, an empty vector
// is returned. Overflows happens when fragments or lines have infinite
// widths or if the widths are so large that the square of a gap at the
// end of a line have infinite width.
//
vector<size_t> wrap_optimal_fit(
    vector<Fragment> &fragments,
    double target_width,
    Penalties &penalties
) {
  vector<double> widths;
  widths.reserve(fragments.size() + 1);
  double width = 0.0;
  widths.push_back(width);
  for (auto fragment : fragments) {
    width += fragment.width + fragment.whitespace_width;
    widths.push_back(width);
  }

  bool overflow_error = false;

  vector<size_t> minima = smawk::online_column_minima<double>(
    0,
    widths.size(),
    [&widths, &fragments, &penalties, target_width, &overflow_error](
      vector<double>& values, size_t i,size_t j
  ) {
    // Compute the width of a line spanning fragments[i..j] in
    // constant time. We need to adjust widths[j] by subtracting
    // the whitespace of fragment[j-1] and then add the penalty.
    double line_width = widths[j] - widths[i]
      - fragments[j - 1].whitespace_width
      + fragments[j - 1].penalty_width;

    // We compute cost of the line containing fragments[i..j].
    // We start with values[i], which is the optimal cost for
    // breaking before fragments[i].
    //
    // First, every extra line cost NLINE_PENALTY.
    double cost = values[i] + penalties.nline_penalty;

    // Next, we add a penalty depending on the line length.
    if (line_width > target_width) {
      // Lines that overflow get a hefty penalty.
      double overflow = line_width - target_width;
      cost += overflow * penalties.overflow_penalty;
    } else if (j < fragments.size()) {
      // Other lines (except for the last line) get a milder
      // penalty which depend on the size of the gap.
      double gap = target_width - line_width;
      cost += gap * gap;
    } else if (i + 1 == j
      && line_width < target_width / penalties.short_last_line_fraction)
    {
      // The last line can have any size gap, but we do add a
      // penalty if the line is very short (typically because
      // it contains just a single word).
      cost += penalties.short_last_line_penalty;
    }

    // Finally, we discourage hyphens.
    if (fragments[j - 1].penalty_width > 0.0) {
      cost += penalties.hyphen_penalty;
    }

    if (isinf(cost)) {
      overflow_error = true;
    }

    return cost;
  });

  // Handle overflow error 
  if (overflow_error) {
    // Return an empty vector to indicate that an overflow occurred
    // during the cost computation.
    vector<size_t> empty;
    return empty;
  }

  // Reconstruct the optimal line breaks by backtracking through the
  // 'minima' array. Starting from the end of the fragments, repeatedly
  // jump to the previous break position stored in 'minima[pos]' until
  // reaching the beginning (pos == 0).
  // This produces the sequence of line break indices in reverse order.
  vector<size_t> lines;
  size_t pos = fragments.size();
  while (pos > 0) {
    pos = minima[pos];
    lines.push_back(pos);
  }

  // Reverse to put line breaks in natural (forward) order.
  reverse(lines.begin(), lines.end());
  return lines;
}

} // namespace textwrap
