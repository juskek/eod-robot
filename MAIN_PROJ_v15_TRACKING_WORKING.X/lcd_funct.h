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
 |  Description: This is the function header file for all LCD related functions
 |               Detailed descriptions of functions can be found in the 
 |               lcd_funct.c file
 +===========================================================================*/
#ifndef LCD_FUNCT_H
#define	LCD_FUNCT_H


void e_tog(void); 
void lcd_out(unsigned char number); 
void send_lcd(unsigned char Byte, char type);
void clear_lcd(void);
void init_lcd(void);
void set_line(char line);
void lcd_string(unsigned char *string);

#endif	

