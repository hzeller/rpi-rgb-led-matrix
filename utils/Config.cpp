// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Matrix configuration parsing class implementation.
// Author: Tony DiCola
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>

#include <libconfig.h++>

#include "Config.h"

using namespace std;

// Get value if it exists, otherwise return default.
static int getWithDefault(const libconfig::Setting& root, const char *key,
                          int default_value) {
  return root.exists(key) ? root[key] : default_value;
}

Config::Config(rgb_matrix::RGBMatrix::Options *options,
               const string& filename)
  : _moptions(options),
    _display_width(-1),
    _display_height(-1),
    _panel_width(-1)
{
  try {
    // Load config file with libconfig.
    libconfig::Config cfg;
    cfg.readFile(filename.c_str());
    libconfig::Setting& root = cfg.getRoot();
    // Parse out the matrix configuration values. If not given, we use
    // reasonable defaults or defaults provided by the flags.
    _panel_width = getWithDefault(root, "panel_width", 32);
    _moptions->rows = getWithDefault(root, "panel_height", _moptions->rows);
    _chain_length = getWithDefault(root, "chain_length",
                                   _moptions->chain_length);
    // While all the code for the transformer assumes number of panels, for
    // the internal representation for the matrix code, we need to normalize
    // that to 32 wide panels.
    _moptions->chain_length = _chain_length * (_panel_width / 32);

    _moptions->parallel = getWithDefault(root, "parallel_count",
                                         _moptions->parallel);

    _display_width = getWithDefault(root, "display_width",
                                    getPanelWidth() * getChainLength());
    _display_height = getWithDefault(root, "display_height",
                                     getPanelHeight() * getParallelCount());

    // Do basic validation of configuration.
    if (_panel_width % 32 != 0) {
      throw invalid_argument("Panel width must be multiple of 32. Typically that is 32, but sometimes 64.");
    }

    if (_display_width % _panel_width != 0) {
      throw invalid_argument("display_width must be a multiple of panel_width!");
    }
    if (_display_height % getPanelHeight() != 0) {
      throw invalid_argument("display_height must be a multiple of panel_height!");
    }
    std::string message;
    if (!_moptions->Validate(&message)) {
      throw invalid_argument(message);
    }

    // Parse out the individual panel configurations.
    if (root.exists("panels")) {
      libconfig::Setting& panels_config = root["panels"];
      for (int i = 0; i < panels_config.getLength(); ++i) {
        libconfig::Setting& row = panels_config[i];
        for (int j = 0; j < row.getLength(); ++j) {
          GridTransformer::Panel panel;
          // Read panel order (required setting for each panel).
          panel.order = row[j]["order"];
          // Set default values for rotation and parallel chain, then override
          // them with any panel-specific configuration values.
          panel.rotate = 0;
          panel.parallel = 0;
          row[j].lookupValue("rotate", panel.rotate);
          row[j].lookupValue("parallel", panel.parallel);
          // Perform validation of panel values.
          // If panels are square allow rotations that are a multiple of 90, otherwise
          // only allow a rotation of 180 degrees.
          if ((_panel_width == getPanelHeight()) && (panel.rotate % 90 != 0)) {
            stringstream error;
            error << "Panel " << i << "," << j << " rotation must be a multiple of 90 degrees!";
            throw invalid_argument(error.str());
          }
          else if ((_panel_width != getPanelHeight()) && (panel.rotate % 180 != 0)) {
            stringstream error;
            error << "Panel row " << j << ", column " << i << " can only be rotated 180 degrees!";
            throw invalid_argument(error.str());
          }
          // Check that parallel is value between 0 and 2 (up to 3 parallel chains).
          if ((panel.parallel < 0) || (panel.parallel > 2)) {
            stringstream error;
            error << "Panel row " << j << ", column " << i << " parallel value must be 0, 1, or 2!";
            throw invalid_argument(error.str());
          }
          // Add the panel to the list of panel configurations.
          _panels.push_back(panel);
        }
      }
      // Check the number of configured panels matches the expected number
      // of panels (# of panel columns * # of panel rows).
      const int expected = (getDisplayWidth() / getPanelWidth())
        * (getDisplayHeight() / getPanelHeight());
      if (_panels.size() != (unsigned int)expected) {
        stringstream error;
        error << "Expected " << expected << " panels in configuration but found " << _panels.size() << "!";
        throw invalid_argument(error.str());
      }
    }
  }
  catch (const libconfig::FileIOException& fioex) {
      throw runtime_error("IO error while reading configuration file.  Does the file exist?");
    }
  catch (const libconfig::ParseException& pex) {
      stringstream error;
      error << "Config file error at " << pex.getFile() << ":" << pex.getLine()
            << " - " << pex.getError();
      throw invalid_argument(error.str());
    }
  catch (const libconfig::SettingNotFoundException& nfex) {
      stringstream error;
      error << "Expected to find setting: " << nfex.getPath();
      throw invalid_argument(error.str());
    }
  catch (const libconfig::ConfigException& ex) {
    throw runtime_error("Error loading configuration!");
  }
}
