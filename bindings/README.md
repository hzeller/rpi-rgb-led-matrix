Various bindings to other programming languages.
Typically these are wrapping the [C-binding](../include/led-matrix-c.h) that
comes with rpi-rgb-led-matrix



git clone https://github.com/superdoret/rpi-rgb-led-matrix.git
cd rpi-rgb-led-matrix/
make build-python PYTHON=$(which python 3)
sudo make install-python
ls
cd rpi-rgb-led-matrix/bindings/python/samples
sudo python3 runtext.py --led-cols=64 --led-gpio-mapping=adafruit-hat --led-slowdown-gpio=4 --text="Hola Emma de mi vida"
