/* ============================================================================
 |  Assignment: Explosive Ordinance Disposal
 |
 |  Author: Lim Siew Han & Justin Kek
 |  Language: C
 |  IDE: MPLAB X IDE v4.20
 |  Compiler: XC8 (v2.00)
 |  Compiler Settings: Optimisation Level 1
 |  Microchip: PIC18F4331 
 |  Programmer: PICkit3
 |  Program Memory Usage: 87% 
 |  Data Memory Usage: 46%
 |  
 |  School: Imperial College London  
 |  Department: Mechanical Engineering 
 |  Module: Embedded C for Microcontrollers
 |  Class: MEng Graduating 2021
 |  
 |  Instructor: Dr Ravi Vaidyanathan
 |  Due Date: 10 December 2019
 |  Last Updated: 9 December 2019 1300
 +-----------------------------------------------------------------------------
 |  Description: This is the function header file for all non LCD or motor
 |               related functions. Detailed function descriptions can be 
 |               found from functions.c file
 +===========================================================================*/

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

/*=============================================================================
 |  Structure Control
 |
 |  Purpose: Main interface for overall control functions, providing clarity
 |           and conciseness to variables and function arguments  
 |
 |  Parameters: 
 |      ir_left (unsigned int) - Value of left IR intensity
 |      ir_right (unsigned int) - Value of right IR intensity
 |      ir_diff (unsigned int) - Value of absolute difference between
 |                               left and right IRs
 |      turn_direction (unsigned char) - 0 (left), 1 (right)
 |      ir_threshold (unsigned int) - Value of IR difference within which
 |                                    vehicle can be considered to be centred.
 |                                    To be calibrated via experiments
 |      ir_min (unsigned int) - Minimum value of IR intensity to be considered 
 |                              as a frontal source. To be calibrated via 
 |                              experiments
 |      ir_buf[16] - Buffer used to display IR readings
 |      i (unsigned char) - Arbitrary counter for use in control 
 |      finding_direction (unsigned char) - Flag for control within while 
 |                                          RFID not found loop
 |      steer_action[220] (unsigned char) - Buffer to store steering actions
+============================================================================*/

struct Control { 
    unsigned int ir_left;
    unsigned int ir_right;
    unsigned int ir_diff;
    unsigned char turn_direction;
    unsigned int ir_threshold;
    unsigned char gain;
    unsigned int time_to_turn;
    unsigned int ir_min;
    unsigned char ir_buf[16];
    unsigned char i;
    unsigned char finding_direction;
    unsigned char steer_action[220];
};

void init_serial(void);
void delay_s(char seconds);
char get_char_serial(void);
void get_packet_serial(char *packet);
void init_interrupt(void);
void init_ir(void);
unsigned int get_ir(unsigned char sensor);
unsigned int ir_filter(unsigned char sensor);
void itoa_5(unsigned int number, unsigned char* buffer);
int power(int number, int power);
void init_counter(void);
void init_button(void);
void ir_difference(struct Control *cont, struct DC_motor *motorL, struct DC_motor *motorR);
void ir_display(struct Control *cont);
void orientate(struct Control *cont, struct DC_motor *motorL, struct DC_motor *motorR);
void steer(struct Control *cont, struct DC_motor *motorL, struct DC_motor *motorR);

#endif

