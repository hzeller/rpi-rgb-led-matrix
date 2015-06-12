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
P 6950 2550
F 0 "Panel-1" H 6950 3000 50  0000 C CNN
F 1 "CONN_02X08" V 6950 2550 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 6950 1350 60  0001 C CNN
F 3 "" H 6950 1350 60  0000 C CNN
	1    6950 2550
	1    0    0    -1  
$EndComp
$Comp
L CONN_02X20 P1
U 1 1 54ECB2B7
P 4500 3700
F 0 "P1" H 4500 4750 50  0000 C CNN
F 1 "CONN_02X20" V 4500 3700 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x20" H 4500 2750 60  0001 C CNN
F 3 "" H 4500 2750 60  0000 C CNN
	1    4500 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 2700 4800 2850
Wire Wire Line
	4750 2750 4950 2750
Wire Wire Line
	4800 2850 4750 2850
Connection ~ 4800 2750
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
U 1 1 54ECB5FE
P 4150 4650
F 0 "#PWR04" H 4150 4650 30  0001 C CNN
F 1 "GND" H 4150 4580 30  0001 C CNN
F 2 "" H 4150 4650 60  0000 C CNN
F 3 "" H 4150 4650 60  0000 C CNN
	1    4150 4650
	0    1    1    0   
$EndComp
Wire Wire Line
	4150 4650 4250 4650
$Comp
L GND #PWR05
U 1 1 54ECB73E
P 4850 4350
F 0 "#PWR05" H 4850 4350 30  0001 C CNN
F 1 "GND" H 4850 4280 30  0001 C CNN
F 2 "" H 4850 4350 60  0000 C CNN
F 3 "" H 4850 4350 60  0000 C CNN
	1    4850 4350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4750 4350 4850 4350
$Comp
L GND #PWR06
U 1 1 54ECB7BC
P 4150 3950
F 0 "#PWR06" H 4150 3950 30  0001 C CNN
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
Text GLabel 4750 4250 2    51   Output ~ 0
p1_r1
Text GLabel 4250 4150 0    51   Output ~ 0
p1_g1
Text GLabel 4250 4250 0    51   Output ~ 0
p1_b1
Text GLabel 4250 4450 0    51   Output ~ 0
p1_r2
Text GLabel 4250 4350 0    51   Output ~ 0
p1_g2
Text GLabel 4750 4550 2    51   Output ~ 0
p1_b2
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
	7200 2500 7750 2500
$Comp
L GND #PWR07
U 1 1 54ECD031
P 7750 2950
F 0 "#PWR07" H 7750 2950 30  0001 C CNN
F 1 "GND" H 7750 2880 30  0001 C CNN
F 2 "" H 7750 2950 60  0000 C CNN
F 3 "" H 7750 2950 60  0000 C CNN
	1    7750 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 2900 7750 2900
Wire Wire Line
	7200 2300 7750 2300
$Comp
L CONN_02X08 Panel-2
U 1 1 54ECE201
P 6950 3800
F 0 "Panel-2" H 6950 4250 50  0000 C CNN
F 1 "CONN_02X08" V 6950 3800 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 6950 2600 60  0001 C CNN
F 3 "" H 6950 2600 60  0000 C CNN
	1    6950 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 3750 7750 3750
$Comp
L GND #PWR08
U 1 1 54ECE20E
P 7750 4200
F 0 "#PWR08" H 7750 4200 30  0001 C CNN
F 1 "GND" H 7750 4130 30  0001 C CNN
F 2 "" H 7750 4200 60  0000 C CNN
F 3 "" H 7750 4200 60  0000 C CNN
	1    7750 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 4150 7750 4150
Wire Wire Line
	7200 3550 7750 3550
$Comp
L VCC #PWR09
U 1 1 54ECD3DE
P 4800 2700
F 0 "#PWR09" H 4800 2800 30  0001 C CNN
F 1 "VCC" H 4800 2800 30  0000 C CNN
F 2 "" H 4800 2700 60  0000 C CNN
F 3 "" H 4800 2700 60  0000 C CNN
	1    4800 2700
	1    0    0    -1  
$EndComp
Text GLabel 7200 3650 2    51   Input ~ 0
p1_g2
Wire Wire Line
	7750 3550 7750 4200
Connection ~ 7750 4150
Connection ~ 7750 3750
Text GLabel 7200 3450 2    51   Input ~ 0
p1_g1
Wire Wire Line
	7750 2300 7750 2950
Connection ~ 7750 2500
Connection ~ 7750 2900
Text GLabel 7200 2400 2    51   Input ~ 0
p0_g2
Text GLabel 7200 2200 2    51   Input ~ 0
p0_g1
Text GLabel 7200 2700 2    51   Input ~ 0
row_D
Text GLabel 7200 2600 2    51   Input ~ 0
row_B
Text GLabel 7200 3850 2    51   Input ~ 0
row_B
Text GLabel 7200 3950 2    51   Input ~ 0
row_D
Text GLabel 6700 2200 0    51   Input ~ 0
p0_r1
Text GLabel 6700 2800 0    51   Input ~ 0
clock
Text GLabel 6700 2500 0    51   Input ~ 0
p0_b2
Text GLabel 6700 2300 0    51   Input ~ 0
p0_b1
Text GLabel 6700 2400 0    51   Input ~ 0
p0_r2
Text GLabel 6700 2900 0    51   Input ~ 0
OE
Text GLabel 6700 3450 0    51   Input ~ 0
p1_r1
Text GLabel 6700 3550 0    51   Input ~ 0
p1_b1
Text GLabel 6700 3650 0    51   Input ~ 0
p1_r2
Text GLabel 6700 3750 0    51   Input ~ 0
p1_b2
Text GLabel 6700 4050 0    51   Input ~ 0
clock
Text GLabel 6700 4150 0    51   Input ~ 0
OE
Text GLabel 4750 3050 2    51   Output ~ 0
p2_r1
Text GLabel 4250 2850 0    51   Output ~ 0
p2_g1
Text GLabel 4250 2950 0    51   Output ~ 0
p2_b1
Text GLabel 4250 4550 0    51   Output ~ 0
p2_r2
Text GLabel 4750 4450 2    51   Output ~ 0
p2_g2
Text GLabel 4750 4650 2    51   Output ~ 0
p2_b2
Text GLabel 6700 3850 0    51   Input ~ 0
row_A
Text GLabel 6700 3950 0    51   Input ~ 0
row_C
Text GLabel 6700 2600 0    51   Input ~ 0
row_A
Text GLabel 6700 2700 0    51   Input ~ 0
row_C
Text GLabel 7200 4050 2    51   Input ~ 0
strobe
Text GLabel 7200 2800 2    51   Input ~ 0
strobe
$Comp
L CONN_02X08 Panel-3
U 1 1 54F3E6D5
P 6950 5250
F 0 "Panel-3" H 6950 5700 50  0000 C CNN
F 1 "CONN_02X08" V 6950 5250 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 6950 4050 60  0001 C CNN
F 3 "" H 6950 4050 60  0000 C CNN
	1    6950 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 5200 7750 5200
$Comp
L GND #PWR010
U 1 1 54F3E6DC
P 7750 5650
F 0 "#PWR010" H 7750 5650 30  0001 C CNN
F 1 "GND" H 7750 5580 30  0001 C CNN
F 2 "" H 7750 5650 60  0000 C CNN
F 3 "" H 7750 5650 60  0000 C CNN
	1    7750 5650
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 5600 7750 5600
Wire Wire Line
	7200 5000 7750 5000
Text GLabel 7200 5100 2    51   Input ~ 0
p2_g2
Wire Wire Line
	7750 5000 7750 5650
Connection ~ 7750 5600
Connection ~ 7750 5200
Text GLabel 7200 4900 2    51   Input ~ 0
p2_g1
Text GLabel 7200 5300 2    51   Input ~ 0
row_B
Text GLabel 7200 5400 2    51   Input ~ 0
row_D
Text GLabel 6700 4900 0    51   Input ~ 0
p2_r1
Text GLabel 6700 5000 0    51   Input ~ 0
p2_b1
Text GLabel 6700 5100 0    51   Input ~ 0
p2_r2
Text GLabel 6700 5200 0    51   Input ~ 0
p2_b2
Text GLabel 6700 5500 0    51   Input ~ 0
clock
Text GLabel 6700 5600 0    51   Input ~ 0
OE
Text GLabel 6700 5300 0    51   Input ~ 0
row_A
Text GLabel 6700 5400 0    51   Input ~ 0
row_C
Text GLabel 7200 5500 2    51   Input ~ 0
strobe
NoConn ~ 4250 2750
NoConn ~ 4250 3150
NoConn ~ 4250 3550
NoConn ~ 4250 4050
NoConn ~ 4750 4050
NoConn ~ 4750 4150
NoConn ~ 4750 3150
$Comp
L PWR_FLAG #FLG011
U 1 1 557B29FD
P 4950 2750
F 0 "#FLG011" H 4950 2845 50  0001 C CNN
F 1 "PWR_FLAG" H 4950 2930 50  0000 C CNN
F 2 "" H 4950 2750 60  0000 C CNN
F 3 "" H 4950 2750 60  0000 C CNN
	1    4950 2750
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG012
U 1 1 557B2BFC
P 4200 4700
F 0 "#FLG012" H 4200 4795 50  0001 C CNN
F 1 "PWR_FLAG" H 4200 4880 50  0000 C CNN
F 2 "" H 4200 4700 60  0000 C CNN
F 3 "" H 4200 4700 60  0000 C CNN
	1    4200 4700
	-1   0    0    1   
$EndComp
Wire Wire Line
	4200 4650 4200 4700
Connection ~ 4200 4650
$EndSCHEMATC
