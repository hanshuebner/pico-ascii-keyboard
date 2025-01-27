#include <stdint.h>
#include <stdbool.h>

#include "hid-to-ascii.h"

#include "usb-hid-keys.h"

#define CONTROL_KEYS        \
    [KEY_LEFT] = 0x08,      \
    [KEY_RIGHT] = 0x09,     \
    [KEY_DOWN] = 0x0a,      \
    [KEY_UP] = 0x0b,        \
    [KEY_HOME] = 0x0f,      \
    [KEY_ESC] = 0x1b,       \
    [KEY_BACKSPACE] = 0x08, \
    [KEY_DELETE] = 0x7f,    \
    [KEY_ENTER] = 0x0d,     \


// HID to ASCII mapping for standard keys (non-shifted)
const char HID_to_ASCII[0x100] = {
    0, 0, 0, 0, 'a', 'b', 'c', 'd',            // 0x00 - 0x07
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',    // 0x08 - 0x0F
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't',    // 0x10 - 0x17
    'u', 'v', 'w', 'x', 'y', 'z', '1', '2',    // 0x18 - 0x1F
    '3', '4', '5', '6', '7', '8', '9', '0',    // 0x20 - 0x27
    '\n', 0, '\b', '\t', ' ', '-', '=', '[',   // 0x28 - 0x2F
    ']', '\\', '#', ';', '\'', '`', ',', '.',  // 0x30 - 0x37
    '/',                                       // 0x38
    CONTROL_KEYS
};

// HID to ASCII mapping for shifted keys
const char HID_to_ASCII_Shifted[0x100] = {
    0, 0, 0, 0, 'A', 'B', 'C', 'D',            // 0x00 - 0x07
    'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',    // 0x08 - 0x0F
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',    // 0x10 - 0x17
    'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',    // 0x18 - 0x1F
    '#', '$', '%', '^', '&', '*', '(', ')',    // 0x20 - 0x27
    '\n', 0, '\b', '\t', ' ', '_', '+', '{',   // 0x28 - 0x2F
    '}', '|', '~', ':', '\"', '~', '<', '>',   // 0x30 - 0x37
    '?',                                       // 0x38
    CONTROL_KEYS
};

// Function to convert HID code and modifiers to ASCII
KeyEvent convert_hid_to_ascii(uint8_t hid_code, uint8_t modifiers) {
    KeyEvent event = {false, 0, 0};

    // Ignore codes outside the defined range
    if (hid_code >= sizeof(HID_to_ASCII)) {
        return event;
    }

    // Check if Shift is active
    bool shift_active = (modifiers & (MODIFIER_LEFT_SHIFT | MODIFIER_RIGHT_SHIFT)) != 0;

    // Get the ASCII value based on the shift state
    if (shift_active) {
        event.ascii = HID_to_ASCII_Shifted[hid_code];
    } else {
        event.ascii = HID_to_ASCII[hid_code];
    }

    // If the ASCII code is null, ignore the key
    if (event.ascii == 0) {
        return event;
    }

    // Set the modifiers
    event.modifiers = modifiers;
    event.valid = true;

    return event;
}
