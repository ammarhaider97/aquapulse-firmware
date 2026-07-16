#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

// -------------------- CONFIG --------------------
#define BUTTON_PIN 0
#define OVERSAMPLE_COUNT 16
#define MEDIAN_WINDOW 5
#define NUM_CHANNELS 5
#define ADC_VREF 3.3f
#define ADC_MAX_RAW 4095

adc1_channel_t channels[NUM_CHANNELS] = {
    ADC1_CHANNEL_0, ADC1_CHANNEL_1, ADC1_CHANNEL_2, ADC1_CHANNEL_3, ADC1_CHANNEL_4
};

volatile int button_pressed = 0;
void IRAM_ATTR button_isr_handler(void* arg) { button_pressed = 1; }

// =====================================================
// ============  pH CALIBRATION (Day 1) ===============
// =====================================================
#define PH_NERNST_SLOPE_25C 0.0592f

typedef struct {
    float slope;
    float offset;
} ph_calibration_t;

float ph_calculate_ideal(float v_measured, float v_ref)
{
    return 7.0f + (v_ref - v_measured) / PH_NERNST_SLOPE_25C;
}

void ph_calibrate_two_point(ph_calibration_t *cal, float v_low, float ph_low, float v_high, float ph_high)
{
    cal->slope  = (ph_high - ph_low) / (v_high - v_low);
    cal->offset = ph_low - cal->slope * v_low;
}

float ph_calculate_calibrated(const ph_calibration_t *cal, float v_measured)
{
    return cal->slope * v_measured + cal->offset;
}

static int check(const char *name, float got, float expected, float tolerance)
{
    int pass = fabsf(got - expected) <= tolerance;
    printf("[%s] %s -> got=%.3f expected=%.3f\n", pass ? "PASS" : "FAIL", name, got, expected);
    return pass;
}

int test_ph_run(void)
{
    int passed = 0, total = 0;
    total++; passed += check("ideal_neutral", ph_calculate_ideal(1.65f, 1.65f), 7.0f, 0.01f);
    total++; passed += check("ideal_plus_one_unit", ph_calculate_ideal(1.65f - 0.0592f, 1.65f), 8.0f, 0.01f);

    ph_calibration_t cal;
    ph_calibrate_two_point(&cal, 2.00f, 4.0f, 1.40f, 10.0f);
    total++; passed += check("calibrated_at_low_point", ph_calculate_calibrated(&cal, 2.00f), 4.0f, 0.01f);
    total++; passed += check("calibrated_at_high_point", ph_calculate_calibrated(&cal, 1.40f), 10.0f, 0.01f);
    total++; passed += check("calibrated_midpoint", ph_calculate_calibrated(&cal, 1.70f), 7.0f, 0.5f);

    printf("pH tests: %d/%d passed\n\n", passed, total);
    return passed == total;
}

// =====================================================
// ============  TDS CALIBRATION (Day 2) ===============
// =====================================================
#define TDS_K_FACTOR   0.5f   // ppm per EC unit, potable water
#define TDS_TEMP_COEFF 0.02f  // 2% conductivity change per °C

// Normalizes EC to 25°C, then converts to TDS (ppm) using the k factor
float tds_calculate_ppm(float ec_measured, float temperature_c)
{
    float compensation_coefficient = 1.0f + TDS_TEMP_COEFF * (temperature_c - 25.0f);
    float ec_25c = ec_measured / compensation_coefficient;
    return ec_25c * TDS_K_FACTOR;
}

int test_tds_run(void)
{
    int passed = 0, total = 0;

    // At exactly 25°C, no compensation needed: TDS = EC * 0.5
    total++; passed += check("no_compensation_at_25C", tds_calculate_ppm(200.0f, 25.0f), 100.0f, 0.01f);

    // At 35°C, EC=240 should normalize to 200 -> TDS still 100 ppm
    total++; passed += check("compensated_at_35C", tds_calculate_ppm(240.0f, 35.0f), 100.0f, 0.01f);

    // At 15°C, EC=160 should normalize to 200 -> TDS still 100 ppm
    total++; passed += check("compensated_at_15C", tds_calculate_ppm(160.0f, 15.0f), 100.0f, 0.01f);

    printf("TDS tests: %d/%d passed\n\n", passed, total);
    return passed == total;
}

// =====================================================
// ==============  ADC HELPER FUNCTIONS ===============
// =====================================================
int read_adc_oversampled(adc1_channel_t channel)
{
    int sum = 0;
    for (int i = 0; i < OVERSAMPLE_COUNT; i++) sum += adc1_get_raw(channel);
    return sum / OVERSAMPLE_COUNT;
}

void sort_array(int arr[], int size)
{
    for (int i = 0; i < size - 1; i++)
        for (int j = 0; j < size - i - 1; j++)
            if (arr[j] > arr[j + 1]) { int t = arr[j]; arr[j] = arr[j+1]; arr[j+1] = t; }
}

int read_adc_filtered(adc1_channel_t channel)
{
    int readings[MEDIAN_WINDOW];
    for (int i = 0; i < MEDIAN_WINDOW; i++) readings[i] = read_adc_oversampled(channel);
    sort_array(readings, MEDIAN_WINDOW);
    return readings[MEDIAN_WINDOW / 2];
}

static float adc_raw_to_voltage(int raw) { return (raw / (float)ADC_MAX_RAW) * ADC_VREF; }

// -------------------- MAIN --------------------
void app_main(void)
{
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < NUM_CHANNELS; i++) adc1_config_channel_atten(channels[i], ADC_ATTEN_DB_11);

    printf("===== Running calibration unit tests =====\n");
    test_ph_run();
    test_tds_run();
    printf("============================================\n\n");

    float ph_v_ref = 1.65f;
    float water_temp_c = 25.0f; // placeholder until DS18B20 (Day 4) gives real value

    while (1)
    {
        for (int i = 0; i < NUM_CHANNELS; i++) {
            int raw_value = adc1_get_raw(channels[i]);
            int filtered_value = read_adc_filtered(channels[i]);
            printf("Channel %d: raw=%d  filtered=%d\n", i + 1, raw_value, filtered_value);
        }

        int ph_raw = read_adc_filtered(channels[0]);
        float ph_voltage = adc_raw_to_voltage(ph_raw);
        printf("pH channel voltage=%.3fV -> pH=%.2f\n", ph_voltage, ph_calculate_ideal(ph_voltage, ph_v_ref));

        // ---------- LIVE TDS DEMO (channel 1) ----------
        int tds_raw = read_adc_filtered(channels[1]);
        float tds_voltage = adc_raw_to_voltage(tds_raw);
        float ec_estimate = tds_voltage * 1000.0f; // placeholder V-to-EC scaling for demo purposes
        float tds_ppm = tds_calculate_ppm(ec_estimate, water_temp_c);
        printf("TDS channel voltage=%.3fV -> TDS=%.1f ppm\n", tds_voltage, tds_ppm);

        printf("--------------------\n");
        if (button_pressed) { printf("Button Pressed!\n"); button_pressed = 0; }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
