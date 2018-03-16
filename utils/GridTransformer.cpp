// LED matrix library transformer to map a rectangular canvas onto a complex
// chain of matrices.
// Author: Tony DiCola
#include "GridTransformer.h"

using namespace rgb_matrix;
using namespace std;

GridTransformer::GridTransformer(int width, int height, int panel_width, int panel_height,
                                 int chain_length, const std::vector<Panel>& panels):
  _width(width),
  _height(height),
  _panel_width(panel_width),
  _panel_height(panel_height),
  _chain_length(chain_length),
  _source(NULL),
  _panels(panels)
{
  // Display width must be a multiple of the panel pixel column count.
  assert(_width % _panel_width == 0);
  // Display height must be a multiple of the panel pixel row count.
  assert(_height % _panel_height == 0);
  // Compute number of rows and columns of panels.
  _rows = _height / _panel_height;
  _cols = _width / _panel_width;
  // Check panel definition list has exactly the expected number of panels.
  assert((_rows * _cols) == (int)_panels.size());
}

void GridTransformer::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) {
  assert(_source != NULL);
  if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) {
    return;
  }

  // Figure out what row and column panel this pixel is within.
  int row = y / _panel_height;
  int col = x / _panel_width;

  // Get the panel information for this pixel.
  Panel panel = _panels[_cols*row + col];

  // Compute location of the pixel within the panel.
  x = x % _panel_width;
  y = y % _panel_height;

  // Perform any panel rotation to the pixel.
  // NOTE: 90 and 270 degree rotation only possible on 32 row (square) panels.
  if (panel.rotate == 90) {
    assert(_panel_height == _panel_width);
    int old_x = x;
    x = (_panel_height-1)-y;
    y = old_x;
  }
  else if (panel.rotate == 180) {
    x = (_panel_width-1)-x;
    y = (_panel_height-1)-y;
  }
  else if (panel.rotate == 270) {
    assert(_panel_height == _panel_width);
    int old_y = y;
    y = (_panel_width-1)-x;
    x = old_y;
  }

  // Determine x offset into the source panel based on its order along the chain.
  // The order needs to be inverted because the matrix library starts with the
  // origin of an image at the end of the chain and not at the start (where
  // ordering begins for this transformer).
  int x_offset = ((_chain_length-1)-panel.order)*_panel_width;

  // Determine y offset into the source panel based on its parrallel chain value.
  int y_offset = panel.parallel*_panel_height;

  _source->SetPixel(x_offset + x,
                    y_offset + y,
                    red, green, blue);
}

Canvas* GridTransformer::Transform(Canvas* source) {
  assert(source != NULL);
  int swidth = source->width();
  int sheight = source->height();
  assert((_width * _height) == (swidth * sheight));
  _source = source;
  return this;
}
