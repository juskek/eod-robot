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
 |  Description: This is the function file for all motor related functions. The
 |               main interface for control is via the motor structure 
 |               DC_motor, where a detailed description of its contents can be    
 |               found in the header file dc_motor_struct.h
 +===========================================================================*/
#include <xc.h>
#include "functions.h"
#include "dc_motor_struct.h"
#define _XTAL_FREQ 8000000 // For an 8MHz clock frequency

/*=============================================================================
 |  Function init_pwm
 |
 |  Purpose: init_pwm initializes the PIC18F4331's PWM module, on the 
 |           assumption that pins B0 and B2 are being used as output for a 
 |           PWM duty cycle   
 |
 |  Parameters: 
 |      PWMperiod (int) - An integer value ranging from 0 - 100
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void init_pwm(int PWMperiod) {
    // Motor PWM frequency 10kHz = 0.1ms period
    // Set direction pins to output
    TRISBbits.RB0 = 0;
    TRISBbits.RB2 = 0;
    // Set direction pins to low for PWMon control (25% = 25%)
    LATBbits.LATB0 = 0;
    LATBbits.LATB2 = 0;

    // PWM settings
    PTCON0 = 0b00000000; // free running mode, 1:1 prescaler = 0.5 us
    PTCON1 = 0b10000000; // enable PWM timer
    PWMCON0 = 0b01101111; // PWM 1 and 3 enabled
    PWMCON1 = 0x00; // special features, all 0 (default)

    /* PTPER max = 199 (12-bit res), set PS so that PTPER is as large 
     * as possible for max resolution
     * Set PWM period to 199 for Tpwm = 0.1 ms */

    PTPERL = 0b11000111; // base PWM period low byte
    PTPERH = 0b0; // base PWM period high byte
}

/*=============================================================================
 |  Function set_motor_pwm
 |
 |  Purpose: set_motor_pwm sets the PWM output from the values in the motor    
 |           structure. This function designed based on the example given in
 |           the ECM Lab notes in chapter 5
 |
 |  Parameters: 
 |      *m (struct DC_motor) - Address of an instance of the DC_motor structure
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void set_motor_pwm(struct DC_motor *m) {
    int PWMduty; //tmp variable to store PWM duty cycle

    //calculate duty cycle (value between 0 and PWMperiod)
    PWMduty = (m->power * m->PWMperiod) / 100;

    if (m->direction) //if forward direction
    {
        //need to invert duty cycle as direction is high (100% power is a duty cycle of 0)
        //set dir_pin high in LATB
        LATB = LATB | (1 << (m->dir_pin));
        PWMduty = m->PWMperiod - PWMduty;
    } else //if reverse direction
    {
        //set dir_pin low in LATB
        LATB = LATB & (~(1 << (m->dir_pin)));
    }

    //write duty cycle value to appropriate registers
    *(m->dutyLowByte) = PWMduty << 2;
    *(m->dutyHighByte) = PWMduty >> 6;
}

/*=============================================================================
 |  Function full_speed
 |
 |  Purpose: full_speed sets the power of two DC_motor structures to 90 (out
 |           of 100), and gradually increases the PWM output to the power value
 |           to avoid slip caused by abrupt speed changes. A direction argument 
 |           allows either forward or reverse movement.
 |
 |  Parameters: 
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      direction (unsigned char) - Value of either 0 or 1, where 0 sets
 |                                  backwards and 1 sets forwards movement
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void full_speed(struct DC_motor *mL, struct DC_motor *mR, unsigned char direction) {
    // 0: forwards, 1: backwards
    mL->direction = direction;
    mR->direction = direction;
    // Equate both speeds to be equal first 
    if (mR->power > mL->power) {
        // was veering left
        mR->power = mL->power;
    } else if (mL->power > mR-> power) {
        //was veering right
        mL->power = mR->power;
    }

    // not to max power to prevent slip
    for (mR->power; (mR->power) < 90; mR->power++) {
        mL->power = mR->power;
        set_motor_pwm(mL);
        set_motor_pwm(mR);
        __delay_ms(2); //delay of 2 ms
    }
}

/*=============================================================================
 |  Function veer_left
 |
 |  Purpose: veer_left increases the power of one DC_motor structure and sets
 |           the PWM output to reflect the change. The direction argument 
 |           allows for both forward and backwards veering
 |
 |  Parameters: 
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      direction (unsigned char) - Value of either 0 or 1, where 0 sets
 |                                  backwards and 1 sets forwards movement
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: Take note that when in reverse (direction = 1), the vehicle will 
 |            veer to the right instead. This is due to the increase in power
 |            acting only on the same motor structure and not the other one.
 |            Future revisions should take care to integrate veer_left and 
 |            veer_right into one function, and account for this issue. 
 |
+============================================================================*/

void veer_left(struct DC_motor *mL, struct DC_motor *mR, unsigned char direction) {
    // 0: forwards, 1: backwards
    mL->direction = direction;
    mR->direction = direction;
    // Equate both speeds to be equal first 
    mL->power = mR->power;

    mL->power = 90;
    mR->power = 90;

    for (mR->power; (mR->power) > (mL->power - 45); mR->power--) {
        //increase motor power until 100
        set_motor_pwm(mL);
        set_motor_pwm(mR);
        __delay_ms(2); //delay of 2 ms
    }
}

/*=============================================================================
 |  Function veer_right
 |
 |  Purpose: veer_right increases the power of one DC_motor structure and sets
 |           the PWM output to reflect the change. The direction argument 
 |           allows for both forward and backwards veering
 |
 |  Parameters: 
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      direction (unsigned char) - Value of either 0 or 1, where 0 sets
 |                                  backwards and 1 sets forwards movement
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: Take note that when in reverse (direction = 1), the vehicle will 
 |            veer to the left instead. This is due to the increase in power
 |            acting only on the same motor structure and not the other one.
 |            Future revisions should take care to integrate veer_left and 
 |            veer_right into one function, and account for this issue. 
 |
+============================================================================*/

void veer_right(struct DC_motor *mL, struct DC_motor *mR, unsigned char direction) {
    // 0: forwards, 1: backwards
    mL->direction = direction;
    mR->direction = direction;
    // Equate both speeds to be equal first 
    mL->power = mR->power;

    mL->power = 90;
    mR->power = 90;

    for (mL->power; (mL->power) > (mR->power - 45); mL->power--) {
        //increase motor power until 100
        set_motor_pwm(mL);
        set_motor_pwm(mR);
        __delay_ms(2); //delay of 2 ms
    }
}

/*=============================================================================
 |  Function stop_all
 |
 |  Purpose: stop_all sets the power of two DC_motor structures to 0 (out
 |           of 100), and gradually decreases the PWM output to the power value
 |           to avoid slip caused by abrupt speed changes.
 |
 |  Parameters: 
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void stop_all(struct DC_motor *mL, struct DC_motor *mR) {
    // Equate both speeds to be equal first 
    mL->power = mR->power;

    for (mR->power; (mR->power) > 0; mR->power--) {
        //decrease motor power until 0
        mL->power = mR->power;
        set_motor_pwm(mL);
        set_motor_pwm(mR);
        __delay_ms(2); //delay of 1 ms
    }
}

/*=============================================================================
 |  Function turn_left
 |
 |  Purpose: turn_left first invokes the stop_all function to set PWM output to
 |           0, then sets the direction for the two DC_motor structures to be 
 |           opposite and power to 70 (out of 100), gradually increasing the
 |           the PWM to reflect that power
 |
 |  Parameters: 
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: Similarly to veer_left and veer_right, the turn_left and 
 |            turn_right functions should be integrated into a single function
 |            in future iterations for memory optimization
 |
+============================================================================*/

void turn_left(struct DC_motor *mL, struct DC_motor *mR) {
    stop_all(mL, mR);
    mL->direction = 0;
    mR->direction = 1;
    for (mL->power; (mL->power) < 70; mL->power++) {
        //increase motorL power until 60
        mR->power = mL->power;
        set_motor_pwm(mL);
        set_motor_pwm(mR);
        __delay_ms(2); //delay of 1 ms
    }
}

/*=============================================================================
 |  Function turn_right
 |
 |  Purpose: turn_right first invokes the stop_all function to set PWM output to
 |           0, then sets the direction for the two DC_motor structures to be 
 |           opposite and power to 70 (out of 100), gradually increasing the
 |           the PWM to reflect that power
 |
 |  Parameters: 
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: Similarly to veer_left and veer_right, the turn_left and 
 |            turn_right functions should be integrated into a single function
 |            in future iterations for memory optimization
 |
+============================================================================*/

void turn_right(struct DC_motor *mL, struct DC_motor *mR) {
    stop_all(mL, mR);
    mL->direction = 1;
    mR->direction = 0;
    for (mL->power; (mL->power) < 70; mL->power++) {
        //increase motorL power until 60
        mR->power = mL->power;
        set_motor_pwm(mL);
        set_motor_pwm(mR);
        __delay_ms(2); //delay of 1 ms
    }

}
