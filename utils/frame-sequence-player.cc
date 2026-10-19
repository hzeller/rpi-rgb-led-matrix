// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Utility showing how to decode an image/animation and play it from an
// in-memory frame sequence.

#include "frame-sequence.h"
#include "led-matrix.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <exception>
#include <Magick++.h>

using rgb_matrix::FrameSequence;
using rgb_matrix::RGBMatrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

class ConsoleBrightnessControl {
 public:
  explicit ConsoleBrightnessControl(int initial)
      : initialized_(false), brightness_(Clamp(initial)), old_flags_(0) {
    memset(&old_termios_, 0, sizeof(old_termios_));
  }

  bool Init() {
    if (!isatty(STDIN_FILENO)) {
      fprintf(stderr, "stdin is not a tty; interactive brightness disabled.\n");
      return false;
    }

    if (tcgetattr(STDIN_FILENO, &old_termios_) != 0) {
      perror("tcgetattr");
      return false;
    }
    old_flags_ = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (old_flags_ < 0) {
      perror("fcntl(F_GETFL)");
      return false;
    }

    struct termios raw = old_termios_;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
      perror("tcsetattr");
      return false;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, old_flags_ | O_NONBLOCK) != 0) {
      perror("fcntl(F_SETFL)");
      tcsetattr(STDIN_FILENO, TCSANOW, &old_termios_);
      return false;
    }

    initialized_ = true;
    fprintf(stderr,
            "Brightness keys: '+' up, '-' down, 'q' quit (current: %d)\n",
            brightness_);
    return true;
  }

  ~ConsoleBrightnessControl() {
    Restore();
  }

  int PollAndGetBrightness() {
    if (!initialized_) return brightness_;

    char c = 0;
    while (read(STDIN_FILENO, &c, 1) == 1) {
      if (c == '+' || c == '=') {
        brightness_ = Clamp(brightness_ + 5);
        fprintf(stderr, "\rBrightness: %d   ", brightness_);
      } else if (c == '-' || c == '_') {
        brightness_ = Clamp(brightness_ - 5);
        fprintf(stderr, "\rBrightness: %d   ", brightness_);
      } else if (c == 'q' || c == 'Q') {
        interrupt_received = true;
      }
    }
    return brightness_;
  }

 private:
  static int Clamp(int value) {
    if (value < 1) return 1;
    if (value > 100) return 100;
    return value;
  }

  void Restore() {
    if (!initialized_) return;
    fcntl(STDIN_FILENO, F_SETFL, old_flags_);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios_);
    fprintf(stderr, "\n");
    initialized_ = false;
  }

  bool initialized_;
  int brightness_;
  struct termios old_termios_;
  int old_flags_;
};

using ImageVector = std::vector<Magick::Image>;

static bool HasSuffix(const char *value, const char *suffix) {
  if (value == NULL || suffix == NULL) return false;
  const size_t value_len = strlen(value);
  const size_t suffix_len = strlen(suffix);
  if (suffix_len > value_len) return false;
  return strcmp(value + value_len - suffix_len, suffix) == 0;
}

static ImageVector LoadAndScaleFrames(const char *filename,
                                      int target_width,
                                      int target_height) {
  ImageVector result;
  ImageVector frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception &e) {
    if (e.what()) {
      fprintf(stderr, "%s\n", e.what());
    }
    return result;
  }

  if (frames.empty()) {
    fprintf(stderr, "No image found.\n");
    return result;
  }

  if (frames.size() > 1) {
    Magick::coalesceImages(&result, frames.begin(), frames.end());
  } else {
    result.push_back(frames[0]);
  }

  for (ImageVector::iterator it = result.begin(); it != result.end(); ++it) {
    it->scale(Magick::Geometry(target_width, target_height));
  }
  return result;
}

static int usage(const char *progname) {
  fprintf(stderr,
          "Usage: %s [led-matrix-options] <image-or-fseq> "
          "[-O<output.fseq> | -O <output.fseq>]\n",
          progname);
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

int main(int argc, char *argv[]) {
  Magick::InitializeMagick(*argv);
  const char *progname = argv[0];

  RGBMatrix::Options matrix_options;
  matrix_options.cols = 128;
  matrix_options.rows = 64;
  matrix_options.row_address_type = 5;
  matrix_options.parallel = 2;
  matrix_options.brightness = 50;

  rgb_matrix::RuntimeOptions runtime_options;
  runtime_options.gpio_slowdown = 5;
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_options)) {
    return usage(progname);
  }

  if (argc != 2 && argc != 3 && argc != 4) {
    return usage(progname);
  }

  const char *filename = argv[1];
  const char *output_fseq = NULL;
  if (argc == 3) {
    if (strncmp(argv[2], "-O", 2) != 0 || argv[2][2] == '\0') {
      return usage(progname);
    }
    output_fseq = argv[2] + 2;
  } else if (argc == 4) {
    if (strcmp(argv[2], "-O") != 0) {
      return usage(progname);
    }
    output_fseq = argv[3];
  }

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // Decode frames before matrix creation, as matrix init may drop privileges.
  const int target_width = matrix_options.cols * matrix_options.chain_length;
  const int target_height = matrix_options.rows * matrix_options.parallel;
  FrameSequence sequence(target_width, target_height);

  if (HasSuffix(filename, ".fseq")) {
    if (!sequence.ReadFromFile(filename)) {
      fprintf(stderr, "Failed to read frame sequence from '%s'\n", filename);
      return 1;
    }
  } else {
    ImageVector images = LoadAndScaleFrames(filename, target_width, target_height);
    if (images.empty()) {
      return 1;
    }

    const size_t frame_bytes =
        static_cast<size_t>(target_width) * static_cast<size_t>(target_height) * 3;
    std::vector<uint8_t> rgb(frame_bytes);
    for (ImageVector::const_iterator image = images.begin(); image != images.end(); ++image) {
      uint8_t *out = rgb.data();
      for (size_t y = 0; y < image->rows(); ++y) {
        for (size_t x = 0; x < image->columns(); ++x) {
          const Magick::Color &c = image->pixelColor(x, y);
          if (c.alphaQuantum() < 256) {
            *out++ = ScaleQuantumToChar(c.redQuantum());
            *out++ = ScaleQuantumToChar(c.greenQuantum());
            *out++ = ScaleQuantumToChar(c.blueQuantum());
          } else {
            *out++ = 0;
            *out++ = 0;
            *out++ = 0;
          }
        }
      }

      const uint32_t hold_us = image->animationDelay() > 0
                                   ? image->animationDelay() * 10000u
                                   : 100000u;
      if (!sequence.AddFrame(rgb.data(), rgb.size(), hold_us)) {
        fprintf(stderr, "Failed to add frame to sequence.\n");
        return 1;
      }
    }
  }

  if (output_fseq != NULL) {
    if (!sequence.WriteToFile(output_fseq)) {
      fprintf(stderr, "Failed to write frame sequence to '%s'\n", output_fseq);
      return 1;
    }
    fprintf(stderr, "Wrote %zu frames to %s\n", sequence.frame_count(), output_fseq);
    return 0;
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options,
                                                   runtime_options);
  if (matrix == NULL) {
    return 1;
  }

  ConsoleBrightnessControl brightness_control(matrix_options.brightness);
  const bool brightness_enabled = brightness_control.Init();
  auto stop_provider = []() {
    return interrupt_received;
  };
  if (brightness_enabled) {
    sequence.PlayForever(matrix, stop_provider, [&brightness_control]() {
      return brightness_control.PollAndGetBrightness();
    });
  } else {
    sequence.PlayForever(matrix, stop_provider);
  }

  matrix->Clear();
  delete matrix;
  return 0;
}
