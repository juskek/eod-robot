# eod-robot
Imperial College London | Mechanical Engineering
Embedded Control for Microcontrollers (ECM) 2019-2020 | Explosive Ordinance Disposal Robot

## Overview
Authors: Lim Siew Han & Justin Kek

IDE: MPLAB X IDE v4.20
Compiler: XC8 (v2.00)
Microchip: PIC18F4331 
Programmer: PICkit3

School: Imperial College London  
Department: Mechanical Engineering 
Module: Embedded C for Microcontrollers
Class: MEng Graduating 2021
  
Instructor: Dr Ravi Vaidyanathan
Due Date: 10 December 2019
Last Updated: 9 December 2019 1300


Description: The given objective was to design code to drive a vehicle 
             from a given start position towards a beacon which emits  
             50 ms infrared (IR) pulses every 250 ms. The vehicle should 
             then collect information from a radio-frequency identification
             (RFID) card and display it upon returning to its initial 
             position. 

             The vehicle consists of two IR sensors, one RFID sensor, and
             a liquid crystal display (LCD) screen. The IR sensors are 
             front facing and positioned side by side in the middle. Each 
             IR sensor is isolated from the other by a cardboard tube with 
             an inner lining of aluminium foil. The RFID sensor is wired
             below the IR sensors and suspended by a cantilever arm to
             match the length of the isolation tubes. 
             The IR and RFID sensors are held together by a 3D printed 
             housing made from polylactide (PLA).

Input: IR sensor signal, RFID signal
Output: RFID information on LCD screen
Main File: main.c
Source Files: functions.c, dc_motor_funct.c, lcd_funct.c

Required Features Not Included: The program adheres to all requirements 

Known Bugs: 
1. After fresh run, upon entering standby mode, motors run at full speed
   - Attempted solutions: Using stop_all() in standby loop

Improvements for future work:
1. When RFID is retrieved, steering may not stop immediately as it depends
   on when the interrupt stops the main program (i.e., if interrupt 
   triggers within __delay_ms(), delay continues running before exiting 
   loop. 
   - Solution: Write new function delay_ms() which incorporates a 
               conditional while loop to delay only when RFID is not 
               not retrieved. For example:
               delay_ms(&G_rfid_retrieved, &G_time_ms, 400);
               void delay_ms(unsigned char *flag, unsigned int *time_ms, 
                  unsigned int delay) 
               {
                 *time_ms = 0; // reset timer
                 while ((*flag == 0) | (*time_ms < delay));
               }
               
               
## Program Flow Chart
![alt text](https://github.com/[username]/[reponame]/blob/[branch]/image.jpg?raw=true)
