EESchema Schematic File Version 4
LIBS:active3-rpi-hub75-adapter-cache
EELAYER 29 0
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
L Connector_Generic:Conn_02x08_Odd_Even Panel-1
U 1 1 54ECB236
P 8450 2500
F 0 "Panel-1" H 8450 2950 50  0000 C CNN
F 1 "CONN_02X08" V 8450 2500 50  0000 C CNN
F 2 "Connector_IDC:IDC-Header_2x08_P2.54mm_Vertical" H 8450 1300 60  0001 C CNN
F 3 "" H 8450 1300 60  0000 C CNN
	1    8450 2500
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x20_Odd_Even P1
U 1 1 54ECB2B7
P 2450 3450
F 0 "P1" H 2450 4500 50  0000 C CNN
F 1 "CONN_02X20" V 2450 3450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x20" H 2450 2500 60  0001 C CNN
F 3 "" H 2450 2500 60  0000 C CNN
	1    2450 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2500 2800 2550
Wire Wire Line
	2750 2550 2800 2550
Wire Wire Line
	2800 2650 2750 2650
Connection ~ 2800 2550
$Comp
L power:GND #PWR09
U 1 1 54ECB3E1
P 2850 3150
F 0 "#PWR09" H 2850 3150 30  0001 C CNN
F 1 "GND" H 2850 3080 30  0001 C CNN
F 2 "" H 2850 3150 60  0000 C CNN
F 3 "" H 2850 3150 60  0000 C CNN
	1    2850 3150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2750 3150 2850 3150
$Comp
L power:GND #PWR010
U 1 1 54ECB417
P 2850 3450
F 0 "#PWR010" H 2850 3450 30  0001 C CNN
F 1 "GND" H 2850 3380 30  0001 C CNN
F 2 "" H 2850 3450 60  0000 C CNN
F 3 "" H 2850 3450 60  0000 C CNN
	1    2850 3450
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2750 3450 2850 3450
$Comp
L power:GND #PWR08
U 1 1 54ECB4A1
P 2850 2750
F 0 "#PWR08" H 2850 2750 30  0001 C CNN
F 1 "GND" H 2850 2680 30  0001 C CNN
F 2 "" H 2850 2750 60  0000 C CNN
F 3 "" H 2850 2750 60  0000 C CNN
	1    2850 2750
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2750 2750 2850 2750
$Comp
L power:GND #PWR02
U 1 1 54ECB5FE
P 2150 4450
F 0 "#PWR02" H 2150 4450 30  0001 C CNN
F 1 "GND" H 2150 4380 30  0001 C CNN
F 2 "" H 2150 4450 60  0000 C CNN
F 3 "" H 2150 4450 60  0000 C CNN
	1    2150 4450
	0    1    1    0   
$EndComp
Wire Wire Line
	2150 4450 2200 4450
$Comp
L power:GND #PWR011
U 1 1 54ECB73E
P 2850 4150
F 0 "#PWR011" H 2850 4150 30  0001 C CNN
F 1 "GND" H 2850 4080 30  0001 C CNN
F 2 "" H 2850 4150 60  0000 C CNN
F 3 "" H 2850 4150 60  0000 C CNN
	1    2850 4150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2750 4150 2850 4150
$Comp
L power:GND #PWR01
U 1 1 54ECB7BC
P 2150 3750
F 0 "#PWR01" H 2150 3750 30  0001 C CNN
F 1 "GND" H 2150 3680 30  0001 C CNN
F 2 "" H 2150 3750 60  0000 C CNN
F 3 "" H 2150 3750 60  0000 C CNN
	1    2150 3750
	0    1    1    0   
$EndComp
Wire Wire Line
	2150 3750 2250 3750
Text GLabel 2250 2850 0    51   Output ~ 0
strobe
Text GLabel 2750 4050 2    51   Output ~ 0
p1_r1
Text GLabel 2250 3950 0    51   Output ~ 0
p1_g1
Text GLabel 2250 4050 0    51   Output ~ 0
p1_b1
Text GLabel 2250 4250 0    51   Output ~ 0
p1_r2
Text GLabel 2250 4150 0    51   Output ~ 0
p1_g2
Text GLabel 2750 4350 2    51   Output ~ 0
p1_b2
Text GLabel 2250 3250 0    51   Output ~ 0
row_A
Text GLabel 2750 3250 2    51   Output ~ 0
row_B
Text GLabel 2750 3350 2    51   Output ~ 0
row_C
Text GLabel 2750 3550 2    51   Output ~ 0
row_D
Text GLabel 2250 3050 0    51   Output ~ 0
clock
Text GLabel 2250 3650 0    51   Output ~ 0
p0_r1
Text GLabel 2250 3150 0    51   Output ~ 0
p0_g1
Text GLabel 2750 3750 2    51   Output ~ 0
p0_b1
Text GLabel 2750 3650 2    51   Output ~ 0
p0_r2
Text GLabel 2250 3550 0    51   Output ~ 0
p0_g2
Text GLabel 2250 3450 0    51   Output ~ 0
p0_b2
Text GLabel 2750 3050 2    51   Output ~ 0
OE
Text GLabel 5200 1700 0    51   Input ~ 0
p0_g1
Text GLabel 5200 1600 0    51   Input ~ 0
p0_r1
Text GLabel 5200 1500 0    51   Input ~ 0
p0_b1
Text GLabel 5200 1300 0    51   Input ~ 0
p0_r2
Text GLabel 5200 1200 0    51   Input ~ 0
p0_b2
Text GLabel 5200 1400 0    51   Input ~ 0
p0_g2
$Comp
L power:GND #PWR021
U 1 1 54ECD031
P 9300 2900
F 0 "#PWR021" H 9300 2900 30  0001 C CNN
F 1 "GND" H 9300 2830 30  0001 C CNN
F 2 "" H 9300 2900 60  0000 C CNN
F 3 "" H 9300 2900 60  0000 C CNN
	1    9300 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 2900 8750 2900
Text GLabel 4400 3300 0    51   Input ~ 0
row_A
Text GLabel 4400 3400 0    51   Input ~ 0
row_B
Text GLabel 4400 3200 0    51   Input ~ 0
row_C
Text GLabel 4400 3100 0    51   Input ~ 0
row_D
Text GLabel 4400 3500 0    51   Input ~ 0
strobe
Text GLabel 5200 1000 0    51   Input ~ 0
OE
$Comp
L Connector_Generic:Conn_02x08_Odd_Even Panel-2
U 1 1 54ECE201
P 8450 3750
F 0 "Panel-2" H 8450 4200 50  0000 C CNN
F 1 "CONN_02X08" V 8450 3750 50  0000 C CNN
F 2 "Connector_IDC:IDC-Header_2x08_P2.54mm_Vertical" H 8450 2550 60  0001 C CNN
F 3 "" H 8450 2550 60  0000 C CNN
	1    8450 3750
	1    0    0    -1  
$EndComp
Text GLabel 5200 5250 0    51   Input ~ 0
p1_g1
Text GLabel 5200 5150 0    51   Input ~ 0
p1_r1
Text GLabel 5200 5050 0    51   Input ~ 0
p1_b1
Text GLabel 5200 4750 0    51   Input ~ 0
p1_b2
Text GLabel 5200 4950 0    51   Input ~ 0
p1_g2
$Comp
L power:GND #PWR022
U 1 1 54ECE20E
P 9300 4150
F 0 "#PWR022" H 9300 4150 30  0001 C CNN
F 1 "GND" H 9300 4080 30  0001 C CNN
F 2 "" H 9300 4150 60  0000 C CNN
F 3 "" H 9300 4150 60  0000 C CNN
	1    9300 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 4150 8750 4150
$Comp
L Device:C C1
U 1 1 54ECBE4F
P 1950 5450
F 0 "C1" H 1950 5550 40  0000 L CNN
F 1 "100n" H 1956 5365 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1988 5300 30  0001 C CNN
F 3 "" H 1950 5450 60  0000 C CNN
	1    1950 5450
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 54ECBEE4
P 2250 5450
F 0 "C2" H 2250 5550 40  0000 L CNN
F 1 "100n" H 2256 5365 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2288 5300 30  0001 C CNN
F 3 "" H 2250 5450 60  0000 C CNN
	1    2250 5450
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 54ECBF0A
P 2550 5450
F 0 "C3" H 2550 5550 40  0000 L CNN
F 1 "100n" H 2556 5365 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2588 5300 30  0001 C CNN
F 3 "" H 2550 5450 60  0000 C CNN
	1    2550 5450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 54ECBF56
P 2250 5700
F 0 "#PWR04" H 2250 5700 30  0001 C CNN
F 1 "GND" H 2250 5630 30  0001 C CNN
F 2 "" H 2250 5700 60  0000 C CNN
F 3 "" H 2250 5700 60  0000 C CNN
	1    2250 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 5600 2250 5700
Wire Wire Line
	2250 5200 2250 5300
Wire Wire Line
	1950 5300 2250 5300
Connection ~ 2250 5300
Wire Wire Line
	1950 5600 2250 5600
Connection ~ 2250 5600
$Comp
L power:VCC #PWR03
U 1 1 54ECD0C1
P 2250 5200
F 0 "#PWR03" H 2250 5300 30  0001 C CNN
F 1 "VCC" H 2250 5300 30  0000 C CNN
F 2 "" H 2250 5200 60  0000 C CNN
F 3 "" H 2250 5200 60  0000 C CNN
	1    2250 5200
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR07
U 1 1 54ECD3DE
P 2800 2500
F 0 "#PWR07" H 2800 2600 30  0001 C CNN
F 1 "VCC" H 2800 2600 30  0000 C CNN
F 2 "" H 2800 2500 60  0000 C CNN
F 3 "" H 2800 2500 60  0000 C CNN
	1    2800 2500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR017
U 1 1 54ECF31C
P 5200 2050
F 0 "#PWR017" H 5200 2050 30  0001 C CNN
F 1 "GND" H 5200 1980 30  0001 C CNN
F 2 "" H 5200 2050 60  0000 C CNN
F 3 "" H 5200 2050 60  0000 C CNN
	1    5200 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 2000 5200 2050
$Comp
L power:GND #PWR018
U 1 1 54ECF3AB
P 4400 3850
F 0 "#PWR018" H 4400 3850 30  0001 C CNN
F 1 "GND" H 4400 3780 30  0001 C CNN
F 2 "" H 4400 3850 60  0000 C CNN
F 3 "" H 4400 3850 60  0000 C CNN
	1    4400 3850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 54ECF608
P 5200 5600
F 0 "#PWR019" H 5200 5600 30  0001 C CNN
F 1 "GND" H 5200 5530 30  0001 C CNN
F 2 "" H 5200 5600 60  0000 C CNN
F 3 "" H 5200 5600 60  0000 C CNN
	1    5200 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 3800 4400 3850
Wire Wire Line
	5200 5550 5200 5600
Text GLabel 5200 4550 0    51   Input ~ 0
OE
Text GLabel 5200 4650 0    51   Input ~ 0
clock
Text GLabel 5200 1100 0    51   Input ~ 0
clock
Text GLabel 6200 4950 2    51   Output ~ 0
p1_g2_buf
Text GLabel 8750 3650 2    51   Input ~ 0
p1_g2_buf
Text GLabel 5200 4850 0    51   Input ~ 0
p1_r2
Text GLabel 6200 4750 2    51   Output ~ 0
p1_b2_buff
Text GLabel 8750 3450 2    51   Input ~ 0
p1_g1_buff
Text GLabel 6200 1400 2    51   Output ~ 0
p0_g2_buff
Text GLabel 8750 2400 2    51   Input ~ 0
p0_g2_buff
Text GLabel 6200 1700 2    51   Output ~ 0
p0_g1_buff
Text GLabel 8750 2200 2    51   Input ~ 0
p0_g1_buff
Text GLabel 5400 3100 2    51   Output ~ 0
row_D_buff
Text GLabel 8750 2700 2    51   Input ~ 0
row_D_buff
Text GLabel 5400 3400 2    51   Output ~ 0
row_B_buff
Text GLabel 8750 2600 2    51   Input ~ 0
row_B_buff
Text GLabel 8750 3850 2    51   Input ~ 0
row_B_buff
Text GLabel 8750 3950 2    51   Input ~ 0
row_D_buff
Text GLabel 4400 3000 0    51   Input ~ 0
strobe
$Comp
L power:VCC #PWR013
U 1 1 54ED6227
P 4950 1900
F 0 "#PWR013" H 4950 2000 30  0001 C CNN
F 1 "VCC" H 4950 2000 30  0000 C CNN
F 2 "" H 4950 1900 60  0000 C CNN
F 3 "" H 4950 1900 60  0000 C CNN
	1    4950 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 1900 4950 1900
$Comp
L power:VCC #PWR014
U 1 1 54ED629A
P 4150 3700
F 0 "#PWR014" H 4150 3800 30  0001 C CNN
F 1 "VCC" H 4150 3800 30  0000 C CNN
F 2 "" H 4150 3700 60  0000 C CNN
F 3 "" H 4150 3700 60  0000 C CNN
	1    4150 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 3700 4400 3700
$Comp
L power:VCC #PWR015
U 1 1 54ED670D
P 5000 5450
F 0 "#PWR015" H 5000 5550 30  0001 C CNN
F 1 "VCC" H 5000 5550 30  0000 C CNN
F 2 "" H 5000 5450 60  0000 C CNN
F 3 "" H 5000 5450 60  0000 C CNN
	1    5000 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 5450 5200 5450
Text GLabel 8250 2200 0    51   Input ~ 0
p0_r1_buff
Text GLabel 8250 2800 0    51   Input ~ 0
clock_buff_0
Text GLabel 6200 1600 2    51   Output ~ 0
p0_r1_buff
Text GLabel 8250 2500 0    51   Input ~ 0
p0_b2_buff
Text GLabel 8250 2300 0    51   Input ~ 0
p0_b1_buff
Text GLabel 8250 2400 0    51   Input ~ 0
p0_r2_buff
Text GLabel 8250 2900 0    51   Input ~ 0
OE_buff_0
Text GLabel 6200 1500 2    51   Output ~ 0
p0_b1_buff
Text GLabel 6200 1200 2    51   Output ~ 0
p0_b2_buff
Text GLabel 6200 1300 2    51   Output ~ 0
p0_r2_buff
Text GLabel 6200 1100 2    51   Output ~ 0
clock_buff_0
Text GLabel 6200 1000 2    51   Output ~ 0
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
Text GLabel 6200 4550 2    51   Output ~ 0
OE_buff_1
Text GLabel 6200 4650 2    51   Output ~ 0
clock_buff_1
Text GLabel 6200 4850 2    51   Output ~ 0
p1_r2_buff
Text GLabel 6200 5050 2    51   Output ~ 0
p1_b1_buff
Text GLabel 6200 5150 2    51   Output ~ 0
p1_r1_buff
Text GLabel 6200 5250 2    51   Output ~ 0
p1_g1_buff
Text GLabel 2750 2850 2    51   Output ~ 0
p2_r1
Text GLabel 2250 2650 0    51   Output ~ 0
p2_g1
Text GLabel 2250 2750 0    51   Output ~ 0
p2_b1
Text GLabel 2250 4350 0    51   Output ~ 0
p2_r2
Text GLabel 2750 4250 2    51   Output ~ 0
p2_g2
Text GLabel 2750 4450 2    51   Output ~ 0
p2_b2
$Comp
L Device:C C4
U 1 1 54F3B6F5
P 2800 5450
F 0 "C4" H 2800 5550 40  0000 L CNN
F 1 "100n" H 2806 5365 40  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2838 5300 30  0001 C CNN
F 3 "" H 2800 5450 60  0000 C CNN
	1    2800 5450
	1    0    0    -1  
$EndComp
Connection ~ 2550 5300
Connection ~ 2550 5600
Text GLabel 5400 3200 2    51   Output ~ 0
row_C_buff
Text GLabel 5400 3300 2    51   Output ~ 0
row_A_buff
Text GLabel 5400 3500 2    51   Output ~ 0
strobe_buff_0
Text GLabel 5400 3000 2    51   Output ~ 0
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
L Connector_Generic:Conn_02x08_Odd_Even Panel-3
U 1 1 54F3E6D5
P 8450 5200
F 0 "Panel-3" H 8450 5650 50  0000 C CNN
F 1 "CONN_02X08" V 8450 5200 50  0000 C CNN
F 2 "Connector_IDC:IDC-Header_2x08_P2.54mm_Vertical" H 8450 4000 60  0001 C CNN
F 3 "" H 8450 4000 60  0000 C CNN
	1    8450 5200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR023
U 1 1 54F3E6DC
P 9300 5600
F 0 "#PWR023" H 9300 5600 30  0001 C CNN
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
Text GLabel 4400 6900 0    51   Input ~ 0
p2_g1
Text GLabel 4400 6800 0    51   Input ~ 0
p2_r1
Text GLabel 4400 6700 0    51   Input ~ 0
p2_b1
Text GLabel 4400 6400 0    51   Input ~ 0
p2_b2
Text GLabel 4400 6600 0    51   Input ~ 0
p2_g2
$Comp
L power:GND #PWR020
U 1 1 54F3F4AD
P 4400 7250
F 0 "#PWR020" H 4400 7250 30  0001 C CNN
F 1 "GND" H 4400 7180 30  0001 C CNN
F 2 "" H 4400 7250 60  0000 C CNN
F 3 "" H 4400 7250 60  0000 C CNN
	1    4400 7250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 7200 4400 7250
Text GLabel 4400 6200 0    51   Input ~ 0
OE
Text GLabel 4400 6300 0    51   Input ~ 0
clock
Text GLabel 5400 6600 2    51   Output ~ 0
p2_g2_buf
Text GLabel 4400 6500 0    51   Input ~ 0
p2_r2
Text GLabel 5400 6400 2    51   Output ~ 0
p2_b2_buff
$Comp
L power:VCC #PWR016
U 1 1 54F3F4B9
P 4200 7100
F 0 "#PWR016" H 4200 7200 30  0001 C CNN
F 1 "VCC" H 4200 7200 30  0000 C CNN
F 2 "" H 4200 7100 60  0000 C CNN
F 3 "" H 4200 7100 60  0000 C CNN
	1    4200 7100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 7100 4400 7100
Text GLabel 5400 6200 2    51   Output ~ 0
OE_buff_2
Text GLabel 5400 6300 2    51   Output ~ 0
clock_buff_2
Text GLabel 5400 6500 2    51   Output ~ 0
p2_r2_buff
Text GLabel 5400 6700 2    51   Output ~ 0
p2_b1_buff
Text GLabel 5400 6800 2    51   Output ~ 0
p2_r1_buff
Text GLabel 5400 6900 2    51   Output ~ 0
p2_g1_buff
Text GLabel 5400 2900 2    51   Output ~ 0
strobe_buff_2
Text GLabel 4400 2900 0    51   Input ~ 0
strobe
NoConn ~ 2250 3850
NoConn ~ 2750 3850
NoConn ~ 2750 3950
NoConn ~ 2250 2950
NoConn ~ 2250 3350
$Comp
L power:PWR_FLAG #FLG02
U 1 1 557B1FD6
P 3000 2550
F 0 "#FLG02" H 3000 2645 50  0001 C CNN
F 1 "PWR_FLAG" H 3000 2730 50  0000 C CNN
F 2 "" H 3000 2550 60  0000 C CNN
F 3 "" H 3000 2550 60  0000 C CNN
	1    3000 2550
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG01
U 1 1 557B23A7
P 2200 4550
F 0 "#FLG01" H 2200 4645 50  0001 C CNN
F 1 "PWR_FLAG" H 2200 4730 50  0000 C CNN
F 2 "" H 2200 4550 60  0000 C CNN
F 3 "" H 2200 4550 60  0000 C CNN
	1    2200 4550
	-1   0    0    1   
$EndComp
Wire Wire Line
	2200 4450 2200 4550
Connection ~ 2200 4450
$Comp
L Device:R R1
U 1 1 55B6F717
P 2250 2300
F 0 "R1" V 2330 2300 50  0000 C CNN
F 1 "10k" V 2250 2300 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2180 2300 30  0001 C CNN
F 3 "" H 2250 2300 30  0000 C CNN
	1    2250 2300
	1    0    0    -1  
$EndComp
Text GLabel 2250 2100 1    51   Input ~ 0
OE
Wire Wire Line
	2250 2450 2250 2550
Wire Wire Line
	2250 2150 2250 2100
$Comp
L power:GND #PWR06
U 1 1 562B120F
P 2750 1650
F 0 "#PWR06" H 2750 1650 30  0001 C CNN
F 1 "GND" H 2750 1580 30  0001 C CNN
F 2 "" H 2750 1650 60  0000 C CNN
F 3 "" H 2750 1650 60  0000 C CNN
	1    2750 1650
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR05
U 1 1 562B12CC
P 2650 1450
F 0 "#PWR05" H 2650 1550 30  0001 C CNN
F 1 "VCC" H 2650 1550 30  0000 C CNN
F 2 "" H 2650 1450 60  0000 C CNN
F 3 "" H 2650 1450 60  0000 C CNN
	1    2650 1450
	1    0    0    -1  
$EndComp
Text Notes 1650 1550 0    60   ~ 0
Pi Power 5V
$Comp
L Device:CP_Small C5
U 1 1 562B14A5
P 2750 1550
F 0 "C5" H 2760 1620 50  0000 L CNN
F 1 "22u" H 2700 1700 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 2750 1550 60  0001 C CNN
F 3 "" H 2750 1550 60  0000 C CNN
	1    2750 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 1550 2600 1650
Wire Wire Line
	2600 1650 2750 1650
Wire Wire Line
	2600 1450 2650 1450
Connection ~ 2650 1450
$Comp
L Connector_Generic:Conn_01x01 P3
U 1 1 562B292A
P 3350 2950
F 0 "P3" H 3350 3050 50  0000 C CNN
F 1 "RxD" V 3450 2950 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 3350 2950 60  0001 C CNN
F 3 "" H 3350 2950 60  0000 C CNN
	1    3350 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	2750 2950 3100 2950
Connection ~ 2750 1650
$Comp
L Device:C_Small C6
U 1 1 562B2FF4
P 2950 1550
F 0 "C6" H 2960 1620 50  0000 L CNN
F 1 "22u" H 2900 1700 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric" H 2950 1550 60  0001 C CNN
F 3 "" H 2950 1550 60  0000 C CNN
	1    2950 1550
	1    0    0    -1  
$EndComp
Connection ~ 2750 1450
Text Notes 2400 1850 0    60   ~ 0
C alt. footprint
$Comp
L Connector_Generic:Conn_01x01 P2
U 1 1 562B3873
P 2400 1450
F 0 "P2" H 2500 1450 50  0000 C CNN
F 1 "+5V" V 2500 1450 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 2400 1450 60  0001 C CNN
F 3 "" H 2400 1450 60  0000 C CNN
	1    2400 1450
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 P4
U 1 1 562B38C3
P 2400 1550
F 0 "P4" H 2500 1550 50  0000 C CNN
F 1 "GND" V 2500 1550 50  0001 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 2400 1550 60  0001 C CNN
F 3 "" H 2400 1550 60  0000 C CNN
	1    2400 1550
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 P5
U 1 1 562BCB6A
P 3600 2950
F 0 "P5" H 3600 3050 50  0000 C CNN
F 1 "GND" V 3700 2950 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x01" H 3600 2950 60  0001 C CNN
F 3 "" H 3600 2950 60  0000 C CNN
	1    3600 2950
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 562BCCEC
P 3600 3150
F 0 "#PWR012" H 3600 3150 30  0001 C CNN
F 1 "GND" H 3600 3080 30  0001 C CNN
F 2 "" H 3600 3150 60  0000 C CNN
F 3 "" H 3600 3150 60  0000 C CNN
	1    3600 3150
	1    0    0    -1  
$EndComp
Text GLabel 3150 2700 2    51   Output ~ 0
row_E
Wire Wire Line
	3150 2700 3100 2700
Wire Wire Line
	3100 2700 3100 2950
Connection ~ 3100 2950
Text GLabel 4400 2800 0    51   Input ~ 0
row_E
$Comp
L Connector_Generic:Conn_01x02 P6
U 1 1 573741C0
P 9600 1500
F 0 "P6" H 9600 1650 50  0000 C CNN
F 1 "CONN_01X02" H 9150 1650 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 9600 1500 60  0001 C CNN
F 3 "" H 9600 1500 60  0000 C CNN
	1    9600 1500
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 P8
U 1 1 57374379
P 10100 1600
F 0 "P8" H 10100 1750 50  0000 C CNN
F 1 "CONN_01X02" H 9950 1450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 10100 1600 60  0001 C CNN
F 3 "" H 10100 1600 60  0000 C CNN
	1    10100 1600
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_02x01 P7
U 1 1 573743CD
P 9850 1500
F 0 "P7" H 9850 1600 50  0000 C CNN
F 1 "CONN_02X01" H 9850 1400 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02" H 9850 300 60  0001 C CNN
F 3 "" H 9850 300 60  0000 C CNN
	1    9850 1500
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
	7350 1500 9400 1500
Wire Wire Line
	9400 1500 9400 1600
Connection ~ 9400 1500
Wire Wire Line
	10300 1500 10300 1600
$Comp
L power:GND #PWR024
U 1 1 57379022
P 10300 1650
F 0 "#PWR024" H 10300 1650 30  0001 C CNN
F 1 "GND" H 10300 1580 30  0001 C CNN
F 2 "" H 10300 1650 60  0000 C CNN
F 3 "" H 10300 1650 60  0000 C CNN
	1    10300 1650
	1    0    0    -1  
$EndComp
Connection ~ 10300 1600
Text Notes 8050 1900 0    60   ~ 0
Jumper for 1:32 multiplexing:\nSelection if E-Line on Pin 4 or 8\nOtherwise: Ground (for non-1:32).
Wire Wire Line
	2800 2550 2800 2650
Wire Wire Line
	2800 2550 3000 2550
Wire Wire Line
	2250 5300 2550 5300
Wire Wire Line
	2250 5600 2550 5600
Wire Wire Line
	2550 5300 2800 5300
Wire Wire Line
	2550 5600 2800 5600
Wire Wire Line
	2200 4450 2250 4450
Wire Wire Line
	2650 1450 2750 1450
Wire Wire Line
	2750 1650 2950 1650
Wire Wire Line
	2750 1450 2950 1450
Wire Wire Line
	3100 2950 3150 2950
Wire Wire Line
	10300 1600 10300 1650
$Comp
L 74xx:74HC245 U1
U 1 1 5D21950C
P 5700 1500
F 0 "U1" H 5700 1750 50  0000 C CNN
F 1 "74HCT245" H 5700 1200 50  0000 C CNN
F 2 "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm" H 5700 1500 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HC245" H 5700 1500 50  0001 C CNN
	1    5700 1500
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC245 U2
U 1 1 5D22AB08
P 4900 3300
F 0 "U2" H 4900 3550 50  0000 C CNN
F 1 "74HCT245" H 4900 3000 50  0000 C CNN
F 2 "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm" H 4900 3300 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HC245" H 4900 3300 50  0001 C CNN
	1    4900 3300
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC245 U3
U 1 1 5D23CC9C
P 5700 5050
F 0 "U3" H 5700 5300 50  0000 C CNN
F 1 "74HCT245" H 5700 4750 50  0000 C CNN
F 2 "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm" H 5700 5050 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HC245" H 5700 5050 50  0001 C CNN
	1    5700 5050
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC245 U4
U 1 1 5D23FE55
P 4900 6700
F 0 "U4" H 4900 6950 50  0000 C CNN
F 1 "74HCT245" H 4900 6400 50  0000 C CNN
F 2 "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm" H 4900 6700 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HC245" H 4900 6700 50  0001 C CNN
	1    4900 6700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5D245A6F
P 5700 2300
F 0 "#PWR0101" H 5700 2300 30  0001 C CNN
F 1 "GND" H 5700 2230 30  0001 C CNN
F 2 "" H 5700 2300 60  0000 C CNN
F 3 "" H 5700 2300 60  0000 C CNN
	1    5700 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5D24DA95
P 4900 4100
F 0 "#PWR0102" H 4900 4100 30  0001 C CNN
F 1 "GND" H 4900 4030 30  0001 C CNN
F 2 "" H 4900 4100 60  0000 C CNN
F 3 "" H 4900 4100 60  0000 C CNN
	1    4900 4100
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0103
U 1 1 5D24E926
P 5700 4250
F 0 "#PWR0103" H 5700 4100 50  0001 C CNN
F 1 "VCC" H 5717 4423 50  0000 C CNN
F 2 "" H 5700 4250 50  0001 C CNN
F 3 "" H 5700 4250 50  0001 C CNN
	1    5700 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5D24F243
P 5700 5850
F 0 "#PWR0104" H 5700 5850 30  0001 C CNN
F 1 "GND" H 5700 5780 30  0001 C CNN
F 2 "" H 5700 5850 60  0000 C CNN
F 3 "" H 5700 5850 60  0000 C CNN
	1    5700 5850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 5D2513C6
P 4900 7500
F 0 "#PWR0105" H 4900 7500 30  0001 C CNN
F 1 "GND" H 4900 7430 30  0001 C CNN
F 2 "" H 4900 7500 60  0000 C CNN
F 3 "" H 4900 7500 60  0000 C CNN
	1    4900 7500
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0106
U 1 1 5D251E34
P 4900 5900
F 0 "#PWR0106" H 4900 5750 50  0001 C CNN
F 1 "VCC" H 4917 6073 50  0000 C CNN
F 2 "" H 4900 5900 50  0001 C CNN
F 3 "" H 4900 5900 50  0001 C CNN
	1    4900 5900
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0107
U 1 1 5D252DED
P 5700 700
F 0 "#PWR0107" H 5700 550 50  0001 C CNN
F 1 "VCC" H 5717 873 50  0000 C CNN
F 2 "" H 5700 700 50  0001 C CNN
F 3 "" H 5700 700 50  0001 C CNN
	1    5700 700 
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0108
U 1 1 5D253721
P 4900 2500
F 0 "#PWR0108" H 4900 2350 50  0001 C CNN
F 1 "VCC" H 4917 2673 50  0000 C CNN
F 2 "" H 4900 2500 50  0001 C CNN
F 3 "" H 4900 2500 50  0001 C CNN
	1    4900 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 2800 7350 2800
Wire Wire Line
	7350 1500 7350 2800
$EndSCHEMATC
