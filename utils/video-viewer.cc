// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// Quick hack based on ffmpeg
// tutorial http://dranger.com/ffmpeg/tutorial01.html
// in turn based on a tutorial by
// Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
//
// HELP NEEDED
// Note, this is known to not be optimal, causing flicker etc. It is at this
// point merely a demonstration of what is possible. It also serves as a
// converter to a 'stream' (-O option) which then can be played quickly with
// the led-image-viewer.
//
// Pull requests are welcome to address
//    * Use hardware acceleration if possible. The Pi does have some
//      acceleration features IIRC, so if we could use these, that would be
//      great.
//    * Other improvements that could reduce the flicker on a Raspberry Pi.
//      Currently it seems to create flicker in particular when decoding larger
//      videos due to memory bandwidth overload (?). Might already be fixed
//      with using hardware acceleration.
//    * Add sound ? Right now, we don't decode the sound. It is usually
//      not very useful as the builtin-sound is disabled when running the
//      LED matrix, but if there is an external USB sound adapter, it might
//      be nice.


// Ancient AV versions forgot to set this.
#define __STDC_CONSTANT_MACROS

// libav: "U NO extern C in header ?"
extern "C" {
#  include <libavcodec/avcodec.h>
#  include <libavformat/avformat.h>
#  include <libavutil/imgutils.h>
#  include <libswscale/swscale.h>
#  include <libavdevice/avdevice.h>
}

#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <thread>

#include "led-matrix.h"
#include "content-streamer.h"

using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamWriter;
using rgb_matrix::StreamIO;

volatile bool interrupt_received = false;
static void InterruptHandler(int) {
  interrupt_received = true;
}

struct LedPixel {
  uint8_t r, g, b;
};
void CopyFrame(AVFrame *pFrame, FrameCanvas *canvas,
               int offset_x, int offset_y,
               int width, int height) {
  for (int y = 0; y < height; ++y) {
    LedPixel *pix = (LedPixel*) (pFrame->data[0] + y*pFrame->linesize[0]);
    for (int x = 0; x < width; ++x, ++pix) {
      canvas->SetPixel(x + offset_x, y + offset_y, pix->r, pix->g, pix->b);
    }
  }
}

// Scale "width" and "height" to fit within target rectangle of given size.
void ScaleToFitKeepAscpet(int fit_in_width, int fit_in_height,
                          int *width, int *height) {
  if (*height < fit_in_height && *width < fit_in_width) return; // Done.
  const float height_ratio = 1.0 * (*height) / fit_in_height;
  const float width_ratio  = 1.0 * (*width) / fit_in_width;
  const float ratio = (height_ratio > width_ratio) ? height_ratio : width_ratio;
  *width = roundf(*width / ratio);
  *height = roundf(*height / ratio);
}

static int usage(const char *progname, const char *msg = NULL) {
  if (msg) {
    fprintf(stderr, "%s\n", msg);
  }
  fprintf(stderr, "Show one or a sequence of video files on the RGB-Matrix\n");
  fprintf(stderr, "usage: %s [options] <video> [<video>...]\n", progname);
  fprintf(stderr, "Options:\n"
          "\t-F                 : Full screen without black bars; aspect ratio might suffer\n"
          "\t-O<streamfile>     : Output to stream-file instead of matrix (don't need to be root).\n"
          "\t-s <count>         : Skip these number of frames in the beginning.\n"
          "\t-c <count>         : Only show this number of frames (excluding skipped frames).\n"
          "\t-V<vsync-multiple> : Instead of native video framerate, playback framerate\n"
          "\t                     is a fraction of matrix refresh. In particular with a stable refresh,\n"
          "\t                     this can result in more smooth playback. Choose multiple for desired framerate.\n"
          "\t                     (Tip: use --led-limit-refresh for stable rate)\n"
	  "\t-T <threads>       : Number of threads used to decode (default 1, max=%d)\n"
          "\t-v                 : verbose; prints video metadata and other info.\n"
          "\t-f                 : Loop forever.\n",
	  (int)std::thread::hardware_concurrency());

  fprintf(stderr, "\nGeneral LED matrix options:\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static void add_nanos(struct timespec *accumulator, long nanoseconds) {
  accumulator->tv_nsec += nanoseconds;
  while (accumulator->tv_nsec > 1000000000) {
    accumulator->tv_nsec -= 1000000000;
    accumulator->tv_sec += 1;
  }
}

// Convert deprecated color formats to new and manually set the color range.
// YUV has funny ranges (16-235), while the YUVJ are 0-255. SWS prefers to
// deal with the YUV range, but then requires to set the output range.
// https://libav.org/documentation/doxygen/master/pixfmt_8h.html#a9a8e335cf3be472042bc9f0cf80cd4c5
SwsContext *CreateSWSContext(const AVCodecContext *codec_ctx,
                             int display_width, int display_height) {
  AVPixelFormat pix_fmt;
  bool src_range_extended_yuvj = true;
  // Remap deprecated to new pixel format.
  switch (codec_ctx->pix_fmt) {
  case AV_PIX_FMT_YUVJ420P: pix_fmt = AV_PIX_FMT_YUV420P; break;
  case AV_PIX_FMT_YUVJ422P: pix_fmt = AV_PIX_FMT_YUV422P; break;
  case AV_PIX_FMT_YUVJ444P: pix_fmt = AV_PIX_FMT_YUV444P; break;
  case AV_PIX_FMT_YUVJ440P: pix_fmt = AV_PIX_FMT_YUV440P; break;
  default:
    src_range_extended_yuvj = false;
    pix_fmt = codec_ctx->pix_fmt;
  }
  SwsContext *swsCtx = sws_getContext(codec_ctx->width, codec_ctx->height,
                                      pix_fmt,
                                      display_width, display_height,
                                      AV_PIX_FMT_RGB24, SWS_BILINEAR,
                                      NULL, NULL, NULL);
  if (src_range_extended_yuvj) {
    // Manually set the source range to be extended. Read modify write.
    int dontcare[4];
    int src_range, dst_range;
    int brightness, contrast, saturation;
    sws_getColorspaceDetails(swsCtx, (int**)&dontcare, &src_range,
                             (int**)&dontcare, &dst_range, &brightness,
                             &contrast, &saturation);
    const int* coefs = sws_getCoefficients(SWS_CS_DEFAULT);
    src_range = 1;  // New src range.
    sws_setColorspaceDetails(swsCtx, coefs, src_range, coefs, dst_range,
                             brightness, contrast, saturation);
  }
  return swsCtx;
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  rgb_matrix::RuntimeOptions runtime_opt;
  // If started with 'sudo': make sure to drop privileges to same user
  // we started with, which is the most expected (and allows us to read
  // files as that user).
  runtime_opt.drop_priv_user = getenv("SUDO_UID");
  runtime_opt.drop_priv_group = getenv("SUDO_GID");
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  int vsync_multiple = 1;
  bool use_vsync_for_frame_timing = false;
  bool maintain_aspect_ratio = true;
  bool verbose = false;
  bool forever = false;
  unsigned thread_count = 1;
  int stream_output_fd = -1;
  unsigned int frame_skip = 0;
  int64_t framecount_limit = INT64_MAX;

  int opt;
  while ((opt = getopt(argc, argv, "vO:R:Lfc:s:FV:T:")) != -1) {
    switch (opt) {
    case 'v':
      verbose = true;
      break;
    case 'f':
      forever = true;
      break;
    case 'O':
      stream_output_fd = open(optarg, O_CREAT|O_TRUNC|O_WRONLY, 0644);
      if (stream_output_fd < 0) {
        perror("Couldn't open output stream");
        return 1;
      }
      break;
    case 'L':
      fprintf(stderr, "-L is deprecated. Use\n\t--led-pixel-mapper=\"U-mapper\" --led-chain=4\ninstead.\n");
      return 1;
      break;
    case 'R':
      fprintf(stderr, "-R is deprecated. "
              "Use --led-pixel-mapper=\"Rotate:%s\" instead.\n", optarg);
      return 1;
      break;
    case 'c':
      framecount_limit = atoll(optarg);
      break;
    case 's':
      frame_skip = atoi(optarg);
      break;
    case 'T':
      thread_count = atoi(optarg);
      break;
    case 'F':
      maintain_aspect_ratio = false;
      break;
    case 'V':
      vsync_multiple = atoi(optarg);
      if (vsync_multiple <= 0)
        return usage(argv[0],
                     "-V: VSync-multiple needs to be a positive integer");
      use_vsync_for_frame_timing = true;
      break;
    default:
      return usage(argv[0]);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Expected video filename.\n");
    return usage(argv[0]);
  }

  const bool multiple_videos = (argc > optind + 1);

  // We want to have the matrix start unless we actually write to a stream.
  runtime_opt.do_gpio_init = (stream_output_fd < 0);
  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    return 1;
  }
  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

  long frame_count = 0;
  StreamIO *stream_io = NULL;
  StreamWriter *stream_writer = NULL;
  if (stream_output_fd >= 0) {
    stream_io = new rgb_matrix::FileStreamIO(stream_output_fd);
    stream_writer = new StreamWriter(stream_io);
    if (forever) {
      fprintf(stderr, "-f (forever) doesn't make sense with -O; disabling\n");
      forever = false;
    }
  }

  // If we only have to loop a single video, we can avoid doing the
  // expensive video stream set-up and just repeat in an inner loop.
  const bool one_video_forever = forever && !multiple_videos;
  const bool multiple_video_forever = forever && multiple_videos;

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
  av_register_all();
#endif
  avdevice_register_all();
  avformat_network_init();

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  do {
    for (int m = optind; m < argc && !interrupt_received; ++m) {
      const char *movie_file = argv[m];
      if (strcmp(movie_file, "-") == 0) {
        movie_file = "/dev/stdin";
      }

      AVFormatContext *format_context = avformat_alloc_context();
      if (avformat_open_input(&format_context, movie_file, NULL, NULL) != 0) {
        perror("Issue opening file: ");
        return -1;
      }

      if (avformat_find_stream_info(format_context, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information\n");
        return -1;
      }

      if (verbose) av_dump_format(format_context, 0, movie_file, 0);

      // Find the first video stream
      int videoStream = -1;
      AVCodecParameters *codec_parameters = NULL;
      const AVCodec *av_codec = NULL;
      for (int i = 0; i < (int)format_context->nb_streams; ++i) {
        codec_parameters = format_context->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(codec_parameters->codec_id);
        if (!av_codec) continue;
        if (codec_parameters->codec_type == AVMEDIA_TYPE_VIDEO) {
          videoStream = i;
          break;
        }
      }
      if (videoStream == -1)
        return false;

      // Frames per second; calculate wait time between frames.
      AVStream *const stream = format_context->streams[videoStream];
      AVRational rate = av_guess_frame_rate(format_context, stream, NULL);
      const long frame_wait_nanos = 1e9 * rate.den / rate.num;
      if (verbose) fprintf(stderr, "FPS: %f\n", 1.0*rate.num / rate.den);

      AVCodecContext *codec_context = avcodec_alloc_context3(av_codec);
      if (thread_count > 1 &&
          av_codec->capabilities & AV_CODEC_CAP_FRAME_THREADS &&
          std::thread::hardware_concurrency() > 1) {
        codec_context->thread_type = FF_THREAD_FRAME;
        codec_context->thread_count =
          std::min(thread_count, std::thread::hardware_concurrency());
      }

      if (avcodec_parameters_to_context(codec_context, codec_parameters) < 0)
        return -1;
      if (avcodec_open2(codec_context, av_codec, NULL) < 0)
        return -1;

      /*
       * Prepare frame to hold the scaled target frame to be send to matrix.
       */
      int display_width = codec_context->width;
      int display_height = codec_context->height;
      if (maintain_aspect_ratio) {
        display_width = codec_context->width;
        display_height = codec_context->height;
        // Make display fit within canvas.
        ScaleToFitKeepAscpet(matrix->width(), matrix->height(),
                             &display_width, &display_height);
      } else {
        display_width = matrix->width();
        display_height = matrix->height();
      }
      // Letterbox or pillarbox black bars.
      const int display_offset_x = (matrix->width() - display_width)/2;
      const int display_offset_y = (matrix->height() - display_height)/2;

      // The output_frame_ will receive the scaled result.
      AVFrame *output_frame = av_frame_alloc();
      if (av_image_alloc(output_frame->data, output_frame->linesize,
                         display_width, display_height, AV_PIX_FMT_RGB24,
                         64) < 0) {
        return -1;
      }

      if (verbose) {
        fprintf(stderr, "Scaling %dx%d -> %dx%d; black border x:%d y:%d\n",
                codec_context->width, codec_context->height,
                display_width, display_height,
                display_offset_x, display_offset_y);
      }

      // initialize SWS context for software scaling
      SwsContext *const sws_ctx = CreateSWSContext(
        codec_context, display_width, display_height);
      if (!sws_ctx) {
        fprintf(stderr, "Trouble doing scaling to %dx%d :(\n",
                matrix->width(), matrix->height());
        return 1;
      }


      struct timespec next_frame;

      AVPacket *packet = av_packet_alloc();
      AVFrame *decode_frame = av_frame_alloc();  // Decode video into this
      do {
        int64_t frames_left = framecount_limit;
        unsigned int frames_to_skip = frame_skip;
        if (one_video_forever) {
          av_seek_frame(format_context, videoStream, 0, AVSEEK_FLAG_ANY);
          avcodec_flush_buffers(codec_context);
        }
        clock_gettime(CLOCK_MONOTONIC, &next_frame);

        int decode_in_flight = 0;
        bool state_reading = true;

        while (!interrupt_received && frames_left > 0) {
          if (state_reading &&
              av_read_frame(format_context, packet) != 0) {
            state_reading = false;  // ran out of packets from input
          }

          if (!state_reading && decode_in_flight == 0)
            break;  // Decoder fully drained.

          // Is this a packet from the video stream?
          if (state_reading && packet->stream_index != videoStream) {
            av_packet_unref(packet);
            continue;  // Not interested in that.
          }

          if (state_reading) {
            // Decode video frame
            if (avcodec_send_packet(codec_context, packet) == 0) {
              ++decode_in_flight;
            }
            av_packet_unref(packet);
          } else {
            avcodec_send_packet(codec_context, nullptr); // Trigger decode drain
          }

          while (decode_in_flight &&
                 avcodec_receive_frame(codec_context, decode_frame) == 0) {
            --decode_in_flight;

            if (frames_to_skip) { frames_to_skip--; continue; }

            // Determine absolute end of this frame now so that we don't include
            // decoding overhead. TODO: skip frames if getting too slow ?
            add_nanos(&next_frame, frame_wait_nanos);

            // Convert the image from its native format to RGB
            sws_scale(sws_ctx, (uint8_t const * const *)decode_frame->data,
                      decode_frame->linesize, 0, codec_context->height,
                      output_frame->data, output_frame->linesize);
            CopyFrame(output_frame, offscreen_canvas,
                      display_offset_x, display_offset_y,
                      display_width, display_height);
            frame_count++;
            frames_left--;
            if (stream_writer) {
              if (verbose) fprintf(stderr, "%6ld", frame_count);
              stream_writer->Stream(*offscreen_canvas, frame_wait_nanos/1000);
            } else {
              offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas,
                                                     vsync_multiple);
            }
            if (!stream_writer && !use_vsync_for_frame_timing) {
              clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_frame, NULL);
            }
          }
        }
      } while (one_video_forever && !interrupt_received);

      av_packet_free(&packet);

      av_frame_free(&output_frame);
      av_frame_free(&decode_frame);
      avcodec_close(codec_context);
      avformat_close_input(&format_context);
    }
  } while (multiple_video_forever && !interrupt_received);

  if (interrupt_received) {
    // Feedback for Ctrl-C, but most importantly, force a newline
    // at the output, so that commandline-shell editing is not messed up.
    fprintf(stderr, "Got interrupt. Exiting\n");
  }

  delete matrix;
  delete stream_writer;
  delete stream_io;
  fprintf(stderr, "Total of %ld frames decoded\n", frame_count);

  return 0;
}
