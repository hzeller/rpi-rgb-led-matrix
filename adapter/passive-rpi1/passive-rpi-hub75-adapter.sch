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
P 6000 3450
F 0 "Panel-1" H 6000 3900 50  0000 C CNN
F 1 "CONN_02X08" V 6000 3450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 6000 2250 60  0001 C CNN
F 3 "" H 6000 2250 60  0000 C CNN
	1    6000 3450
	1    0    0    -1  
$EndComp
$Comp
L CONN_02X13 P1
U 1 1 54ECB2B7
P 4500 3350
F 0 "P1" H 4500 4100 50  0000 C CNN
F 1 "RPi-Header" V 4500 3350 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x13" H 4500 2400 60  0001 C CNN
F 3 "" H 4500 2400 60  0000 C CNN
	1    4500 3350
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR01
U 1 1 54ECB3E1
P 4850 3350
F 0 "#PWR01" H 4850 3350 30  0001 C CNN
F 1 "GND" H 4850 3280 30  0001 C CNN
F 2 "" H 4850 3350 60  0000 C CNN
F 3 "" H 4850 3350 60  0000 C CNN
	1    4850 3350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4750 3350 4850 3350
$Comp
L GND #PWR02
U 1 1 54ECB417
P 4850 3650
F 0 "#PWR02" H 4850 3650 30  0001 C CNN
F 1 "GND" H 4850 3580 30  0001 C CNN
F 2 "" H 4850 3650 60  0000 C CNN
F 3 "" H 4850 3650 60  0000 C CNN
	1    4850 3650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4750 3650 4850 3650
$Comp
L GND #PWR03
U 1 1 54ECB4A1
P 4850 2950
F 0 "#PWR03" H 4850 2950 30  0001 C CNN
F 1 "GND" H 4850 2880 30  0001 C CNN
F 2 "" H 4850 2950 60  0000 C CNN
F 3 "" H 4850 2950 60  0000 C CNN
	1    4850 2950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4750 2950 4850 2950
$Comp
L GND #PWR04
U 1 1 54ECB7BC
P 4150 3950
F 0 "#PWR04" H 4150 3950 30  0001 C CNN
F 1 "GND" H 4150 3880 30  0001 C CNN
F 2 "" H 4150 3950 60  0000 C CNN
F 3 "" H 4150 3950 60  0000 C CNN
	1    4150 3950
	0    1    1    0   
$EndComp
Wire Wire Line
	4150 3950 4250 3950
Text GLabel 4250 3050 0    51   Output ~ 0
strobe
Text GLabel 4250 3450 0    51   Output ~ 0
row_A
Text GLabel 4750 3450 2    51   Output ~ 0
row_B
Text GLabel 4750 3550 2    51   Output ~ 0
row_C
Text GLabel 4750 3750 2    51   Output ~ 0
row_D
Text GLabel 4250 3250 0    51   Output ~ 0
clock
Text GLabel 4250 3850 0    51   Output ~ 0
p0_r1
Text GLabel 4250 3350 0    51   Output ~ 0
p0_g1
Text GLabel 4750 3950 2    51   Output ~ 0
p0_b1
Text GLabel 4750 3850 2    51   Output ~ 0
p0_r2
Text GLabel 4250 3750 0    51   Output ~ 0
p0_g2
Text GLabel 4250 3650 0    51   Output ~ 0
p0_b2
Text GLabel 4750 3250 2    51   Output ~ 0
OE
Wire Wire Line
	6250 3400 6800 3400
$Comp
L GND #PWR05
U 1 1 54ECD031
P 6800 3850
F 0 "#PWR05" H 6800 3850 30  0001 C CNN
F 1 "GND" H 6800 3780 30  0001 C CNN
F 2 "" H 6800 3850 60  0000 C CNN
F 3 "" H 6800 3850 60  0000 C CNN
	1    6800 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 3800 6800 3800
Wire Wire Line
	6250 3200 6800 3200
Wire Wire Line
	6800 3200 6800 3850
Connection ~ 6800 3400
Connection ~ 6800 3800
Text GLabel 6250 3300 2    51   Input ~ 0
p0_g2
Text GLabel 6250 3100 2    51   Input ~ 0
p0_g1
Text GLabel 6250 3600 2    51   Input ~ 0
row_D
Text GLabel 6250 3500 2    51   Input ~ 0
row_B
Text GLabel 5750 3100 0    51   Input ~ 0
p0_r1
Text GLabel 5750 3700 0    51   Input ~ 0
clock
Text GLabel 5750 3400 0    51   Input ~ 0
p0_b2
Text GLabel 5750 3200 0    51   Input ~ 0
p0_b1
Text GLabel 5750 3300 0    51   Input ~ 0
p0_r2
Text GLabel 5750 3800 0    51   Input ~ 0
OE
Text GLabel 5750 3500 0    51   Input ~ 0
row_A
Text GLabel 5750 3600 0    51   Input ~ 0
row_C
Text GLabel 6250 3700 2    51   Input ~ 0
strobe
NoConn ~ 4250 2750
NoConn ~ 4250 3150
NoConn ~ 4250 3550
NoConn ~ 4750 3150
NoConn ~ 4750 2750
NoConn ~ 4750 2850
NoConn ~ 4250 2850
NoConn ~ 4250 2950
NoConn ~ 4750 3050
$Comp
L PWR_FLAG #FLG06
U 1 1 557E1359
P 4200 4050
F 0 "#FLG06" H 4200 4145 50  0001 C CNN
F 1 "PWR_FLAG" H 4200 4230 50  0000 C CNN
F 2 "" H 4200 4050 60  0000 C CNN
F 3 "" H 4200 4050 60  0000 C CNN
	1    4200 4050
	-1   0    0    1   
$EndComp
Wire Wire Line
	4200 3950 4200 4050
Connection ~ 4200 3950
$EndSCHEMATC
