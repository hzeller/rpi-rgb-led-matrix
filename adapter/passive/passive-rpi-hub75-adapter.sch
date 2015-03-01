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
LIBS:special
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
L CONN_02X08 Panel-1
U 1 1 54ECB236
P 5350 3100
F 0 "Panel-1" H 5350 3550 50  0000 C CNN
F 1 "CONN_02X08" V 5350 3100 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 5350 1900 60  0001 C CNN
F 3 "" H 5350 1900 60  0000 C CNN
	1    5350 3100
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
	3550 2700 3500 2700
Wire Wire Line
	3550 2800 3500 2800
Connection ~ 3550 2700
$Comp
L GND #PWR01
U 1 1 54ECB3E1
P 3600 3300
F 0 "#PWR01" H 3600 3300 30  0001 C CNN
F 1 "GND" H 3600 3230 30  0001 C CNN
F 2 "" H 3600 3300 60  0000 C CNN
F 3 "" H 3600 3300 60  0000 C CNN
	1    3600 3300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 3300 3600 3300
$Comp
L GND #PWR02
U 1 1 54ECB417
P 3600 3600
F 0 "#PWR02" H 3600 3600 30  0001 C CNN
F 1 "GND" H 3600 3530 30  0001 C CNN
F 2 "" H 3600 3600 60  0000 C CNN
F 3 "" H 3600 3600 60  0000 C CNN
	1    3600 3600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 3600 3600 3600
$Comp
L GND #PWR03
U 1 1 54ECB4A1
P 3600 2900
F 0 "#PWR03" H 3600 2900 30  0001 C CNN
F 1 "GND" H 3600 2830 30  0001 C CNN
F 2 "" H 3600 2900 60  0000 C CNN
F 3 "" H 3600 2900 60  0000 C CNN
	1    3600 2900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 2900 3600 2900
$Comp
L GND #PWR04
U 1 1 54ECB5FE
P 2900 4600
F 0 "#PWR04" H 2900 4600 30  0001 C CNN
F 1 "GND" H 2900 4530 30  0001 C CNN
F 2 "" H 2900 4600 60  0000 C CNN
F 3 "" H 2900 4600 60  0000 C CNN
	1    2900 4600
	0    1    1    0   
$EndComp
Wire Wire Line
	2900 4600 3000 4600
$Comp
L GND #PWR05
U 1 1 54ECB73E
P 3600 4300
F 0 "#PWR05" H 3600 4300 30  0001 C CNN
F 1 "GND" H 3600 4230 30  0001 C CNN
F 2 "" H 3600 4300 60  0000 C CNN
F 3 "" H 3600 4300 60  0000 C CNN
	1    3600 4300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3500 4300 3600 4300
$Comp
L GND #PWR06
U 1 1 54ECB7BC
P 2900 3900
F 0 "#PWR06" H 2900 3900 30  0001 C CNN
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
Text GLabel 3500 3900 2    51   Output ~ 0
row_A
Text GLabel 3500 3800 2    51   Output ~ 0
row_B
Text GLabel 3000 3700 0    51   Output ~ 0
row_C
Text GLabel 3000 3600 0    51   Output ~ 0
row_D
Text GLabel 3000 3800 0    51   Output ~ 0
clock
Text GLabel 3000 3200 0    51   Output ~ 0
p0_r1
Text GLabel 3500 3200 2    51   Output ~ 0
p0_g1
Text GLabel 3000 3400 0    51   Output ~ 0
p0_b1
Text GLabel 3500 3400 2    51   Output ~ 0
p0_r2
Text GLabel 3500 3500 2    51   Output ~ 0
p0_g2
Text GLabel 3500 3700 2    51   Output ~ 0
p0_b2
Text GLabel 3000 3300 0    51   Output ~ 0
OE
Wire Wire Line
	5600 3050 6150 3050
$Comp
L GND #PWR07
U 1 1 54ECD031
P 6150 3500
F 0 "#PWR07" H 6150 3500 30  0001 C CNN
F 1 "GND" H 6150 3430 30  0001 C CNN
F 2 "" H 6150 3500 60  0000 C CNN
F 3 "" H 6150 3500 60  0000 C CNN
	1    6150 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 3450 6150 3450
Wire Wire Line
	5600 2850 6150 2850
$Comp
L CONN_02X08 Panel-2
U 1 1 54ECE201
P 5350 4300
F 0 "Panel-2" H 5350 4750 50  0000 C CNN
F 1 "CONN_02X08" V 5350 4300 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 5350 3100 60  0001 C CNN
F 3 "" H 5350 3100 60  0000 C CNN
	1    5350 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 4250 6150 4250
$Comp
L GND #PWR08
U 1 1 54ECE20E
P 6150 4700
F 0 "#PWR08" H 6150 4700 30  0001 C CNN
F 1 "GND" H 6150 4630 30  0001 C CNN
F 2 "" H 6150 4700 60  0000 C CNN
F 3 "" H 6150 4700 60  0000 C CNN
	1    6150 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 4650 6150 4650
Wire Wire Line
	5600 4050 6150 4050
$Comp
L VCC #PWR09
U 1 1 54ECD3DE
P 3550 2650
F 0 "#PWR09" H 3550 2750 30  0001 C CNN
F 1 "VCC" H 3550 2750 30  0000 C CNN
F 2 "" H 3550 2650 60  0000 C CNN
F 3 "" H 3550 2650 60  0000 C CNN
	1    3550 2650
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X02 P2
U 1 1 54ECE3B2
P 2650 2850
F 0 "P2" H 2650 3000 50  0000 C CNN
F 1 "CONN_01X02" V 2750 2850 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 2650 2850 60  0001 C CNN
F 3 "" H 2650 2850 60  0000 C CNN
	1    2650 2850
	-1   0    0    1   
$EndComp
Wire Wire Line
	2850 2800 3000 2800
Wire Wire Line
	2850 2900 3000 2900
$Comp
L CONN_01X02 P3
U 1 1 54ECE47C
P 3850 3050
F 0 "P3" H 3850 3200 50  0000 C CNN
F 1 "CONN_01X02" V 3950 3050 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 3850 3050 60  0001 C CNN
F 3 "" H 3850 3050 60  0000 C CNN
	1    3850 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 3000 3650 3000
Wire Wire Line
	3500 3100 3650 3100
Text GLabel 5600 4150 2    51   Input ~ 0
p1_g2
Wire Wire Line
	6150 4050 6150 4700
Connection ~ 6150 4650
Connection ~ 6150 4250
Text GLabel 5600 3950 2    51   Input ~ 0
p1_g1
Wire Wire Line
	6150 2850 6150 3500
Connection ~ 6150 3050
Connection ~ 6150 3450
Text GLabel 5600 2950 2    51   Input ~ 0
p0_g2
Text GLabel 5600 2750 2    51   Input ~ 0
p0_g1
Text GLabel 5600 3250 2    51   Input ~ 0
row_D
Text GLabel 5600 3150 2    51   Input ~ 0
row_B
Text GLabel 5600 4350 2    51   Input ~ 0
row_B
Text GLabel 5600 4450 2    51   Input ~ 0
row_D
Text GLabel 5100 2750 0    51   Input ~ 0
p0_r1
Text GLabel 5100 3350 0    51   Input ~ 0
clock
Text GLabel 5100 3050 0    51   Input ~ 0
p0_b2
Text GLabel 5100 2850 0    51   Input ~ 0
p0_b1
Text GLabel 5100 2950 0    51   Input ~ 0
p0_r2
Text GLabel 5100 3450 0    51   Input ~ 0
OE
Text GLabel 5100 3950 0    51   Input ~ 0
p1_r1
Text GLabel 5100 4050 0    51   Input ~ 0
p1_b1
Text GLabel 5100 4150 0    51   Input ~ 0
p1_r2
Text GLabel 5100 4250 0    51   Input ~ 0
p1_b2
Text GLabel 5100 4550 0    51   Input ~ 0
clock
Text GLabel 5100 4650 0    51   Input ~ 0
OE
Text GLabel 5100 4450 0    51   Input ~ 0
row_C
Text GLabel 5100 4350 0    51   Input ~ 0
row_A
Text GLabel 5100 3150 0    51   Input ~ 0
row_A
Text GLabel 5100 3250 0    51   Input ~ 0
row_C
Text GLabel 5600 4550 2    51   Input ~ 0
strobe
Text GLabel 5600 3350 2    51   Input ~ 0
strobe
$EndSCHEMATC
