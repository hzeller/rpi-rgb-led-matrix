#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
//#include <algorithm>

//using std::min;
//using std::max;

using namespace rgb_matrix;



// This is an example how to use the Canvas abstraction to map coordinates.
//
// This is a Canvas that delegates to some other Canvas (typically, the RGB
// matrix).
//
// Here, we want to address four 32x32 panels as one big 64x64 panel. Physically,
// we chain them together and do a 180 degree 'curve', somewhat like this:
// [>] [>]
//         v
// [<] [<]
class LargeSquare64x64Canvas : public Canvas {
public:
  // This class takes over ownership of the delegatee.
  LargeSquare64x64Canvas(Canvas *delegatee) : delegatee_(delegatee) {
    // Our assumptions of the underlying geometry:
    assert(delegatee->height() == 32);
    assert(delegatee->width() == 128);
  }
  virtual ~LargeSquare64x64Canvas() { delete delegatee_; }

  virtual void Clear() { delegatee_->Clear(); }
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) {
    delegatee_->Fill(red, green, blue);
  }
  virtual int width() const { return 64; }
  virtual int height() const { return 64; }
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue) {
    if (x < 0 || x >= width() || y < 0 || y >= height()) return;
    // We have up to column 64 one direction, then folding around. Lets map
    if (y > 31) {
      x = 127 - x;
      y = 63 - y;
    }
    delegatee_->SetPixel(x, y, red, green, blue);
  }

private:
  Canvas *delegatee_;
};


// Abelian sandpile
class Sandpile : public ThreadedCanvasManipulator {
private:
	int width_;
	int height_;
	int** values_;
	int** newValues_;
	int delay_ms_;
	
	void updateValues() {
		// Copy values to newValues
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				newValues_[x][y] = values_[x][y];
			}
		}
		// Update newValues based on values
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				if (values_[x][y] > 3) {
					// Collapse
					if (x>0)
						newValues_[x-1][y]++;
					if (x<width_-1)
						newValues_[x+1][y]++;
					if (y>0)
						newValues_[x][y-1]++;
					if (y<height_-1)
						newValues_[x][y+1]++;
					newValues_[x][y] -= 4;
				}
			}
		}
		// Copy newValues to values
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				values_[x][y] = newValues_[x][y];
			}
		}
	}
public:
	Sandpile(Canvas *m, int delay_ms=50) : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
		width_ = canvas()->width() - 1; // We need an odd width
		height_ = canvas()->height() - 1; // We need an odd height
		
		// Allocate memory
		values_ = new int*[width_];
		for (int x=0; x<width_; ++x) {
			values_[x] = new int[height_];
		}
		newValues_ = new int*[width_];
		for (int x=0; x<width_; ++x) {
			newValues_[x] = new int[height_];
		}
		
		// Init values
		srand(time(NULL));
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				values_[x][y] = 0;
			}
		}
	}
	~Sandpile() {
		for (int x=0; x<width_; ++x) {
			delete [] values_[x];
		}
		delete [] values_;
		for (int x=0; x<width_; ++x) {
			delete [] newValues_[x];
		}
		delete [] newValues_;
	}
	void Run() {
		while (running()) {
			// Drop a sand grain in the centre
			values_[width_/2][height_/2]++;
			updateValues();
			
			for (int x=0; x<width_; ++x) {
				for (int y=0; y<height_; ++y) {
					switch (values_[x][y]) {
						case 0:
							canvas()->SetPixel(x, y, 0, 0, 0);
							break;
						case 1:
							canvas()->SetPixel(x, y, 0, 0, 200);
							break;
						case 2:
							canvas()->SetPixel(x, y, 0, 200, 0);
							break;
						case 3:
							canvas()->SetPixel(x, y, 150, 100, 0);
							break;
						default:
							canvas()->SetPixel(x, y, 200, 0, 0);
					}
				}
			}
			usleep(delay_ms_ * 1000); // ms
		}
	}
};



// Conway's game of life
// Next define makes the matrix a torus, comment it out to make the edges dead cells
#define GAME_LIFE_TORUS
class GameLife : public ThreadedCanvasManipulator {
private:
	int** values_;
	int** newValues_;
	int delay_ms_;
	int r_;
	int g_;
	int b_;
	int width_;
	int height_;
	
	int numAliveNeighbours(int x, int y) {
		int num=0;
#ifdef GAME_LIFE_TORUS
		// Edges are connected (torus)
		num += values_[(x-1+width_)%width_][(y-1+height_)%height_];
		num += values_[(x-1+width_)%width_][y                    ];
		num += values_[(x-1+width_)%width_][(y+1        )%height_];
		num += values_[(x+1       )%width_][(y-1+height_)%height_];
		num += values_[(x+1       )%width_][y                    ];
		num += values_[(x+1       )%width_][(y+1        )%height_];
		num += values_[x                  ][(y-1+height_)%height_];
		num += values_[x                  ][(y+1        )%height_];
#else
		// Edges are not connected (no torus)
		if (x>0) {
			if (y>0)
				num += values_[x-1][y-1];
			if (y<height_-1)
				num += values_[x-1][y+1];
			num += values_[x-1][y];
		}
		if (x<width_-1) {
			if (y>0)
				num += values_[x+1][y-1];
			if (y<31)
				num += values_[x+1][y+1];
			num += values_[x+1][y];
		}
		if (y>0)
			num += values_[x][y-1];
		if (y<height_-1)
			num += values_[x][y+1];
#endif
		return num;
	}
	
	void updateValues() {
		// Copy values to newValues
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				newValues_[x][y] = values_[x][y];
			}
		}
		// update newValues based on values
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				int num = numAliveNeighbours(x,y);
				if (values_[x][y]) {
					// cell is alive
					if (num < 2 || num > 3)
						newValues_[x][y] = 0;
				}
				else {
					// cell is dead
					if (num == 3)
						newValues_[x][y] = 1;
				}
			}
		}
		// copy newValues to values
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				values_[x][y] = newValues_[x][y];
			}
		}
	}
public:
	GameLife(Canvas *m, int delay_ms=500) : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
		width_ = canvas()->width();
		height_ = canvas()->height();
		
		// Allocate memory
		values_ = new int*[width_];
		for (int x=0; x<width_; ++x) {
			values_[x] = new int[height_];
		}
		newValues_ = new int*[width_];
		for (int x=0; x<width_; ++x) {
			newValues_[x] = new int[height_];
		}
		
		// Init values randomly
		srand(time(NULL));
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				values_[x][y]=rand()%2;
			}
		}
		r_ = rand()%255;
		g_ = rand()%255;
		b_ = rand()%255;
		
		if (r_<150 && g_<150 && b_<150) {
			int c = rand()%3;
			switch (c) {
				case 0:
					r_ = 200;
					break;
				case 1:
					g_ = 200;
					break;
				case 2:
					b_ = 200;
					break;
			}
		}
		
	}
	~GameLife() {
		for (int x=0; x<width_; ++x) {
			delete [] values_[x];
		}
		delete [] values_;
		for (int x=0; x<width_; ++x) {
			delete [] newValues_[x];
		}
		delete [] newValues_;
	}
	void Run() {
		while (running()) {
			
			updateValues();
			
			for (int x=0; x<width_; ++x) {
				for (int y=0; y<height_; ++y) {
					if (values_[x][y])
						canvas()->SetPixel(x, y, r_, g_, b_);
					else
						canvas()->SetPixel(x, y, 0, 0, 0);
				}
			}
			usleep(delay_ms_ * 1000); // ms
		}
	}
};



// Langton's ant
class Ant : public ThreadedCanvasManipulator {
private:
	int numColors_;
	int** values_;
	int antX_;
	int antY_;
	int antDir_; // 0 right, 1 up, 2 left, 3 down
	int delay_ms_;
	int width_;
	int height_;
	
	void updatePixel(int x, int y) {
		switch (values_[x][y]) {
			case 0:
				canvas()->SetPixel(x, y, 200, 0, 0);
				break;
			case 1:
				canvas()->SetPixel(x, y, 0, 200, 0);
				break;
			case 2:
				canvas()->SetPixel(x, y, 0, 0, 200);
				break;
			case 3:
				canvas()->SetPixel(x, y, 150, 100, 0);
				break;
		}
		if (x == antX_ && y == antY_)
			canvas()->SetPixel(x, y, 0, 0, 0);
	}
	
public:
	Ant(Canvas *m, int delay_ms=500) : ThreadedCanvasManipulator(m), delay_ms_(delay_ms) {
		numColors_ = 4;
		width_ = canvas()->width();
		height_ = canvas()->height();
		values_ = new int*[width_];
		for (int x=0; x<width_; ++x) {
			values_[x] = new int[height_];
		}
	}
	~Ant() {
		for (int x=0; x<width_; ++x) {
			delete [] values_[x];
		}
		delete [] values_;
	}
	void Run() {
		antX_ = width_/2;
		antY_ = height_/2-3;
		antDir_ = 0;
		for (int x=0; x<width_; ++x) {
			for (int y=0; y<height_; ++y) {
				values_[x][y] = 0;
				updatePixel(x, y);
			}
		}
		
		while (running()) {
			
			// LLRR
			switch (values_[antX_][antY_]) {
				case 0:
				case 1:
					antDir_ = (antDir_+1+4) % 4;
					break;
				case 2:
				case 3:
					antDir_ = (antDir_-1+4) % 4;
					break;
			}
			
			values_[antX_][antY_] = (values_[antX_][antY_] + 1) % numColors_;
			int oldX = antX_;
			int oldY = antY_;
			switch (antDir_) {
				case 0:
					antX_++;
					break;
				case 1:
					antY_++;
					break;
				case 2:
					antX_--;
					break;
				case 3:
					antY_--;
					break;
			}
			updatePixel(oldX, oldY);
			if (antX_ < 0 || antX_ >= width_ || antY_ < 0 || antY_ >= height_)
				return;
			updatePixel(antX_, antY_);
			usleep(delay_ms_ * 1000);
		}
	}
};



// Imitation of volume bars
// Purely random height doesn't look realistic
class VolumeBars : public ThreadedCanvasManipulator {
private:
	int delay_ms_;
	int numBars_;
	int* barHeights_;
	int barWidth_;
	int height_;
	int heightGreen_;
	int heightYellow_;
	int heightOrange_;
	int heightRed_;
	int* barFreqs_;
	int* barMeans_;
	int t_;
	
	void drawBarRow(int bar, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
		for (uint8_t x=bar*barWidth_; x<(bar+1)*barWidth_; ++x) {
			canvas()->SetPixel(x, height_-1-y, r, g, b);
		}
	}
public:
	VolumeBars(Canvas *m, int delay_ms=50, int numBars=8) : ThreadedCanvasManipulator(m), delay_ms_(delay_ms), numBars_(numBars), t_(0) {
	}
	~VolumeBars() {
		delete [] barHeights_;
		delete [] barFreqs_;
		delete [] barMeans_;
	}
	void Run() {
		const int width = canvas()->width();
		height_ = canvas()->height();
		barWidth_ = width/numBars_;
		barHeights_ = new int[numBars_];
		barMeans_ = new int[numBars_];
		barFreqs_ = new int[numBars_];
		heightGreen_  = height_*4/12;
		heightYellow_ = height_*8/12;
		heightOrange_ = height_*10/12;
		heightRed_    = height_*12/12;
		
		// Array of possible bar means
		int numMeans = 10;
		int means[10] = {1,2,3,4,5,6,7,8,16,32};
		for (int i=0; i<numMeans; ++i) {
			means[i] = height_ - means[i]*height_/8;
		}
		// Initialize bar means randomly
		srand(time(NULL));
		for (int i=0; i<numBars_; ++i) {
			barMeans_[i] = rand()%numMeans;
			barFreqs_[i] = 1<<(rand()%3);
//			printf("bar %i freq=%i mean=%i\n", i, barFreqs_[i], means[barMeans_[i]]);
		}
		
		// Start the loop
		while (running()) {
			if (t_ % 8 == 0) {
				// Change the means
//				printf("bar means=[");
				for (int i=0; i<numBars_; ++i) {
					barMeans_[i] += rand()%3 - 1;
					if (barMeans_[i] >= numMeans)
						barMeans_[i] = numMeans-1;
					if (barMeans_[i] < 0)
						barMeans_[i] = 0;
//					printf("%i ",means[barMeans_[i]]);
				}
//				printf("]\n");
			}
			
			// Update bar heights
			t_++;
			for (int i=0; i<numBars_; ++i) {
				barHeights_[i] = (height_ - means[barMeans_[i]])* sin(0.1*t_*barFreqs_[i]) + means[barMeans_[i]];
				if (barHeights_[i] < height_/8)
					barHeights_[i] = rand() % (height_/8) + 1;
			}
			
			for (int i=0; i<numBars_; ++i) {
				int y;
				for (y=0; y<barHeights_[i]; ++y) {
					if (y<heightGreen_) {
						drawBarRow(i, y, 0, 200, 0);
					}
					else if (y<heightYellow_) {
						drawBarRow(i, y, 150, 150, 0);
					}
					else if (y<heightOrange_) {
						drawBarRow(i, y, 250, 100, 0);
					}
					else {
						drawBarRow(i, y, 200, 0, 0);
					}
				}
				// Anything above the bar should be black
				for (; y<height_; ++y) {
					drawBarRow(i, y, 0, 0, 0);
				}
			}
			usleep(delay_ms_ * 1000);
		}
	}
};



static int usage(const char *progname) {
  fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          progname);
  fprintf(stderr, "Options:\n"
          "\t-r <rows>         : Display rows. 16 for 16x32, 32 for 32x32. "
          "Default: 32\n"
          "\t-c <chained>      : Daisy-chained boards. Default: 1.\n"
          "\t-L                : 'Large' display, composed out of 4 times 32x32\n"
          "\t-p <pwm-bits>     : Bits used for PWM. Something between 1..11\n"
          "\t-l                : Don't do luminance correction (CIE1931)\n"
          "\t-D <demo-nr>      : Always needs to be set\n"
          "\t-d                : run as daemon. Use this when starting in\n"
          "\t                    /etc/init.d, but also when running without\n"
          "\t                    terminal (e.g. cron).\n"
          "\t-t <seconds>      : Run for these number of seconds, then exit.\n"
          "\t       (if neither -d nor -t are supplied, waits for <RETURN>)\n"
          "\t-m <milliseconds> : Number of milliseconds each time step takes.\n");
  fprintf(stderr, "Demos, choosen with -D\n");
  fprintf(stderr, "\t0  - Abelian sandpile model\n"
          "\t1  - Conway's game of life\n"
          "\t2  - Langton's ant\n"
          "\t3  - Volume bars (random)\n");
  fprintf(stderr, "Example:\n\t%s -t 10 -D 0 -m 50\n"
          "Displays the sandpile model for 10 seconds with time steps of 50 milli seconds\n", progname);
  return 1;
}

int main(int argc, char *argv[]) {
  bool as_daemon = false;
  int runtime_seconds = -1;
  int demo = -1;
  int rows = 32;
  int chain = 1;
  int step_ms = 50;
  int pwm_bits = -1;
  bool large_display = false;
  bool do_luminance_correct = true;

  const char *demo_parameter = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "dlD:t:r:p:c:m:L")) != -1) {
    switch (opt) {
    case 'D':
      demo = atoi(optarg);
      break;

    case 'd':
      as_daemon = true;
      break;

    case 't':
      runtime_seconds = atoi(optarg);
      break;

    case 'r':
      rows = atoi(optarg);
      break;

    case 'c':
      chain = atoi(optarg);
      break;

    case 'm':
      step_ms = atoi(optarg);
      break;

    case 'p':
      pwm_bits = atoi(optarg);
      break;

    case 'l':
      do_luminance_correct = !do_luminance_correct;
      break;

    case 'L':
      // The 'large' display assumes a chain of four displays with 32x32
      chain = 4;
      rows = 32;
      large_display = true;
      break;

    default: /* '?' */
      return usage(argv[0]);
    }
  }

  if (optind < argc) {
    demo_parameter = argv[optind];
  }

  if (demo < 0) {
    fprintf(stderr, "Expected required option -D <demo>\n");
    return usage(argv[0]);
  }

  if (getuid() != 0) {
    fprintf(stderr, "Must run as root to be able to access /dev/mem\n"
            "Prepend 'sudo' to the command:\n\tsudo %s ...\n", argv[0]);
    return 1;
  }

  if (rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can either be 16 or 32\n");
    return 1;
  }

  if (chain < 1) {
    fprintf(stderr, "Chain outside usable range\n");
    return 1;
  }
  if (chain > 8) {
    fprintf(stderr, "That is a long chain. Expect some flicker.\n");
  }

  // Initialize GPIO pins. This might fail when we don't have permissions.
  GPIO io;
  if (!io.Init())
    return 1;

  // Start daemon before we start any threads.
  if (as_daemon) {
    if (fork() != 0)
      return 0;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  // The matrix, our 'frame buffer' and display updater.
  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain);
  matrix->set_luminance_correct(do_luminance_correct);
  if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
    fprintf(stderr, "Invalid range of pwm-bits\n");
    return 1;
  }

  Canvas *canvas = matrix;

  if (large_display) {
    // Mapping the coordinates of a 32x128 display mapped to a square of 64x64
    canvas = new LargeSquare64x64Canvas(canvas);
  }

  // The ThreadedCanvasManipulator objects are filling
  // the matrix continuously.
  ThreadedCanvasManipulator *image_gen = NULL;
  switch (demo) {
  case 0:
    image_gen = new Sandpile(canvas, step_ms);
    break;

  case 1:
    image_gen = new GameLife(canvas, step_ms);
    break;

  case 2:
    image_gen = new Ant(canvas, step_ms);
    break;

  case 3:
    image_gen = new VolumeBars(canvas, step_ms, canvas->width()/2);
    break;
  }

  if (image_gen == NULL)
    return usage(argv[0]);

  // Image generating demo is crated. Now start the thread.
  image_gen->Start();

  // Now, the image genreation runs in the background. We can do arbitrary
  // things here in parallel. In this demo, we're essentially just
  // waiting for one of the conditions to exit.
  if (as_daemon) {
    sleep(runtime_seconds > 0 ? runtime_seconds : INT_MAX);
  } else if (runtime_seconds > 0) {
    sleep(runtime_seconds);
  } else {
    // Things are set up. Just wait for <RETURN> to be pressed.
    printf("Press <RETURN> to exit and reset LEDs\n");
    getchar();
  }

  // Stop image generating thread.
  delete image_gen;
  delete canvas;

  return 0;
}
