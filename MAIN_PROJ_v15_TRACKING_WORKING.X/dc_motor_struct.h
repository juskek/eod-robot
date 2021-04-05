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
 |  Description: This is the function header file for all motor related 
 |               functions. The main interface for control is via the motor 
 |               structure DC_motor. Detailed information on functions can be
 |               found on the dc_motor_func.c file
 |
 +===========================================================================*/

#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H

/*=============================================================================
 |  Structure DC_motor
 |
 |  Purpose: Main interface for PWM control functions, intended for a DC motor
 |           This structure was adapted from the ECM Lab notes chapter 5.  
 |
 |  Parameters: 
 |      power (char) - The motor power, which accepts values from 0 - 100
 |      direction (char) - A flag where 1 indicates forward and 0 backwards
 |      *dutyLowByte (unsigned char) - Address of the PWM duty low byte
 |      *dutyHighByte (unsigned char) - Address of the PWM duty high byte
 |      dir_pin (char) - A flag that indicates the PORTB direction
 |      PWMperiod (int) - The base period of the PWM cycle
 |
 |  Comments: The direction and dir_pin are flags, and could be lumped into a 
 |            single variable and bitwise operations used to determine status,
 |            for greater memory efficiency. Future revisions could 
 |            alternatively consider encapsulating direction as the sign of 
 |            the power variable.
 |
+============================================================================*/

struct DC_motor { 
    char power;         
    char direction;    
    unsigned char *dutyLowByte; 
    unsigned char *dutyHighByte; 
    char dir_pin; 
    int PWMperiod;
};

void init_pwm(int PWMperiod); 
void set_motor_pwm(struct DC_motor *m);
void stop_all(struct DC_motor *mL, struct DC_motor *mR);
void turn_left(struct DC_motor *mL, struct DC_motor *mR);
void turn_right(struct DC_motor *mL, struct DC_motor *mR);
void full_speed(struct DC_motor *mL, struct DC_motor *mR, unsigned char direction);
void veer_left(struct DC_motor *mL, struct DC_motor *mR, unsigned char direction);
void veer_right(struct DC_motor *mL, struct DC_motor *mR, unsigned char direction);

#endif
