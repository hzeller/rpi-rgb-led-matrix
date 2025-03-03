// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2023 Hendrik
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "emulator.h"
#ifdef ENABLE_EMULATOR
#include "framebuffer-internal.h"  // Add this include for Framebuffer class
#include <SDL2/SDL.h>
#include <assert.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <string.h>
#include <string>
#include <thread>
#include <unistd.h>

namespace rgb_matrix {

EmulatorOptions::EmulatorOptions() 
  : display_scale(10), 
    window_title("RGB Matrix Emulator"), 
    emulate_hardware_timing(false),
    refresh_rate_hz(60) {
}

namespace {

// Structure to hold pixel data for the display
struct EmulatedPixel {
  uint8_t r, g, b;
};

static bool sdl_initialized = false;

// Initialize SDL if not already initialized
static bool InitSDL() {
  if (sdl_initialized) return true;
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
    return false;
  }
  
  atexit(SDL_Quit);
  sdl_initialized = true;
  return true;
}

// Class to handle SDL window and rendering
class SDLDisplay {
public:
  SDLDisplay(int width, int height, int scale, const std::string& title)
    : width_(width), height_(height), scale_(scale) {
    
    if (!InitSDL()) return;
    
    window_ = SDL_CreateWindow(
      title.c_str(),
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      width * scale, height * scale,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
      
    if (!window_) {
      fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
      return;
    }
    
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
      fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
      return;
    }
    
    texture_ = SDL_CreateTexture(
      renderer_,
      SDL_PIXELFORMAT_RGB24,
      SDL_TEXTUREACCESS_STREAMING,
      width, height);
      
    if (!texture_) {
      fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
      return;
    }
    
    pixels_ = new EmulatedPixel[width * height];
    memset(pixels_, 0, width * height * sizeof(EmulatedPixel));
  }
  
  ~SDLDisplay() {
    if (pixels_) delete[] pixels_;
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
  }
  
  void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
    
    const int pos = y * width_ + x;
    pixels_[pos].r = r;
    pixels_[pos].g = g;
    pixels_[pos].b = b;
  }
  
  void Clear() {
    memset(pixels_, 0, width_ * height_ * sizeof(EmulatedPixel));
  }
  
  void Fill(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < width_ * height_; ++i) {
      pixels_[i].r = r;
      pixels_[i].g = g;
      pixels_[i].b = b;
    }
  }
  
  void Render(uint8_t brightness) {
    // Handle SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        exit(0);  // Quit the application
      } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          // Update scale based on new window size
          int new_width = event.window.data1;
          int new_height = event.window.data2;
          
          scale_ = std::min(new_width / width_, new_height / height_);
          if (scale_ < 1) scale_ = 1;
        }
      }
    }
    
    // Copy pixels to texture with brightness adjustment
    uint8_t *tex_pixels;
    int pitch;
    SDL_LockTexture(texture_, NULL, (void**)&tex_pixels, &pitch);
    
    const float brightness_factor = brightness / 100.0f;
    
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        const int src_pos = y * width_ + x;
        const int dst_pos = y * pitch + x * 3;
        
        tex_pixels[dst_pos]     = (uint8_t)(pixels_[src_pos].r * brightness_factor);
        tex_pixels[dst_pos + 1] = (uint8_t)(pixels_[src_pos].g * brightness_factor);
        tex_pixels[dst_pos + 2] = (uint8_t)(pixels_[src_pos].b * brightness_factor);
      }
    }
    
    SDL_UnlockTexture(texture_);
    
    // Render the texture to the window
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    
    // Calculate destination rectangle to center the texture
    SDL_Rect dest_rect;
    dest_rect.w = width_ * scale_;
    dest_rect.h = height_ * scale_;
    
    // Get window size for centering
    int win_width, win_height;
    SDL_GetWindowSize(window_, &win_width, &win_height);
    
    dest_rect.x = (win_width - dest_rect.w) / 2;
    dest_rect.y = (win_height - dest_rect.h) / 2;
    
    SDL_RenderCopy(renderer_, texture_, NULL, &dest_rect);
    SDL_RenderPresent(renderer_);
  }
  
private:
  int width_;
  int height_;
  int scale_;
  SDL_Window *window_ = nullptr;
  SDL_Renderer *renderer_ = nullptr;
  SDL_Texture *texture_ = nullptr;
  EmulatedPixel *pixels_ = nullptr;
};

}  // namespace

// Create internal framebuffer implementation for the emulator to match the RGBMatrix design
namespace internal {
class EmulatorFramebuffer : public rgb_matrix::internal::Framebuffer {
public:
  EmulatorFramebuffer(int width, int height)
    : rgb_matrix::internal::Framebuffer(width, height, 1, 0, "RGB", false, &shared_pixel_mapper_),
      width_(width), height_(height), brightness_(100), pwm_bits_(8),
      do_luminance_correct_(false), shared_pixel_mapper_(NULL) {
      
    pixels_ = new EmulatedPixel[width_ * height_];
    Clear();
  }
  
  ~EmulatorFramebuffer() override {
    delete[] pixels_;
  }
  
  void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
    
    const int pos = y * width_ + x;
    pixels_[pos].r = r;
    pixels_[pos].g = g;
    pixels_[pos].b = b;
  }
  
  void Clear() override {
    memset(pixels_, 0, width_ * height_ * sizeof(EmulatedPixel));
  }
  
  void Fill(uint8_t r, uint8_t g, uint8_t b) override {
    for (int i = 0; i < width_ * height_; ++i) {
      pixels_[i].r = r;
      pixels_[i].g = g;
      pixels_[i].b = b;
    }
  }
  
  // Override the width and height methods to use our local values
  // REVIEW This was a sigfault before overwriting, should probably review this
  int width() const override { return width_; }
  int height() const override { return height_; }
  
  // Remove the override keyword from methods that don't match exactly
  bool SetPWMBits(uint8_t bits) {
    if (bits < 1 || bits > 11) return false;
    pwm_bits_ = bits;
    return true;
  }
  
  uint8_t pwmbits() const {
    return pwm_bits_;
  }
  
  void set_luminance_correct(bool correct) {
    do_luminance_correct_ = correct;
  }
  
  bool luminance_correct() const {
    return do_luminance_correct_;
  }
  
  void SetBrightness(uint8_t brightness) {
    brightness_ = brightness > 100 ? 100 : brightness;
  }
  
  uint8_t brightness() const {
    return brightness_;
  }
  
  void DumpToMatrix(GPIO* io, int bit_plane) override {
    // Not used in the emulator - this would normally send data to GPIO
  }
  
  void Serialize(const char** data, size_t* len) const {
    // Not implemented for emulator
    *data = nullptr;
    *len = 0;
  }
  
  bool Deserialize(const char* data, size_t len) {
    // Not implemented for emulator
    return false;
  }
  
  void CopyFrom(const Framebuffer& other) {
    const EmulatorFramebuffer* o = dynamic_cast<const EmulatorFramebuffer*>(&other);
    if (!o) return;
    
    memcpy(pixels_, o->pixels_, width_ * height_ * sizeof(EmulatedPixel));
  }
  
  // Render this framebuffer to the display
  void Render(SDLDisplay* display) const {
    if (!display) return;
    
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; ++x) {
        const int pos = y * width_ + x;
        display->SetPixel(x, y, pixels_[pos].r, pixels_[pos].g, pixels_[pos].b);
      }
    }
  }
  
private:
  const int width_;
  const int height_;
  uint8_t brightness_;
  uint8_t pwm_bits_;
  bool do_luminance_correct_;
  EmulatedPixel* pixels_;
  rgb_matrix::internal::PixelDesignatorMap* shared_pixel_mapper_;
};
} // namespace internal

class EmulatorMatrix::Impl {
public:
  Impl(const RGBMatrix::Options& opts, const EmulatorOptions& emulator_opts)
    : matrix_options_(opts),
      emulator_options_(emulator_opts),
      display_(nullptr),
      brightness_(100),
      pwm_bits_(opts.pwm_bits),
      luminance_correct_(false),
      running_(false),
      do_vsync_callback_(false) {
    
    width_ = opts.cols * opts.chain_length;
    height_ = opts.rows * opts.parallel;
    
    // Initialize hardware mapping before creating framebuffer
    rgb_matrix::internal::Framebuffer::InitHardwareMapping(opts.hardware_mapping);
    
    display_ = new SDLDisplay(width_, height_, 
                              emulator_opts.display_scale,
                              emulator_opts.window_title);
    
    // Create initial framebuffer and canvas
    rgb_matrix::internal::Framebuffer* fb = new internal::EmulatorFramebuffer(width_, height_);
    active_buffer_ = new FrameCanvas(fb);
  }
  
  ~Impl() {
    Stop();
    
    if (display_) delete display_;
    
    // Delete any active framebuffers
    for (auto canvas : created_frames_) {
      delete canvas;
    }
  }
  
  void Start() {
    if (running_) return;
    running_ = true;
    
    matrix_thread_ = std::thread(&EmulatorMatrix::Impl::MatrixThread, this);
  }
  
  void Stop() {
    if (!running_) return;
    running_ = false;
    
    if (matrix_thread_.joinable()) {
      matrix_thread_.join();
    }
  }
  
  FrameCanvas* SwapOnVSync(FrameCanvas* other, unsigned frame_fraction) {
    std::unique_lock<std::mutex> lock(mutex_);
    FrameCanvas* previous = active_buffer_;
    active_buffer_ = other;
    return previous;
  }
  
  void SetVSyncCallback(std::function<void()> callback) {
    vsync_callback_ = callback;
    do_vsync_callback_ = true;
  }
  
  bool ApplyPixelMapper(const PixelMapper* mapper) {
    // Pixel mapping not implemented in emulator
    return false;
  }
  
  // Canvas interface
  int width() const { return width_; }
  int height() const { return height_; }
  
  void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (!active_buffer_) return;
    active_buffer_->SetPixel(x, y, r, g, b);
  }
  
  void Clear() {
    if (!active_buffer_) return;
    active_buffer_->Clear();
  }
  
  void Fill(uint8_t r, uint8_t g, uint8_t b) {
    if (!active_buffer_) return;
    active_buffer_->Fill(r, g, b);
  }
  
  // Frame canvas management
  FrameCanvas* CreateFrameCanvas() {
    rgb_matrix::internal::Framebuffer* fb = new internal::EmulatorFramebuffer(width_, height_);
    FrameCanvas* canvas = new FrameCanvas(fb);
    
    canvas->SetBrightness(brightness_);
    canvas->SetPWMBits(pwm_bits_);
    canvas->set_luminance_correct(luminance_correct_);
    
    created_frames_.push_back(canvas);
    return canvas;
  }
  
  // Settings
  void SetBrightness(uint8_t brightness) {
    brightness_ = brightness > 100 ? 100 : brightness;
    if (active_buffer_) active_buffer_->SetBrightness(brightness_);
  }
  
  uint8_t GetBrightness() const { return brightness_; }
  
  bool SetPWMBits(uint8_t bits) {
    if (bits < 1 || bits > 11) return false;
    pwm_bits_ = bits;
    if (active_buffer_) active_buffer_->SetPWMBits(pwm_bits_);
    return true;
  }
  
  uint8_t PWMBits() const { return pwm_bits_; }
  
  void SetLuminanceCorrect(bool correct) {
    luminance_correct_ = correct;
    if (active_buffer_) active_buffer_->set_luminance_correct(correct);
  }
  
  bool LuminanceCorrect() const { return luminance_correct_; }
  
private:
  // Main thread function that updates the display
  void MatrixThread() {
    const int64_t frame_time_us = 1000000 / emulator_options_.refresh_rate_hz;
    
    auto next_frame_time = std::chrono::steady_clock::now();
    
    while (running_) {
      // Render the current active buffer
      {
        std::unique_lock<std::mutex> lock(mutex_);
        if (active_buffer_) {
          rgb_matrix::internal::Framebuffer* fb = active_buffer_->framebuffer();
          internal::EmulatorFramebuffer* emulator_fb = static_cast<internal::EmulatorFramebuffer*>(fb);
          emulator_fb->Render(display_);
          display_->Render(brightness_);
        }
      }
      
      // Call the vsync callback if registered
      if (do_vsync_callback_ && vsync_callback_) {
        vsync_callback_();
      }
      
      // Wait until next frame time
      next_frame_time += std::chrono::microseconds(frame_time_us);
      std::this_thread::sleep_until(next_frame_time);
    }
  }
  
  // Matrix configuration
  const RGBMatrix::Options matrix_options_;
  const EmulatorOptions emulator_options_;
  
  // Display state
  int width_;
  int height_;
  SDLDisplay* display_;
  uint8_t brightness_;
  uint8_t pwm_bits_;
  bool luminance_correct_;
  
  // Threading
  std::thread matrix_thread_;
  std::mutex mutex_;
  bool running_;
  
  // Frame buffer management
  FrameCanvas* active_buffer_ = nullptr;
  std::vector<FrameCanvas*> created_frames_;
  
  // VSync callback
  std::function<void()> vsync_callback_;
  bool do_vsync_callback_;
};

// Constructor
EmulatorMatrix::EmulatorMatrix(Impl* impl) : impl_(impl) {}

// Destructor
EmulatorMatrix::~EmulatorMatrix() {
  delete impl_;
}

// Factory method to create a new emulator
EmulatorMatrix* EmulatorMatrix::Create(const RGBMatrix::Options& options,
                                       const EmulatorOptions& emulator_options) {
  Impl* impl = new Impl(options, emulator_options);
  return new EmulatorMatrix(impl);
}

// Canvas interface
int EmulatorMatrix::width() const { return impl_->width(); }
int EmulatorMatrix::height() const { return impl_->height(); }

void EmulatorMatrix::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  impl_->SetPixel(x, y, r, g, b);
}

void EmulatorMatrix::Clear() { impl_->Clear(); }
void EmulatorMatrix::Fill(uint8_t r, uint8_t g, uint8_t b) { impl_->Fill(r, g, b); }

// Frame canvas management
FrameCanvas* EmulatorMatrix::CreateFrameCanvas() {
  return impl_->CreateFrameCanvas();
}

FrameCanvas* EmulatorMatrix::SwapOnVSync(FrameCanvas* other, unsigned framerate_fraction) {
  return impl_->SwapOnVSync(other, framerate_fraction);
}

// Settings
bool EmulatorMatrix::ApplyPixelMapper(const PixelMapper* mapper) {
  return impl_->ApplyPixelMapper(mapper);
}

void EmulatorMatrix::SetBrightness(uint8_t brightness) {
  impl_->SetBrightness(brightness);
}

uint8_t EmulatorMatrix::brightness() {
  return impl_->GetBrightness();
}

bool EmulatorMatrix::SetPWMBits(uint8_t bits) {
  return impl_->SetPWMBits(bits);
}

uint8_t EmulatorMatrix::pwmbits() {
  return impl_->PWMBits();
}

void EmulatorMatrix::set_luminance_correct(bool correct) {
  impl_->SetLuminanceCorrect(correct);
}

bool EmulatorMatrix::luminance_correct() const {
  return impl_->LuminanceCorrect();
}

bool EmulatorMatrix::StartRefresh() {
  impl_->Start();
  return true;
}

// Parse emulator-specific flags
bool ParseEmulatorOptionsFromFlags(int* argc, char*** argv,
                                  EmulatorOptions* options,
                                  bool remove_consumed_flags) {
  if (!options) return false;
  
  for (int i = 1; i < *argc; ++i) {
    if (!(*argv)[i]) continue;  // Skip already consumed flags
    
    const char* option = (*argv)[i];
    bool consumed = false;
    
    if (strncmp(option, "--led-emulator-scale=", 21) == 0) {
      options->display_scale = atoi(option + 21);
      consumed = true;
    } else if (strncmp(option, "--led-emulator-title=", 21) == 0) {
      options->window_title = option + 21;
      consumed = true;
    } else if (strncmp(option, "--led-emulator-refresh=", 23) == 0) {
      options->refresh_rate_hz = atoi(option + 23);
      consumed = true;
    } else if (strcmp(option, "--led-emulator-hardware-timing") == 0) {
      options->emulate_hardware_timing = true;
      consumed = true;
    }
    
    if (consumed && remove_consumed_flags) {
      (*argv)[i] = nullptr;
    }
  }
  
  if (remove_consumed_flags) {
    // Remove all the flags we have consumed
    int new_argc = 1;  // Keep argv[0]
    for (int i = 1; i < *argc; ++i) {
      if ((*argv)[i] != nullptr) {
        (*argv)[new_argc++] = (*argv)[i];
      }
    }
    
    // Set the remaining positions to nullptr
    for (int i = new_argc; i < *argc; ++i) {
      (*argv)[new_argc++] = nullptr;
    }
    
    *argc = new_argc;
  }
  
  return true;
}

// Print emulator-specific flags
void PrintEmulatorFlags(FILE* out, const EmulatorOptions& defaults) {
  fprintf(out,
          "\t--led-emulator-scale=<scale>    : Window scale factor (Default: %d)\n"
          "\t--led-emulator-title=<title>    : Window title (Default: %s)\n"
          "\t--led-emulator-refresh=<hz>     : Display refresh rate Hz (Default: %d)\n"
          "\t--led-emulator-hardware-timing  : Emulate hardware timing (Default: %s)\n",
          defaults.display_scale, defaults.window_title.c_str(),
          defaults.refresh_rate_hz,
          defaults.emulate_hardware_timing ? "on" : "off");
}

}  // namespace rgb_matrix

#endif