#include <stdio.h>
#include <math.h>
#include <stdbool.h>
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

typedef struct { float slope; float offset; } ph_calibration_t;

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

static int check_bool(const char *name, bool got, bool expected)
{
    int pass = (got == expected);
    printf("[%s] %s -> got=%d expected=%d\n", pass ? "PASS" : "FAIL", name, got, expected);
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
#define TDS_K_FACTOR   0.5f
#define TDS_TEMP_COEFF 0.02f

float tds_calculate_ppm(float ec_measured, float temperature_c)
{
    float compensation_coefficient = 1.0f + TDS_TEMP_COEFF * (temperature_c - 25.0f);
    float ec_25c = ec_measured / compensation_coefficient;
    return ec_25c * TDS_K_FACTOR;
}

int test_tds_run(void)
{
    int passed = 0, total = 0;
    total++; passed += check("no_compensation_at_25C", tds_calculate_ppm(200.0f, 25.0f), 100.0f, 0.01f);
    total++; passed += check("compensated_at_35C", tds_calculate_ppm(240.0f, 35.0f), 100.0f, 0.01f);
    total++; passed += check("compensated_at_15C", tds_calculate_ppm(160.0f, 15.0f), 100.0f, 0.01f);
    printf("TDS tests: %d/%d passed\n\n", passed, total);
    return passed == total;
}

// =====================================================
// ==========  TURBIDITY CALIBRATION (Day 3) ===========
// =====================================================
typedef struct { float voltage; float ntu; } turbidity_point_t;

// Datasheet anchor points: higher voltage = clearer water = lower NTU
static const turbidity_point_t anchor_points[] = {
    {4.10f,    0.0f},
    {3.80f,   50.0f},
    {3.30f,  200.0f},
    {2.50f,  500.0f},
    {1.50f, 1000.0f},
};
#define NUM_ANCHOR_POINTS (sizeof(anchor_points) / sizeof(anchor_points[0]))

float turbidity_calculate(float voltage)
{
    if (voltage >= anchor_points[0].voltage) return anchor_points[0].ntu;
    if (voltage <= anchor_points[NUM_ANCHOR_POINTS - 1].voltage)
        return anchor_points[NUM_ANCHOR_POINTS - 1].ntu;

    for (int i = 0; i < NUM_ANCHOR_POINTS - 1; i++) {
        float v_high = anchor_points[i].voltage;
        float v_low  = anchor_points[i + 1].voltage;
        if (voltage <= v_high && voltage >= v_low) {
            float ntu_high = anchor_points[i].ntu;
            float ntu_low  = anchor_points[i + 1].ntu;
            float fraction = (v_high - voltage) / (v_high - v_low);
            return ntu_high + fraction * (ntu_low - ntu_high);
        }
    }
    return 0.0f;
}

// =====================================================
// ============  EC CROSS-VALIDATION (Day 3) ===========
// =====================================================
bool ec_tds_cross_validate(float tds_from_ec, float tds_from_tds_channel,
                           float tolerance_percent, float *diff_percent_out)
{
    float reference = (tds_from_tds_channel != 0.0f) ? tds_from_tds_channel : 1.0f;
    float diff_percent = fabsf(tds_from_ec - tds_from_tds_channel) / reference * 100.0f;
    if (diff_percent_out) *diff_percent_out = diff_percent;
    return diff_percent <= tolerance_percent;
}

int test_turbidity_ec_run(void)
{
    int passed = 0, total = 0;

    total++; passed += check("turbidity_exact_anchor_clear", turbidity_calculate(4.10f), 0.0f, 0.01f);
    total++; passed += check("turbidity_exact_anchor_turbid", turbidity_calculate(1.50f), 1000.0f, 0.01f);
    total++; passed += check("turbidity_midpoint_interp", turbidity_calculate(3.55f), 125.0f, 1.0f);
    total++; passed += check("turbidity_clamp_high", turbidity_calculate(5.0f), 0.0f, 0.01f);
    total++; passed += check("turbidity_clamp_low", turbidity_calculate(0.5f), 1000.0f, 0.01f);

    total++; passed += check_bool("ec_validate_pass", ec_tds_cross_validate(100.0f, 103.0f, 5.0f, NULL), true);
    total++; passed += check_bool("ec_validate_fail", ec_tds_cross_validate(100.0f, 140.0f, 5.0f, NULL), false);

    printf("Turbidity/EC tests: %d/%d passed\n\n", passed, total);
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
    test_turbidity_ec_run();
    printf("============================================\n\n");

    float ph_v_ref = 1.65f;
    float water_temp_c = 25.0f;

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

        int tds_raw = read_adc_filtered(channels[1]);
        float tds_voltage = adc_raw_to_voltage(tds_raw);
        float ec_from_tds_channel = tds_voltage * 1000.0f;
        float tds_ppm = tds_calculate_ppm(ec_from_tds_channel, water_temp_c);
        printf("TDS channel voltage=%.3fV -> TDS=%.1f ppm\n", tds_voltage, tds_ppm);

        // ---------- LIVE TURBIDITY DEMO (channel 2) ----------
        int turb_raw = read_adc_filtered(channels[2]);
        float turb_voltage = adc_raw_to_voltage(turb_raw);
        float ntu = turbidity_calculate(turb_voltage);
        printf("Turbidity channel voltage=%.3fV -> NTU=%.1f\n", turb_voltage, ntu);

        // ---------- LIVE EC CROSS-VALIDATION (channel 3 vs TDS channel) ----------
        int ec_raw = read_adc_filtered(channels[3]);
        float ec_voltage = adc_raw_to_voltage(ec_raw);
        float ec_reading = ec_voltage * 1000.0f;
        float tds_from_ec = tds_calculate_ppm(ec_reading, water_temp_c);
        float diff_percent = 0.0f;
        bool ec_ok = ec_tds_cross_validate(tds_from_ec, tds_ppm, 10.0f, &diff_percent);
        printf("EC cross-check: TDS_from_EC=%.1f vs TDS_channel=%.1f -> diff=%.1f%% (%s)\n",
               tds_from_ec, tds_ppm, diff_percent, ec_ok ? "OK" : "MISMATCH");

        printf("--------------------\n");
        if (button_pressed) { printf("Button Pressed!\n"); button_pressed = 0; }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
