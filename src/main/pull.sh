#!/bin/sh
git pull
sudo python3 index.py --padding 10 --centered True --font "7x14B.bdf" --fade "top" --verbose False --led-brightness 50
