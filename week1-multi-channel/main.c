#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

// -------------------- CONFIG --------------------
#define BUTTON_PIN 0
#define OVERSAMPLE_COUNT 16
#define MEDIAN_WINDOW 5
#define NUM_CHANNELS 5

// 5 ADC channels - map these to your 5 potentiometers in Wokwi
adc1_channel_t channels[NUM_CHANNELS] = {
    ADC1_CHANNEL_0,  // GPIO1 - pH (placeholder)
    ADC1_CHANNEL_1,  // GPIO2 - TDS (placeholder)
    ADC1_CHANNEL_2,  // GPIO3 - Turbidity (placeholder)
    ADC1_CHANNEL_3,  // GPIO4 - EC (placeholder)
    ADC1_CHANNEL_4   // GPIO5 - extra channel
};

// -------------------- GLOBAL FLAG --------------------
volatile int button_pressed = 0;

// -------------------- ISR --------------------
void IRAM_ATTR button_isr_handler(void* arg) {
    button_pressed = 1;
}

// -------------------- OVERSAMPLING FUNCTION --------------------
int read_adc_oversampled(adc1_channel_t channel)
{
    int sum = 0;
    for (int i = 0; i < OVERSAMPLE_COUNT; i++) {
        sum += adc1_get_raw(channel);
    }
    return sum / OVERSAMPLE_COUNT;
}

// -------------------- SORT FUNCTION --------------------
void sort_array(int arr[], int size)
{
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// -------------------- MEDIAN FILTER FUNCTION --------------------
int read_adc_filtered(adc1_channel_t channel)
{
    int readings[MEDIAN_WINDOW];
    for (int i = 0; i < MEDIAN_WINDOW; i++) {
        readings[i] = read_adc_oversampled(channel);
    }
    sort_array(readings, MEDIAN_WINDOW);
    return readings[MEDIAN_WINDOW / 2];
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

    // ---------- ADC SETUP FOR ALL 5 CHANNELS ----------
    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < NUM_CHANNELS; i++) {
        adc1_config_channel_atten(channels[i], ADC_ATTEN_DB_11);
    }

    while (1)
    {
        // ---------- READ ALL 5 CHANNELS ----------
        for (int i = 0; i < NUM_CHANNELS; i++)
        {
            int raw_value = adc1_get_raw(channels[i]);
            int filtered_value = read_adc_filtered(channels[i]);

            printf("Channel %d: raw=%d  filtered=%d\n", i + 1, raw_value, filtered_value);
        }

        printf("--------------------\n");

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
