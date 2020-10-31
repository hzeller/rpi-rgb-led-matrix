#!/usr/bin/env python3

# Copyright (C) John Ioannidis, ji@tla.org
# Beer license:
#   If you like this code, it would be nice if you don't take this notice out.
#   If you make any money out of it (I doubt it), it would be nice if you send
#   me some, but you don't have to!
#   Either way, you should buy me a beer if we ever meet!

# requirements:
#   gpiozero
#   pycparser

from collections import namedtuple
import sys
import time

import gpiozero
# import pycparser

# ## Pins Up (male header)
#
# | PIN  |       |      | PIN  |
# | ---: | :---  | :--- | ---: |
# |    1 | R1    | G1   |    2 |
# |    3 | B1    | GND  |    4 |
# |    5 | R2    | G2   |    6 |
# |    7 | B2    | E    |    8 |
# |    9 | A     | B    |   10 |
# |   11 | C     | D    |   12 |
# |   13 | CLK   | STR  |   14 |
# |   15 | #OE   | GND  |   16 |
#
# ## Holes Up (female header)
#
# | PIN  |      |       | PIN  |
# | ---: | :--- | :---  | ---: |
# |    2 | G1   | R1    |    1 |
# |    4 | GND  | B1    |    3 |
# |    6 | G2   | R2    |    5 |
# |    8 | E    | B2    |    7 |
# |   10 | B    | A     |    9 |
# |   12 | D    | C     |   11 |
# |   14 | STR  | CLK   |   13 |
# |   16 | GND  | #OE   |   15 |


TEENSY_LIKE = True

# class JiHat(object):
#   def __init__(self, socket=0):
#     self.output_enable = gpiozero.LED(18)
#     self.clock         = gpiozero.LED(27)
#     self.strobe        = gpiozero.LED(21)
#     self.a             = gpiozero.LED(16)
#     self.b             = gpiozero.LED(19)
#     self.c             = gpiozero.LED(20)
#     self.d             = gpiozero.LED(26)
#     self.e             = gpiozero.LED(13)
#     if socket == 0:
#       self.r1          = gpiozero.LED(4)
#       self.g1          = gpiozero.LED(15)
#       self.b1          = gpiozero.LED(14)
#       self.r2          = gpiozero.LED(3)
#       self.g2          = gpiozero.LED(17)
#       self.b2          = gpiozero.LED(2)
#     elif socket == 1:
#       self.r1          = gpiozero.LED(10)
#       self.g1          = gpiozero.LED(25)
#       self.b1          = gpiozero.LED(24)
#       self.r2          = gpiozero.LED(22)
#       self.g2          = gpiozero.LED(9)
#       self.b2          = gpiozero.LED(23)
#     elif socket == 2:
#       self.r1          = gpiozero.LED(5) 
#       self.g1          = gpiozero.LED(12)
#       self.b1          = gpiozero.LED(7) 
#       self.r2          = gpiozero.LED(8) 
#       self.g2          = gpiozero.LED(6) 
#       self.b2          = gpiozero.LED(11)
#     else:
#       raise ValueError('0 <= socket <= 2')
#   def off(self):
#     self.r1.off()
#     self.g1.off()
#     self.b1.off()
#     self.r2.off()
#     self.g2.off()
#     self.b2.off()
#   def on(self):
#     self.r1.on()
#     self.g1.on()
#     self.b1.on()
#     self.r2.on()
#     self.g2.on()
#     self.b2.on()

M = namedtuple('M', ('oe', 'clk', 'lat', 'a', 'b', 'c', 'd', 'e'))
m = M(gpiozero.LED(18),
      gpiozero.LED(27),
      gpiozero.LED(21),
      gpiozero.LED(16),
      gpiozero.LED(19),
      gpiozero.LED(20),
      gpiozero.LED(26),
      gpiozero.LED(13))

L = namedtuple('L', ('r1', 'g1', 'b1', 'r2', 'b2', 'g2'))
l0 = L(gpiozero.LED(4),
       gpiozero.LED(15),
       gpiozero.LED(14),
       gpiozero.LED(3),
       gpiozero.LED(17),
       gpiozero.LED(2))
l1 = L(gpiozero.LED(10),
       gpiozero.LED(25),
       gpiozero.LED(24),
       gpiozero.LED(22),
       gpiozero.LED(9),
       gpiozero.LED(23))
l2 = L(gpiozero.LED(5),
       gpiozero.LED(12),
       gpiozero.LED(7),
       gpiozero.LED(8),
       gpiozero.LED(6),
       gpiozero.LED(11))

def off(l):
  l.r1.off()
  l.g1.off()
  l.b1.off()
  l.r2.off()
  l.g2.off()
  l.b2.off()
def on(l):
  l.r1.on()
  l.g1.on()
  l.b1.on()
  l.r2.on()
  l.g2.on()
  l.b2.on()


def main(max_led):
  while True:
    l2.r1.on()
    l2.r1.off()

  print('Initializing...', end='')
  m = JiHat()
  m1 = JiHat(2)
  print(' sleeping...', end='')
  time.sleep(1)
  print()
  if TEENSY_LIKE:
    m.strobe.off()
    m.clock.off()
    m.output_enable.on()
    for l in range(max_led):
      y = l % 16
      if y != 0:
        m.on()
      else:
        m.off()
      if l > max_led - 12:
        m.strobe.on()
      else:
        m.strobe.off()
      m.clock.on()
      time.sleep(.000001)
      m.clock.off()
      time.sleep(.000001)
    m.strobe.off()
    m.clock.off()
    for l in range(max_led):
      y = l % 16
      if y == 9:
        m.on()
      else:
        m.off()
      if l > max_led - 12:
        m.strobe.on()
      else:
        m.strobe.off()
      m.clock.on()
      time.sleep(.000001)
      m.clock.off()
      time.sleep(.000001)
    m.strobe.off()
    m.clock.off()
    m.output_enable.off()
  # else:
  #   m.clock.off()
  #   m.output_enable.off()
  #   m.a.on()
  #   m.b.off()
  #   m.c.off()
  #   m.d.off()
  #   m.e.off()
  #   m.off()
  #   for l in range(max_led):
  #     y = l % 16
  #     if y == 0:
  #       m.off()
  #     else:
  #       m.on()
  #     m.clock.on()
  #     time.sleep(.0001)
  #     m.clock.off()
  #     time.sleep(.0001)
  #     if l == max_led - 12:
  #       m.strobe.on()
  #   m.strobe.off()
  #   for l in range(max_led):
  #     y = l % 16
  #     if y == 9:
  #       m.on()
  #     else:
  #       m.off()
  #     m.clock.on()
  #     time.sleep(.0001)
  #     m.clock.off()
  #     time.sleep(.0001)
  #     if l == max_led - 12:
  #       m.strobe.on()
  #   m.strobe.off()
  #   m.output_enable.on()

if __name__ == '__main__':
  main(int(sys.argv[1]))
  print('done')


      #
#
#
# from __future__ import print_function
# from gpiozero import LED
# from time import sleep
# #     0123456789012345
#
# b12a="0111111111111111"
# b12b="0111111111111111"
# b12c="0111111111111111"
# b12d="0111111111111111"
#
# b13a="0000000001000000"
# b13b="0000000001000000"
# b13c="0000000001000000"
# b13d="0000000001000000"
#
# # b12  - 1  adds red tinge
# # b12  - 9/8/7/6/5  =  4 bit brightness
# # b13  - 9   =1 screen on
# # b13  - 6   =1 screen off
# xr1=LED(11)
# xr2=LED(8)
# xg1=LED(27)
# xg2=LED(9)
# xb1=LED(7)
# xb2=LED(10)
#
# xA=LED(22)
# xB=LED(23)
# xC=LED(24)
# xD=LED(25)
#
# xLAT=LED(4)
# xCLK=LED(17)
# xOE=LED(18)
#
# xCLK.off()
#
# xOE.off()
#
# xA.on()
# xB.off()
# xC.off()
# xD.off()
#
# xr1.off()
# xr2.off()
# xg1.off()
# xg2.off()
# xb1.off()
# xb2.off()
#
# b12=b12a
# b13=b13a
# max = 128 *2
# for x in range(max):
# 	y=x%16
# 	if(y==0):
# 		print(' ',end='')
# 	if (b12[y:y+1] is "0"):
# 		print('0',end='')
# 		xr1.off()
# 		xr2.off()
# 		xg1.off()
# 		xg2.off()
# 		xb1.off()
# 		xb2.off()
# 	else:
# 		print('1',end='')
# 		xr1.on()
# 		xr2.on()
# 		xg1.on()
# 		xg2.on()
# 		xb1.on()
# 		xb2.on()
# 	xCLK.on()
# 	sleep(0.001)
# 	xCLK.off()
# 	sleep(0.001)
# 	if(x>31):
# 		b12=b12b
# 	if(x>63):
# 		b12=b12c
# 	if(x>95):
# 		b12=b12d
# 	if(x==(max-12)):
# 		print('*',end='')
# 		xLAT.on()
#
# xLAT.off();
# print('')
#
# for x in range(max):
#         y=x%16
#         if(y==0):
#                 print(' ',end='')
#         if (b13[y:y+1] is "0"):
#                 print('0',end='')
#                 xr1.off()
#                 xr2.off()
#                 xg1.off()
#                 xg2.off()
#                 xb1.off()
#                 xb2.off()
#         else:
#                 print('1',end='')
#                 xr1.on()
#                 xr2.on()
#                 xg1.on()
#                 xg2.on()
#                 xb1.on()
#                 xb2.on()
#         xCLK.on()
#         sleep(0.001)
#         xCLK.off()
#         sleep(0.001)
#         if(x>31):
#                 b13=b13b
#         if(x>63):
#                 b13=b13c
#         if(x>95):
#                 b13=b13d
#         if(x==(max-13)):
#                 print('*',end='')
#                 xLAT.on()
#
# xLAT.off();
# print('')
#
# xOE.on();
#
# ZZ
#