#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include <stdlib.h>

// -------------------- CONFIG --------------------
#define BUTTON_PIN 0                 
#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO1 
#define OVERSAMPLE_COUNT 16
#define MEDIAN_WINDOW 5

// -------------------- GLOBAL FLAG --------------------
volatile int button_pressed = 0;

// -------------------- ISR --------------------
void IRAM_ATTR button_isr_handler(void* arg) {
    button_pressed = 1;
}

// -------------------- OVERSAMPLING FUNCTION --------------------
// Reads ADC 16 times and returns the average
int read_adc_oversampled(adc1_channel_t channel)
{
    int sum = 0;
    for (int i = 0; i < OVERSAMPLE_COUNT; i++) {
        int noise = rand() % 50;  // Nosie added manually as potentiometer is stable in wokwoi
        sum += adc1_get_raw(channel) + noise;
    }
    return sum / OVERSAMPLE_COUNT;
}

// -------------------- SIMPLE SORT FUNCTION --------------------
// Sorts a small array from smallest to largest
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
// Takes 5 oversampled readings, returns the middle (median) value
int read_adc_filtered(adc1_channel_t channel)
{
    int readings[MEDIAN_WINDOW];

    for (int i = 0; i < MEDIAN_WINDOW; i++) {
        readings[i] = read_adc_oversampled(channel);
    }

    sort_array(readings, MEDIAN_WINDOW);

    return readings[MEDIAN_WINDOW / 2]; // middle value after sorting
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
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

    while (1)
    {
        // ---------- RAW READING (no filter) ----------
        int raw_value = adc1_get_raw(ADC_CHANNEL);

        // ---------- FILTERED READING (oversampled + median) ----------
        int filtered_value = read_adc_filtered(ADC_CHANNEL);

        printf("raw=%d  filtered=%d\n", raw_value, filtered_value);

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
