#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

// -------------------- CONFIG --------------------
#define BUTTON_PIN 0                 
#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO1 

// -------------------- GLOBAL FLAG --------------------
volatile int button_pressed = 0;

// -------------------- ISR --------------------
void IRAM_ATTR button_isr_handler(void* arg) {
    button_pressed = 1;
}

// -------------------- MAIN --------------------
void app_main(void)
{
    // ---------- BUTTON SETUP ----------
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    // ---------- ADC SETUP ----------
    adc1_config_width(ADC_WIDTH_BIT_12);  // 0–4095 range
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11); // 0–3.3V

    while (1)
    {
        // ---------- READ ADC ----------
        int adc_value = adc1_get_raw(ADC_CHANNEL);
        printf("ADC Value: %d\n", adc_value);

        // ---------- CHECK BUTTON ----------
        if (button_pressed)
        {
            printf("Button Pressed!\n");
            button_pressed = 0;
        }

        // ---------- DELAY ----------
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
