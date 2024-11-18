# Requirements 
-> Atmega328P
-> 16x2 LCD Display
-> 4x4 Arduino Keypad 
# Notes
* in the actual ciruit different keypad is used hence the characters mapped for the simulation are wrong, you can however adjust that in the main.c file to change it according to your need
* if the code entered by the user is correct (1234) an LED turns on pin PC0, if the same code is reversed it will turn off the LED
* Button R1C4 is used for backspace, Button R3C4 clears the input code and Button R4C4 is used to submit the code 
