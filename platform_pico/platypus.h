#ifndef PLATYPUS_PICO_H
#define PLATYPUS_PICO_H

#include "../common/types.h"

// pico SDK headers. 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

// pico 2WH / RP2350 pin Layouts
#define PIN_JOYSTICK_UP    2
#define PIN_JOYSTICK_DOWN  3
#define PIN_JOYSTICK_PRESS 4
#define PIN_LCD_DC         8
#define PIN_LCD_RST        9
#define PIN_LCD_BL         10

// pico specific globals
extern spi_inst_t *spi_instance; 

// Screen Prototypes
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_clear(unsigned int color);
void lcd_init(void);
void lcd_draw_string(int x, int y, const char *str, unsigned int color, unsigned int bg);

// HID Prototypes
void send_keypress(char c, KeymapLang lang);
void send_unicode_codepoint(int hid_fd, int codepoint, int target_os, KeymapLang lang); 
void send_raw_hid_keypress(uint8_t keycode); 

// paps parser request to pico GPIO
static inline bool hw_get_pin_value(void *ctx, int pin) {
    return gpio_get(pin) == 0; // Assuming active-low
}

// maps parser request to pico SDK sleep
static inline void hw_sleep_ms(int ms) {
    sleep_ms(ms);
}

// maps parser request to HID implementation
static inline void hw_send_unicode(void *req, int cp, int os_mode, KeymapLang lang) {
    send_unicode_codepoint(0, cp, os_mode, lang); // Pass a dummy 0 internally to keep hid.c happy
}

#endif
