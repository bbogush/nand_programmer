EESchema Schematic File Version 4
LIBS:adapter_soic8_solder-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "NANDO SOIC-8 adapter solder"
Date ""
Rev "v1.0"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L nand_programmator:AT45DB161D-SU U1
U 1 1 5F656105
P 5400 3350
F 0 "U1" H 5841 3396 50  0000 L CNN
F 1 "AT45DB" H 5841 3305 50  0000 L CNN
F 2 "lib_fp:SOIC-8_3.9x4.9mm_P1.27mm" H 6550 2750 50  0001 C CNN
F 3 "https://www.adestotech.com/wp-content/uploads/doc3500.pdf" H 5400 3350 50  0001 C CNN
	1    5400 3350
	1    0    0    -1  
$EndComp
$Comp
L nand_programmator:Conn_02x07_Odd_Even J1
U 1 1 5F6563AD
P 3750 3300
F 0 "J1" H 3800 3817 50  0000 C CNN
F 1 "Conn_02x07_Odd_Even" H 3800 3726 50  0000 C CNN
F 2 "lib_fp:PinSocket_2x07_P2.54mm_Vertical" H 3750 3300 50  0001 C CNN
F 3 "~" H 3750 3300 50  0001 C CNN
F 4 "1392520" H 3750 3300 50  0001 C CNN "Farnell Ref"
	1    3750 3300
	1    0    0    -1  
$EndComp
$Comp
L nand_programmator:Conn_02x07_Odd_Even J2
U 1 1 5F65640F
P 6750 3300
F 0 "J2" H 6800 3817 50  0000 C CNN
F 1 "Conn_02x07_Odd_Even" H 6800 3726 50  0000 C CNN
F 2 "lib_fp:PinSocket_2x07_P2.54mm_Vertical" H 6750 3300 50  0001 C CNN
F 3 "~" H 6750 3300 50  0001 C CNN
F 4 "1392520" H 6750 3300 50  0001 C CNN "Farnell Ref"
	1    6750 3300
	1    0    0    -1  
$EndComp
$Comp
L nand_programmator:GNDD #PWR0101
U 1 1 5F65723C
P 5400 3850
F 0 "#PWR0101" H 5400 3600 50  0001 C CNN
F 1 "GNDD" H 5405 3677 50  0000 C CNN
F 2 "" H 5400 3850 50  0000 C CNN
F 3 "" H 5400 3850 50  0000 C CNN
	1    5400 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 3750 5400 3850
$Comp
L nand_programmator:GNDD #PWR0102
U 1 1 5F657271
P 3400 3650
F 0 "#PWR0102" H 3400 3400 50  0001 C CNN
F 1 "GNDD" H 3405 3477 50  0000 C CNN
F 2 "" H 3400 3650 50  0000 C CNN
F 3 "" H 3400 3650 50  0000 C CNN
	1    3400 3650
	1    0    0    -1  
$EndComp
$Comp
L nand_programmator:+3.3V #PWR0103
U 1 1 5F6572C8
P 4200 3500
F 0 "#PWR0103" H 4200 3350 50  0001 C CNN
F 1 "+3.3V" H 4215 3673 50  0000 C CNN
F 2 "" H 4200 3500 50  0000 C CNN
F 3 "" H 4200 3500 50  0000 C CNN
	1    4200 3500
	1    0    0    -1  
$EndComp
$Comp
L nand_programmator:+3.3V #PWR0104
U 1 1 5F6572F3
P 5400 2800
F 0 "#PWR0104" H 5400 2650 50  0001 C CNN
F 1 "+3.3V" H 5415 2973 50  0000 C CNN
F 2 "" H 5400 2800 50  0000 C CNN
F 3 "" H 5400 2800 50  0000 C CNN
	1    5400 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5400 2800 5400 2950
Wire Wire Line
	4200 3500 4050 3500
Text GLabel 3300 3000 0    50   Output ~ 0
CS
Text GLabel 4800 3450 0    50   Input ~ 0
CS
Wire Wire Line
	3300 3000 3550 3000
Wire Wire Line
	4800 3150 5000 3150
Text GLabel 4300 3000 2    50   Output ~ 0
SCK
Text GLabel 4800 3550 0    50   Input ~ 0
SCK
Wire Wire Line
	4800 3550 5000 3550
Wire Wire Line
	4050 3000 4300 3000
Wire Wire Line
	3400 3650 3400 3600
Wire Wire Line
	3400 3600 3550 3600
Text GLabel 3300 3100 0    50   Input ~ 0
SO
Wire Wire Line
	3300 3100 3550 3100
Text GLabel 6000 3150 2    50   Output ~ 0
SO
Wire Wire Line
	5800 3150 6000 3150
Text GLabel 4300 3100 2    50   Output ~ 0
SI
Text GLabel 4800 3150 0    50   Input ~ 0
SI
Wire Wire Line
	4800 3450 5000 3450
Wire Wire Line
	4050 3100 4300 3100
$Comp
L nand_programmator:+3.3V #PWR0105
U 1 1 5F6579FE
P 4800 3300
F 0 "#PWR0105" H 4800 3150 50  0001 C CNN
F 1 "+3.3V" V 4815 3428 50  0000 L CNN
F 2 "" H 4800 3300 50  0000 C CNN
F 3 "" H 4800 3300 50  0000 C CNN
	1    4800 3300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5000 3250 4800 3250
Wire Wire Line
	4800 3250 4800 3300
Wire Wire Line
	5000 3350 4800 3350
Wire Wire Line
	4800 3350 4800 3300
Connection ~ 4800 3300
$EndSCHEMATC
