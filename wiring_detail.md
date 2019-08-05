# Description
This library supports a number of GPIO mappings.  The header column of this table can be passed to `--led-gpio-mapping=<name>` to have the library work with that GPIO mappings.

The `pi1` mappings were quite limited on the number of GPIO outputs, so used the pins which hardware I²C needs.  If you use a modern RPi with more GPIO pins that mapping will work, but you cannot use I²C modules along with it.

## HardwareMapping Property Name
Bold numbers are **physical pins**, everything else is BCM GPIO pins

|  | regular | adafruit-hat | adafruit-hat-pwm | regular-pi1 | classic | classic-pi1 | **HUB75 physical pin** |
|---|---|---|---|---|---|---|---|
| .output_enable | 18 | 4 | 18 | 18 | 27 | 0 \| 2 | **15** |
| .clock | 17 | 17 | 17 | 17 | 11 | 1 \| 3 | **13** |
| .strobe | 4 | 21 | 21 | 4 | 4 | 4 | **14** |
| /* Address lines */ ||||
| .a | 22 | 22 | 22 | 22 | 7 | 7 | **9** |
| .b | 23 | 26 | 26 | 23 | 8 | 8 | **10** |
| .c | 24 | 27 | 27 | 24 | 9 | 9 | **11** |
| .d | 25 | 20 | 20 | 25 | 10 | 10 | **12** |
| .e | 15 | 24 | 24 | 15 |  |  |  |
| /* Parallel chain 0, RGB for both sub-panels */ ||||
| .p0_r1 | 11 | 5 | 5 | 11 | 17 | 17 | **1** |
| .p0_g1 | 27 | 13 | 13 | 21 \| 27 | 18 | 18 | **2** |
| .p0_b1 | 7 | 6 | 6 | 7 | 22 | 22 | **3** |
| .p0_r2 | 8 | 12 | 12 | 8 | 23 | 23 | **5** |
| .p0_g2 | 9 | 16 | 16 | 9 | 24 | 24 | **6** |
| .p0_b2 | 10 | 23 | 23 | 10 | 25 | 25 | **7** |
| /* Chain 1 */ ||||
| .p1_r1 | 12 |  |  |  | 12 |  |  |
| .p1_g1 | 5 |  |  |  | 5 |  |  |
| .p1_b1 | 6 |  |  |  | 6 |  |  |
| .p1_r2 | 19 |  |  |  | 19 |  |  |
| .p1_g2 | 13 |  |  |  | 13 |  |  |
| .p1_b2 | 20 |  |  |  | 20 |  |  |
| /* Chain 2 */ ||||
| .p2_r1 | 14 |  |  |  | 14 |  |  |
| .p2_g1 | 2 |  |  |  | 2 |  |  |
| .p2_b1 | 3 |  |  |  | 3 |  |  |
| .p2_r2 | 26 |  |  |  | 15 |  |  |
| .p2_g2 | 16 |  |  |  | 26 |  |  |
| .p2_b2 | 21 |  |  |  | 21 |  |  |

## BCM pin to physical pin

For a mapping of physical pin to BCM we suggest looking at the output of `gpio readall`

`GPIO_BIT(18)` == `18` in the above table

| BCM GPIO | Physical Pin | Wiring Pi | Name |
|---|---|---|---|
| GPIO_BIT(1) | **28** | 31 | SCL.0 |
| GPIO_BIT(2) | **3** | 8 | SDA.1 |
| GPIO_BIT(3) | **5** | 9 | SCL.1 |
| GPIO_BIT(4) | **7** | 7 | GPIO. 7 |
| GPIO_BIT(5) | **29** | 21 | GPIO.21 |
| GPIO_BIT(6) | **31** | 22 | GPIO.22 |
| GPIO_BIT(7) | **26** | 11 | CE1 |
| GPIO_BIT(8) | **24** | 10 | CE0 |
| GPIO_BIT(9) | **21** | 13 | MISO |
| GPIO_BIT(10) | **19** | 12 | MOSI |
| GPIO_BIT(11) | **23** | 14 | SCLK |
| GPIO_BIT(12) | **32** | 26 | GPIO.26 |
| GPIO_BIT(13) | **33** | 23 | GPIO.23 |
| GPIO_BIT(14) | **8** | 15 | TxD |
| GPIO_BIT(15) | **10** | 16 | RxD |
| GPIO_BIT(16) | **36** | 27 | GPIO.27 |
| GPIO_BIT(17) | **11** | 0 | GPIO. 0 |
| GPIO_BIT(18) | **12** | 1 | GPIO. 1 |
| GPIO_BIT(19) | **35** | 24 | GPIO.24 |
| GPIO_BIT(20) | **38** | 28 | GPIO.28 |
| GPIO_BIT(21) | **40** | 29 | GPIO.29 |
| GPIO_BIT(22) | **15** | 3 | GPIO. 3 |
| GPIO_BIT(23) | **16** | 4 | GPIO. 4 |
| GPIO_BIT(24) | **18** | 5 | GPIO. 5 |
| GPIO_BIT(25) | **22** | 6 | GPIO. 6 |
| GPIO_BIT(26) | **37** | 25 | GPIO.25 |
| GPIO_BIT(27) | **13** | 2 | GPIO. 2 |

## Tutorial Mappins

#### adafruit-hat | adafruit-hat-pwm:
https://learn.adafruit.com/adafruit-rgb-matrix-plus-real-time-clock-hat-for-raspberry-pi/pinouts

#### clasic-pi1:
https://learn.adafruit.com/connecting-a-16x32-rgb-led-matrix-panel-to-a-raspberry-pi/wiring-the-display
