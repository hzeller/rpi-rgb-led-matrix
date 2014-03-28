#include "led-matrix.h"
#include "thread.h"

#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>

int file_count = 0;
int folder_count = 0;
char path[100];
int frame_delay;
char folders[1000][100];

// Base-class for a Thread that does something with a matrix.
class RGBMatrixManipulator : public Thread {
public:
  RGBMatrixManipulator(RGBMatrix *m) : running_(true), matrix_(m) {}
  virtual ~RGBMatrixManipulator() { running_ = false; }

  // Run() implementation needs to check running_ regularly.

protected:
  volatile bool running_;  // TODO: use mutex, but this is good enough for now.
  RGBMatrix *const matrix_;
};

// Pump pixels to screen. Needs to be high priority real-time because jitter
// here will make the PWM uneven.
class DisplayUpdater : public RGBMatrixManipulator {
public:
  DisplayUpdater(RGBMatrix *m) : RGBMatrixManipulator(m) {}

  void Run() {
    while (running_) {
      matrix_->UpdateScreen();
    }
  }
};

// -- The following are demo image generators.

// Simple generator that goes through RGB for all pixel.
class ColorGenerator : public RGBMatrixManipulator {
public:
  ColorGenerator(RGBMatrix *m) : RGBMatrixManipulator(m) {}
  void Run() {
    const int columns = matrix_->columns();
    while (running_) {
      for (int x = 0; x < columns; ++x)
        for (int y = 0; y < 32; ++y)
          matrix_->SetPixel(x, y, 255, 0, 0);
      usleep(500000);
      for (int x = 0; x < columns; ++x)
        for (int y = 0; y < 32; ++y)
          matrix_->SetPixel(x, y, 0, 255, 0);
      usleep(500000);
      for (int x = 0; x < columns; ++x)
        for (int y = 0; y < columns; ++y)
          matrix_->SetPixel(x, y, 0, 0, 255);
      usleep(500000);
    }
  }
};

// Simple generator that pulses through RGB for all pixel.
class ColorPulseGenerator : public RGBMatrixManipulator {
public:
  ColorPulseGenerator(RGBMatrix *m) : RGBMatrixManipulator(m) {}
  void Run() {
	const int columns = matrix_->columns();
	while (running_) {
	    for (int z = 0; z < 256; z++) {
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					matrix_->SetPixel(x, y, z, 0, 0);
				}
			}
			usleep(2000);
		}
	    for (int z = 254; z >= 0; z--) {
		    for (int x = 0; x < columns; ++x) {
			    for (int y = 0; y < 32; ++y) {
				    matrix_->SetPixel(x, y, z, 0, 0);
			    }
		    }
		    usleep(2000);
	    }
		for (int z = 1; z < 256; z++) {
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					matrix_->SetPixel(x, y, 0, z, 0);
				}
			}
			usleep(2000);
		}
		for (int z = 254; z >= 0; z--) {
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					matrix_->SetPixel(x, y, 0, z, 0);
				}
			}
			usleep(2000);
		}
		for (int z = 1; z < 256; z++) {
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					matrix_->SetPixel(x, y, 0, 0, z);
				}
			}
			usleep(2000);
		}
		for (int z = 254; z > 0; z--) {
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					matrix_->SetPixel(x, y, 0, 0, z);
				}
			}
			usleep(2000);
		}
    }
  }
};

// Simple class that simulates rain on the screen.
class RotatingBlockGenerator : public RGBMatrixManipulator {
public:
  RotatingBlockGenerator(RGBMatrix *m) : RGBMatrixManipulator(m) {}
  void Run() {
	int start[65][2], weiter[65][2], geschw[65];
	srand(time(NULL));
	matrix_->FillScreen(0, 0, 0);
	for (int i = 64; i >= 0; i--) {
      start[i][0] = rand() % 32;
	  start[i][1] = rand() % 32;
	  weiter[i][0] = start[i][0];
	  weiter[i][1] = start[i][1];
	  matrix_->SetPixel(start[i][0], start[i][1], 0, 0, 255);
	  geschw[i] = 1;
	}
	while (running_) {
	  matrix_->FillScreen(0, 0, 0);
	  for (int i = 64; i >= 0; i--) {
	    if ((start[i][1] + 1) <= 0) {
		  start[i][0] = rand() % 32;
		  start[i][1] = 31;
		  weiter[i][0] = start[i][0];
		  weiter[i][1] = start[i][1];
		  matrix_->SetPixel(start[i][0], start[i][1], 0, 0, 255);
		}
		start[i][1] = weiter[i][1];
		weiter[i][1] = start[i][1] - geschw[i];
		matrix_->SetPixel(start[i][0], start[i][1] + 1, 0, 0, 25);
		matrix_->SetPixel(start[i][0], start[i][1], 0, 0, 80);
		matrix_->SetPixel(weiter[i][0], weiter[i][1], 0, 0, 255);
	  }
	  usleep(50000);
	}
  }
};

// Clock design 1.
class TestGen : public RGBMatrixManipulator {
public:
	TestGen(RGBMatrix *m) : RGBMatrixManipulator(m) {}
	void Run() {
		int color[32][32][3], min, hour, sec, usec, daytime, columns;

		matrix_->FillScreen(0, 0, 0);
		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				for (int k = 0; k < 3; k++) {
					color[i][j][k] = 0;
				}
			}
		}

		time_t mytime;
		mytime = time(NULL);
		struct tm *tm_p;
		tm_p = localtime(&mytime);
		timeval time_val;
		gettimeofday(&time_val, 0);

		sec = tm_p->tm_sec;
		columns = (int)((int)((int)(sec - 1) / 5) + 1);
		columns = (columns * 2) - 1;
		if (sec > 0) {
			for (int i = 1; i <= columns; i += 2) {
				if (i < columns) {
					for (int j = 1; j < 31; j++) {
						color[i][j][0] = 255;
					}
				}
				else {
					for (int j = 1; j <= (sec - ((((columns + 1) / 2) - 1) * 5)) * 6; j++) {
						color[i][j][0] = 255;
					}
				}
			}
		}
		else {
			for (int i = 1; i <= 23; i += 2) {
				for (int j = 1; j <= 31; j++) {
					color[i][j][0] = 0;
				}
			}
		}

		usec = time_val.tv_usec;
		if (usec > 0 && (sec % 5 != 0 || sec == 0)) {
			for (int i = 1; i <= columns; i += 2) {
				if (i < columns) {
					for (int j = 1; j < 31; j++) {
						color[i][j][0] = 255;
					}
				}
				else {
					if (usec >= 833333) {
						for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 5; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 666667) {
						for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 4; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 500000) {
						for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 3; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 333333) {
						for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 2; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 166667) {
						for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 1; j++) {
							color[i][j][0] = 255;
						}
					}
				}
			}
		}
		else if (usec > 0 && sec % 5 == 0 && sec < 60) {
			for (int i = 1; i <= columns + 2; i += 2) {
				if (i < columns + 2) {
					for (int j = 1; j < 31; j++) {
						color[i][j][0] = 255;
					}
				}
				else {
					if (usec >= 833333) {
						for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 5; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 666667) {
						for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 4; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 500000) {
						for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 3; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 333333) {
						for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 2; j++) {
							color[i][j][0] = 255;
						}
					}
					else if (usec >= 166667) {
						for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 1; j++) {
							color[i][j][0] = 255;
						}
					}
				}
			}
		}

		min = tm_p->tm_min;
		columns = (int)(((min - 1) / 5) + 1);
		columns = (columns * 2) - 1;
		if (min > 0) {
			for (int i = 1; i <= columns; i += 2) {
				if (i < columns) {
					for (int j = 1; j < 31; j++) {
						color[i][j][1] = 255;
					}
				}
				else {
					for (int j = 1; j <= (min - ((((columns + 1) / 2) - 1) * 5)) * 6; j++) {
						color[i][j][1] = 255;
					}
				}
			}
		}
		else {
			for (int i = 1; i <= 23; i += 2) {
				for (int j = 1; j <= 31; j++) {
					color[i][j][1] = 0;
				}
			}
		}

		hour = tm_p->tm_hour;
		if (hour > 12) {
			daytime = 1;
		}
		else {
			daytime = 0;
		}
		if (hour > 0) {
			for (int i = 1; i <= (((hour * 2) - 1) - (24 * daytime)); i += 2) {
				for (int j = 1; j < 31; j++) {
					color[i][j][2] = 255;
				}
			}
		}
		else {
			for (int i = 1; i <= 23; i += 2) {
				for (int j = 1; j <= 31; j++) {
					color[i][j][2] = 0;
				}
			}
		}

		if (daytime == 0) {
			for (int i = 0; i < 3; i++) {
				color[26][3][i] = 0;
				color[27][3][i] = 255;
				color[28][3][i] = 255;
				color[29][3][i] = 0;
				color[26][4][i] = 255;
				color[27][4][i] = 0;
				color[28][4][i] = 0;
				color[29][4][i] = 255;
				color[26][5][i] = 255;
				color[27][5][i] = 255;
				color[28][5][i] = 255;
				color[29][5][i] = 255;
				color[26][6][i] = 255;
				color[27][6][i] = 0;
				color[28][6][i] = 0;
				color[29][6][i] = 255;
				color[26][7][i] = 255;
				color[27][7][i] = 0;
				color[28][7][i] = 0;
				color[29][7][i] = 255;

				color[26][9][i] = 255;
				color[27][9][i] = 0;
				color[28][9][i] = 0;
				color[29][9][i] = 0;
				color[30][9][i] = 255;
				color[26][10][i] = 255;
				color[27][10][i] = 255;
				color[28][10][i] = 0;
				color[29][10][i] = 255;
				color[30][10][i] = 255;
				color[26][11][i] = 255;
				color[27][11][i] = 0;
				color[28][11][i] = 255;
				color[29][11][i] = 0;
				color[30][11][i] = 255;
				color[26][12][i] = 255;
				color[27][12][i] = 0;
				color[28][12][i] = 0;
				color[29][12][i] = 0;
				color[30][12][i] = 255;
				color[26][13][i] = 255;
				color[27][13][i] = 0;
				color[28][13][i] = 0;
				color[29][13][i] = 0;
				color[30][13][i] = 255;
			}
		}
		else {
			for (int i = 0; i < 3; i++) {
				color[26][18][i] = 255;
				color[27][18][i] = 255;
				color[28][18][i] = 255;
				color[29][18][i] = 0;
				color[26][19][i] = 255;
				color[27][19][i] = 0;
				color[28][19][i] = 0;
				color[29][19][i] = 255;
				color[26][20][i] = 255;
				color[27][20][i] = 255;
				color[28][20][i] = 255;
				color[29][20][i] = 0;
				color[26][21][i] = 255;
				color[27][21][i] = 0;
				color[28][21][i] = 0;
				color[29][21][i] = 0;
				color[26][22][i] = 255;
				color[27][22][i] = 0;
				color[28][22][i] = 0;
				color[29][22][i] = 0;

				color[26][24][i] = 255;
				color[27][24][i] = 0;
				color[28][24][i] = 0;
				color[29][24][i] = 0;
				color[30][24][i] = 255;
				color[26][25][i] = 255;
				color[27][25][i] = 255;
				color[28][25][i] = 0;
				color[29][25][i] = 255;
				color[30][25][i] = 255;
				color[26][26][i] = 255;
				color[27][26][i] = 0;
				color[28][26][i] = 255;
				color[29][26][i] = 0;
				color[30][26][i] = 255;
				color[26][27][i] = 255;
				color[27][27][i] = 0;
				color[28][27][i] = 0;
				color[29][27][i] = 0;
				color[30][27][i] = 255;
				color[26][28][i] = 255;
				color[27][28][i] = 0;
				color[28][28][i] = 0;
				color[29][28][i] = 0;
				color[30][28][i] = 255;
			}
		}

		for (int i = 2; i <= 22; i += 2) {
			for (int j = 7; j <= 25; j += 6) {
				color[i][j][0] = 255;
				color[i][j][1] = 95;
				color[i][j][2] = 17;
			}
		}

		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				matrix_->SetPixel(i, j, color[i][j][0], color[i][j][1], color[i][j][2]);
			}
		}

		while (running_) {
			mytime = time(NULL);
			tm_p = localtime(&mytime);
			gettimeofday(&time_val, 0);
			if (time_val.tv_usec - usec != 0) {

				sec = tm_p->tm_sec;
				columns = (int)((int)((int)(sec - 1) / 5) + 1);
				columns = (columns * 2) - 1;
				if (sec > 0) {
					for (int i = 1; i <= columns; i += 2) {
						if (i < columns) {
							for (int j = 1; j < 31; j++) {
								color[i][j][0] = 255;
							}
						}
						else {
							for (int j = 1; j <= (sec - ((((columns + 1) / 2) - 1) * 5)) * 6; j++) {
								color[i][j][0] = 255;
							}
						}
					}
				}
				else {
					for (int i = 1; i <= 23; i += 2) {
						for (int j = 1; j <= 31; j++) {
							color[i][j][0] = 0;
						}
					}
				}

				usec = time_val.tv_usec;
				if (usec > 0 && (sec % 5 != 0 || sec == 0)) {
					for (int i = 1; i <= columns; i += 2) {
						if (i < columns) {
							for (int j = 1; j < 31; j++) {
								color[i][j][0] = 255;
							}
						}
						else {
							if (usec >= 833333) {
								for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 5; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 666667) {
								for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 4; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 500000) {
								for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 3; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 333333) {
								for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 2; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 166667) {
								for (int j = 1; j <= ((sec - ((((columns + 1) / 2) - 1) * 5)) * 6) + 1; j++) {
									color[i][j][0] = 255;
								}
							}
						}
					}
				}
				else if (usec > 0 && sec % 5 == 0 && sec < 60) {
					for (int i = 1; i <= columns + 2; i += 2) {
						if (i < columns + 2) {
							for (int j = 1; j < 31; j++) {
								color[i][j][0] = 255;
							}
						}
						else {
							if (usec >= 833333) {
								for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 5; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 666667) {
								for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 4; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 500000) {
								for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 3; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 333333) {
								for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 2; j++) {
									color[i][j][0] = 255;
								}
							}
							else if (usec >= 166667) {
								for (int j = 1; j <= ((sec - ((((columns + 2 + 1) / 2) - 1) * 5)) * 6) + 1; j++) {
									color[i][j][0] = 255;
								}
							}
						}
					}
				}

				min = tm_p->tm_min;
				columns = (int)(((min - 1) / 5) + 1);
				columns = (columns * 2) - 1;
				if (min > 0) {
					for (int i = 1; i <= columns; i += 2) {
						if (i < columns) {
							for (int j = 1; j < 31; j++) {
								color[i][j][1] = 255;
							}
						}
						else {
							for (int j = 1; j <= (min - ((((columns + 1) / 2) - 1) * 5)) * 6; j++) {
								color[i][j][1] = 255;
							}
						}
					}
				}
				else {
					for (int i = 1; i <= 23; i += 2) {
						for (int j = 1; j <= 31; j++) {
							color[i][j][1] = 0;
						}
					}
				}

				hour = tm_p->tm_hour;
				if (hour > 12) {
					daytime = 1;
				}
				else {
					daytime = 0;
				}
				if (hour > 0) {
					for (int i = 1; i <= (((hour * 2) - 1) - (24 * daytime)); i += 2) {
						for (int j = 1; j < 31; j++) {
							color[i][j][2] = 255;
						}
					}
				}
				else {
					for (int i = 1; i <= 23; i += 2) {
						for (int j = 1; j <= 31; j++) {
							color[i][j][2] = 0;
						}
					}
				}

				if (daytime == 0) {
					for (int i = 0; i < 3; i++) {
						color[26][3][i] = 0;
						color[27][3][i] = 255;
						color[28][3][i] = 255;
						color[29][3][i] = 0;
						color[26][4][i] = 255;
						color[27][4][i] = 0;
						color[28][4][i] = 0;
						color[29][4][i] = 255;
						color[26][5][i] = 255;
						color[27][5][i] = 255;
						color[28][5][i] = 255;
						color[29][5][i] = 255;
						color[26][6][i] = 255;
						color[27][6][i] = 0;
						color[28][6][i] = 0;
						color[29][6][i] = 255;
						color[26][7][i] = 255;
						color[27][7][i] = 0;
						color[28][7][i] = 0;
						color[29][7][i] = 255;

						color[26][9][i] = 255;
						color[27][9][i] = 0;
						color[28][9][i] = 0;
						color[29][9][i] = 0;
						color[30][9][i] = 255;
						color[26][10][i] = 255;
						color[27][10][i] = 255;
						color[28][10][i] = 0;
						color[29][10][i] = 255;
						color[30][10][i] = 255;
						color[26][11][i] = 255;
						color[27][11][i] = 0;
						color[28][11][i] = 255;
						color[29][11][i] = 0;
						color[30][11][i] = 255;
						color[26][12][i] = 255;
						color[27][12][i] = 0;
						color[28][12][i] = 0;
						color[29][12][i] = 0;
						color[30][12][i] = 255;
						color[26][13][i] = 255;
						color[27][13][i] = 0;
						color[28][13][i] = 0;
						color[29][13][i] = 0;
						color[30][13][i] = 255;
					}
				}
				else {
					for (int i = 0; i < 3; i++) {
						color[26][18][i] = 255;
						color[27][18][i] = 255;
						color[28][18][i] = 255;
						color[29][18][i] = 0;
						color[26][19][i] = 255;
						color[27][19][i] = 0;
						color[28][19][i] = 0;
						color[29][19][i] = 255;
						color[26][20][i] = 255;
						color[27][20][i] = 255;
						color[28][20][i] = 255;
						color[29][20][i] = 0;
						color[26][21][i] = 255;
						color[27][21][i] = 0;
						color[28][21][i] = 0;
						color[29][21][i] = 0;
						color[26][22][i] = 255;
						color[27][22][i] = 0;
						color[28][22][i] = 0;
						color[29][22][i] = 0;

						color[26][24][i] = 255;
						color[27][24][i] = 0;
						color[28][24][i] = 0;
						color[29][24][i] = 0;
						color[30][24][i] = 255;
						color[26][25][i] = 255;
						color[27][25][i] = 255;
						color[28][25][i] = 0;
						color[29][25][i] = 255;
						color[30][25][i] = 255;
						color[26][26][i] = 255;
						color[27][26][i] = 0;
						color[28][26][i] = 255;
						color[29][26][i] = 0;
						color[30][26][i] = 255;
						color[26][27][i] = 255;
						color[27][27][i] = 0;
						color[28][27][i] = 0;
						color[29][27][i] = 0;
						color[30][27][i] = 255;
						color[26][28][i] = 255;
						color[27][28][i] = 0;
						color[28][28][i] = 0;
						color[29][28][i] = 0;
						color[30][28][i] = 255;
					}
				}

				for (int i = 0; i < 32; i++) {
					for (int j = 0; j < 32; j++) {
						matrix_->SetPixel(i, j, color[i][j][0], color[i][j][1], color[i][j][2]);
					}
				}
			}
		}
	}
};

// Clock design 2.
class TestGen2 : public RGBMatrixManipulator {
public:
	TestGen2(RGBMatrix *m) : RGBMatrixManipulator(m) {}
	void Run() {

		matrix_->FillScreen(0, 0, 0);

		int color[32][32][3], gradient[12][3], min, hour, sec, usec, daytime, columns;

		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				for (int k = 0; k < 3; k++) {
					color[i][j][k] = 0;
				}
			}
		}

		gradient[0][0] = 255;
		gradient[0][1] = 0;
		gradient[0][2] = 0;
		gradient[1][0] = 255;
		gradient[1][1] = 128;
		gradient[1][2] = 0;
		gradient[2][0] = 255;
		gradient[2][1] = 255;
		gradient[2][2] = 0;
		gradient[3][0] = 128;
		gradient[3][1] = 255;
		gradient[3][2] = 0;
		gradient[4][0] = 0;
		gradient[4][1] = 255;
		gradient[4][2] = 0;
		gradient[5][0] = 0;
		gradient[5][1] = 255;
		gradient[5][2] = 128;
		gradient[6][0] = 0;
		gradient[6][1] = 255;
		gradient[6][2] = 255;
		gradient[7][0] = 0;
		gradient[7][1] = 128;
		gradient[7][2] = 255;
		gradient[8][0] = 0;
		gradient[8][1] = 0;
		gradient[8][2] = 255;
		gradient[9][0] = 128;
		gradient[9][1] = 0;
		gradient[9][2] = 255;
		gradient[10][0] = 255;
		gradient[10][1] = 0;
		gradient[10][2] = 255;
		gradient[11][0] = 255;
		gradient[11][1] = 0;
		gradient[11][2] = 128;

		time_t mytime;
		mytime = time(NULL);
		struct tm *tm_p;
		tm_p = localtime(&mytime);
		timeval time_val;
		gettimeofday(&time_val, 0);
		usec = time_val.tv_usec;
		sec = tm_p->tm_sec;
		min = tm_p->tm_min;
		hour = tm_p->tm_hour;

		columns = (int)((int)((int)(sec - 1) / 5) + 1);

		if (sec > 0) {
			for (int i = 8; i <= (((columns * 2) - 1) + 7); i += 2) {
				if (i < (((columns * 2) - 1) + 7)) {
					for (int j = 17; j <= 21; j++) {
						color[i][j][0] = gradient[((i / 2) - 4)][0];
						color[i][j][1] = gradient[((i / 2) - 4)][1];
						color[i][j][2] = gradient[((i / 2) - 4)][2];
					}
				}
				else {
					for (int j = 17; j <= (sec + (16 + ((columns - 1) * -5))); j++) {
						color[i][j][0] = gradient[((i / 2) - 4)][0];
						color[i][j][1] = gradient[((i / 2) - 4)][1];
						color[i][j][2] = gradient[((i / 2) - 4)][2];
					}
				}
			}
		}
		else {
			for (int i = 8; i <= 30; i += 2) {
				for (int j = 17; j <= 21; j++) {
					color[i][j][0] = 0;
					color[i][j][1] = 0;
					color[i][j][2] = 0;
				}
			}
		}

		columns = (int)(((min - 1) / 5) + 1);

		if (min > 0) {
			for (int i = 8; i <= (((columns * 2) - 1) + 7); i += 2) {
				if (i < (((columns * 2) - 1) + 7)) {
					for (int j = 9; j <= 13; j++) {
						color[i][j][0] = gradient[((i / 2) - 4)][0];
						color[i][j][1] = gradient[((i / 2) - 4)][1];
						color[i][j][2] = gradient[((i / 2) - 4)][2];
					}
				}
				else {
					for (int j = 9; j <= (min + (8 + ((columns - 1) * -5))); j++) {
						color[i][j][0] = gradient[((i / 2) - 4)][0];
						color[i][j][1] = gradient[((i / 2) - 4)][1];
						color[i][j][2] = gradient[((i / 2) - 4)][2];
					}
				}
			}
		}
		else {
			for (int i = 8; i <= 30; i += 2) {
				for (int j = 9; j <= 13; j++) {
					color[i][j][0] = 0;
					color[i][j][1] = 0;
					color[i][j][2] = 0;
				}
			}
		}

		if (hour > 12) {
			daytime = 1;
			for (int i = 0; i < 3; i++) {
				color[11][26][i] = 255;
				color[12][26][i] = 255;
				color[13][26][i] = 255;
				color[14][26][i] = 0;
				color[11][27][i] = 255;
				color[12][27][i] = 0;
				color[13][27][i] = 0;
				color[14][27][i] = 255;
				color[11][28][i] = 255;
				color[12][28][i] = 255;
				color[13][28][i] = 255;
				color[14][28][i] = 0;
				color[11][29][i] = 255;
				color[12][29][i] = 0;
				color[13][29][i] = 0;
				color[14][29][i] = 0;
				color[11][30][i] = 255;
				color[12][30][i] = 0;
				color[13][30][i] = 0;
				color[14][30][i] = 0;

				color[16][26][i] = 255;
				color[17][26][i] = 0;
				color[18][26][i] = 0;
				color[19][26][i] = 0;
				color[20][26][i] = 255;
				color[16][27][i] = 255;
				color[17][27][i] = 255;
				color[18][27][i] = 0;
				color[19][27][i] = 255;
				color[20][27][i] = 255;
				color[16][28][i] = 255;
				color[17][28][i] = 0;
				color[18][28][i] = 255;
				color[19][28][i] = 0;
				color[20][28][i] = 255;
				color[16][29][i] = 255;
				color[17][29][i] = 0;
				color[18][29][i] = 0;
				color[19][29][i] = 0;
				color[20][29][i] = 255;
				color[16][30][i] = 255;
				color[17][30][i] = 0;
				color[18][30][i] = 0;
				color[19][30][i] = 0;
				color[20][30][i] = 255;
			}
		}
		else {
			daytime = 0;
			for (int i = 0; i < 3; i++) {
				color[11][26][i] = 0;
				color[12][26][i] = 255;
				color[13][26][i] = 255;
				color[14][26][i] = 0;
				color[11][27][i] = 255;
				color[12][27][i] = 0;
				color[13][27][i] = 0;
				color[14][27][i] = 255;
				color[11][28][i] = 255;
				color[12][28][i] = 255;
				color[13][28][i] = 255;
				color[14][28][i] = 255;
				color[11][29][i] = 255;
				color[12][29][i] = 0;
				color[13][29][i] = 0;
				color[14][29][i] = 255;
				color[11][30][i] = 255;
				color[12][30][i] = 0;
				color[13][30][i] = 0;
				color[14][30][i] = 255;

				color[16][26][i] = 255;
				color[17][26][i] = 0;
				color[18][26][i] = 0;
				color[19][26][i] = 0;
				color[20][26][i] = 255;
				color[16][27][i] = 255;
				color[17][27][i] = 255;
				color[18][27][i] = 0;
				color[19][27][i] = 255;
				color[20][27][i] = 255;
				color[16][28][i] = 255;
				color[17][28][i] = 0;
				color[18][28][i] = 255;
				color[19][28][i] = 0;
				color[20][28][i] = 255;
				color[16][29][i] = 255;
				color[17][29][i] = 0;
				color[18][29][i] = 0;
				color[19][29][i] = 0;
				color[20][29][i] = 255;
				color[16][30][i] = 255;
				color[17][30][i] = 0;
				color[18][30][i] = 0;
				color[19][30][i] = 0;
				color[20][30][i] = 255;
			}
		}

		if (hour == 0 || hour == 13) {
			for (int i = 8; i <= 30; i += 2) {
				for (int j = 1; j <= 5; j++) {
					color[i][j][0] = 0;
					color[i][j][1] = 0;
					color[i][j][2] = 0;
				}
			}
		}

		if (hour > 0) {
			for (int i = 8; i <= ((((hour * 2) - 1) - (24 * daytime)) + 7); i += 2) {
				for (int j = 1; j <= 5; j++) {
					color[i][j][0] = gradient[((i / 2) - 4)][0];
					color[i][j][1] = gradient[((i / 2) - 4)][1];
					color[i][j][2] = gradient[((i / 2) - 4)][2];
				}
			}
		}

		for (int i = 0; i < 3; i++) {
			color[1][1][i] = 255;
			color[2][1][i] = 0;
			color[3][1][i] = 0;
			color[4][1][i] = 255;
			color[1][2][i] = 255;
			color[2][2][i] = 0;
			color[3][2][i] = 0;
			color[4][2][i] = 255;
			color[1][3][i] = 255;
			color[2][3][i] = 255;
			color[3][3][i] = 255;
			color[4][3][i] = 255;
			color[1][4][i] = 255;
			color[2][4][i] = 0;
			color[3][4][i] = 0;
			color[4][4][i] = 255;
			color[1][5][i] = 255;
			color[2][5][i] = 0;
			color[3][5][i] = 0;
			color[4][5][i] = 255;

			color[1][9][i] = 255;
			color[2][9][i] = 0;
			color[3][9][i] = 0;
			color[4][9][i] = 0;
			color[5][9][i] = 255;
			color[1][10][i] = 255;
			color[2][10][i] = 255;
			color[3][10][i] = 0;
			color[4][10][i] = 255;
			color[5][10][i] = 255;
			color[1][11][i] = 255;
			color[2][11][i] = 0;
			color[3][11][i] = 255;
			color[4][11][i] = 0;
			color[5][11][i] = 255;
			color[1][12][i] = 255;
			color[2][12][i] = 0;
			color[3][12][i] = 0;
			color[4][12][i] = 0;
			color[5][12][i] = 255;
			color[1][13][i] = 255;
			color[2][13][i] = 0;
			color[3][13][i] = 0;
			color[4][13][i] = 0;
			color[5][13][i] = 255;

			color[1][17][i] = 0;
			color[2][17][i] = 255;
			color[3][17][i] = 255;
			color[4][17][i] = 255;
			color[1][18][i] = 255;
			color[2][18][i] = 0;
			color[3][18][i] = 0;
			color[4][18][i] = 0;
			color[1][19][i] = 0;
			color[2][19][i] = 255;
			color[3][19][i] = 255;
			color[4][19][i] = 0;
			color[1][20][i] = 0;
			color[2][20][i] = 0;
			color[3][20][i] = 0;
			color[4][20][i] = 255;
			color[1][21][i] = 255;
			color[2][21][i] = 255;
			color[3][21][i] = 255;
			color[4][21][i] = 0;
		}

		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				matrix_->SetPixel(i, j, color[i][j][0], color[i][j][1], color[i][j][2]);
			}
		}

		while (running_) {

			mytime = time(NULL);
			tm_p = localtime(&mytime);
			gettimeofday(&time_val, 0);

			if (time_val.tv_usec - usec != 0) {

				usec = time_val.tv_usec;
				sec = tm_p->tm_sec;
				min = tm_p->tm_min;
				hour = tm_p->tm_hour;

				columns = (int)((int)((int)(sec - 1) / 5) + 1);

				if (sec > 0) {
					for (int i = 8; i <= (((columns * 2) - 1) + 7); i += 2) {
						if (i < (((columns * 2) - 1) + 7)) {
							for (int j = 17; j <= 21; j++) {
								color[i][j][0] = gradient[((i / 2) - 4)][0];
								color[i][j][1] = gradient[((i / 2) - 4)][1];
								color[i][j][2] = gradient[((i / 2) - 4)][2];
							}
						}
						else {
							for (int j = 17; j <= (sec + (16 + ((columns - 1) * -5))); j++) {
								color[i][j][0] = gradient[((i / 2) - 4)][0];
								color[i][j][1] = gradient[((i / 2) - 4)][1];
								color[i][j][2] = gradient[((i / 2) - 4)][2];
							}
						}
					}
				}
				else {
					for (int i = 8; i <= 30; i += 2) {
						for (int j = 17; j <= 21; j++) {
							color[i][j][0] = 0;
							color[i][j][1] = 0;
							color[i][j][2] = 0;
						}
					}
				}

				columns = (int)(((min - 1) / 5) + 1);

				if (min > 0) {
					for (int i = 8; i <= (((columns * 2) - 1) + 7); i += 2) {
						if (i < (((columns * 2) - 1) + 7)) {
							for (int j = 9; j <= 13; j++) {
								color[i][j][0] = gradient[((i / 2) - 4)][0];
								color[i][j][1] = gradient[((i / 2) - 4)][1];
								color[i][j][2] = gradient[((i / 2) - 4)][2];
							}
						}
						else {
							for (int j = 9; j <= (min + (8 + ((columns - 1) * -5))); j++) {
								color[i][j][0] = gradient[((i / 2) - 4)][0];
								color[i][j][1] = gradient[((i / 2) - 4)][1];
								color[i][j][2] = gradient[((i / 2) - 4)][2];
							}
						}
					}
				}
				else {
					for (int i = 8; i <= 30; i += 2) {
						for (int j = 9; j <= 13; j++) {
							color[i][j][0] = 0;
							color[i][j][1] = 0;
							color[i][j][2] = 0;
						}
					}
				}

				if (hour > 12) {
					daytime = 1;
					for (int i = 0; i < 3; i++) {
						color[11][26][i] = 255;
						color[12][26][i] = 255;
						color[13][26][i] = 255;
						color[14][26][i] = 0;
						color[11][27][i] = 255;
						color[12][27][i] = 0;
						color[13][27][i] = 0;
						color[14][27][i] = 255;
						color[11][28][i] = 255;
						color[12][28][i] = 255;
						color[13][28][i] = 255;
						color[14][28][i] = 0;
						color[11][29][i] = 255;
						color[12][29][i] = 0;
						color[13][29][i] = 0;
						color[14][29][i] = 0;
						color[11][30][i] = 255;
						color[12][30][i] = 0;
						color[13][30][i] = 0;
						color[14][30][i] = 0;

						color[16][26][i] = 255;
						color[17][26][i] = 0;
						color[18][26][i] = 0;
						color[19][26][i] = 0;
						color[20][26][i] = 255;
						color[16][27][i] = 255;
						color[17][27][i] = 255;
						color[18][27][i] = 0;
						color[19][27][i] = 255;
						color[20][27][i] = 255;
						color[16][28][i] = 255;
						color[17][28][i] = 0;
						color[18][28][i] = 255;
						color[19][28][i] = 0;
						color[20][28][i] = 255;
						color[16][29][i] = 255;
						color[17][29][i] = 0;
						color[18][29][i] = 0;
						color[19][29][i] = 0;
						color[20][29][i] = 255;
						color[16][30][i] = 255;
						color[17][30][i] = 0;
						color[18][30][i] = 0;
						color[19][30][i] = 0;
						color[20][30][i] = 255;
					}
				}
				else {
					daytime = 0;
					for (int i = 0; i < 3; i++) {
						color[11][26][i] = 0;
						color[12][26][i] = 255;
						color[13][26][i] = 255;
						color[14][26][i] = 0;
						color[11][27][i] = 255;
						color[12][27][i] = 0;
						color[13][27][i] = 0;
						color[14][27][i] = 255;
						color[11][28][i] = 255;
						color[12][28][i] = 255;
						color[13][28][i] = 255;
						color[14][28][i] = 255;
						color[11][29][i] = 255;
						color[12][29][i] = 0;
						color[13][29][i] = 0;
						color[14][29][i] = 255;
						color[11][30][i] = 255;
						color[12][30][i] = 0;
						color[13][30][i] = 0;
						color[14][30][i] = 255;

						color[16][26][i] = 255;
						color[17][26][i] = 0;
						color[18][26][i] = 0;
						color[19][26][i] = 0;
						color[20][26][i] = 255;
						color[16][27][i] = 255;
						color[17][27][i] = 255;
						color[18][27][i] = 0;
						color[19][27][i] = 255;
						color[20][27][i] = 255;
						color[16][28][i] = 255;
						color[17][28][i] = 0;
						color[18][28][i] = 255;
						color[19][28][i] = 0;
						color[20][28][i] = 255;
						color[16][29][i] = 255;
						color[17][29][i] = 0;
						color[18][29][i] = 0;
						color[19][29][i] = 0;
						color[20][29][i] = 255;
						color[16][30][i] = 255;
						color[17][30][i] = 0;
						color[18][30][i] = 0;
						color[19][30][i] = 0;
						color[20][30][i] = 255;
					}
				}

				if (hour == 0 || hour == 13) {
					for (int i = 8; i <= 30; i += 2) {
						for (int j = 1; j <= 5; j++) {
							color[i][j][0] = 0;
							color[i][j][1] = 0;
							color[i][j][2] = 0;
						}
					}
				}

				if (hour > 0) {
					for (int i = 8; i <= ((((hour * 2) - 1) - (24 * daytime)) + 7); i += 2) {
						for (int j = 1; j <= 5; j++) {
							color[i][j][0] = gradient[((i / 2) - 4)][0];
							color[i][j][1] = gradient[((i / 2) - 4)][1];
							color[i][j][2] = gradient[((i / 2) - 4)][2];
						}
					}
				}

				for (int i = 0; i < 32; i++) {
					for (int j = 0; j < 32; j++) {
						matrix_->SetPixel(i, j, color[i][j][0], color[i][j][1], color[i][j][2]);
					}
				}
			}
		}
	}
};

// Clock design 3.
class TestGen3 : public RGBMatrixManipulator {
public:
	TestGen3(RGBMatrix *m) : RGBMatrixManipulator(m) {}
	void Run() {

		matrix_->FillScreen(0, 0, 0);

		int color[32][32][3], min, hour, sec, usec, daytime;

		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				for (int k = 0; k < 3; k++) {
					color[i][j][k] = 0;
				}
			}
		}

		time_t mytime;
		mytime = time(NULL);
		struct tm *tm_p;
		tm_p = localtime(&mytime);
		timeval time_val;
		gettimeofday(&time_val, 0);
		usec = time_val.tv_usec;
		sec = tm_p->tm_sec;
		min = tm_p->tm_min;
		hour = tm_p->tm_hour;

		if (hour > 12) {
			daytime = 1;
		}
		else {
			daytime = 0;
		}

		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 32; j++) {
				matrix_->SetPixel(i, j, color[i][j][0], color[i][j][1], color[i][j][2]);
			}
		}

		while (running_) {

			mytime = time(NULL);
			tm_p = localtime(&mytime);
			gettimeofday(&time_val, 0);

			if (time_val.tv_usec - usec != 0) {

				usec = time_val.tv_usec;
				sec = tm_p->tm_sec;
				min = tm_p->tm_min;
				hour = tm_p->tm_hour;

				if (hour > 12) {
					daytime = 1;
				}
				else {
					daytime = 0;
				}

				for (int i = 0; i < 32; i++) {
					for (int j = 0; j < 32; j++) {
						matrix_->SetPixel(i, j, color[i][j][0], color[i][j][1], color[i][j][2]);
					}
				}
			}
		}
	}
};

// Memset & memcpy class.
class MemTest : public RGBMatrixManipulator {
public:
	MemTest(RGBMatrix *m) : RGBMatrixManipulator(m) {}
	void Run() {
		while (running_) {
			matrix_->ClearScreen();
			matrix_->FillScreen(100, 0, 0);
			usleep(4000000);
			matrix_->ClearScreen();
			matrix_->FillScreen(0, 100, 0);
			usleep(4000000);
			matrix_->ClearScreen();
			matrix_->FillScreen(0, 0, 100);
			usleep(4000000);
			matrix_->ClearScreen();
			matrix_->FillScreen(100, 100, 100);
			usleep(4000000);
		}
	}
};

class ImageScroller : public RGBMatrixManipulator {
public:
  ImageScroller(RGBMatrix *m)
    : RGBMatrixManipulator(m), image_(NULL), horizontal_position_(0) {
  }

  // _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
  // Not really robust. Use at your own risk :)
  bool LoadPPM(const char *filename) {
    if (image_) {
      delete [] image_;
      image_ = NULL;
    }
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;
    char header_buf[256];
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
      fclose(f); return false; }
    if (sscanf(line, "P6 ") == EOF)
      EXIT_WITH_MSG("Can only handle P6 as PPM type.");
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d %d ", &width_, &height_) != 2)
      EXIT_WITH_MSG("Width/height expected");
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
      EXIT_WITH_MSG("Only 255 for maxval allowed.");
    const size_t pixel_count = width_ * height_;
    image_ = new Pixel [ pixel_count ];
    assert(sizeof(Pixel) == 3);   // we make that assumption.
    if (fread(image_, sizeof(Pixel), pixel_count, f) != pixel_count) {
      line = "";
      EXIT_WITH_MSG("Not enough pixels read.");
    }
#undef EXIT_WITH_MSG
    fclose(f);
    fprintf(stderr, "Read image with %dx%d\n", width_, height_);
    horizontal_position_ = 0;
    return true;
  }

  void Run() {
    const int columns = matrix_->columns();
	int pic_frame = 1;
    while (running_) {
		if (pic_frame == 1) {
			LoadPPM("poke1.ppm");
		} else if (pic_frame == 2) {
			LoadPPM("poke2.ppm");
		} else if (pic_frame == 3) {
			LoadPPM("poke3.ppm");
		} else if (pic_frame == 4) {
			LoadPPM("poke4.ppm");
		} else if (pic_frame == 5) {
			LoadPPM("poke5.ppm");
		}
		if (image_ == NULL) {
			usleep(100 * 1000);
			continue;
		}
		usleep(100 * 1000);
		for (int x = 0; x < columns; ++x) {
			for (int y = 0; y < 32; ++y) {
				const Pixel &p = getPixel((horizontal_position_ + x) % width_, y);
				matrix_->SetPixel(x, y, p.red, p.green, p.blue);
			}
		}
		if (pic_frame == 1) usleep(600 * 1000);
		if (pic_frame < 5) {
			pic_frame += 1;
		} else {
			pic_frame = 2;
		}
		//++horizontal_position_;
    }
  }

private:
  struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  // Read line, skip comments.
  char *ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
      result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
  }

  const Pixel &getPixel(int x, int y) {
    static Pixel dummy;
    if (x < 0 || x > width_ || y < 0 || y > height_) return dummy;
    return image_[x + width_ * y];
  }

  int width_;
  int height_;
  Pixel *image_;
  uint32_t horizontal_position_;
};

class ImageScroller2 : public RGBMatrixManipulator {
public:
	ImageScroller2(RGBMatrix *m)
		: RGBMatrixManipulator(m), image_(NULL), horizontal_position_(0) {
	}

	// _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
	// Not really robust. Use at your own risk :)
	bool LoadPPM(const char *filename) {
		if (image_) {
			delete[] image_;
			image_ = NULL;
		}
		FILE *f = fopen(filename, "r");
		if (f == NULL) return false;
		char header_buf[256];
		const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
	fclose(f); return false; }
		if (sscanf(line, "P6 ") == EOF)
			EXIT_WITH_MSG("Can only handle P6 as PPM type.");
		line = ReadLine(f, header_buf, sizeof(header_buf));
		if (!line || sscanf(line, "%d %d ", &width_, &height_) != 2)
			EXIT_WITH_MSG("Width/height expected");
		int value;
		line = ReadLine(f, header_buf, sizeof(header_buf));
		if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
			EXIT_WITH_MSG("Only 255 for maxval allowed.");
		const size_t pixel_count = width_ * height_;
		image_ = new Pixel[pixel_count];
		assert(sizeof(Pixel) == 3);   // we make that assumption.
		if (fread(image_, sizeof(Pixel), pixel_count, f) != pixel_count) {
			line = "";
			EXIT_WITH_MSG("Not enough pixels read.");
		}
#undef EXIT_WITH_MSG
		fclose(f);
		fprintf(stderr, "Read image with %dx%d\n", width_, height_);
		horizontal_position_ = 0;
		return true;
	}

	void Run() {
		const int columns = matrix_->columns();
		while (running_) {
			if (image_ == NULL) {
				usleep(100 * 1000);
				continue;
			}
			usleep(100 * 1000);
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					const Pixel &p = getPixel((horizontal_position_ + x) % width_, y);
					matrix_->SetPixel(x, y, p.red, p.green, p.blue);
				}
			}
			//++horizontal_position_;
		}
	}

private:
	struct Pixel {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	// Read line, skip comments.
	char *ReadLine(FILE *f, char *buffer, size_t len) {
		char *result;
		do {
			result = fgets(buffer, len, f);
		} while (result != NULL && result[0] == '#');
		return result;
	}

	const Pixel &getPixel(int x, int y) {
		static Pixel dummy;
		if (x < 0 || x > width_ || y < 0 || y > height_) return dummy;
		return image_[x + width_ * y];
	}

	int width_;
	int height_;
	Pixel *image_;
	uint32_t horizontal_position_;
};

class ImageScroller3 : public RGBMatrixManipulator {
public:
	ImageScroller3(RGBMatrix *m)
		: RGBMatrixManipulator(m), image_(NULL), horizontal_position_(0) {
	}

	// _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
	// Not really robust. Use at your own risk :)
	bool LoadPPM(const char *filename) {
		if (image_) {
			delete[] image_;
			image_ = NULL;
		}
		FILE *f = fopen(filename, "r");
		if (f == NULL) return false;
		char header_buf[256];
		const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
	fclose(f); return false; }
		if (sscanf(line, "P6 ") == EOF)
			EXIT_WITH_MSG("Can only handle P6 as PPM type.");
		line = ReadLine(f, header_buf, sizeof(header_buf));
		if (!line || sscanf(line, "%d %d ", &width_, &height_) != 2)
			EXIT_WITH_MSG("Width/height expected");
		int value;
		line = ReadLine(f, header_buf, sizeof(header_buf));
		if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
			EXIT_WITH_MSG("Only 255 for maxval allowed.");
		const size_t pixel_count = width_ * height_;
		image_ = new Pixel[pixel_count];
		assert(sizeof(Pixel) == 3);   // we make that assumption.
		if (fread(image_, sizeof(Pixel), pixel_count, f) != pixel_count) {
			line = "";
			EXIT_WITH_MSG("Not enough pixels read.");
		}
#undef EXIT_WITH_MSG
		fclose(f);
		fprintf(stderr, "Read image with %dx%d\n", width_, height_);
		horizontal_position_ = 0;
		return true;
	}

	void Run() {
		const int columns = matrix_->columns();
		int pic_frame = 1;
		int frames = file_count;
		int current_folder = 1;
		while (running_) {
			LoadPPM(path);
			if (image_ == NULL) {
				break;
			}
			usleep(frame_delay);
			for (int x = 0; x < columns; ++x) {
				for (int y = 0; y < 32; ++y) {
					const Pixel &p = getPixel((horizontal_position_ + x) % width_, y);
					matrix_->SetPixel(x, y, p.red, p.green, p.blue);
				}
			}
			pic_frame++;
			if (pic_frame > frames) {
				pic_frame = 1;
				if (folder_count > 0) {
					strcpy(path, folders[current_folder]);
					file_count = 0;
					DIR * dirp;
					struct dirent * entry;

					dirp = opendir(path); /* There should be error handling after this */
					while ((entry = readdir(dirp)) != NULL) {
						if (entry->d_type == DT_REG) { /* If the entry is a regular file */
							file_count++;
						}
					}
					closedir(dirp);
					frames = file_count;
					strcat(path, "/");
					strcat(path, folders[current_folder]);
					strcat(path, "_frame_0001.ppm");
					current_folder++;
					if (current_folder == folder_count) {
						current_folder = 0;
					}
				}
			}
			path[strlen(path) - 8] = ((pic_frame / 1000) % 10) + 48;
			path[strlen(path) - 7] = ((pic_frame / 100) % 10) + 48;
			path[strlen(path) - 6] = ((pic_frame / 10) % 10) + 48;
			path[strlen(path) - 5] = (pic_frame % 10) + 48;
		}
	}

private:
	struct Pixel {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	// Read line, skip comments.
	char *ReadLine(FILE *f, char *buffer, size_t len) {
		char *result;
		do {
			result = fgets(buffer, len, f);
		} while (result != NULL && result[0] == '#');
		return result;
	}

	const Pixel &getPixel(int x, int y) {
		static Pixel dummy;
		if (x < 0 || x > width_ || y < 0 || y > height_) return dummy;
		return image_[x + width_ * y];
	}

	int width_;
	int height_;
	Pixel *image_;
	uint32_t horizontal_position_;
};

int main(int argc, char *argv[]) {
  int demo = 0;
  if (argc > 1) {
    demo = atoi(argv[1]);
  }
  fprintf(stderr, "Using demo %d\n", demo);

  GPIO io;
  if (!io.Init())
    return 1;

  RGBMatrix m(&io);
    
  RGBMatrixManipulator *image_gen = NULL;
  switch (demo) {
  case 0:
    image_gen = new RotatingBlockGenerator(&m);
    break;

  case 1:
    if (argc > 2) {
      ImageScroller *scroller = new ImageScroller(&m);
      if (!scroller->LoadPPM(argv[2]))
        return 1;
      image_gen = scroller;
    } else {
      fprintf(stderr, "Demo %d Requires PPM image as parameter", demo);
      return 1;
    }
    break;

  case 2:
	  image_gen = new ColorPulseGenerator(&m);
	  break;

  case 3:
	  image_gen = new TestGen2(&m);
	  break;

  case 4:
	  image_gen = new MemTest(&m);
	  break;

  case 5:
	  if (argc > 2) {
		  ImageScroller2 *scroller = new ImageScroller2(&m);
		  if (!scroller->LoadPPM(argv[2]))
			  return 1;
		  image_gen = scroller;
	  }
	  else {
		  fprintf(stderr, "Demo %d Requires PPM image as parameter", demo);
		  return 1;
	  }
	  break;

  case 6:
	  folder_count = 0;
	  if (argc > 2) {
		  ImageScroller3 *scroller = new ImageScroller3(&m);
		  strcpy(path, argv[2]);
		  if (argc > 3) {
			  frame_delay = atoi(argv[3]);
		  }
		  else {
			  frame_delay = 100000;
		  }
		  file_count = 0;
		  DIR * dirp;
		  struct dirent * entry;

		  dirp = opendir(path); /* There should be error handling after this */
		  while ((entry = readdir(dirp)) != NULL) {
			  if (entry->d_type == DT_REG) { /* If the entry is a regular file */
				  file_count++;
			  }
		  }
		  closedir(dirp);
		  strcat(path, "/");
		  strcat(path, argv[2]);
		  strcat(path, "_frame_0001.ppm");
		  if (!scroller->LoadPPM(path))
			  return 1;
		  image_gen = scroller;
	  }
	  else {
		  ImageScroller3 *scroller = new ImageScroller3(&m);
		  frame_delay = 100000;
		  DIR * dirp;
		  struct dirent * entry;
		  char cwd[1024];
		  if (getcwd(cwd, sizeof(cwd)) != NULL) {
			  dirp = opendir(cwd);
			  while ((entry = readdir(dirp)) != NULL) {
				  if (entry->d_type == DT_DIR && strstr(entry->d_name, ".") == NULL) { /* If the entry is a regular folder & does not contain a "." */
					  strcpy(folders[folder_count], entry->d_name);
					  folder_count++;
				  }
			  }
			  closedir(dirp);
			  if (folder_count > 0) {
				  strcpy(path, folders[0]);
				  file_count = 0;
				  DIR * dirp;
				  struct dirent * entry;

				  dirp = opendir(path); /* There should be error handling after this */
				  while ((entry = readdir(dirp)) != NULL) {
					  if (entry->d_type == DT_REG) { /* If the entry is a regular file */
						  file_count++;
					  }
				  }
				  closedir(dirp);
				  strcat(path, "/");
				  strcat(path, folders[0]);
				  strcat(path, "_frame_0001.ppm");
				  if (!scroller->LoadPPM(path)) {
					  fprintf(stderr, "Could not load picture!");
					  return 1;
				  }
				  else {
					  image_gen = scroller;
				  }
			  }
			  else {
				  fprintf(stderr, "No folders available!");
				  return 1;
			  }
		  }
		  else {
			  fprintf(stderr, "Error while getting the current working directory!");
			  return 1;
		  }
	  }
	  break;

  default:
    image_gen = new ColorGenerator(&m);
    break;
  }

  if (image_gen == NULL)
    return 1;

  RGBMatrixManipulator *updater = new DisplayUpdater(&m);
  updater->Start(50);  // high priority

  image_gen->Start();

  // Things are set up. Just wait for <RETURN> to be pressed.
  printf("Press <RETURN> to exit and reset LEDs\n");
  getchar();

  // Stopping threads and wait for them to join.
  delete image_gen;
  delete updater;

  // Final thing before exit: clear screen and update once, so that
  // we don't have random pixels burn
  m.ClearScreen();
  m.UpdateScreen();

  return 0;
}
