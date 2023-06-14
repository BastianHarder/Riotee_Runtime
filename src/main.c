#include "nrf.h"
#include "nrf_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "riotee_timing.h"
#include "riotee_gpint.h"
#include "printf.h"
#include "riotee.h"
#include "runtime.h"
#include "riotee_thresholds.h"
#include "riotee_uart.h"
#include "riotee_ble.h"
#include "riotee_adc.h"
#include "max2769.h"
#include "riotee_spic.h"
#include "riotee_spis.h"

riotee_ble_ll_addr_t adv_address = {.addr_bytes = {0xBE, 0xEF, 0xDE, 0xAD, 0x00, 0x01}};

static struct {
  uint32_t counter;
} ble_data;

static void led_blink(unsigned int us) {
  taskENTER_CRITICAL();
  nrf_gpio_pin_set(PIN_LED_CTRL);
  riotee_delay_us(us);
  nrf_gpio_pin_clear(PIN_LED_CTRL);
  taskEXIT_CRITICAL();
}

/* This gets called one time after flashing new firmware */
void bootstrap_callback(void) {
  printf("All new!");
}

/* This gets called after every reset */
void reset_callback(void) {
  nrf_gpio_cfg_output(PIN_LED_CTRL);

  riotee_ble_init();
  riotee_ble_prepare_adv(&adv_address, "RIOTEE", 6, sizeof(ble_data));
  ble_data.counter = 0;

  //Functions for batteryfree-gps from here on
  //Initialize spi master 
  riotee_spic_cfg_t spic_config;
  spic_config.mode = SPIC_MODE0_CPOL0_CPHA0;
  spic_config.frequency = SPIC_FREQUENCY_K500;
  spic_init(&spic_config);
  //Initialize for max2769 usage
  max2769_init();
  //Initialize spi slave for receiving serial data from max2769
  riotee_spis_cfg_t spis_config;
  spis_config.mode = SPIC_MODE1_CPOL0_CPHA1;    //Configure the SPI mode to Mode 1 and MSB First
  spis_config.pin_cs_out = PIN_D4;
  spis_config.pin_cs_in = PIN_D5;
  spis_config.pin_sck = PIN_D2;
  spis_config.pin_mosi = PIN_D3;
  spis_init(&spis_config);
}

void user_task(void *pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  for (;;) {
    wait_until_charged();
    led_blink(250);
    riotee_sleep_ms(500);
    riotee_ble_advertise(&ble_data, ADV_CH_ALL);
    ble_data.counter++;
    //Functions for batteryfree-gps from here on
    enable_max2769();
    riotee_delay_us(200);
    configure_max2769();
    riotee_delay_us(800);
    max2769_capture_snapshot(SNAPSHOT_SIZE_BYTES, snapshot_buf);
    disable_max2769();
  }
}
