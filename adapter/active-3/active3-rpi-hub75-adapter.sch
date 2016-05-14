EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:active3-rpi-hub75-adapter-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L 74HC245 U1
U 1 1 54ECAC85
P 5650 1750
F 0 "U1" H 5750 2325 60  0000 L BNN
F 1 "74HCT245" H 5700 1175 60  0000 L TNN
F 2 "Housings_SOIC:SOIC-20_7.5x12.8mm_Pitch1.27mm" H 5650 1750 60  0001 C CNN
F 3 "" H 5650 1750 60  0000 C CNN
	1    5650 1750
	1    0    0    -1  
$EndComp
$Comp
L 74HC245 U2
U 1 1 54ECB18C
P 5650 3100
F 0 "U2" H 5750 3675 60  0000 L BNN
F 1 "74HCT245" H 5700 2525 60  0000 L TNN
F 2 "Housings_SOIC:SOIC-20_7.5x12.8mm_Pitch1.27mm" H 5650 3100 60  0001 C CNN
F 3 "" H 5650 3100 60  0000 C CNN
	1    5650 3100
	1    0    0    -1  
$EndComp
$Comp
L 74HC245 U3
U 1 1 54ECB1EA
P 5650 4450
F 0 "U3" H 5750 5025 60  0000 L BNN
F 1 "74HCT245" H 5700 3875 60  0000 L TNN
F 2 "Housings_SOIC:SOIC-20_7.5x12.8mm_Pitch1.27mm" H 5650 4450 60  0001 C CNN
F 3 "" H 5650 4450 60  0000 C CNN
	1    5650 4450
	1    0    0    -1  
$EndComp
$Comp
L CONN_02X08 Panel-1
U 1 1 54ECB236
P 8500 2550
F 0 "Panel-1" H 8500 3000 50  0000 C CNN
F 1 "CONN_02X08" V 8500 2550 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 8500 1350 60  0001 C CNN
F 3 "" H 8500 1350 60  0000 C CNN
	1    8500 2550
	1    0    0    -1  
$EndComp
$Comp
L CONN_02X20 P1
U 1 1 54ECB2B7
P 3250 3650
F 0 "P1" H 3250 4700 50  0000 C CNN
F 1 "CONN_02X20" V 3250 3650 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x20" H 3250 2700 60  0001 C CNN
F 3 "" H 3250 2700 60  0000 C CNN
	1    3250 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 2650 3550 2800
Wire Wire Line
	3500 2700 3750 2700
Wire Wire Line
	3550 2800 3500 2800
Connection ~ 3550 2700
$Comp
L GND #PWR9
U 1 1 54ECB3E1
P 3600 3300
F 0 "#PWR9" H 3600 3300 30  0001 C CNN
F 1 "GND" H 3600 3230 30  0001 C CNN
F 2 "" H 3600 3300 60  0000 C CNN
F 3 "" H 3600 3300 60  0000 C CNN
	1    3600 3300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 3300 3600 3300
$Comp
L GND #PWR10
U 1 1 54ECB417
P 3600 3600
F 0 "#PWR10" H 3600 3600 30  0001 C CNN
F 1 "GND" H 3600 3530 30  0001 C CNN
F 2 "" H 3600 3600 60  0000 C CNN
F 3 "" H 3600 3600 60  0000 C CNN
	1    3600 3600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 3600 3600 3600
$Comp
L GND #PWR8
U 1 1 54ECB4A1
P 3600 2900
F 0 "#PWR8" H 3600 2900 30  0001 C CNN
F 1 "GND" H 3600 2830 30  0001 C CNN
F 2 "" H 3600 2900 60  0000 C CNN
F 3 "" H 3600 2900 60  0000 C CNN
	1    3600 2900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 2900 3600 2900
$Comp
L GND #PWR2
U 1 1 54ECB5FE
P 2900 4600
F 0 "#PWR2" H 2900 4600 30  0001 C CNN
F 1 "GND" H 2900 4530 30  0001 C CNN
F 2 "" H 2900 4600 60  0000 C CNN
F 3 "" H 2900 4600 60  0000 C CNN
	1    2900 4600
	0    1    1    0   
$EndComp
Wire Wire Line
	2900 4600 3000 4600
$Comp
L GND #PWR11
U 1 1 54ECB73E
P 3600 4300
F 0 "#PWR11" H 3600 4300 30  0001 C CNN
F 1 "GND" H 3600 4230 30  0001 C CNN
F 2 "" H 3600 4300 60  0000 C CNN
F 3 "" H 3600 4300 60  0000 C CNN
	1    3600 4300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 4300 3600 4300
$Comp
L GND #PWR1
U 1 1 54ECB7BC
P 2900 3900
F 0 "#PWR1" H 2900 3900 30  0001 C CNN
F 1 "GND" H 2900 3830 30  0001 C CNN
F 2 "" H 2900 3900 60  0000 C CNN
F 3 "" H 2900 3900 60  0000 C CNN
	1    2900 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	2900 3900 3000 3900
Text GLabel 3000 3000 0    51   Output ~ 0
strobe
Text GLabel 3500 4200 2    51   Output ~ 0
p1_r1
Text GLabel 3000 4100 0    51   Output ~ 0
p1_g1
Text GLabel 3000 4200 0    51   Output ~ 0
p1_b1
Text GLabel 3000 4400 0    51   Output ~ 0
p1_r2
Text GLabel 3000 4300 0    51   Output ~ 0
p1_g2
Text GLabel 3500 4500 2    51   Output ~ 0
p1_b2
Text GLabel 3000 3400 0    51   Output ~ 0
row_A
Text GLabel 3500 3400 2    51   Output ~ 0
row_B
Text GLabel 3500 3500 2    51   Output ~ 0
row_C
Text GLabel 3500 3700 2    51   Output ~ 0
row_D
Text GLabel 3000 3200 0    51   Output ~ 0
clock
Text GLabel 3000 3800 0    51   Output ~ 0
p0_r1
Text GLabel 3000 3300 0    51   Output ~ 0
p0_g1
Text GLabel 3500 3900 2    51   Output ~ 0
p0_b1
Text GLabel 3500 3800 2    51   Output ~ 0
p0_r2
Text GLabel 3000 3700 0    51   Output ~ 0
p0_g2
Text GLabel 3000 3600 0    51   Output ~ 0
p0_b2
Text GLabel 3500 3200 2    51   Output ~ 0
OE
Text GLabel 4950 1950 0    51   Input ~ 0
p0_g1
Text GLabel 4950 1850 0    51   Input ~ 0
p0_r1
Text GLabel 4950 1750 0    51   Input ~ 0
p0_b1
Text GLabel 4950 1550 0    51   Input ~ 0
p0_r2
Text GLabel 4950 1450 0    51   Input ~ 0
p0_b2
Text GLabel 4950 1650 0    51   Input ~ 0
p0_g2
$Comp
L GND #PWR21
U 1 1 54ECD031
P 9300 2900
F 0 "#PWR21" H 9300 2900 30  0001 C CNN
F 1 "GND" H 9300 2830 30  0001 C CNN
F 2 "" H 9300 2900 60  0000 C CNN
F 3 "" H 9300 2900 60  0000 C CNN
	1    9300 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 2900 8750 2900
Text GLabel 4950 3100 0    51   Input ~ 0
row_A
Text GLabel 4950 3200 0    51   Input ~ 0
row_B
Text GLabel 4950 3000 0    51   Input ~ 0
row_C
Text GLabel 4950 2900 0    51   Input ~ 0
row_D
Text GLabel 4950 3300 0    51   Input ~ 0
strobe
Text GLabel 4950 1250 0    51   Input ~ 0
OE
$Comp
L CONN_02X08 Panel-2
U 1 1 54ECE201
P 8500 3800
F 0 "Panel-2" H 8500 4250 50  0000 C CNN
F 1 "CONN_02X08" V 8500 3800 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 8500 2600 60  0001 C CNN
F 3 "" H 8500 2600 60  0000 C CNN
	1    8500 3800
	1    0    0    -1  
$EndComp
Text GLabel 4950 4650 0    51   Input ~ 0
p1_g1
Text GLabel 4950 4550 0    51   Input ~ 0
p1_r1
Text GLabel 4950 4450 0    51   Input ~ 0
p1_b1
Text GLabel 4950 4150 0    51   Input ~ 0
p1_b2
Text GLabel 4950 4350 0    51   Input ~ 0
p1_g2
$Comp
L GND #PWR22
U 1 1 54ECE20E
P 9300 4150
F 0 "#PWR22" H 9300 4150 30  0001 C CNN
F 1 "GND" H 9300 4080 30  0001 C CNN
F 2 "" H 9300 4150 60  0000 C CNN
F 3 "" H 9300 4150 60  0000 C CNN
	1    9300 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 4150 8750 4150
$Comp
L C C1
U 1 1 54ECBE4F
P 2700 5600
F 0 "C1" H 2700 5700 40  0000 L CNN
F 1 "100n" H 2706 5515 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 2738 5450 30  0001 C CNN
F 3 "" H 2700 5600 60  0000 C CNN
	1    2700 5600
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 54ECBEE4
P 3000 5600
F 0 "C2" H 3000 5700 40  0000 L CNN
F 1 "100n" H 3006 5515 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 3038 5450 30  0001 C CNN
F 3 "" H 3000 5600 60  0000 C CNN
	1    3000 5600
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 54ECBF0A
P 3300 5600
F 0 "C3" H 3300 5700 40  0000 L CNN
F 1 "100n" H 3306 5515 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 3338 5450 30  0001 C CNN
F 3 "" H 3300 5600 60  0000 C CNN
	1    3300 5600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR4
U 1 1 54ECBF56
P 3000 5850
F 0 "#PWR4" H 3000 5850 30  0001 C CNN
F 1 "GND" H 3000 5780 30  0001 C CNN
F 2 "" H 3000 5850 60  0000 C CNN
F 3 "" H 3000 5850 60  0000 C CNN
	1    3000 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 5750 3000 5850
Wire Wire Line
	3000 5350 3000 5450
Wire Wire Line
	2700 5450 3550 5450
Connection ~ 3000 5450
Wire Wire Line
	2700 5750 3550 5750
Connection ~ 3000 5750
$Comp
L VCC #PWR3
U 1 1 54ECD0C1
P 3000 5350
F 0 "#PWR3" H 3000 5450 30  0001 C CNN
F 1 "VCC" H 3000 5450 30  0000 C CNN
F 2 "" H 3000 5350 60  0000 C CNN
F 3 "" H 3000 5350 60  0000 C CNN
	1    3000 5350
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR7
U 1 1 54ECD3DE
P 3550 2650
F 0 "#PWR7" H 3550 2750 30  0001 C CNN
F 1 "VCC" H 3550 2750 30  0000 C CNN
F 2 "" H 3550 2650 60  0000 C CNN
F 3 "" H 3550 2650 60  0000 C CNN
	1    3550 2650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR17
U 1 1 54ECF31C
P 4950 2300
F 0 "#PWR17" H 4950 2300 30  0001 C CNN
F 1 "GND" H 4950 2230 30  0001 C CNN
F 2 "" H 4950 2300 60  0000 C CNN
F 3 "" H 4950 2300 60  0000 C CNN
	1    4950 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2250 4950 2300
$Comp
L GND #PWR18
U 1 1 54ECF3AB
P 4950 3650
F 0 "#PWR18" H 4950 3650 30  0001 C CNN
F 1 "GND" H 4950 3580 30  0001 C CNN
F 2 "" H 4950 3650 60  0000 C CNN
F 3 "" H 4950 3650 60  0000 C CNN
	1    4950 3650
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR19
U 1 1 54ECF608
P 4950 5000
F 0 "#PWR19" H 4950 5000 30  0001 C CNN
F 1 "GND" H 4950 4930 30  0001 C CNN
F 2 "" H 4950 5000 60  0000 C CNN
F 3 "" H 4950 5000 60  0000 C CNN
	1    4950 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 3600 4950 3650
Wire Wire Line
	4950 4950 4950 5000
Text GLabel 4950 3950 0    51   Input ~ 0
OE
Text GLabel 4950 4050 0    51   Input ~ 0
clock
Text GLabel 4950 1350 0    51   Input ~ 0
clock
Text GLabel 6350 4350 2    51   Output ~ 0
p1_g2_buf
Text GLabel 8750 3650 2    51   Input ~ 0
p1_g2_buf
Text GLabel 4950 4250 0    51   Input ~ 0
p1_r2
Text GLabel 6350 4150 2    51   Output ~ 0
p1_b2_buff
Text GLabel 8750 3450 2    51   Input ~ 0
p1_g1_buff
Text GLabel 6350 1650 2    51   Output ~ 0
p0_g2_buff
Text GLabel 8750 2400 2    51   Input ~ 0
p0_g2_buff
Text GLabel 6350 1950 2    51   Output ~ 0
p0_g1_buff
Text GLabel 8750 2200 2    51   Input ~ 0
p0_g1_buff
Text GLabel 6350 2900 2    51   Output ~ 0
row_D_buff
Text GLabel 8750 2700 2    51   Input ~ 0
row_D_buff
Text GLabel 6350 3200 2    51   Output ~ 0
row_B_buff
Text GLabel 8750 2600 2    51   Input ~ 0
row_B_buff
Text GLabel 8750 3850 2    51   Input ~ 0
row_B_buff
Text GLabel 8750 3950 2    51   Input ~ 0
row_D_buff
Text GLabel 4950 2800 0    51   Input ~ 0
strobe
$Comp
L VCC #PWR13
U 1 1 54ED6227
P 4700 2150
F 0 "#PWR13" H 4700 2250 30  0001 C CNN
F 1 "VCC" H 4700 2250 30  0000 C CNN
F 2 "" H 4700 2150 60  0000 C CNN
F 3 "" H 4700 2150 60  0000 C CNN
	1    4700 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 2150 4700 2150
$Comp
L VCC #PWR14
U 1 1 54ED629A
P 4700 3500
F 0 "#PWR14" H 4700 3600 30  0001 C CNN
F 1 "VCC" H 4700 3600 30  0000 C CNN
F 2 "" H 4700 3500 60  0000 C CNN
F 3 "" H 4700 3500 60  0000 C CNN
	1    4700 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 3500 4950 3500
$Comp
L VCC #PWR15
U 1 1 54ED670D
P 4750 4850
F 0 "#PWR15" H 4750 4950 30  0001 C CNN
F 1 "VCC" H 4750 4950 30  0000 C CNN
F 2 "" H 4750 4850 60  0000 C CNN
F 3 "" H 4750 4850 60  0000 C CNN
	1    4750 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 4850 4950 4850
Text GLabel 8250 2200 0    51   Input ~ 0
p0_r1_buff
Text GLabel 8250 2800 0    51   Input ~ 0
clock_buff_0
Text GLabel 6350 1850 2    51   Output ~ 0
p0_r1_buff
Text GLabel 8250 2500 0    51   Input ~ 0
p0_b2_buff
Text GLabel 8250 2300 0    51   Input ~ 0
p0_b1_buff
Text GLabel 8250 2400 0    51   Input ~ 0
p0_r2_buff
Text GLabel 8250 2900 0    51   Input ~ 0
OE_buff_0
Text GLabel 6350 1750 2    51   Output ~ 0
p0_b1_buff
Text GLabel 6350 1450 2    51   Output ~ 0
p0_b2_buff
Text GLabel 6350 1550 2    51   Output ~ 0
p0_r2_buff
Text GLabel 6350 1350 2    51   Output ~ 0
clock_buff_0
Text GLabel 6350 1250 2    51   Output ~ 0
OE_buff_0
Text GLabel 8250 3450 0    51   Input ~ 0
p1_r1_buff
Text GLabel 8250 3550 0    51   Input ~ 0
p1_b1_buff
Text GLabel 8250 3650 0    51   Input ~ 0
p1_r2_buff
Text GLabel 8250 3750 0    51   Input ~ 0
p1_b2_buff
Text GLabel 8250 4050 0    51   Input ~ 0
clock_buff_1
Text GLabel 8250 4150 0    51   Input ~ 0
OE_buff_1
Text GLabel 6350 3950 2    51   Output ~ 0
OE_buff_1
Text GLabel 6350 4050 2    51   Output ~ 0
clock_buff_1
Text GLabel 6350 4250 2    51   Output ~ 0
p1_r2_buff
Text GLabel 6350 4450 2    51   Output ~ 0
p1_b1_buff
Text GLabel 6350 4550 2    51   Output ~ 0
p1_r1_buff
Text GLabel 6350 4650 2    51   Output ~ 0
p1_g1_buff
Text GLabel 3500 3000 2    51   Output ~ 0
p2_r1
Text GLabel 3000 2800 0    51   Output ~ 0
p2_g1
Text GLabel 3000 2900 0    51   Output ~ 0
p2_b1
Text GLabel 3000 4500 0    51   Output ~ 0
p2_r2
Text GLabel 3500 4400 2    51   Output ~ 0
p2_g2
Text GLabel 3500 4600 2    51   Output ~ 0
p2_b2
$Comp
L C C4
U 1 1 54F3B6F5
P 3550 5600
F 0 "C4" H 3550 5700 40  0000 L CNN
F 1 "100n" H 3556 5515 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 3588 5450 30  0001 C CNN
F 3 "" H 3550 5600 60  0000 C CNN
	1    3550 5600
	1    0    0    -1  
$EndComp
Connection ~ 3300 5450
Connection ~ 3300 5750
Text GLabel 6350 3000 2    51   Output ~ 0
row_C_buff
Text GLabel 6350 3100 2    51   Output ~ 0
row_A_buff
Text GLabel 6350 3300 2    51   Output ~ 0
strobe_buff_0
Text GLabel 6350 2800 2    51   Output ~ 0
strobe_buff_1
Text GLabel 8250 3850 0    51   Input ~ 0
row_A_buff
Text GLabel 8250 3950 0    51   Input ~ 0
row_C_buff
Text GLabel 8250 2600 0    51   Input ~ 0
row_A_buff
Text GLabel 8250 2700 0    51   Input ~ 0
row_C_buff
Text GLabel 8750 4050 2    51   Input ~ 0
strobe_buff_1
Text GLabel 8750 2800 2    51   Input ~ 0
strobe_buff_0
$Comp
L CONN_02X08 Panel-3
U 1 1 54F3E6D5
P 8500 5250
F 0 "Panel-3" H 8500 5700 50  0000 C CNN
F 1 "CONN_02X08" V 8500 5250 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 8500 4050 60  0001 C CNN
F 3 "" H 8500 4050 60  0000 C CNN
	1    8500 5250
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR23
U 1 1 54F3E6DC
P 9300 5600
F 0 "#PWR23" H 9300 5600 30  0001 C CNN
F 1 "GND" H 9300 5530 30  0001 C CNN
F 2 "" H 9300 5600 60  0000 C CNN
F 3 "" H 9300 5600 60  0000 C CNN
	1    9300 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 5600 8750 5600
Text GLabel 8750 5100 2    51   Input ~ 0
p2_g2_buf
Text GLabel 8750 4900 2    51   Input ~ 0
p2_g1_buff
Text GLabel 8750 5300 2    51   Input ~ 0
row_B_buff
Text GLabel 8750 5400 2    51   Input ~ 0
row_D_buff
Text GLabel 8250 4900 0    51   Input ~ 0
p2_r1_buff
Text GLabel 8250 5000 0    51   Input ~ 0
p2_b1_buff
Text GLabel 8250 5100 0    51   Input ~ 0
p2_r2_buff
Text GLabel 8250 5200 0    51   Input ~ 0
p2_b2_buff
Text GLabel 8250 5500 0    51   Input ~ 0
clock_buff_2
Text GLabel 8250 5600 0    51   Input ~ 0
OE_buff_2
Text GLabel 8250 5300 0    51   Input ~ 0
row_A_buff
Text GLabel 8250 5400 0    51   Input ~ 0
row_C_buff
Text GLabel 8750 5500 2    51   Input ~ 0
strobe_buff_2
$Comp
L 74HC245 U4
U 1 1 54F3F4A2
P 5650 5950
F 0 "U4" H 5750 6525 60  0000 L BNN
F 1 "74HCT245" H 5700 5375 60  0000 L TNN
F 2 "Housings_SOIC:SOIC-20_7.5x12.8mm_Pitch1.27mm" H 5650 5950 60  0001 C CNN
F 3 "" H 5650 5950 60  0000 C CNN
	1    5650 5950
	1    0    0    -1  
$EndComp
Text GLabel 4950 6150 0    51   Input ~ 0
p2_g1
Text GLabel 4950 6050 0    51   Input ~ 0
p2_r1
Text GLabel 4950 5950 0    51   Input ~ 0
p2_b1
Text GLabel 4950 5650 0    51   Input ~ 0
p2_b2
Text GLabel 4950 5850 0    51   Input ~ 0
p2_g2
$Comp
L GND #PWR20
U 1 1 54F3F4AD
P 4950 6500
F 0 "#PWR20" H 4950 6500 30  0001 C CNN
F 1 "GND" H 4950 6430 30  0001 C CNN
F 2 "" H 4950 6500 60  0000 C CNN
F 3 "" H 4950 6500 60  0000 C CNN
	1    4950 6500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4950 6450 4950 6500
Text GLabel 4950 5450 0    51   Input ~ 0
OE
Text GLabel 4950 5550 0    51   Input ~ 0
clock
Text GLabel 6350 5850 2    51   Output ~ 0
p2_g2_buf
Text GLabel 4950 5750 0    51   Input ~ 0
p2_r2
Text GLabel 6350 5650 2    51   Output ~ 0
p2_b2_buff
$Comp
L VCC #PWR16
U 1 1 54F3F4B9
P 4750 6350
F 0 "#PWR16" H 4750 6450 30  0001 C CNN
F 1 "VCC" H 4750 6450 30  0000 C CNN
F 2 "" H 4750 6350 60  0000 C CNN
F 3 "" H 4750 6350 60  0000 C CNN
	1    4750 6350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 6350 4950 6350
Text GLabel 6350 5450 2    51   Output ~ 0
OE_buff_2
Text GLabel 6350 5550 2    51   Output ~ 0
clock_buff_2
Text GLabel 6350 5750 2    51   Output ~ 0
p2_r2_buff
Text GLabel 6350 5950 2    51   Output ~ 0
p2_b1_buff
Text GLabel 6350 6050 2    51   Output ~ 0
p2_r1_buff
Text GLabel 6350 6150 2    51   Output ~ 0
p2_g1_buff
Text GLabel 6350 2700 2    51   Output ~ 0
strobe_buff_2
Text GLabel 4950 2700 0    51   Input ~ 0
strobe
NoConn ~ 3000 4000
NoConn ~ 3500 4000
NoConn ~ 3500 4100
NoConn ~ 3000 3100
NoConn ~ 3000 3500
$Comp
L PWR_FLAG #FLG2
U 1 1 557B1FD6
P 3750 2700
F 0 "#FLG2" H 3750 2795 50  0001 C CNN
F 1 "PWR_FLAG" H 3750 2880 50  0000 C CNN
F 2 "" H 3750 2700 60  0000 C CNN
F 3 "" H 3750 2700 60  0000 C CNN
	1    3750 2700
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG1
U 1 1 557B23A7
P 2950 4700
F 0 "#FLG1" H 2950 4795 50  0001 C CNN
F 1 "PWR_FLAG" H 2950 4880 50  0000 C CNN
F 2 "" H 2950 4700 60  0000 C CNN
F 3 "" H 2950 4700 60  0000 C CNN
	1    2950 4700
	-1   0    0    1   
$EndComp
Wire Wire Line
	2950 4600 2950 4700
Connection ~ 2950 4600
$Comp
L R R1
U 1 1 55B6F717
P 3000 2450
F 0 "R1" V 3080 2450 50  0000 C CNN
F 1 "10k" V 3000 2450 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 2930 2450 30  0001 C CNN
F 3 "" H 3000 2450 30  0000 C CNN
	1    3000 2450
	1    0    0    -1  
$EndComp
Text GLabel 3000 2250 1    51   Input ~ 0
OE
Wire Wire Line
	3000 2600 3000 2700
Wire Wire Line
	3000 2300 3000 2250
$Comp
L GND #PWR6
U 1 1 562B120F
P 3500 1800
F 0 "#PWR6" H 3500 1800 30  0001 C CNN
F 1 "GND" H 3500 1730 30  0001 C CNN
F 2 "" H 3500 1800 60  0000 C CNN
F 3 "" H 3500 1800 60  0000 C CNN
	1    3500 1800
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR5
U 1 1 562B12CC
P 3400 1600
F 0 "#PWR5" H 3400 1700 30  0001 C CNN
F 1 "VCC" H 3400 1700 30  0000 C CNN
F 2 "" H 3400 1600 60  0000 C CNN
F 3 "" H 3400 1600 60  0000 C CNN
	1    3400 1600
	1    0    0    -1  
$EndComp
Text Notes 2400 1700 0    60   ~ 0
Pi Power 5V
$Comp
L CP_Small C5
U 1 1 562B14A5
P 3500 1700
F 0 "C5" H 3510 1770 50  0000 L CNN
F 1 "22u" H 3450 1850 50  0000 L CNN
F 2 "Capacitors_ThroughHole:C_Radial_D6.3_L11.2_P2.5" H 3500 1700 60  0001 C CNN
F 3 "" H 3500 1700 60  0000 C CNN
	1    3500 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 1700 3350 1800
Wire Wire Line
	3350 1800 3700 1800
Wire Wire Line
	3350 1600 3700 1600
Connection ~ 3400 1600
$Comp
L CONN_01X01 P3
U 1 1 562B292A
P 4100 3100
F 0 "P3" H 4100 3200 50  0000 C CNN
F 1 "RxD" V 4200 3100 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 4100 3100 60  0001 C CNN
F 3 "" H 4100 3100 60  0000 C CNN
	1    4100 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 3100 3900 3100
Connection ~ 3500 1800
$Comp
L C_Small C6
U 1 1 562B2FF4
P 3700 1700
F 0 "C6" H 3710 1770 50  0000 L CNN
F 1 "22u" H 3650 1850 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206_HandSoldering" H 3700 1700 60  0001 C CNN
F 3 "" H 3700 1700 60  0000 C CNN
	1    3700 1700
	1    0    0    -1  
$EndComp
Connection ~ 3500 1600
Text Notes 3150 2000 0    60   ~ 0
C alt. footprint
$Comp
L CONN_01X01 P2
U 1 1 562B3873
P 3150 1600
F 0 "P2" H 3250 1600 50  0000 C CNN
F 1 "+5V" V 3250 1600 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 3150 1600 60  0001 C CNN
F 3 "" H 3150 1600 60  0000 C CNN
	1    3150 1600
	-1   0    0    1   
$EndComp
$Comp
L CONN_01X01 P4
U 1 1 562B38C3
P 3150 1700
F 0 "P4" H 3250 1700 50  0000 C CNN
F 1 "GND" V 3250 1700 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 3150 1700 60  0001 C CNN
F 3 "" H 3150 1700 60  0000 C CNN
	1    3150 1700
	-1   0    0    1   
$EndComp
$Comp
L CONN_01X01 P5
U 1 1 562BCB6A
P 4350 3100
F 0 "P5" H 4350 3200 50  0000 C CNN
F 1 "GND" V 4450 3100 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 4350 3100 60  0001 C CNN
F 3 "" H 4350 3100 60  0000 C CNN
	1    4350 3100
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR12
U 1 1 562BCCEC
P 4350 3300
F 0 "#PWR12" H 4350 3300 30  0001 C CNN
F 1 "GND" H 4350 3230 30  0001 C CNN
F 2 "" H 4350 3300 60  0000 C CNN
F 3 "" H 4350 3300 60  0000 C CNN
	1    4350 3300
	1    0    0    -1  
$EndComp
Text GLabel 3900 2850 2    51   Output ~ 0
row_E
Wire Wire Line
	3900 2850 3850 2850
Wire Wire Line
	3850 2850 3850 3100
Connection ~ 3850 3100
Text GLabel 4950 2600 0    51   Input ~ 0
row_E
$Comp
L CONN_01X02 P6
U 1 1 573741C0
P 9600 1550
F 0 "P6" H 9600 1700 50  0000 C CNN
F 1 "CONN_01X02" H 9150 1700 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 9600 1550 60  0001 C CNN
F 3 "" H 9600 1550 60  0000 C CNN
	1    9600 1550
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X02 P8
U 1 1 57374379
P 10100 1550
F 0 "P8" H 10100 1700 50  0000 C CNN
F 1 "CONN_01X02" H 9950 1400 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 10100 1550 60  0001 C CNN
F 3 "" H 10100 1550 60  0000 C CNN
	1    10100 1550
	-1   0    0    1   
$EndComp
$Comp
L CONN_02X01 P7
U 1 1 573743CD
P 9850 1550
F 0 "P7" H 9850 1650 50  0000 C CNN
F 1 "CONN_02X01" H 9850 1450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 9850 350 60  0001 C CNN
F 3 "" H 9850 350 60  0000 C CNN
	1    9850 1550
	0    1    1    0   
$EndComp
Text GLabel 9850 1300 1    51   Output ~ 0
Sel-Pin4
Text GLabel 9850 1800 3    51   Output ~ 0
Sel-Pin8
Text GLabel 8750 2300 2    51   Input ~ 0
Sel-Pin4
Text GLabel 8750 2500 2    51   Input ~ 0
Sel-Pin8
Text GLabel 8750 3550 2    51   Input ~ 0
Sel-Pin4
Text GLabel 8750 5000 2    51   Input ~ 0
Sel-Pin4
Text GLabel 8750 3750 2    51   Input ~ 0
Sel-Pin8
Text GLabel 8750 5200 2    51   Input ~ 0
Sel-Pin8
Wire Wire Line
	6350 2600 7350 2600
Wire Wire Line
	7350 2600 7350 1500
Wire Wire Line
	7350 1500 9400 1500
Wire Wire Line
	9400 1500 9400 1600
Connection ~ 9400 1500
Wire Wire Line
	10300 1500 10300 1650
$Comp
L GND #PWR24
U 1 1 57379022
P 10300 1650
F 0 "#PWR24" H 10300 1650 30  0001 C CNN
F 1 "GND" H 10300 1580 30  0001 C CNN
F 2 "" H 10300 1650 60  0000 C CNN
F 3 "" H 10300 1650 60  0000 C CNN
	1    10300 1650
	1    0    0    -1  
$EndComp
Connection ~ 10300 1600
Text Notes 8050 1900 0    60   ~ 0
Jumper for 1:32 multiplexing:\nSelection if E-Line on Pin 4 or 8\nOtherwise: Ground (for non-1:32).
$EndSCHEMATC
