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
 |  Description: This is the function file for all non LCD or motor related
 |               functions. The description of the struct Control can be found
 |               on the header file functions.h
 +===========================================================================*/
#include <stdio.h>
#include <xc.h>
#include "dc_motor_struct.h"
#include "lcd_funct.h"
#include "functions.h"
#pragma config OSC = IRCIO // internal oscillator
#define _XTAL_FREQ 8000000

/*=============================================================================
 |  Function power
 |
 |  Purpose: power computes the result of a number to a certain power
 |
 |  Parameters: 
 |      number (int) - The number to be raised to a power
 |      power (int) - The power to which the number is raised
 |
 |  Returns: 
 |      Integer of number^power
 |
 |  Comments: This is to avoid the use of the memory intensive math module,
 |            but can only return values of up to 32767.
 |
+============================================================================*/

int power(int number, int power) {
    unsigned char i = 1;
    int temp = number;
    while (i < power) {
        number = number * temp;
        i++;
    }
    if (power == 0) {
        number = 1;
    }
    return number;
}

/*=============================================================================
 |  Function itoa_5
 |
 |  Purpose: itoa_5 recieves an integer and returns an array containing the
 |           string representation of the integer as per ASCII standards
 |
 |  Parameters: 
 |      number (unsigned int) - The number to be converted to a string
 |      *buffer (unsigned char buffer) - Array to which string is written
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: Implemented to avoid the memory intensive sprintf function, but
 |            only converts integers of up to 5 digits (which should be 
 |            expected).
 |
+============================================================================*/

void itoa_5(unsigned int number, unsigned char* buffer) {
    unsigned char k = 0;
    while (k < 16) {
        buffer[k] = 0;
        k++;
    }
    unsigned int remainder = 0;
    unsigned int quotient = 0;
    unsigned char i = 4;
    unsigned char j = 0;
    while (i > 0) {
        quotient = number / power(10, i);
        remainder = number % power(10, i);
        buffer[j] = (quotient + 48); // Convert the numbers to ASCII chars
        number = remainder;
        i--;
        j++;
    }
    buffer[j] = (number + 48); // Convert the last number to ASCII char
}

/*=============================================================================
 |  Function init_serial
 |
 |  Purpose: init_serial initializes the serial communication register of the
 |           PIC18F4331 microcontroller, with a baud rate of 9600
 |
 |  Parameters: 
 |      Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: Ports are set up according to the schematic in the ECM Lab notes
 |            chapter 7.
 |
+============================================================================*/

void init_serial(void) {
    //set data direction registers for TX, RX
    TRISCbits.RC7 = 1; //RX
    TRISCbits.RC6 = 1; //TX
    //both need to be 1 even though RC6 is an output
    SPBRG = 207; //set baud rate to 9600
    SPBRGH = 0;
    BAUDCONbits.BRG16 = 1; //set baud rate scaling to 16 bit mode
    TXSTAbits.BRGH = 1; //high baud rate select bit
    RCSTAbits.CREN = 1; //continuous receive mode
    RCSTAbits.SPEN = 1; //enable serial port, other settings default
    TXSTAbits.TXEN = 1; //enable transmitter, other settings default
}

/*=============================================================================
 |  Function delay_s
 |
 |  Purpose: delay_s implements a delay in multiples of seconds, using the 
 |           internal __delay_ms function
 |
 |  Parameters: 
 |      seconds (char) - The number of seconds to be delayed
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: With the previous XC8 compiler function, __delay_ms() took in
 |            a max argument of 179200 instruction cycles, i.e. 89.6ms max
 |            delay, which prompted the need for this function to achieve 
 |            higher accurate delays. 
 |            However, the current XC8 compiler (v2)
 |            takes in a max argument of 50463240 instruction cycles, which
 |            is approximately a 25.2s max delay. The need for delay_s outside
 |            of clarity of code is obviated, and subsequent revisions could
 |            consider eliminating this function entirely.
+============================================================================*/

void delay_s(char seconds) {
    
    unsigned int cycles = 20 * seconds; //0.5us
    unsigned int i = 0; // 0.5 us
    for (i = 0; i < cycles; i++) {
        __delay_ms(50);
    }
} 

/*=============================================================================
 |  Function get_char_serial
 |
 |  Purpose: get_char_serial returns a byte from the RCREG serial register
 |
 |  Parameters: 
 |      Nothing (No arguments)
 |
 |  Returns: 
 |      Character from the RCREG serial register
+============================================================================*/

char get_char_serial(void) {
    while (!PIR1bits.RCIF); //wait for the data to arrive
    return RCREG; //return byte in RCREG
}

/*=============================================================================
 |  Function get_packet_serial
 |
 |  Purpose: get_packet_serial sequentially calls the get_char_serial to 
 |           to recieve a string of characters, and checks for the starting
 |           and ending characters 0x02 and 0x03 respectively, if not setting
 |           the packet array to zero (null)
 |
 |  Parameters: 
 |      *packet (char) - Array to which data string will be written to. Must be
 |                       of at least 16 elements
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void get_packet_serial(char *packet) {
    unsigned char i = 0;
    packet[i] = get_char_serial(); // from RX
    // check for start of statement
    if (packet[i] == 0x02) {
        packet[i] = 0; // overwrite 0x02 with current char from RX
        while (i < 16) {
            packet[i] = get_char_serial(); // store bits
            // check for end of statement
            if ((packet[i] == 0x03) & ((i < 15))) {
                packet[i] = 0; // overwrite 0x03 with empty
                i = 16; // exit loop
            } else if ((i == 15) & (packet[i] != 0x03)) {
                // last element and not end of statement
                // invalid, return empty packet
                packet = 0;
            }
            i++; // next bit

        }
    } else {
        packet[i] = 0;
    }
}

/*=============================================================================
 |  Function init_interrupt
 |
 |  Purpose: init_interrupt enables global interupt for both high and low
 |           priorities, enabling the peripheral interrupt function
 |
 |  Parameters: 
 |      Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void init_interrupt() {
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit
    PIE1bits.RCIE = 1; // Receive Flag Interrupt Enable
    INTCONbits.PEIE = 1; // Peripheral Interrupt Enable
    INTCONbits.GIEL = 1; // Global Low priority interrupt enable bit
    RCONbits.IPEN = 1; //Enable priorities

}

/*=============================================================================
 |  Function init_ir
 |
 |  Purpose: init_ir initializes the infared reciever connected to the MFM
 |           (Motion Feedback Module) module of the PIC18F4331. The PWM falling
 |           to rising mode is enabled for the CAP2 and CAP3 pins with a 
 |           prescaler of 1:2
 |
 |  Parameters: 
 |      Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void init_ir(void) {
    ANSEL0 = 0; // Disable analogue input for 0 - 7
    QEICON = 0; // Disable quadrature encoder 
    DFLTCON = 0b00110110; // Enable noise filter for CAP2 and CAP3 with 1:128 clock divider ratio
    TRISAbits.RA3 = 1; // Sets pins RA3/CAP2 and RA4/CAP3 as input
    TRISAbits.RA4 = 1;
    CAP2CONbits.CAP2M = 0b0110; // Enables PWM falling - rising for CAP2
    CAP3CONbits.CAP3M = 0b0110; // Enables PWM falling - rising for CAP3


    T5CONbits.TMR5ON = 1; // Enables the TMR5 module
    T5CONbits.T5PS = 0b01; // Prescaler of 1:2
    T5CONbits.RESEN = 0; // Enabled Special Event Trigger
    T5CONbits.T5MOD = 0; // Continuous Count Mode Enabled
    T5CONbits.T5SEN = 0; // Disable Timer during Sleep
    T5CONbits.TMR5CS = 0; // No Clock Source, use Internal Clock
    PR5H = 0xFF;
    PR5L = 0xFF;
}

/*=============================================================================
 |  Function get_ir
 |
 |  Purpose: get_ir recieves the value from the IR reciever, based on the 
 |           sensor specified
 |
 |  Parameters: 
 |      sensor (unsigned char) - 0 (IR3), 1 (IR2)
 |
 |  Returns: 
 |      Unsigned integer from the CAP2BUF or CAP3BUF, whichever is specified
+============================================================================*/

unsigned int get_ir(unsigned char sensor) {
    // If sensor = 0, for ir3
    // If sensor = 1, for ir2
    unsigned int ir_value = 0;
    if (sensor == 0) {
        ir_value = CAP2BUFL;
        ir_value += ((unsigned int) CAP2BUFH << 8);
        return ir_value;
    } else if (sensor == 1) {
        ir_value = CAP3BUFL;
        ir_value += ((unsigned int) CAP3BUFH << 8);
        return ir_value;
    }
}

/*=============================================================================
 |  Function ir_filter
 |
 |  Purpose: ir_filter takes in a sensor argument that determines the IR
 |           detector, and returns the average of 4 consecutive readings if
 |           at least one value is different.
 |           This prevents the same IR value to be read over and over in the
 |           case of the CAPXBUF not updating (i.e. weak signal) 
 |
 |  Parameters: 
 |      sensor (unsigned char) - 0 (IR3), 1 (IR2)
 |
 |  Returns:
 |      Unsigned integer that is the average of four consecutive values, or 
 |      zero if every value is the same.
 |
 |  Comments: A delay of 250ms between readings is needed to match the 
 |            broadcasting frequency of the IR transmitter
+============================================================================*/

unsigned int ir_filter(unsigned char sensor) {
    unsigned char i = 0;
    unsigned int array[4];
    unsigned int filtered_ir = 0;
    unsigned char same_values_flag = 1; // Assumption of same values de facto on
    while (i < 4) {
        array[i] = get_ir(sensor); // Acquire and store the eight values in a buffer        
        // Checks if the values are the same
        // Requires only one different value to deactivate the same_values flag
        if (i > 0) {
            // Above condition prevents indexing out of bounds
            if (array[i] != array [i - 1]) {
                same_values_flag = 0;
            }
        }
        __delay_ms(250); 
        i++;
    }
    if (same_values_flag == 1) {
        /* If all five values are exactly the same, 
        *sensor is probably not reading anything new 
        *and just using the same stored buffer value */
        
        if (array[i] > 45000) {
            return array[i];
        } else {
            return 0;
        }
    } else {
        // Gets the average of the 8 values and returns it
        i = 0;
        while (i < 4) {
            filtered_ir += (array[i] / 4);
            i++;
        }
    }
    return filtered_ir;

}

/*=============================================================================
 |  Function init_counter
 |
 |  Purpose: init_counter initializes a timer low priority interrupt with a
 |           prescaler value of 1:8 in 8 bit mode, to allow for the interrupt
 |           flag INTCONbits.TMR0IF to be triggered once per milisecond
 |
 |  Parameters: 
 |      Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void init_counter() {
    INTCONbits.TMR0IE = 1; // enable TMR0 overflow interrupt
    INTCON2bits.TMR0IP = 0; // TMR0 low priority
    T0CONbits.T016BIT = 1; // 8 bit mode
    T0CONbits.T0CS = 0; // use internal clock
    T0CONbits.PSA = 0; // enable prescaler
    T0CONbits.T0PS = 0b010; // set prescaler value of 1:8 
    T0CONbits.TMR0ON = 1; //turn on timer0
    TMR0L = 3; // Timer for 1 mili-second#
}

/*=============================================================================
 |  Function init_button
 |
 |  Purpose: init_button initializes the button on pin C3 of the 
 |           microcontroller, enabling external interrupt on button press
 |
 |  Parameters: 
 |      Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void init_button() {
    LATC = 0; // Set the output data latch levels to 0 on all pins
    TRISCbits.RC3 = 1; // Sets pin C3 to input
    INTCONbits.INT0IE = 1; //INT0 External Interrupt Enable bit 
}

/*=============================================================================
 |  Function ir_difference
 |
 |  Purpose: ir_difference computes the absolute difference between the two IR
 |           sensors, and sets the direction on the Control structure based
 |           on which IR sensor is larger
 |
 |  Parameters: 
 |      *cont (struct Control) - Address of the instance of the structure
 |                               Control
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void ir_difference(struct Control *cont, struct DC_motor *motorL, struct DC_motor *motorR) {
    // Get absolute difference between IR sensors
    if (cont->ir_left > cont->ir_right) {
        // IR on left, turn left
        cont->ir_diff = cont->ir_left - cont->ir_right;
        cont->turn_direction = 0;
    } else if (cont->ir_right > cont->ir_left) {
        // IR on right, turn right
        cont->ir_diff = cont->ir_right - cont->ir_left;
        cont->turn_direction = 1;
    }
}

/*=============================================================================
 |  Function ir_display
 |
 |  Purpose: ir_display prints the left and right IR readings on the LCD for 
 |           reference
 |
 |  Parameters: 
 |      *cont (struct Control) - Address of an instance of the Control 
 |                               structure
 |
 |  Returns: Nothing (Void function)
+============================================================================*/

void ir_display(struct Control *cont) {
    // LCD: Line 1
    // LCD: Left IR
    lcd_string("L");
    itoa_5(cont->ir_left, cont ->ir_buf);
    lcd_string(cont->ir_buf);
    // LCD: Separator
    lcd_string("|");

    // LCD: Right IR
    lcd_string("R");
    itoa_5(cont->ir_right, cont->ir_buf);
    lcd_string(cont->ir_buf);
}

/*=============================================================================
 |  Function orientate
 |
 |  Purpose: orientate enables the vehicle to align itself with the target, by
 |           considering four scenarios based on field of view:
 |           1. Target not in FOV of both IR sensors: 
 |           2IRs = 0 (no readings on both IRs), turn left for 200ms
 |           2. Target in FOV of one IR sensors:
 |           1IR = 0 (no readings on one side), turn to side with stronger 
 |           signal for 100ms
 |           3. Target in FOV of both sensors, and centred within threshold
 |           IR difference less than threshold and individual IRs more than 
 |           min IR when facing front (centred), finding_direction = 0,
 |           move forward for 5s
 |           4. Target in FOV of both sensors, but not centred within threshold
 |           Turn by small increments in direction of sensor with larger IR
 |           reading
 |
 |  Parameters: 
 |      *cont (struct Control) - Address of an instance of the Control 
 |                               structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: The use of gain to proportionally turn the vehicle was tested,
 |            but proved to be inconsistent in performance. The fluctuations
 |            in IR readings, despite being passed through ir_filter(), 
 |            produced occasional over-turning. It was found that a fixed 
 |            increment approach was more reliable, and despite a slower
 |            "rise time" to within both sensors' FOV, centred within fewer
 |            iterations of orientate(). 
 |            Scenario 1 suffers from having to turn a complete round if the 
 |            target is on the right side of the vehicle. Subsequent revisions
 |            could consider revising scenario 1, to address this issue.
 | 
+============================================================================*/

void orientate(struct Control *cont, struct DC_motor *motorL, struct DC_motor *motorR) {
    // Compare difference with threshold 
    // Compare individual readings with minimum
    if ((cont ->ir_right == 0) & (cont ->ir_left == 0)) {
        // Both sensors = 0, turn left to check for values
        lcd_string("2 IRs=0");
        turn_left(motorL, motorR);
        delay_s(1); // turn left for 1 s
        stop_all(motorL, motorR);
    } else if ((cont->ir_left == 0) | (cont->ir_right == 0)) {
        lcd_string("1 IR=0");
        // Only one sensor = 0, turn in direction of stronger signal
        if ((cont->turn_direction) == 0) {
            turn_left(motorL, motorR);
            __delay_ms(60); // turn for 60 ms
            stop_all(motorL, motorR);
        } else {
            turn_right(motorL, motorR);
            __delay_ms(60); // turn for 60 ms
            stop_all(motorL, motorR);
        }
    } else if (((cont->ir_diff) < (cont->ir_threshold)) & ((cont->ir_left) > (cont->ir_min)) & ((cont->ir_right) > (cont->ir_min))) {
        // Robot centred, stop finding direction and start moving
        // Display centred readings for calibration
        set_line(1);
        ir_display(cont);
        full_speed(motorL, motorR, 0);
        set_line(2);
        lcd_string("CTR");
        delay_s(5); // Move vehicle forward for 5s
        stop_all(motorL, motorR);
        cont->finding_direction = 0; // Leave finding direction loop
    } else {
        // Robot not centred, but close to target
        // Turn by small increments
        if (cont->turn_direction == 0) {
            lcd_string("L");
            // Turn left
            turn_left(motorL, motorR);
            __delay_ms(25);
            stop_all(motorL, motorR);
        }
        if (cont->turn_direction == 1) {
            lcd_string("R");
            // Turn right
            turn_right(motorL, motorR);
            __delay_ms(25);
            stop_all(motorL, motorR);
        }
    }
}

/*=============================================================================
 |  Function steer
 |
 |  Purpose: steer enables the vehicle to maintain its course (ahead or in 
 |           reverse), veer left or veer right based on instantaneous IR 
 |           values
 |
 |  Parameters: 
 |      *cont (struct Control) - Address of an instance of the Control 
 |                               structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |      *mL (struct DC_motor) - Address of an instance of the DC_motor 
 |                              structure
 |
 |  Returns: Nothing (Void function)
 |
 |  Comments: This is to avoid storing turning on the spot and encoder data,
 |            which is data intensive. Furthermore, although the veering the 
 |            vehicle causes slip an a slight offset in the return position, 
 |            its speed ensures that the RFID is retrieved as fast as possible. 
 |
+============================================================================*/

void steer(struct Control *cont, struct DC_motor *motorL, struct DC_motor *motorR) {
    if ((cont->turn_direction == 1) & (cont->ir_diff > (cont->ir_threshold))) {
        // Scenario 1
        // Veer left
        cont->steer_action[cont->i] = 1;
        lcd_string(":VL");
        veer_left(motorL, motorR, 0);

    } else if ((cont->turn_direction == 0) & (cont->ir_diff > (cont->ir_threshold))) {
        // Scenario 2
        // Veer right
        cont->steer_action[cont->i] = 2;
        lcd_string(":VR");
        veer_right(motorL, motorR, 0);

    } else {
        // Scenario 0
        // Robot centred, maintain course
        cont->steer_action[cont->i] = 3;
        lcd_string(":MC");
        full_speed(motorL, motorR, 0);

    }
} // end steer