/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
Compile in rpi-rgb-led-matrix directory.
g++ -Wall -O3 -g -Iinclude zmq.cc -o zmq -Llib -lrgbmatrix -lrt -lm -lpthread -lzmq
*/
#include "led-matrix.h"

#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

#include <zmq.hpp>

const int kListenPort = 5555;
const int kRows = 32;
const int kChain = 4;
const int kParallel = 1;

zmq::context_t context (1);

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
  free(context);
}

static FrameCanvas *FillFramebuffer(RGBMatrix *matrix, FrameCanvas *canvas,
const char *buffer) {
  const int width = matrix->width();
  const int height = matrix->height();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int r = *buffer++;
      int g = *buffer++;
      int b = *buffer++;
      canvas->SetPixel(x, y, r, g, b);
    }
  }
  return matrix->SwapOnVSync(canvas);
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options>\n",
  progname);
  fprintf(stderr, "Options:\n"
    "\t-r <rows>     : Panel rows. '16' for 16x32 (1:8 multiplexing),\n"
      "\t                '32' for 32x32 (1:16), '8' for 1:4 multiplexing; 64 for 1:32 multiplexing. "
        "Default: 32\n"
          "\t-P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. "
            "Default: 1\n"
              "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
                "\t-p <port>     : port to listen on. Default: 5555\n");
  return 1;
}

int main(int argc, char *argv[]) {
  GPIO io;
  int rows = 32;
  int chain = 1;
  int parallel = 1;
  int port = 5555;

  int opt;
  while ((opt = getopt(argc, argv, "dlD:t:r:P:c:p:b:m:LR:")) != -1) {
    switch (opt) {
      case 'r':
      rows = atoi(optarg);
      break;

      case 'P':
      parallel = atoi(optarg);
      break;

      case 'c':
      chain = atoi(optarg);
      break;

      case 'p':
      port = atoi(optarg);
      break;

      default: /* '?' */
      return usage(argv[0]);
    }
  }
  
  char zmq_bind[128];
  snprintf(zmq_bind, 128,"tcp://*:%u", port);

  if (!io.Init())
    return 1;

  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);
  FrameCanvas *swap_buffer = matrix->CreateFrameCanvas();

  const uint framebuffer_size = matrix->width() * matrix->height() * 3;
  char *const packet_buffer = new char[framebuffer_size];

  struct sigaction sa;
  sa.sa_handler = InterruptHandler;
  sa.sa_flags = SA_RESETHAND | SA_NODEFER;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT,  &sa, NULL);

  zmq::socket_t socket (context, ZMQ_PULL);
  int linger = 0;
  socket.setsockopt(ZMQ_LINGER,&linger,sizeof(int));
  socket.bind (zmq_bind);

  matrix->Clear();

  while (!interrupt_received) {
    zmq::message_t request;

    try{
      //  Wait for next request from client
      while(-1 == socket.recv (&request))
      {
        if (EAGAIN != errno || interrupt_received)
        {
          break;
        }
      }
    
      if (interrupt_received)
        break;
    
      if (request.size() == framebuffer_size) {
        memcpy(packet_buffer, request.data(),request.size());
      } else {
        continue;
      }

      swap_buffer = FillFramebuffer(matrix, swap_buffer, packet_buffer);
    } catch(zmq::error_t e) {
      break;
    }
  }

  delete matrix;

  return 0;
}
