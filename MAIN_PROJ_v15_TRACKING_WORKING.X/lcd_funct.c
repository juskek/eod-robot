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
 |  Description: This is the function file for all LCD related functions. This
 |               assumes that the LCD is connected as per schematic seen in
 |               the ECM Lab notes chapter 4 (which is too involved to be 
 |               described here).
 |               Most functions make use of the __delay_us() function, which 
 |               requires #define _XTAL_FREQ clk to be defined, with clk being
 |               the clock frequency
 +===========================================================================*/
#include <stdio.h>
#include <xc.h>
#include <string.h> 
#include "lcd_funct.h"
#define _XTAL_FREQ 8000000 //i.e. for an 8MHz clock frequency

/*=============================================================================
 |  Function e_tog
 |
 |  Purpose: e_tog accounts for the inherent delay between processing commands
 |           for the LCD module, which is approximately 5us.
 |
 |  Parameters: 
 |     Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void e_tog(void) {
    LATCbits.LATC0 = 1; // Sends high to E
    __delay_us(5); //
    LATCbits.LATC0 = 0; // Sends low to E
}

/*=============================================================================
 |  Function lcd_out
 |
 |  Purpose: lcd_out sends the first four bits of an 8 bit varable to the LCD
 |
 |  Parameters: 
 |     number (unsigned char) - 8bit variable from which the first four bits
 |                              are to be sent
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void lcd_out(unsigned char number) {
    //set data pins using the four bits from number
    //toggle the enable bit to send data
    LATCbits.LATC1 = number & 1; // DB4
    LATCbits.LATC2 = (number & 2) >> 1; // DB5
    LATDbits.LATD0 = (number & 4) >> 2; // DB6
    LATDbits.LATD1 = (number & 8) >> 3; // DB7
    e_tog(); // Toggle enable pin
    __delay_us(5); 

}

/*=============================================================================
 |  Function send_lcd
 |
 |  Purpose: send_lcd sends a whole 8 bit (1 byte) command or data to the 
 |           interface, assuming that 4bit instruction mode has been configured
 |           on the LCD module. The type argument determines the nature of the
 |           8 bit variable
 |
 |  Parameters: 
 |     Byte (unsigned char) - 8bit variable to be sent
 |     type (char) - Command (0) or Data/Char (1)
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void send_lcd(unsigned char Byte, char type) {
    // set RS pin whether it is a Command (0) or Data/Char (1)
    // using type as the argument
    LATAbits.LATA6 = type;
    // send high bits of Byte using LCDout function
    lcd_out(Byte >> 4);
    __delay_us(10); // 10us delay
    // send low bits of Byte using LCDout function
    lcd_out(Byte); 
}

/*=============================================================================
 |  Function clear_lcd
 |
 |  Purpose: clear_lcd sends a command via send_lcd to clear the lcd display
 |           and set the cursor back at the top left
 |
 |  Parameters: 
 |     Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void clear_lcd(void) {
    send_lcd(1, 0); // Display Clear
    __delay_us(5000);
}

/*=============================================================================
 |  Function init_lcd
 |
 |  Purpose: init_lcd sets up the LCD module when interfaced to the PIC18F4331
 |           microcontroller chip with 4 bit instruction mode.
 |
 |  Parameters: 
 |     Nothing (No arguments)
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void init_lcd(void) {
    // set initial LAT output values (they start up in a random state)
    //set the output data latch levels to 0 on all pins
    LATCbits.LATC0 = 0;
    LATCbits.LATC1 = 0;
    LATCbits.LATC2 = 0;
    LATDbits.LATD0 = 0;
    LATDbits.LATD1 = 0;
    LATAbits.LATA6 = 0;
    // set LCD pins as output (TRIS registers)
    //set the data direction registers to output on all pins
    TRISC = 0; 
    TRISD = 0;
    TRISA = 0;
    TRISCbits.RC0 = 0;
    TRISCbits.RC1 = 0;
    TRISCbits.RC2 = 0;
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISAbits.RA6 = 0;
    // Initialisation sequence code - see the data sheet

    __delay_us(15); //delay 15mS
    //send 0b0011 using lcd_out
    lcd_out(3); 
    __delay_us(5); //delay 5ms
    //send 0b0011 using lcd_out
    lcd_out(3); 
    __delay_us(200); //delay 200us
    //send 0b0011 using lcd_out
    lcd_out(3); 
    __delay_us(50); //delay 50us
    //send 0b0010 using lcd_out set to four bit mode
    lcd_out(2); 
    // Function Set with 2 line display, 4 bit interface, 5x10 dots
    send_lcd(40, 0); 
    __delay_us(80);
    // Display Clear
    send_lcd(1, 0); 
    __delay_ms(5);
    // Entry mode set, cursor direction increase, display not shifted
    send_lcd(6, 0); 
    __delay_us(80);
    // Display turned on, cursor on, blinking off
    send_lcd(14, 0); 
    __delay_us(80);
}

/*=============================================================================
 |  Function set_line
 |
 |  Purpose: set_line brings the cursor to the beginning of the first or second
 |           row depending on the input argument
 |
 |  Parameters: 
 |     line (char) - Takes 1 (line 1) or 2 (line 2)
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void set_line(char line) {
    //Send 0x80 to set line to 1 (0x00 ddram address)
    //Send 0xC0 to set line to 2 (0x40 ddram address)
    if (line == 1) {
        send_lcd(128, 0);
    } else if (line == 2) {
        send_lcd(192, 0);
    }
    __delay_us(50); // 50us delay
}

/*=============================================================================
 |  Function lcd_string
 |
 |  Purpose: lcd_string takes in an array of characters and displays them on
 |           on the LCD screen
 |
 |  Parameters: 
 |     *string (unsigned char) - Address of an unsigned character, or an array
 |
 |  Returns: Nothing (Void function)
 |
+============================================================================*/

void lcd_string(unsigned char *string) {
    //While the data pointed to isn?t a 0x00 (null) do below
    while (*string != 0) {
        //Send out the current byte pointed to
        // and increment the pointer
        send_lcd(*string++, 1);
    }
}