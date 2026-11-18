# textwrap-example

This example demonstrates advanced text wrapping for RGB LED matrix displays,
utilizing the **optimal-fit text wrapping feature** of the rpi-rgb-led-matrix
library. The algorithm behind this feature is inspired by TeX and the Rust
`textwrap` crate.

## Features of the DrawTextWrapped Function

The following features are provided by the `DrawTextWrapped` function from the
rpi-rgb-led-matrix library, which is used in this example:

- Wraps UTF-8 text to fit the display width and draws it on the LED matrix.
- Uses an **optimal-fit line breaking algorithm** to minimize raggedness
  (uneven line endings).
- Supports custom fonts, colors, kerning, and line spacing.
- Supports both monospaced and proportional fonts.

## How the Text Wrapping Works

Unlike simple greedy wrapping (which just fills each line until it overflows),
the **optimal-fit algorithm** in the rpi-rgb-led-matrix library works as
follows:

- The algorithm considers all possible places to break lines.
- It assigns a penalty to each possible break, based on how much empty space
  (gap) is left at the end of each line.
- The penalty for a line is `gap * gap`, where `gap = target_width - line_width`,
  and `target_width` is the maximal allowed width of a line in pixels (usually
  the display width).
- The algorithm finds the set of breaks that **minimizes the total penalty**
  for the whole paragraph.
- This is similar to the line breaking used in TeX and produces more visually
  pleasing results, especially for narrow displays.

### Example: Optimal Text Wrapping

Suppose you want to wrap the text:

    To be, or not to be: that is the question

for a display that fits 10 characters per line. The algorithm will try all
possible ways to break the text and choose the one with the lowest total
penalty. For example:

**Simple greedy wrapping:**

    To be, or   (gap=1, penalty=1)
    not to be:  (gap=0, penalty=0)
    that is     (gap=3, penalty=9)
    the         (gap=7, penalty=49)
    question    (gap=2, penalty=4)
    Total penalty: 63

**Optimal-fit wrapping:**

    To be,      (gap=4, penalty=16)
    or not to   (gap=1, penalty=1)
    be: that    (gap=2, penalty=4)
    is the      (gap=4, penalty=16)
    question    (gap=2, penalty=4)
    Total penalty: 41

The optimal-fit algorithm finds the second arrangement, which is more balanced
and visually appealing.

> **Note:**  
> The text wrapping featur works equally well with proportional fonts.  
> It does not rely on a fixed character width, but instead uses the actual
> pixel width of each word (or fragment) as measured in the selected font.
> This allows optimal wrapping for both monospaced and proportional fonts,
> ensuring that lines are filled as evenly as possible regardless of
> widths.

## Build

In the `examples-api-use` directory, run:

```sh
make textwrap-example
```

## Usage

```sh
sudo ./textwrap-example -f ../fonts/Grand9K-Pixel.bdf
```

- Type or pipe text into the program. It will be wrapped and displayed on the
  matrix.
- Press `CTRL+D` to exit.

### Options

- `-f <font-file>`: Path to BDF font file (required)
- `-x <x-origin>`: X origin for text (default: 0)
- `-y <y-origin>`: Y origin for text (default: 0)
- `-S <spacing>`: Letter spacing in pixels (default: 0)
- `-C <r,g,b>`: Text color (default: 255,255,0)
- `-B <r,g,b>`: Font background color (default: 0,0,0)
- `-F <r,g,b>`: Panel background color (default: 0,0,0)
