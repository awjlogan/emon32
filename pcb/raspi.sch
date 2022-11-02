EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 5
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
L Connector:Raspberry_Pi_2_3 J7
U 1 1 5FA255A2
P 5700 4200
F 0 "J7" H 5700 5681 50  0000 C CNN
F 1 "Raspberry_Pi_2_3" H 5700 5590 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x20_P2.54mm_Vertical" H 5700 4200 50  0001 C CNN
F 3 "https://www.raspberrypi.org/documentation/hardware/raspberrypi/schematics/rpi_SCH_3bplus_1p0_reduced.pdf" H 5700 4200 50  0001 C CNN
	1    5700 4200
	1    0    0    -1  
$EndComp
NoConn ~ 5800 2900
NoConn ~ 5900 2900
$Comp
L power:GND #PWR0105
U 1 1 5FA28AF5
P 5300 5700
F 0 "#PWR0105" H 5300 5450 50  0001 C CNN
F 1 "GND" H 5305 5527 50  0000 C CNN
F 2 "" H 5300 5700 50  0001 C CNN
F 3 "" H 5300 5700 50  0001 C CNN
	1    5300 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 5700 5300 5600
Wire Wire Line
	5300 5600 5400 5600
Wire Wire Line
	6000 5600 6000 5500
Connection ~ 5300 5600
Wire Wire Line
	5300 5600 5300 5500
Wire Wire Line
	5900 5500 5900 5600
Connection ~ 5900 5600
Wire Wire Line
	5900 5600 6000 5600
Wire Wire Line
	5800 5500 5800 5600
Connection ~ 5800 5600
Wire Wire Line
	5800 5600 5900 5600
Wire Wire Line
	5700 5500 5700 5600
Connection ~ 5700 5600
Wire Wire Line
	5700 5600 5800 5600
Wire Wire Line
	5600 5500 5600 5600
Connection ~ 5600 5600
Wire Wire Line
	5600 5600 5700 5600
Wire Wire Line
	5500 5500 5500 5600
Connection ~ 5500 5600
Wire Wire Line
	5500 5600 5600 5600
Wire Wire Line
	5400 5500 5400 5600
Connection ~ 5400 5600
Wire Wire Line
	5400 5600 5500 5600
Wire Wire Line
	4900 3300 4500 3300
Wire Wire Line
	4900 3400 4500 3400
Text HLabel 4500 3300 0    50   Output ~ 0
UART_TX
Text HLabel 4500 3400 0    50   Input ~ 0
UART_RX
Wire Wire Line
	4900 4600 4500 4600
Wire Wire Line
	4900 4700 4500 4700
Wire Wire Line
	4900 3800 4500 3800
Text HLabel 3950 3800 0    50   Output ~ 0
SW_nRST
Text HLabel 3950 4600 0    50   BiDi ~ 0
SWDIO
Text HLabel 3950 4700 0    50   Output ~ 0
SWCLK
$Comp
L power:+5V #PWR0107
U 1 1 6006B40C
P 5500 2500
F 0 "#PWR0107" H 5500 2350 50  0001 C CNN
F 1 "+5V" H 5515 2673 50  0000 C CNN
F 2 "" H 5500 2500 50  0001 C CNN
F 3 "" H 5500 2500 50  0001 C CNN
	1    5500 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 2900 5500 2750
Wire Wire Line
	5600 2900 5600 2750
Wire Wire Line
	5600 2750 5500 2750
Connection ~ 5500 2750
Wire Wire Line
	5500 2750 5500 2500
NoConn ~ 4900 4800
NoConn ~ 4900 4900
NoConn ~ 4900 4500
NoConn ~ 4900 4400
NoConn ~ 4900 4200
NoConn ~ 4900 4100
NoConn ~ 4900 4000
NoConn ~ 4900 3700
NoConn ~ 6500 3300
NoConn ~ 6500 3400
NoConn ~ 6500 3600
NoConn ~ 6500 3700
NoConn ~ 6500 3900
NoConn ~ 6500 4000
NoConn ~ 6500 4100
NoConn ~ 6500 4300
NoConn ~ 6500 4400
NoConn ~ 6500 4500
NoConn ~ 6500 4600
NoConn ~ 6500 4700
NoConn ~ 6500 4900
NoConn ~ 6500 5000
$Comp
L Device:R R46
U 1 1 601A0472
P 4350 4600
F 0 "R46" V 4300 4400 50  0000 C CNN
F 1 "1K" V 4350 4600 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 4280 4600 50  0001 C CNN
F 3 "~" H 4350 4600 50  0001 C CNN
	1    4350 4600
	0    1    1    0   
$EndComp
$Comp
L Device:R R47
U 1 1 601A4169
P 4350 4700
F 0 "R47" V 4300 4500 50  0000 C CNN
F 1 "1K" V 4350 4700 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 4280 4700 50  0001 C CNN
F 3 "~" H 4350 4700 50  0001 C CNN
	1    4350 4700
	0    1    1    0   
$EndComp
Wire Wire Line
	3950 4600 4200 4600
Wire Wire Line
	3950 4700 4200 4700
$Comp
L Device:R R44
U 1 1 601BADCE
P 4350 3800
F 0 "R44" V 4300 3600 50  0000 C CNN
F 1 "1K" V 4350 3800 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 4280 3800 50  0001 C CNN
F 3 "~" H 4350 3800 50  0001 C CNN
	1    4350 3800
	0    1    1    0   
$EndComp
Wire Wire Line
	4200 3800 3950 3800
Wire Wire Line
	4900 3600 4500 3600
Text HLabel 4500 3600 0    50   Input ~ 0
EXTINT
$EndSCHEMATC
