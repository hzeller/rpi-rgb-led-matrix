Original readme can be found [here](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md).

# Installation Instructions:
1. [Configure Raspberry Pi for SSH access](https://www.tomshardware.com/reviews/raspberry-pi-headless-setup-how-to,6028.html)
1. `ping raspberrypi.local` to get the local IP address of the Raspberry Pi
1. `ssh pi@ip.address.from.above.step`
    - If you get an error similar to `WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!` just follow the instructions listed
    - When prompted, enter your Raspberry Pi password, default is `raspberry`
1. Install git: `sudo apt install git`
1. `cd rpi-rgb-led-matrix`
1. In the root directory above build for Python 3:
    1. `sudo apt-get update && sudo apt-get install python3-dev python3-pillow -y`
    1. `make build-python PYTHON=$(command -v python3)`
    1. `sudo make install-python PYTHON=$(command -v python3)`
1. Ensure the Raspberry Pi's sound is disbaled:
    1. From the Pi's root directory, `sudo nano /boot/config.txt`
    1. Ensure `dtparam=audio=off`
1. From the `rpi-rgb-led-matrix` navigate to `cd bindings/python/samples`
1. Run `sudo python harry.py` to start the LED sign