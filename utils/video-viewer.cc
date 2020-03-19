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
//    * Ancient code: this is based on a very old ffmpeg demo. The API probably
//      evolved over time.
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
#  include <libswscale/swscale.h>
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

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#  define av_frame_alloc avcodec_alloc_frame
#  define av_frame_free avcodec_free_frame
#endif

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

static int usage(const char *progname, const char *msg = nullptr) {
  if (msg) {
    fprintf(stderr, "%s\n", msg);
  }
  fprintf(stderr, "usage: %s [options] <video>\n", progname);
  fprintf(stderr, "Options:\n"
          "\t-F                 : Full screen without black bars; aspect ratio might suffer\n"
          "\t-O<streamfile>     : Output to stream-file instead of matrix (don't need to be root).\n"
          "\t-s <count>         : Skip these number of frames in the beginning.\n"
          "\t-c <count>         : Only show this number of frames (excluding skipped frames).\n"
          "\t-V<vsync-multiple> : Instead of native video framerate, playback framerate\n"
          "\t                     is a fraction of matrix refresh. In particular with a stable refresh,\n"
          "\t                     this can result in more smooth playback. Choose multiple for desired framerate.\n"
          "\t                     (Tip: use --led-limit-refresh for stable rate)\n"
          "\t-v                 : verbose; prints video metadata and other info.\n"
          "\t-f                 : Loop forever.\n");

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
  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                         &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  int vsync_multiple = 1;
  bool use_vsync_for_frame_timing = false;
  bool maintain_aspect_ratio = true;
  bool verbose = false;
  bool forever = false;
  int stream_output_fd = -1;
  unsigned int frame_skip = 0;
  unsigned int framecount_limit = UINT_MAX;  // even at 60fps, that is > 2yrs

  int opt;
  while ((opt = getopt(argc, argv, "vO:R:Lfc:s:FV:")) != -1) {
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
      framecount_limit = atoi(optarg);
      break;
    case 's':
      frame_skip = atoi(optarg);
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

  // Initalizing these to NULL prevents segfaults!
  AVFormatContext   *pFormatCtx = NULL;
  int               i, videoStream;
  AVCodecContext    *pCodecCtxOrig = NULL;
  AVCodecContext    *pCodecCtx = NULL;
  AVCodec           *pCodec = NULL;
  AVPacket          packet;
  int               frameFinished;

  const char *movie_file = argv[optind];

  // Register all formats and codecs
  av_register_all();
  avformat_network_init();

  // Open video file
  if(avformat_open_input(&pFormatCtx, movie_file, NULL, NULL)!=0)
    return -1; // Couldn't open file

  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information

  // Dump information about file onto standard error
  if (verbose) {
    av_dump_format(pFormatCtx, 0, movie_file, 0);
  }

  long frame_count = 0;
  runtime_opt.do_gpio_init = (stream_output_fd < 0);
  RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    return 1;
  }

  FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
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
  // Find the first video stream
  videoStream=-1;
  for (i=0; i < (int)pFormatCtx->nb_streams; ++i) {
    if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  }
  if (videoStream == -1)
    return -1; // Didn't find a video stream

  // Get a pointer to the codec context for the video stream
  pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
  double fps = av_q2d(pFormatCtx->streams[videoStream]->avg_frame_rate);
  if (fps < 0) {
    fps = 1.0 / av_q2d(pFormatCtx->streams[videoStream]->codec->time_base);
  }
  if (verbose) fprintf(stderr, "FPS: %f\n", fps);

  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
  if (pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1;
  }
  // Copy context
  pCodecCtx = avcodec_alloc_context3(pCodec);
  if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
    fprintf(stderr, "Couldn't copy codec context");
    return -1;
  }

  // Open codec
  if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return -1;


  /*
   * Prepare frame to hold the scaled target frame to be send to matrix.
   */
  AVFrame *output_frame = av_frame_alloc();  // Target frame for output
  int display_width = pCodecCtx->width;
  int display_height = pCodecCtx->height;
  if (maintain_aspect_ratio) {
    display_width = pCodecCtx->width;
    display_height = pCodecCtx->height;
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

  // Allocate buffer to meet output size requirements
  const size_t output_size = avpicture_get_size(AV_PIX_FMT_RGB24,
                                                display_width,
                                                display_height);
  uint8_t *output_buffer = (uint8_t *) av_malloc(output_size);

  // Assign appropriate parts of buffer to image planes in output_frame.
  // Note that output_frame is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)output_frame, output_buffer, AV_PIX_FMT_RGB24,
                 display_width, display_height);

  if (verbose) {
    fprintf(stderr, "Scaling %dx%d -> %dx%d; black border x:%d y:%d\n",
            pCodecCtx->width, pCodecCtx->height,
            display_width, display_height,
            display_offset_x, display_offset_y);
  }

  // initialize SWS context for software scaling
  SwsContext *const sws_ctx = CreateSWSContext(pCodecCtx,
                                               display_width, display_height);
  if (!sws_ctx) {
    fprintf(stderr, "Trouble doing scaling to %dx%d :(\n",
            matrix->width(), matrix->height());
    return 1;
  }

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  const long frame_wait_nanos = 1e9 / fps;
  struct timespec next_frame;

  AVFrame *decode_frame = av_frame_alloc();  // Decode video into this
  do {
    unsigned int frames_left = framecount_limit;
    unsigned int frames_to_skip = frame_skip;
    if (forever) {
      av_seek_frame(pFormatCtx, videoStream, 0, AVSEEK_FLAG_ANY);
      avcodec_flush_buffers(pCodecCtx);
    }
    clock_gettime(CLOCK_MONOTONIC, &next_frame);
    while (!interrupt_received && av_read_frame(pFormatCtx, &packet) >= 0
           && frames_left > 0) {
      // Is this a packet from the video stream?
      if (packet.stream_index==videoStream) {
        // Determine absolute end of this frame now so that we don't include
        // decoding overhead. TODO: skip frames if getting too slow ?
        add_nanos(&next_frame, frame_wait_nanos);

        // Decode video frame
        avcodec_decode_video2(pCodecCtx, decode_frame, &frameFinished, &packet);

        if (frames_to_skip) { frames_to_skip--; continue; }

        // Did we get a video frame?
        if (frameFinished) {
          // Convert the image from its native format to RGB
          sws_scale(sws_ctx, (uint8_t const * const *)decode_frame->data,
                    decode_frame->linesize, 0, pCodecCtx->height,
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
        }
        if (!stream_writer && !use_vsync_for_frame_timing) {
          clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_frame, NULL);
        }
      }
      // Free the packet that was allocated by av_read_frame
      av_free_packet(&packet);
    }
  } while (forever && !interrupt_received);

  if (interrupt_received) {
    // Feedback for Ctrl-C, but most importantly, force a newline
    // at the output, so that commandline-shell editing is not messed up.
    fprintf(stderr, "Got interrupt. Exiting\n");
  }

  delete matrix;

  av_free(output_buffer);
  av_frame_free(&output_frame);
  av_frame_free(&decode_frame);

  // Close the codecs
  avcodec_close(pCodecCtx);
  avcodec_close(pCodecCtxOrig);

  // Close the video file
  avformat_close_input(&pFormatCtx);

  delete stream_writer;
  delete stream_io;
  fprintf(stderr, "Total of %ld frames decoded\n", frame_count);

  return 0;
}
