/* ============================================================================
 |  Assignment: Explosive Ordinance Disposal
 |
 |  Author: Lim Siew Han & Justin Kek
 |  Language: C
 |  IDE: MPLAB X IDE v4.20
 |  Compiler: XC8 (v2.00)
 |  Compiler Settings: Optimisation Level 2
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
 |  Description: The given objective was to design code to drive a vehicle 
 |               from a given start position towards a beacon which emits  
 |               50 ms infrared (IR) pulses every 250 ms. The vehicle should 
 |               then collect information from a radio-frequency identification
 |               (RFID) card and display it upon returning to its initial 
 |               position. 
 |  
 |               The vehicle consists of two IR sensors, one RFID sensor, and
 |               a liquid crystal display (LCD) screen. The IR sensors are 
 |               front facing and positioned side by side in the middle. Each 
 |               IR sensor is isolated from the other by a cardboard tube with 
 |               an inner lining of aluminium foil. The RFID sensor is wired
 |               below the IR sensors and suspended by a cantilever arm to
 |               match the length of the isolation tubes. 
 |               The IR and RFID sensors are held together by a 3D printed 
 |               housing made from polylactide (PLA).
 | 
 |  Input: IR sensor signal, RFID signal
 |  Output: RFID information on LCD screen
 | 
 |  Main File: main.c
 |  Source Files: functions.c, dc_motor_funct.c, lcd_funct.c
 |  
 | 
 | 
 |  Required Features Not Included: The program adheres to all requirements 
 |  
 |  Known Bugs: 
 |  1. After fresh run, upon entering standby mode, motors run at full speed
 |     - Attempted solutions: Using stop_all() in standby loop
 | 
 |  Improvements for future work:
 |  1. When RFID is retrieved, steering may not stop immediately as it depends
 |     on when the interrupt stops the main program (i.e., if interrupt 
 |     triggers within __delay_ms(), delay continues running before exiting 
 |     loop. 
 |     - Solution: Write new function delay_ms() which incorporates a 
 |                 conditional while loop to delay only when RFID is not 
 |                 not retrieved. For example:
 |      delay_ms(&G_rfid_retrieved, &G_time_ms, 400);
 |      void delay_ms(unsigned char *flag, unsigned int *time_ms, 
 |                    unsigned int delay) 
 |      {
 |          *time_ms = 0; // reset timer
 |          while ((*flag == 0) | (*time_ms < delay));
 |      }
 +===========================================================================*/

// LIBRARIES AND HEADER FILES
#include <xc.h>
#include <stdio.h>
#include "dc_motor_struct.h"
#include "functions.h"
#include "lcd_funct.h"

// PIC18F4331 CONFIGURATIONS
#pragma config OSC = IRCIO, WDTEN = OFF // internal oscillator 
#define _XTAL_FREQ 8000000 //i.e. for an 8MHz clock frequency
// near the top of your main.c file so the functions can calculate how
// long to make the delay for different clock frequencies.

// -------------------- GLOBAL VARIABLES --------------------
// General
volatile unsigned char G_rfid_buf[16]; // for storing and displaying RFID
volatile unsigned int G_time_ms; // counting time using T0CON (up to 65s)
// Flags
volatile unsigned char G_rfid_retrieved; // flag: 1 = RFID retrieved
volatile unsigned char G_run; // flag: 1 = run program, 0 = standby program

// -------------------- INTERRUPTS --------------------
// HIGH PRIORITY INTERRUPT

void __interrupt(high_priority) hp_rfid(void) {
    // Trigger: First full receive register received (0x02)
    // Runs get_packet_serial() until 0x03 received
    // Sets flag to indicate that RFID has been retrieved
    if (PIR1bits.RCIF) {
        get_packet_serial(G_rfid_buf); // stores serial bytes in packet form
        G_rfid_retrieved = 1;
    }

    // Trigger: Button RC3 pressed 
    // Toggles between STANDBY (S) and RUN (R) mode
    // Three if statements for debouncing mechanical switch
    if (INTCONbits.INT0IF) {
        if (INTCONbits.INT0IF) {
            if (INTCONbits.INT0IF) {
                if (G_run == 1) {
                    // Was in RUN before, enter STANDBY
                    G_run = 0;
                    clear_lcd();
                    set_line(1);
                    lcd_string(">S");
                    __delay_ms(700);
                } else if (G_run == 0) {
                    // Was in STANDBY before, enter RUN
                    G_run = 1;
                    clear_lcd();
                    set_line(1);
                    lcd_string(">R");
                    __delay_ms(700);
                }
                // toggles G_run
                INTCONbits.INT0IF = 0; //clear interrupt flag to exit
            }
        }
    }
}

// LOW PRIORITY INTERRUPT

void __interrupt(low_priority) lp_timer(void) {
    if (INTCONbits.TMR0IF) {
        G_time_ms++; // increments when TIMER0 overflows
        TMR0L = 3; // TIMER0 counts from 3 to 255 for 1 ms duration
        INTCONbits.TMR0IF = 0; // clears TIMER0 interrupt flag
    }
}

// -------------------- MAIN PROGRAM --------------------

void main(void) {
    // Initialise PIC18F4331 oscillator
    OSCCON = 0b11110010; // internal oscillator, 8MHz
    while (!OSCCONbits.IOFS); //Wait for OSC to become stable

    // Initisalise functions
    // See function comments for more details 
    init_button();
    init_lcd();
    init_serial();
    init_interrupt();
    init_ir();
    init_pwm(100);
    init_counter();
    clear_lcd();

    // Assigning global variables
    G_run = 0; // enter standby

    // Assigning local variables (NIL)

    // Initialise structures
    // See header comments for more details 
    struct DC_motor motorL, motorR; // 
    struct Control cont;

    motorL.power = 0; // Zero power to start
    motorL.direction = 0; // Set default motor direction (0: forward)
    // Store address of PWM duty low byte
    motorL.dutyLowByte = (unsigned char *) (&PDC0L);
    motorL.dutyHighByte = (unsigned char *) (&PDC0H);
    motorL.dir_pin = 0; // Pin RB0/PWM0 controls direction
    motorL.PWMperiod = 199; //store PWMperiod for motor

    motorR.power = 0;
    motorR.direction = 0;
    motorR.dutyLowByte = (unsigned char *) (&PDC1L);
    motorR.dutyHighByte = (unsigned char *) (&PDC1H);
    motorR.dir_pin = 2;
    motorR.PWMperiod = 199;

    cont.ir_left = 0;
    cont.ir_right = 0;
    cont.ir_diff = 0;
    cont.turn_direction = 0; // 0: left, 1: right

    // Constants for calibration
    cont.gain = 40; // higher gain, lower time to turn
    cont.time_to_turn = 0; // how much vehicle turns
    cont.ir_threshold = 350; // Lower threshold, more centred 
    cont.ir_min = 47000; // Ensures vehicle is not facing backwards
    cont.ir_buf[6];
    cont.i = 0;
    cont.finding_direction = 0;
    cont.steer_action[220]; // Stores up to 219 actions, last element for exit

    // Initialise checksum buffer
    unsigned char chksm[5]; // Stores 2 8 bit characters for comparison
    unsigned char j = 0; // Counter for checksum

    // Inform user that main has been initialised
    clear_lcd();
    set_line(1);
    lcd_string("INIT");
    __delay_ms(500);

    /* ------------------------------------------------------------------------
     * WHILE: STANDBY PROGRAM
     * 
     * Purpose: 
     * - Clear RFID buffer
     * - Clear steer action buffer
     * - Stop all motors (buggy after first run, power cycling required)
     * - Display instantaneous/filtered IR values for mechanical calibration
     * Set flags: 
     * - No RFID
     * - Finding direction
     * 
     * Exit condition: Button pressed for entering RUN mode
     * ------------------------------------------------------------------------
     */
    while (G_run == 0) {

        G_rfid_retrieved = 0; // No RFID
        cont.finding_direction = 1; // Robot not centred, finding direction
        G_time_ms = 0; // time reset

        // Clear RFID buffer to null bytes
        for (cont.i = 0; cont.i < 16; cont.i++) {
            G_rfid_buf[cont.i] = 0;
            cont.i++;
        }

        stop_all(&motorL, &motorR); // Stop all motors

        // Inform user that program is in STANDBY mode
        clear_lcd();
        set_line(1);
        lcd_string("S");
        __delay_ms(500);

        // Clear steer action buffer
        for (cont.i = 0; cont.i < 220; cont.i++) {
            cont.steer_action[cont.i] = 0;
            cont.i++;
        }

        // Display IR values for calibration
        // Exit condition prevents continuous looping of entire standby program
        while (G_run == 0) {
            cont.ir_left = 0;
            cont.ir_right = 0;

            // Retrieve instantaneous IR values
            cont.ir_left = get_ir(1);
            cont.ir_right = get_ir(0);

            // Retrieve filtered IR values
            // cont.ir_left = ir_filter(1);
            // cont.ir_right = ir_filter(0);

            set_line(2);
            ir_display(&cont);
            __delay_ms(200);
        }

        // Exit condition prevents continuous looping of entire standby program
        while (G_run == 0);
    } // end STANDBY



    /* ------------------------------------------------------------------------
     * WHILE: RUN PROGRAM
     * Purpose: 
     * - Assignment of variables for operation
     *      (RFID not retrieved, finding direction)
     * - Orientate and steer towards target
     * - Obtain RFID and return to starting position
     * Modes:
     * 1. WHILE: RFID not retrieved
     * a) Initial orientation (WHILE: finding_direction = 1)
     * - Moves forward for 5s when centred
     * b) Steering (WHILE: finding_direction = 0)
     * - Up to 219 steering actions, 400ms each
     * 
     * 2. IF: RFID retrieved
     * a) Return sequence
     * - Performs steering actions in reverse
     * - Moves in reverse for 5s
     * b) Display RFID
     * - Remove LF and CR, validate checksum
     * ------------------------------------------------------------------------
     */
    while (G_run == 1) {
        // Inform user that program is in RUN mode
        clear_lcd();
        set_line(1);
        lcd_string("R");
        __delay_ms(500);

        // Resetting global variables
        G_rfid_retrieved = 0; // RFID not retrieved
        cont.finding_direction = 1; // Robot not oriented, finding direction
        G_time_ms = 0; // time reset

        // Resetting local variables (NIL)

        // Resetting structure objects 
        cont.i = 0;
        /*---------------------------------------------------------------------
         * 1. WHILE: NO RFID
         +-------------------------------------------------------------------*/


        while ((G_rfid_retrieved == 0) & (G_run == 1)) {
            // Inform user that program is in NO RFID mode
            clear_lcd();
            set_line(1);
            lcd_string("1");
            delay_s(1);

            /*
             * INITIAL ORIENTATION TO TARGET
             * Purpose:
             * - Obtain and process filtered IR values
             * - Orientate vehicle to target
             * - Move vehicle towards target for 5s
             * Note: 
             * - Reducing time delays in orientate() increases response accuracy 
             *   but decreases response time
             */
            while ((cont.finding_direction == 1) & (G_run == 1)) {
                // Retrieve filtered IR values
                cont.ir_left = ir_filter(1);
                cont.ir_right = ir_filter(0);
                // Display IR values
                clear_lcd();
                set_line(1);
                ir_display(&cont);

                // Inform user that program is in INITIAL ORIENTATION mode
                set_line(2);
                lcd_string("1a|");

                // Process IR values (see function for more details)
                ir_difference(&cont, &motorL, &motorR);

                // Orientate or move vehicle (see function for more details)
                orientate(&cont, &motorL, &motorR);
                __delay_ms(100);
            } // end finding direction

            /*
             * STEERING TOWARDS TARGET
             * Purpose:
             * - Obtain and process instantaneous IR values
             * - Correct approach of vehicle towards target by steering
             * - Store steering actions in buffer for return sequence
             * Note:
             * - Decreasing time of each steering action increases response
             *   speed but increase data memory usage
             */
            while ((cont.finding_direction == 0) & (G_rfid_retrieved == 0) & (G_run == 1)) {
                // Retrieve instantaneous IR values
                // IR values will not go to 0 anymore if no signal detected;
                cont.ir_left = get_ir(0);
                cont.ir_right = get_ir(1);
                // Display IR values
                clear_lcd();
                set_line(1);
                ir_display(&cont);

                // Inform user that program is in STEERING mode
                set_line(2);
                lcd_string("1b|");

                // Process IR values (see function for more details)
                ir_difference(&cont, &motorL, &motorR);

                // Display current steering action
                itoa_5(cont.i, cont.ir_buf);
                lcd_string(cont.ir_buf);

                // Steer vehicle towards target (see function for more details)
                steer(&cont, &motorL, &motorR);
                __delay_ms(400); // Keep each steering action at 400 ms

                cont.i++; // Increase counter for storing next steering action
            } // end steering
        } // end RFID not retrieved

        /*
         * IF: RFID COLLECTED
         * Purpose:
         * - Stop the vehicle
         * - Perform return sequence based on recorded steering actions
         * - Perform full speed reverse in response to full speed ahead after
         *   initial orientation
         * - Remove line feed, carriage return and validate checksum
         * - Display RFID
         */
        if ((G_rfid_retrieved == 1) & (G_run == 1)) {
            // Stop vehicle
            stop_all(&motorL, &motorR);
            __delay_ms(500); // prevent abrupt return

            // Inform user that program is in RFID RETRIEVED mode
            clear_lcd();
            set_line(1);
            lcd_string("2");

            // Initiate return sequence in opposite direction
            while ((cont.i >= 0) & (G_run == 1) & (cont.i < 255)) {
                // Inform user that program is in RETURNING mode
                clear_lcd();
                set_line(1);
                lcd_string("2a");

                // Display current steering action
                set_line(2);
                itoa_5(cont.i, cont.ir_buf);
                lcd_string(cont.ir_buf);

                // Steer based on action stored in steer action buffer
                if (cont.steer_action[cont.i] == 1) {
                    // Veer left in opposite direction (left side still slower)
                    lcd_string(":VL");
                    veer_left(&motorL, &motorR, 1);
                } else if (cont.steer_action[cont.i] == 2) {
                    // Veer right in opposite direction (right side still slower)
                    lcd_string(":VR");
                    veer_right(&motorL, &motorR, 1);
                } else if (cont.steer_action[cont.i] == 3) {
                    // go back
                    lcd_string(":MC");
                    full_speed(&motorL, &motorR, 1);
                }
                __delay_ms(400); // Keep each steering action at 400 ms
                cont.i--;
            }

            // Initial full speed reverse for 5s
            stop_all(&motorL, &motorR);
            clear_lcd();
            full_speed(&motorL, &motorR, 1);
            delay_s(5);
            stop_all(&motorL, &motorR);

            // Display RFID
            clear_lcd();
            set_line(1);
            lcd_string("2b");
            delay_s(1);

            // Remove line feed and carriage return from RFID
            for (cont.i = 0; cont.i < 14; cont.i++) {
                if (G_rfid_buf[cont.i] == 10 | G_rfid_buf[cont.i] == 13) {
                    // ASCII Dec Value: LF (10), CR (13)
                    G_rfid_buf[cont.i] = 0; // set to null byte
                }
            }

            // OR the even and odd bits of the 10 DATA bytes together
            chksm[0] = (G_rfid_buf[0]<<4) | G_rfid_buf[1];
            chksm[1] = (G_rfid_buf[2]<<4) | G_rfid_buf[3];
            chksm[2] = (G_rfid_buf[4] <<4) | G_rfid_buf[5];
            chksm[3] = (G_rfid_buf[6]<<4) | G_rfid_buf[7];
            chksm[4] = (G_rfid_buf[8]<<4) | G_rfid_buf[9];
            
            // Checking the checksum by XORing the valus in chksm array together
            j = 1;
            while (j < 5) {
                chksm[0] = chksm[0] ^ chksm[j];
                j++;
            }
            
            
            // If the XORed result is equal to checksum, display valid
            // Else display invalid
            if (chksm[0] == (G_rfid_buf[10] | G_rfid_buf[11])) {
                // Display Checksum
                set_line(1);
                lcd_string("CHECKSUM VALID");

                // Remove checksum
                G_rfid_buf[10] = 0;
                G_rfid_buf[11] = 0;

                // Display RFID
                set_line(2);
                lcd_string(G_rfid_buf);

                while (G_run == 1); // Pauses the program until the button is pressed
            } else {
                // Checksum Invalid
                set_line(1);
                lcd_string("CHECKSUM INVALID");
            }



        } // end RFID collected
    } // end RUN 

} // end main
