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
LIBS:rpi-hub75-adapter-cache
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
P 6250 2700
F 0 "U1" H 6350 3275 60  0000 L BNN
F 1 "74HC245" H 6300 2125 60  0000 L TNN
F 2 "Housings_SSOP:TSSOP-20_4.4x6.5mm_Pitch0.65mm" H 6250 2700 60  0001 C CNN
F 3 "" H 6250 2700 60  0000 C CNN
	1    6250 2700
	1    0    0    -1  
$EndComp
$Comp
L 74HC245 U2
U 1 1 54ECB18C
P 6250 4050
F 0 "U2" H 6350 4625 60  0000 L BNN
F 1 "74HC245" H 6300 3475 60  0000 L TNN
F 2 "Housings_SSOP:TSSOP-20_4.4x6.5mm_Pitch0.65mm" H 6250 4050 60  0001 C CNN
F 3 "" H 6250 4050 60  0000 C CNN
	1    6250 4050
	1    0    0    -1  
$EndComp
$Comp
L 74HC245 U3
U 1 1 54ECB1EA
P 6250 5400
F 0 "U3" H 6350 5975 60  0000 L BNN
F 1 "74HC245" H 6300 4825 60  0000 L TNN
F 2 "Housings_SSOP:TSSOP-20_4.4x6.5mm_Pitch0.65mm" H 6250 5400 60  0001 C CNN
F 3 "" H 6250 5400 60  0000 C CNN
	1    6250 5400
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
Text GLabel 5550 2900 0    51   Input ~ 0
p0_g1
Text GLabel 5550 2800 0    51   Input ~ 0
p0_r1
Text GLabel 5550 2700 0    51   Input ~ 0
p0_b1
Text GLabel 5550 2500 0    51   Input ~ 0
p0_r2
Text GLabel 5550 2400 0    51   Input ~ 0
p0_b2
Text GLabel 5550 2600 0    51   Input ~ 0
p0_g2
Wire Wire Line
	8750 2500 9300 2500
$Comp
L GND #PWR07
U 1 1 54ECD031
P 9300 2950
F 0 "#PWR07" H 9300 2950 30  0001 C CNN
F 1 "GND" H 9300 2880 30  0001 C CNN
F 2 "" H 9300 2950 60  0000 C CNN
F 3 "" H 9300 2950 60  0000 C CNN
	1    9300 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8750 2900 9300 2900
Wire Wire Line
	8750 2300 9300 2300
Text GLabel 5550 3850 0    51   Input ~ 0
row_A
Text GLabel 5550 3950 0    51   Input ~ 0
row_B
Text GLabel 5550 4050 0    51   Input ~ 0
row_C
Text GLabel 5550 4150 0    51   Input ~ 0
row_D
Text GLabel 5550 4250 0    51   Input ~ 0
strobe
Text GLabel 5550 2200 0    51   Input ~ 0
OE
$Comp
L CONN_02X08 Panel-2
U 1 1 54ECE201
P 8500 5100
F 0 "Panel-2" H 8500 5550 50  0000 C CNN
F 1 "CONN_02X08" V 8500 5100 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x08" H 8500 3900 60  0001 C CNN
F 3 "" H 8500 3900 60  0000 C CNN
	1    8500 5100
	1    0    0    -1  
$EndComp
Text GLabel 5550 5600 0    51   Input ~ 0
p1_g1
Text GLabel 5550 5500 0    51   Input ~ 0
p1_r1
Text GLabel 5550 5400 0    51   Input ~ 0
p1_b1
Text GLabel 5550 5100 0    51   Input ~ 0
p1_b2
Text GLabel 5550 5300 0    51   Input ~ 0
p1_g2
Wire Wire Line
	8750 5050 9300 5050
$Comp
L GND #PWR08
U 1 1 54ECE20E
P 9300 5500
F 0 "#PWR08" H 9300 5500 30  0001 C CNN
F 1 "GND" H 9300 5430 30  0001 C CNN
F 2 "" H 9300 5500 60  0000 C CNN
F 3 "" H 9300 5500 60  0000 C CNN
	1    9300 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8750 5450 9300 5450
Wire Wire Line
	8750 4850 9300 4850
$Comp
L C C1
U 1 1 54ECBE4F
P 4400 6550
F 0 "C1" H 4400 6650 40  0000 L CNN
F 1 "C" H 4406 6465 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 4438 6400 30  0001 C CNN
F 3 "" H 4400 6550 60  0000 C CNN
	1    4400 6550
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 54ECBEE4
P 4700 6550
F 0 "C2" H 4700 6650 40  0000 L CNN
F 1 "C" H 4706 6465 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 4738 6400 30  0001 C CNN
F 3 "" H 4700 6550 60  0000 C CNN
	1    4700 6550
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 54ECBF0A
P 5000 6550
F 0 "C3" H 5000 6650 40  0000 L CNN
F 1 "C" H 5006 6465 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 5038 6400 30  0001 C CNN
F 3 "" H 5000 6550 60  0000 C CNN
	1    5000 6550
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR09
U 1 1 54ECBF56
P 4700 6850
F 0 "#PWR09" H 4700 6850 30  0001 C CNN
F 1 "GND" H 4700 6780 30  0001 C CNN
F 2 "" H 4700 6850 60  0000 C CNN
F 3 "" H 4700 6850 60  0000 C CNN
	1    4700 6850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 6750 4700 6850
Wire Wire Line
	4700 6250 4700 6350
Wire Wire Line
	4400 6350 5000 6350
Connection ~ 4700 6350
Wire Wire Line
	4400 6750 5000 6750
Connection ~ 4700 6750
$Comp
L VCC #PWR010
U 1 1 54ECD0C1
P 4700 6250
F 0 "#PWR010" H 4700 6350 30  0001 C CNN
F 1 "VCC" H 4700 6350 30  0000 C CNN
F 2 "" H 4700 6250 60  0000 C CNN
F 3 "" H 4700 6250 60  0000 C CNN
	1    4700 6250
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR011
U 1 1 54ECD3DE
P 3550 2650
F 0 "#PWR011" H 3550 2750 30  0001 C CNN
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
$Comp
L GND #PWR012
U 1 1 54ECF31C
P 5550 3250
F 0 "#PWR012" H 5550 3250 30  0001 C CNN
F 1 "GND" H 5550 3180 30  0001 C CNN
F 2 "" H 5550 3250 60  0000 C CNN
F 3 "" H 5550 3250 60  0000 C CNN
	1    5550 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 3200 5550 3250
$Comp
L GND #PWR013
U 1 1 54ECF3AB
P 5550 4600
F 0 "#PWR013" H 5550 4600 30  0001 C CNN
F 1 "GND" H 5550 4530 30  0001 C CNN
F 2 "" H 5550 4600 60  0000 C CNN
F 3 "" H 5550 4600 60  0000 C CNN
	1    5550 4600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR014
U 1 1 54ECF608
P 5550 5950
F 0 "#PWR014" H 5550 5950 30  0001 C CNN
F 1 "GND" H 5550 5880 30  0001 C CNN
F 2 "" H 5550 5950 60  0000 C CNN
F 3 "" H 5550 5950 60  0000 C CNN
	1    5550 5950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 4550 5550 4600
Wire Wire Line
	5550 5900 5550 5950
Text GLabel 5550 4900 0    51   Input ~ 0
OE
Text GLabel 5550 5000 0    51   Input ~ 0
clock
Text GLabel 5550 2300 0    51   Input ~ 0
clock
Text GLabel 6950 5300 2    51   Output ~ 0
p1_g2_buf
Text GLabel 8750 4950 2    51   Input ~ 0
p1_g2_buf
Wire Wire Line
	9300 4850 9300 5500
Connection ~ 9300 5450
Connection ~ 9300 5050
Text GLabel 5550 5200 0    51   Input ~ 0
p1_r2
Text GLabel 6950 5100 2    51   Output ~ 0
p1_b2_buff
Text GLabel 8750 4750 2    51   Input ~ 0
p1_g1_buff
Wire Wire Line
	9300 2300 9300 2950
Connection ~ 9300 2500
Connection ~ 9300 2900
Text GLabel 6950 2600 2    51   Output ~ 0
p0_g2_buff
Text GLabel 8750 2400 2    51   Input ~ 0
p0_g2_buff
Text GLabel 6950 2900 2    51   Output ~ 0
p0_g1_buff
Text GLabel 8750 2200 2    51   Input ~ 0
p0_g1_buff
Text GLabel 6950 4150 2    51   Output ~ 0
row_D_buff
Text GLabel 8750 2700 2    51   Input ~ 0
row_D_buff
Text GLabel 6950 3950 2    51   Output ~ 0
row_B_buff
Text GLabel 8750 2600 2    51   Input ~ 0
row_B_buff
Wire Wire Line
	6950 3850 7900 3850
Wire Wire Line
	7900 2600 7900 5150
Wire Wire Line
	7900 2600 8250 2600
Wire Wire Line
	6950 4050 8050 4050
Wire Wire Line
	8050 2700 8050 5250
Wire Wire Line
	8050 2700 8250 2700
Text GLabel 8750 5150 2    51   Input ~ 0
row_B_buff
Text GLabel 8750 5250 2    51   Input ~ 0
row_D_buff
Wire Wire Line
	8050 5250 8250 5250
Connection ~ 8050 4050
Wire Wire Line
	7900 5150 8250 5150
Connection ~ 7900 3850
Text GLabel 5550 3750 0    51   Input ~ 0
strobe
Wire Wire Line
	6950 4250 9000 4250
Wire Wire Line
	9000 4250 9000 2800
Wire Wire Line
	9000 2800 8750 2800
Wire Wire Line
	6950 3750 9450 3750
Wire Wire Line
	9450 3750 9450 5350
Wire Wire Line
	9450 5350 8750 5350
$Comp
L VCC #PWR015
U 1 1 54ED6227
P 5300 3100
F 0 "#PWR015" H 5300 3200 30  0001 C CNN
F 1 "VCC" H 5300 3200 30  0000 C CNN
F 2 "" H 5300 3100 60  0000 C CNN
F 3 "" H 5300 3100 60  0000 C CNN
	1    5300 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 3100 5300 3100
$Comp
L VCC #PWR016
U 1 1 54ED629A
P 5300 4450
F 0 "#PWR016" H 5300 4550 30  0001 C CNN
F 1 "VCC" H 5300 4550 30  0000 C CNN
F 2 "" H 5300 4450 60  0000 C CNN
F 3 "" H 5300 4450 60  0000 C CNN
	1    5300 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 4450 5550 4450
$Comp
L VCC #PWR017
U 1 1 54ED670D
P 5350 5800
F 0 "#PWR017" H 5350 5900 30  0001 C CNN
F 1 "VCC" H 5350 5900 30  0000 C CNN
F 2 "" H 5350 5800 60  0000 C CNN
F 3 "" H 5350 5800 60  0000 C CNN
	1    5350 5800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5350 5800 5550 5800
Text GLabel 8250 2200 0    51   Input ~ 0
p0_r1_buff
Text GLabel 8250 2800 0    51   Input ~ 0
clock_buff_0
Text GLabel 6950 2800 2    51   Output ~ 0
p0_r1_buff
Text GLabel 8250 2500 0    51   Input ~ 0
p0_b2_buff
Text GLabel 8250 2300 0    51   Input ~ 0
p0_b1_buff
Text GLabel 8250 2400 0    51   Input ~ 0
p0_r2_buff
Text GLabel 8250 2900 0    51   Input ~ 0
OE_buff_0
Text GLabel 6950 2700 2    51   Output ~ 0
p0_b1_buff
Text GLabel 6950 2400 2    51   Output ~ 0
p0_b2_buff
Text GLabel 6950 2500 2    51   Output ~ 0
p0_r2_buff
Text GLabel 6950 2300 2    51   Output ~ 0
clock_buff_0
Text GLabel 6950 2200 2    51   Output ~ 0
OE_buff_0
Text GLabel 8250 4750 0    51   Input ~ 0
p1_r1_buff
Text GLabel 8250 4850 0    51   Input ~ 0
p1_b1_buff
Text GLabel 8250 4950 0    51   Input ~ 0
p1_r2_buff
Text GLabel 8250 5050 0    51   Input ~ 0
p1_b2_buff
Text GLabel 8250 5350 0    51   Input ~ 0
clock_buff_1
Text GLabel 8250 5450 0    51   Input ~ 0
OE_buff_1
Text GLabel 6950 4900 2    51   Output ~ 0
OE_buff_1
Text GLabel 6950 5000 2    51   Output ~ 0
clock_buff_1
Text GLabel 6950 5200 2    51   Output ~ 0
p1_r2_buff
Text GLabel 6950 5400 2    51   Output ~ 0
p1_b1_buff
Text GLabel 6950 5500 2    51   Output ~ 0
p1_r1_buff
Text GLabel 6950 5600 2    51   Output ~ 0
p1_g1_buff
$EndSCHEMATC
