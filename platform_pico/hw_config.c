#include "hw_config.h"

/* Configuration of hardware SPI object */
static spi_t spi = {
    .hw_inst = spi0,  
    .sck_gpio = 18,   // Physical Pin 24
    .mosi_gpio = 19,  // Physical Pin 26
    .miso_gpio = 16,  // Physical Pin 21
    .baud_rate = 2000 * 1000  // 12.5 MHz, change later
};

/* SPI Interface Configuration */
static sd_spi_if_t spi_if = {
    .spi = &spi,  
    .ss_gpio = 17    // Physical Pin 22 
};

/* config of the SD Card socket object */
static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &spi_if       
};

size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) {
        return &sd_card;
    }
    return NULL;
}
