#include "platypus.h"
#include "tusb.h"

// platform agnostic layout mapping for pico and zero
HID_Lookup get_hid_code(char c, KeymapLang lang) {
    HID_Lookup code = {0x00, 0x00};
    
    if (c >= 'a' && c <= 'z')      code.scancode = HID_KEY_A + (c - 'a');
    else if (c >= 'A' && c <= 'Z') { code.modifier = 0x02; code.scancode = HID_KEY_A + (c - 'A'); }
    else if (c >= '1' && c <= '9') code.scancode = HID_KEY_1 + (c - '1');
    else if (c == '0')             code.scancode = HID_KEY_0;
    else if (c == ' ')             code.scancode = HID_KEY_SPACE;
    else if (c == '\n')            code.scancode = HID_KEY_ENTER;
    
    if (lang == LANG_ES) {
        if (c == '.')      code.scancode = HID_KEY_PERIOD;
        else if (c == '-') code.scancode = HID_KEY_MINUS; 
        else if (c == '/') { code.modifier = 0x02; code.scancode = HID_KEY_7; } 
        else if (c == ':') { code.modifier = 0x02; code.scancode = HID_KEY_PERIOD; } 
    } else { 
        if (c == '.')      code.scancode = HID_KEY_PERIOD;
        else if (c == '-') code.scancode = HID_KEY_MINUS;
        else if (c == '/') code.scancode = HID_KEY_SLASH;
        else if (c == ':') { code.modifier = 0x02; code.scancode = HID_KEY_SEMICOLON; }
    }
    return code;
}

// Platform-Agnostic UTF-8 Decoder 
int get_utf8_codepoint(const char *str, int *bytes_consumed) {
    unsigned char b1 = str[0];
    if (b1 < 0x80) { *bytes_consumed = 1; return b1; }
    if ((b1 & 0xE0) == 0xC0) { *bytes_consumed = 2; return ((b1 & 0x1F) << 6) | (str[1] & 0x3F); }
    if ((b1 & 0xF0) == 0xE0) { *bytes_consumed = 3; return ((b1 & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F); }
    *bytes_consumed = 1; return 0;
}

void send_keypress(char c, KeymapLang lang) {
    // DEBUG Print the incoming character
    printf("HID: Received char '%c' (ASCII: %d)\n", c, (int)c);

    HID_Lookup lookup = get_hid_code(c, lang);
    
    // DEBUG  Print the resulting scancode lookup
    printf("HID: Lookup scancode = 0x%02X, modifier = 0x%02X\n", lookup.scancode, lookup.modifier);

    if (lookup.scancode == 0x00) return;

    // Wait until TinyUSB is ready
    while (!tud_hid_ready()) { tud_task(); }

    // Send Key Down (including Shift modifier if needed)
    uint8_t keycode_arr[6] = {lookup.scancode, 0, 0, 0, 0, 0};
    tud_hid_keyboard_report(0, lookup.modifier, keycode_arr);
    sleep_ms(10);
    tud_task();

    // Send Key Release
    while (!tud_hid_ready()) { tud_task(); }
    tud_hid_keyboard_report(0, 0, NULL);
    sleep_ms(15);
    tud_task();
}

// Unicode Injector
void send_unicode_codepoint(int hid_fd, int codepoint, int target_os, KeymapLang lang) {
    // If it's a standard ASCII character, route it straight to the keypress injector
    if (codepoint < 128) { 
        send_keypress((char)codepoint, lang); 
        return; 
    }
    
    char hex[16]; 
    snprintf(hex, sizeof(hex), "%x", codepoint);
    
    // target_os: 1 = Windows, 2 = Mac/Linux default
    if (target_os == 2) { 
        // Linux/Mac Unicode Sequence
        while (!tud_hid_ready()) { tud_task(); }
        uint8_t r[6] = {HID_KEY_U, 0, 0, 0, 0, 0};
        tud_hid_keyboard_report(0, 0x01 | 0x02, r); // Ctrl + Shift
        sleep_ms(10); tud_task();
        
        while (!tud_hid_ready()) { tud_task(); }
        tud_hid_keyboard_report(0, 0, NULL);
        sleep_ms(10); tud_task();
        
        for (int i = 0; hex[i]; i++) send_keypress(hex[i], lang);
        send_keypress('\n', lang);
        
    } else if (target_os == 1) { 
        // Windows Alt-Code Sequence
        while (!tud_hid_ready()) { tud_task(); }
        tud_hid_keyboard_report(0, 0x04, NULL); // Alt Down
        sleep_ms(10); tud_task();
        
        send_raw_hid_keypress(HID_KEY_KEYPAD_ADD);
        for (int i = 0; hex[i]; i++) send_keypress(hex[i], lang);
        
        while (!tud_hid_ready()) { tud_task(); }
        tud_hid_keyboard_report(0, 0, NULL); // Alt Up
        sleep_ms(10); tud_task();
    }
}
