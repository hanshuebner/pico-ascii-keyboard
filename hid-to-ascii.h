#ifndef HID_TO_ASCII_H
#define HID_TO_ASCII_H

typedef struct {
    bool valid;         // True if the key is valid
    char ascii;         // ASCII code
    uint8_t modifiers;  // Bitmask for modifiers (e.g., Shift, Ctrl, Alt)
} KeyEvent;

// Modifier key bitmask definitions
#define MODIFIER_LEFT_CTRL  0x01
#define MODIFIER_LEFT_SHIFT 0x02
#define MODIFIER_LEFT_ALT   0x04
#define MODIFIER_LEFT_GUI   0x08
#define MODIFIER_RIGHT_CTRL 0x10
#define MODIFIER_RIGHT_SHIFT 0x20
#define MODIFIER_RIGHT_ALT  0x40
#define MODIFIER_RIGHT_GUI  0x80

KeyEvent convert_hid_to_ascii(uint8_t hid_code, uint8_t modifiers);

#endif