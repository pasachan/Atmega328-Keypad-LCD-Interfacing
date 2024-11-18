#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>  // For strcmp

// LCD Control Pins
#define ctrl PORTD
#define en 2
#define rw 1
#define rs 0

// Keypad Pins
#define keypadDDR_R DDRB
#define keypadPORT_R PORTB
#define keypadPIN_R PINB

#define keypadDDR_C DDRB
#define keypadPORT_C PORTB
#define keypadPIN_C PINB

// LCD Functions
void lcd_command(unsigned char cmd);
void lcd_init(void);
void lcd_data(unsigned char data);
void lcd_print(const char *str);
void lcd_gotoxy(unsigned char x, unsigned char y);
char keypadScan(void);

// Predefined Messages
const char keys[] PROGMEM = "123A456B789C*0#D";

// Gate Opening Codes
const char fullGateCode[] = "1234";  // Full gate opening code
const char halfGateCode[] = "4321";  // Half gate opening code

int main() {
    DDRD = 0xFF;  // Set PORTD as output for LCD
    lcd_init();
    _delay_ms(30);

    lcd_gotoxy(1, 1);
    lcd_print(PSTR("Enter Code:"));

    // Keypad Initialization
    keypadDDR_R = 0x0F;  // Rows (PB0-PB3) as output
    keypadPORT_R = 0x00; // Set rows low initially
    keypadPORT_C = 0xFF; // Enable pull-up resistors on columns (PB4-PB7)

    // Set PC0 as output for controlling the signal
    DDRC |= (1 << PC0); // Set PC0 as output
    PORTC &= ~(1 << PC0); // Ensure PC0 starts low (OFF)

    uint8_t position = 1; // To keep track of where to print the next character on the second row
    char inputCode[5] = {0}; // To store the input code (max 4 characters + null terminator)
    uint8_t codeIndex = 0; // To keep track of the current position in the entered code

    while (1) {
        char key = keypadScan();
        if (key != '\0') {
            if (key == 'A' && position > 1) { // Backspace (A) - Remove last character
                position--;
                inputCode[position - 1] = '\0'; // Clear the last character
                lcd_gotoxy(position, 2); // Move the cursor back
                lcd_data(' '); // Clear the last character on the LCD
                lcd_gotoxy(position, 2); // Move the cursor back again
            } else if (key == 'C') { // Clear (C) - Clear all characters
                position = 1; // Reset position
                for (uint8_t i = 0; i < 5; i++) {
                    inputCode[i] = '\0'; // Clear the buffer
                }
                lcd_gotoxy(1, 2);
                lcd_print("    "); // Clear the second row
            } else if (key == 'D') { // Submit Code (D)
                inputCode[position - 1] = '\0'; // Null-terminate the input string
                if (strcmp(inputCode, fullGateCode) == 0) { // Full gate opening code
                    lcd_gotoxy(1, 2);
                    lcd_print("Code Accepted");
                    _delay_ms(10); // Shorten delay
                    lcd_gotoxy(1, 2);
                    lcd_print("                "); // Clear the row after showing the message
                    position = 1; // Reset position for re-entering code

                    // Turn PC0 ON when "1234" is entered
                    PORTC |= (1 << PC0); // Set PC0 high (turn ON)
                } else if (strcmp(inputCode, halfGateCode) == 0) { // Half gate opening code
                    lcd_gotoxy(1, 2);
                    lcd_print("Code Accepted");
                    _delay_ms(10); // Short delay before clearing
                    lcd_gotoxy(1, 2);
                    lcd_print("                "); // Clear the row after showing the message
                    position = 1; // Reset position for re-entering code

                    // Turn PC0 OFF when "4321" is entered
                    PORTC &= ~(1 << PC0); // Set PC0 low (turn OFF)
                } else { // Incorrect code
                    lcd_gotoxy(1, 2);
                    lcd_print("Incorrect Code");
                    _delay_ms(10); // Short delay before clearing
                    lcd_gotoxy(1, 2);
                    lcd_print("                "); // Immediately clear the entire row
                    position = 1; // Reset position for re-entering code
                }
                for (uint8_t i = 0; i < 5; i++) {
                    inputCode[i] = '\0'; // Clear the buffer after submission
                }
            } else if (position <= 4) { // Enter a new character
                inputCode[position - 1] = key; // Store the character in the buffer
                lcd_gotoxy(position++, 2); // Move to the next position on the second row
                lcd_data(key); // Display the character
            }
            _delay_ms(10);
        }
    }

    return 0;
}

// Scan the keypad and return the character pressed
char keypadScan() {
    for (uint8_t row = 0; row < 4; row++) {
        keypadPORT_R = ~(1 << row); // Set current row low
        _delay_us(5); // Small delay for stability

        // Check columns PB4 to PB7 (Column 1 is PB4)
        for (uint8_t col = 4; col < 8; col++) { // Columns 4 to 7 (PB4 to PB7)
            if (!(keypadPIN_C & (1 << col))) { // Check if a column is low (pressed)
                uint8_t keyCode = (row * 4) + (col - 4); // Adjust for columns 4 to 7
                return pgm_read_byte(&keys[keyCode]); // Return the corresponding key
            }
        }
    }
    return '\0'; // Return null if no key is pressed
}

// LCD Functions
void lcd_gotoxy(unsigned char x, unsigned char y) {
    unsigned char firstCharAdr[] = {0x80, 0xC0, 0x94, 0xD4};
    lcd_command(firstCharAdr[y - 1] + x - 1);
    _delay_ms(1);
}

void lcd_init(void) {
    lcd_command(0x02); // Initialize LCD in 4-bit mode
    _delay_ms(1);
    lcd_command(0x28); // 2 lines, 5x7 dots
    _delay_ms(1);
    lcd_command(0x0E); // Display on, cursor on
    _delay_ms(1);
    lcd_command(0x01); // Clear display
    _delay_ms(2);
}

void lcd_command(unsigned char cmd) {
    ctrl = (cmd & 0xF0);
    ctrl &= ~(1 << rs);
    ctrl &= ~(1 << rw);
    ctrl |= (1 << en);
    _delay_us(1);
    ctrl &= ~(1 << en);

    ctrl = (cmd << 4);
    ctrl &= ~(1 << rs);
    ctrl &= ~(1 << rw);
    ctrl |= (1 << en);
    _delay_us(1);
    ctrl &= ~(1 << en);
    _delay_ms(2);
}

void lcd_data(unsigned char data) {
    ctrl = (data & 0xF0);
    ctrl |= (1 << rs);
    ctrl &= ~(1 << rw);
    ctrl |= (1 << en);
    _delay_us(1);
    ctrl &= ~(1 << en);

    ctrl = (data << 4);
    ctrl |= (1 << rs);
    ctrl &= ~(1 << rw);
    ctrl |= (1 << en);
    _delay_us(1);
    ctrl &= ~(1 << en);
    _delay_ms(2);
}

void lcd_print(const char *str) {
    char c;
    while ((c = pgm_read_byte(str++))) {
        lcd_data(c);
    }
}
