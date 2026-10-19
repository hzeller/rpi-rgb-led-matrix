// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// C++ implementation of the SMAWK algorithm (see
// https://en.wikipedia.org/wiki/SMAWK_algorithm) for finding row  or column
// minima in a totally monotone matrix with *m* rows and *n* columns in
// time O(*m* + *n*). This is much better than the brute force solution 
// which would take O(*mn*). When *m* and *n* are of the same order, this
// turns a quadratic function into a linear function.
// 
// This code is a C++ port of the Rust implementation from
// https://github.com/mgeisler/smawk/ by Martin Geisler released under the
// MIT License:
//
// Copyright (c) 2017 Martin Geisler
//
// The following MIT License applies to the relevant portions of this file:
//
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
// Public License version 2 (GPLv2). See the file LICENSE for the full GPLv2
// license text.

#include <functional>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <iostream>

namespace smawk {

using std::vector;
using std::function;
using std::unordered_map;

// Compute column minima in the given area of the matrix. The
// `minima` vector is updated inplace.
template <typename T>
void smawk_inner(
  const vector<size_t>& rows,
  const vector<size_t>& cols,
  const function<T(size_t, size_t)>& matrix,
  vector<size_t>* minima
) {
  if (cols.size() == 0) {
    return; // No columns left, end recursion.
  }

  // Create reduced matrix onsists of surviving rows and all columns.
  // The columns minima of the reduced matrix are the columns minima
  // of the initial matrix.
  vector<size_t> rows_stack; // Stack of surviving rows
  rows_stack.reserve(cols.size());
  for (size_t row : rows) {
    while (   (rows_stack.size() > 0) 
           && ( matrix(rows_stack[rows_stack.size() - 1], cols[rows_stack.size() - 1])
                > matrix(row, cols[rows_stack.size() - 1]))
    ) {
      rows_stack.pop_back();
    }
    if (rows_stack.size() != cols.size()) {
      rows_stack.push_back(row);
    }
  }

  // Call recursively on odd-indexed columns
  vector<size_t> odd_cols;
  odd_cols.reserve(1 + cols.size() / 2);
  for (size_t i = 1; i < cols.size(); i += 2) {
    odd_cols.push_back(cols[i]);
  }
  smawk_inner(rows_stack, odd_cols, matrix, minima);

  // Compute minima for even-indexed columns
  size_t r = 0;
  for (size_t c = 0; c < cols.size(); c += 2) {
    size_t col = cols[c];
    size_t row = rows_stack[r];
    size_t last_row;

    // Determine the last row to consider for the current column
    if (c == cols.size() - 1) {
      // For the last column, use the last row
      last_row = rows_stack.back();
    } else {
      // Otherwise, use the minimum row from the next column
      last_row = (*minima)[cols[c + 1]];
    }

    // Initialize with the matrix value at the first position
    size_t value = matrix(row, col);

    // Set the current row as the initial minimum
    size_t minimum = row;

    // Search for the minimum across rows until the last row is reached
    while (row != last_row) {
      r++;
      row = rows_stack[r];
      // Get the matrix value at the new position
      size_t new_value = matrix(row, col);
      if (new_value < value) {
        value = new_value;
        minimum = row; // New minimum found
      }
    }

    // Store the minimum row for the current column
    (*minima)[col] = minimum;
  }
}

// Computes upper-right column minima in O(*m* + *n*) time.
//
// The input matrix must be totally monotone.
//
// The function returns a vector of `size_t`. The value at index `j`
// tells you the row of the minimum value in column `j`.
//
// The algorithm only considers values above the main diagonal, which
// means that it computes values `v(j)` where:
//
// ```text
// v(0) = initial
// v(j) = min { M[i, j] | i < j } for j > 0
// ```
//
// The algorithm is an *online* algorithm, in the sense that `matrix`
// function can refer back to previously computed column minima when
// determining an entry in the matrix. The guarantee is that we only
// call `matrix(i, j)` after having computed `v(i)`. This is
// reflected in the `vector<T>&` argument to `matrix`, which grows
// as more and more values are computed.
//
// Parameters:
//   - T initial: The initial value used for the first column's minimum.
//   - size_t size: The total number of columns in the matrix. This
//     defines how many columns the function will compute the minima for.
//   - function<T(vector<T>&, size_t, size_t)> matrix: A function that
//     calculates a matrix value. It takes a vector of previously
//     computed values, along with the row and column indices, and
//     returns the matrix value at that position.
template <typename T>
vector<size_t> online_column_minima(
  T initial, // Initial value for column minima
  size_t size,
  const function<T(vector<T>&, size_t, size_t)>& matrix
) {
  // Initialize result vector to store the row indices of the column minima
  vector<size_t> result{0};   // result[j] = row with min in column j 

  // Initialize values vector to store the minima for each column
  // (starting with the initial value)
  vector<T> values{initial};  // values[j] = min value in column j

  // State used by the algorithm
  size_t finished = 0;  // Index of the last processed row
  size_t base = 0;      // Base index for the calculation of minima
  size_t tentative = 0; // The current "tentative" index, which might
                        // determine the next column minimum

  // Keep going until we have finished all size columns. Since the
  // columns are zero-indexed, we're done when finished == size - 1.
  while (finished < size - 1) {

    // Lambda function for evaluating the matrix given the current computed values
    auto _matrix = [&values, &matrix, size](size_t i,size_t j) -> T {
      assert(i < j);                  // (i, j) not above diagonal
      assert(i < size && j < size);   // (i, j) out of bounds
      return matrix(values, i, j);
    };

    // First case: we have already advanced past the previous
    // tentative value. We make a new tentative value by applying
    // smawk_inner to the largest square submatrix that fits under
    // the base.
    size_t i = finished + 1;
    if (i > tentative) {
      vector<size_t> rows(finished + 1 - base);
      iota(begin(rows), end(rows), base);

      tentative = std::min(finished + rows.size(), size -1);

      vector<size_t> cols(tentative + 1 - (finished + 1));
      iota(begin(cols), end(cols), finished + 1);

      vector<size_t> minima(tentative + 1, 0);

      // Compute the minima for the submatrix
      smawk_inner<T>(rows, cols, _matrix, &minima);

      // Update the minima for each column based on the computed values
      for (size_t &col : cols) {
        auto row = minima[col];     // Row with the minimum for the current column
        auto v = _matrix(row, col); // Value of the matrix entry at this position

        if (col >= result.size()) {
          // The minimum for the column has not been computed yet, so add it
          result.push_back(row);
          values.push_back(v);
        } else if (v < values[col]) {
          // The new value is smaller than the current value, so update the minimum
          result[col] = row;
          values[col] = v;
        }
      }

      // Update the index of the last processed row
      finished = i;
      continue;
    }

    // Second case: the new column minimum is on the diagonal. All
    // subsequent ones will be at least as low, so we can clear
    // out all our work from higher rows. As in the fourth case,
    // the loss of tentative is amortized against the increase in
    // base.
    auto diag = _matrix(i - 1, i);
    if (diag < values[i]) {
      result[i] = i - 1;
      values[i] = diag;
      base = i - 1;
      tentative = i;
      finished = i;
      continue;
    }

    // Third case: row i-1 does not supply a column minimum in any
    // column up to tentative. We simply advance finished while
    // maintaining the invariant.
    if (_matrix(i - 1, tentative) >= values[tentative]) {
      finished = i;
      continue;
    }

    // Fourth and final case: a new column minimum at tentative.
    // This allows us to make progress by incorporating rows prior
    // to finished into the base. The base invariant holds because
    // these rows cannot supply any later column minima. The work
    // done when we last advanced tentative (and undone by this
    // step) can be amortized against the increase in base.
    base = i - 1;
    tentative = i;
    finished = i;
  }

  return result;
}

} // namespace smawk
