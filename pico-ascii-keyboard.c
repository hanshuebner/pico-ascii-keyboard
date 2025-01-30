#include <ctype.h>
#include <stdio.h>
#include <pico/stdio.h>
#include <bsp/board.h>
#include <tusb.h>
#include <hardware/gpio.h>

#include "hid-to-ascii.h"

#define GPIO_FIRST_BIT 12
#define BIT_COUNT 10
#define GPIO_DATA_MASK (0xFF << GPIO_FIRST_BIT)
#define GPIO_STB_MASK (1 << (GPIO_FIRST_BIT + 8))
#define GPIO_AKD_MASK (1 << (GPIO_FIRST_BIT + 9))

#define REPEAT_DELAY 700000
#define REPEAT_INTERVAL 50000
absolute_time_t repeat_key_at = 0;
uint8_t repeat_char = 0;

// Helper to check if a keycode is in the report
bool key_in_report(uint8_t keycode, hid_keyboard_report_t const *report) {
    for (uint8_t i = 0; i < 6; i++) {
        if (report->keycode[i] == keycode) {
            return true;
        }
    }
    return false;
}

uint32_t current_output = 0;

void
send_key(uint8_t c) {
    current_output = (c << GPIO_FIRST_BIT) | GPIO_AKD_MASK | GPIO_STB_MASK;
    gpio_put_all(current_output);
    sleep_us(11);
    current_output &= ~GPIO_STB_MASK;
    gpio_put_all(current_output);
    sleep_ms(1);
}

// Callback for HID keyboard reports
void process_kbd_report(hid_keyboard_report_t const *report) {
    static hid_keyboard_report_t prev_report = { 0 }; // To detect key changes
    static uint8_t pressed_count = 0;

    // Process key release
    for (uint8_t i = 0; i < 6; i++) {
        if (prev_report.keycode[i] && !key_in_report(prev_report.keycode[i], report)) {
            printf("Key 0x%02x released\n", prev_report.keycode[i]);
            const KeyEvent event = convert_hid_to_ascii(prev_report.keycode[i], report->modifier);
            if (event.valid) {
                pressed_count--;
                if (pressed_count == 0) {
                    printf("All keys released\n");
                    current_output &= ~GPIO_AKD_MASK;
                    gpio_put_all(current_output);
                    repeat_key_at = 0;
                }
            }
        }
    }

    // Process key press
    for (uint8_t i = 0; i < 6; i++) {
        if (report->keycode[i] && !key_in_report(report->keycode[i], &prev_report)) {
            const KeyEvent event = convert_hid_to_ascii(report->keycode[i], report->modifier);
            if (!event.valid) {
                printf("Unmapped keycode: 0x%02X\n", report->keycode[i]);
            } else {
                uint8_t c = event.ascii;
                if (event.modifiers & (MODIFIER_LEFT_CTRL | MODIFIER_RIGHT_CTRL)) {
                    c &= 0x1F; // Turn into control character
                }
                printf("Keycode 0x%02x => ASCII: 0x%02x (%c)\n", report->keycode[i], c, isprint(c) ? c : '?');
                send_key(c);
                pressed_count++;
                if (pressed_count == 1) {
                    repeat_key_at = get_absolute_time() + REPEAT_DELAY;
                    repeat_char = c;
                } else {
                    repeat_key_at = 0;
                }
            }
        }
    }

    // Save current report as previous
    prev_report = *report;
}

// TinyUSB callback when a device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
    printf("HID device mounted: Address %d, Instance %d\n", dev_addr, instance);

    // Claim the device if it's a keyboard
    uint8_t itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        if (!tuh_hid_receive_report(dev_addr, instance)) {
            printf("Error: Unable to request report\n");
        }
    }
}

// TinyUSB callback when a device is unmounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID device unmounted: Address %d, Instance %d\n", dev_addr, instance);
}

// TinyUSB callback for received HID reports
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        process_kbd_report((hid_keyboard_report_t const *)report);

        // Request the next report
        if (!tuh_hid_receive_report(dev_addr, instance)) {
            printf("Error: Unable to request next report\n");
        }
    }
}

void init_keyboard_port() {
    for (int i = GPIO_FIRST_BIT; i < GPIO_FIRST_BIT + BIT_COUNT; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_pull_down(i);
    }
}


int main(void) {
    stdio_init_all();

    board_init();
    tuh_init(BOARD_TUH_RHPORT);
    board_init_after_tusb();

    init_keyboard_port();

    printf("pico-ascii-keyboard running\n");

    while (1) {
        // Poll the TinyUSB host stack
        tuh_task();
        if (repeat_key_at > 0 && get_absolute_time() > repeat_key_at) {
            send_key(repeat_char);
            repeat_key_at = get_absolute_time() + REPEAT_INTERVAL;
        }
    }
}
