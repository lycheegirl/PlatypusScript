#include <stdio.h>
#include "pico/stdlib.h"
#include "ff_stdio.h"
#include "hw_config.h"
#include "tusb.h"
#include "pico/cyw43_arch.h"

#include "platypus.h" 

int global_cached_os = 2; // default to mac/linux

// helper function to turn basic ASCII chars into USB HID keycodes
// only covers lowercase and basic numbers
uint8_t ascii_to_hid(char c) {
    if (c >= 'a' && c <= 'z') return HID_KEY_A + (c - 'a');
    if (c >= 'A' && c <= 'Z') return HID_KEY_A + (c - 'A'); // Typed as lowercase for now
    if (c >= '1' && c <= '9') return HID_KEY_1 + (c - '1');
    if (c == '0') return HID_KEY_0;
    if (c == '\n' || c == '\r') return HID_KEY_ENTER;
    if (c == ' ') return HID_KEY_SPACE;
    return 0; // unsupported character
}

// Helper to send a clean keystroke down the USB pipe
void send_raw_hid_keypress(uint8_t keycode) {
    if (keycode == 0) return;

    // wait until the USB interface is clear and ready to accept a report
    while (!tud_hid_ready()) {
        tud_task(); // Feed TinyUSB keep-alive ticks while waiting
    }

    // send key press report
    uint8_t keycode_arr[6] = {keycode, 0, 0, 0, 0, 0};
    tud_hid_keyboard_report(0, 0, keycode_arr);
    
    // give host tiny frame window to register down-event
    sleep_ms(10); 
    tud_task();

    // send the Key Release Report
    while (!tud_hid_ready()) {
        tud_task();
    }
    tud_hid_keyboard_report(0, 0, NULL);
    
    // gap delay between distinct characters so things don't ghost blur together
    sleep_ms(15); 
    tud_task();
}

int main() {
    // init Pico 2W wireless/LED architecture
    if (cyw43_arch_init()) {
        printf("Wi-Fi/LED init failed!\n");
    }

    //  init hardware GPIO blocks before applying settings
    for (int pin = 16; pin <= 19; pin++) {
        gpio_init(pin);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_12MA);
    }
    gpio_set_dir(16, GPIO_IN);  // MISO is an input
    gpio_pull_up(16);           // Actively pull up MISO line to fight module noise

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // DEBUG - LED on to check code is running

    stdio_init_all();
    tusb_init(); // init USB hardware driver layer
    
    printf("Waiting for USB host configuration mapping...\n");
    while (!tud_hid_ready()) {
        tud_task(); // Keep feeding TinyUSB ticks so it can finish the handshake
        sleep_ms(1);
    }

    // extra headroom 
    printf("USB Ready! Final window-focus delay...\n");
    for (int i = 0; i < 2000; i++) {
        tud_task();
        sleep_ms(1);
    }

    // Grab ref to sd card (slot 0)
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("Error: Could not find SD card configuration.\n");
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    } else {
        // mount the fs cleanly by explicitly targeting volume "0:"
        FATFS fs;
        FRESULT fr = f_mount(&fs, "0:", 1); 
        
        if (fr == FR_OK) {
            printf("SD Card initialized and mounted successfully!\n");
            
            // open custom target script file using the stdio wrapper
            FILE *fil = fopen("0:payload.plat", "r"); // "0:" targets the mounted volume explicitly
            
            if (fr == FR_OK) {
            printf("SD Card initialized and mounted successfully!\n");
            
            FIL fil;            // Use raw FatFS file handle
            FRESULT res;        
            
            // open custom target script file with raw FatFS READ access
            res = f_open(&fil, "test.plat", FA_READ);
            
            if (res == FR_OK) {
                printf("Successfully opened test.plat! Running parser engine...\n");
                
                char line_buffer[256]; 
                
                ParseContext parser_ctx = {
                    .inside_if = 0,
                    .condition_met = 0,
                    .executing = 1,
                    .loop_start_pos = -1,
                    .for_start_pos = -1,
                    .line_number = 0,
                    .active_lang = LANG_US,
                    .var_count = 0
                };
                
                set_var(&parser_ctx, "_OS", global_cached_os);
                
                // Read lines using raw f_gets
                while (f_gets(line_buffer, sizeof(line_buffer), &fil)) {
                    printf("Parser read raw line: [%s]\n", line_buffer);
                    
                    parse_line(line_buffer, NULL, &parser_ctx, &fil);

                    printf("   [CHECKPOINT A] parse_line execution complete.\n");
                }

                printf("[CHECKPOINT B] Exited read loop. Attempting to close file...\n");
                
                f_close(&fil); // Clean raw close
                printf("[CHECKPOINT C] f_close complete! Pass finished cleanly.\n");
                
            } else {
                printf("Failed to open test.plat. Error: %d\n", res);
                while(true) {
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); sleep_ms(100);
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); sleep_ms(100);
                }
            }
        }
            
        } else {
            // DEBUG STUFF
            // MOUNT FAILED (FR_NOT_READY)
            // scan the physical hardware state of Pin 21 (MISO)
            while (true) {
                // read raw physical logic level of GPIO 16
                bool miso_state = gpio_get(16);
                
                if (miso_state == true) {
                    // FAST PANIC FLICKER: MISO is stuck HIGH (1)
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); sleep_ms(50);
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); sleep_ms(50);
                } else {
                    // SLOW EERIE PULSE: MISO is stuck LOW (0)
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); sleep_ms(1000);
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); sleep_ms(1000);
                }
            }
        }
    }

    // keep tinyUSB stack operating infinitely
    while (true) {
        tud_task(); 
    }
}

