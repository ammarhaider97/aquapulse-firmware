#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"

// =====================================================
// ============  I2C / OLED CONFIG (Week 3)  ==========
// =====================================================
#define I2C_MASTER_SDA_IO   8
#define I2C_MASTER_SCL_IO   9
#define I2C_MASTER_FREQ_HZ  400000
#define I2C_MASTER_PORT     I2C_NUM_0
#define OLED_I2C_ADDR       0x3C

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   (OLED_HEIGHT / 8)

static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES];

// =====================================================
// ==========  SENSOR / ADC / BUTTON CONFIG  ==========
// =====================================================
#define OVERSAMPLE_COUNT 16
#define MEDIAN_WINDOW 5
#define NUM_CHANNELS 5
#define ADC_VREF 3.3f
#define ADC_MAX_RAW 4095
#define BUTTON_PIN 0
#define LONG_PRESS_MS 2000

adc1_channel_t channels[NUM_CHANNELS] = {
    ADC1_CHANNEL_0, ADC1_CHANNEL_1, ADC1_CHANNEL_2, ADC1_CHANNEL_3, ADC1_CHANNEL_4
};

volatile int button_pressed = 0;
void IRAM_ATTR button_isr_handler(void* arg) { button_pressed = !gpio_get_level(BUTTON_PIN); }

// =====================================================
// ================  I2C MASTER INIT  =================
// =====================================================
void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_PORT, &conf);
    i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);
}

static esp_err_t ssd1306_send(uint8_t control_byte, const uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, control_byte, true);
    i2c_master_write(cmd, (uint8_t *)data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void ssd1306_init(void)
{
    uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };
    ssd1306_send(0x00, init_cmds, sizeof(init_cmds));
}

void ssd1306_display(void)
{
    uint8_t col_cmd[] = { 0x21, 0, OLED_WIDTH - 1 };
    uint8_t page_cmd[] = { 0x22, 0, OLED_PAGES - 1 };
    ssd1306_send(0x00, col_cmd, sizeof(col_cmd));
    ssd1306_send(0x00, page_cmd, sizeof(page_cmd));
    ssd1306_send(0x40, framebuffer, sizeof(framebuffer));
}

void ssd1306_clear(void) { memset(framebuffer, 0, sizeof(framebuffer)); }

typedef struct { char c; uint8_t col[5]; } glyph_t;

static const glyph_t font_table[] = {
    {'0', {0x3E,0x51,0x49,0x45,0x3E}}, {'1', {0x00,0x42,0x7F,0x40,0x00}},
    {'2', {0x62,0x51,0x49,0x49,0x46}}, {'3', {0x22,0x41,0x49,0x49,0x36}},
    {'4', {0x18,0x14,0x12,0x7F,0x10}}, {'5', {0x27,0x45,0x45,0x45,0x39}},
    {'6', {0x3C,0x4A,0x49,0x49,0x30}}, {'7', {0x01,0x71,0x09,0x05,0x03}},
    {'8', {0x36,0x49,0x49,0x49,0x36}}, {'9', {0x06,0x49,0x49,0x29,0x1E}},
    {'A', {0x7E,0x11,0x11,0x11,0x7E}}, {'P', {0x7F,0x09,0x09,0x09,0x06}},
    {'D', {0x7F,0x41,0x41,0x22,0x1C}}, {'N', {0x7F,0x04,0x08,0x10,0x7F}},
    {'E', {0x7F,0x49,0x49,0x49,0x41}}, {'T', {0x01,0x01,0x7F,0x01,0x01}},
    {'O', {0x3E,0x41,0x41,0x41,0x3E}}, {'K', {0x7F,0x08,0x14,0x22,0x41}},
    {'R', {0x7F,0x09,0x19,0x29,0x46}}, {'Q', {0x3E,0x41,0x51,0x21,0x5E}},
    {'U', {0x3F,0x40,0x40,0x40,0x3F}}, {'L', {0x7F,0x40,0x40,0x40,0x40}},
    {'S', {0x46,0x49,0x49,0x49,0x31}}, {'.', {0x00,0x00,0x40,0x40,0x00}},
    {':', {0x00,0x00,0x14,0x00,0x00}}, {'-', {0x00,0x08,0x08,0x08,0x00}},
    {' ', {0x00,0x00,0x00,0x00,0x00}},
};
#define FONT_COUNT (sizeof(font_table)/sizeof(font_table[0]))

static const uint8_t *get_glyph(char c)
{
    for (size_t i = 0; i < FONT_COUNT; i++)
        if (font_table[i].c == c) return font_table[i].col;
    return font_table[FONT_COUNT - 1].col;
}

void ssd1306_draw_char(int page, int col, char c)
{
    const uint8_t *glyph = get_glyph(c);
    for (int i = 0; i < 5; i++)
        if (col + i < OLED_WIDTH) framebuffer[page * OLED_WIDTH + col + i] = glyph[i];
}

void ssd1306_draw_string(int page, int col, const char *str)
{
    int x = col;
    while (*str) { ssd1306_draw_char(page, x, *str); x += 6; str++; }
}

// =====================================================
// ================  CALIBRATION MATH  ================
// =====================================================
#define PH_NERNST_SLOPE_25C 0.0592f
typedef struct { float slope; float offset; } ph_calibration_t;

float ph_calculate_ideal(float v_measured, float v_ref)
{
    return 7.0f + (v_ref - v_measured) / PH_NERNST_SLOPE_25C;
}

#define TDS_K_FACTOR   0.5f
#define TDS_TEMP_COEFF 0.02f

float tds_calculate_ppm(float ec_measured, float temperature_c)
{
    float compensation_coefficient = 1.0f + TDS_TEMP_COEFF * (temperature_c - 25.0f);
    float ec_25c = ec_measured / compensation_coefficient;
    return ec_25c * TDS_K_FACTOR;
}

typedef struct { float voltage; float ntu; } turbidity_point_t;

static const turbidity_point_t anchor_points[] = {
    {4.10f, 0.0f}, {3.80f, 50.0f}, {3.30f, 200.0f}, {2.50f, 500.0f}, {1.50f, 1000.0f},
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

float ds18b20_decode_temperature(const uint8_t scratchpad[9])
{
    int16_t raw = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
    return raw * 0.0625f;
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

// =====================================================
// ========  CALIBRATION DATA + NVS (Week 3)  =========
// =====================================================
typedef struct {
    ph_calibration_t ph_cal;
    float tds_k_factor;
    float tds_temp_coeff;
    turbidity_point_t turb_points[5];
    float ec_tolerance_percent;
    float ds18b20_offset_c;
} all_calibration_t;

#define NVS_NAMESPACE   "aqp_cal"
#define NVS_KEY_CALDATA "cal_blob"
#define CAL_MAGIC_NUMBER 0xA9C4B27E

typedef struct {
    uint32_t magic;
    all_calibration_t data;
    uint32_t checksum;
} cal_storage_t;

static uint32_t calculate_checksum(const all_calibration_t *data)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(all_calibration_t); i++) sum += bytes[i];
    return sum;
}

void calibration_load_defaults(all_calibration_t *cal)
{
    cal->ph_cal.slope = 1.0f;
    cal->ph_cal.offset = 0.0f;
    cal->tds_k_factor = TDS_K_FACTOR;
    cal->tds_temp_coeff = TDS_TEMP_COEFF;
    for (int i = 0; i < 5; i++) cal->turb_points[i] = anchor_points[i];
    cal->ec_tolerance_percent = 10.0f;
    cal->ds18b20_offset_c = 0.0f;
}

bool calibration_save_to_nvs(const all_calibration_t *cal)
{
    cal_storage_t storage;
    storage.magic = CAL_MAGIC_NUMBER;
    storage.data = *cal;
    storage.checksum = calculate_checksum(cal);

    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return false;
    esp_err_t err = nvs_set_blob(handle, NVS_KEY_CALDATA, &storage, sizeof(storage));
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    return err == ESP_OK;
}

bool calibration_load_from_nvs(all_calibration_t *cal)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) return false;

    cal_storage_t storage;
    size_t required_size = sizeof(storage);
    esp_err_t err = nvs_get_blob(handle, NVS_KEY_CALDATA, &storage, &required_size);
    nvs_close(handle);

    if (err != ESP_OK || required_size != sizeof(storage)) return false;
    if (storage.magic != CAL_MAGIC_NUMBER) return false;
    if (storage.checksum != calculate_checksum(&storage.data)) return false;

    *cal = storage.data;
    return true;
}

// =====================================================
// =========  SHARED STATE + MUTEX (Week 4 - Day 1) ===
// =====================================================
// Five separate tasks will read this "calibration" struct,
// and a button press will modify it (write). When two or more tasks
// access the same variable simultaneously, a race condition can occur
// (one task might read incomplete data while another is writing). To prevent this,
// a "mutex" (lock) is used: any task accessing the calibration structure
// must acquire the lock first and release it once finished.
static all_calibration_t calibration;
static SemaphoreHandle_t calibration_mutex;
static float ph_v_ref = 1.65f;
static float water_temp_c = 25.0f;  // updated by temp_task every cycle

// =====================================================
// ============  FREERTOS TASKS (Week 4 - Day 1) ======
// =====================================================
// A FreeRTOS "task" is an independent function that runs with its own
// stack, operating like a mini-program inside the system. The scheduler gives
// CPU time to each task turn by turn based on its priority. The parameters of xTaskCreate() are:
//   1) Task function pointer
//   2) Name (for debugging purposes)
//   3) Stack size (in bytes, for ESP-IDF)
//   4) Parameter pointer (passing NULL here)
//   5) Priority (higher number = higher priority)
//   6) Task handle (for future reference, used in Day 3/4)
//
// Each task maintains its own timing interval using vTaskDelayUntil().
// This is better than vTaskDelay() because the next wake-up time is calculated from
// "last wake time + period", preventing timing drift even if execution takes a bit of time.

#define PH_TASK_STACK        3072
#define TDS_TASK_STACK        3072
#define TURBIDITY_TASK_STACK  2560
#define EC_TASK_STACK         2560
#define TEMP_TASK_STACK       2048
#define SENSOR_TASK_PRIORITY  2

static void ph_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        int raw = read_adc_filtered(channels[0]);
        float voltage = adc_raw_to_voltage(raw);

        xSemaphoreTake(calibration_mutex, portMAX_DELAY);
        float slope = calibration.ph_cal.slope;
        float offset = calibration.ph_cal.offset;
        xSemaphoreGive(calibration_mutex);

        float ph_value = ph_calculate_ideal(voltage, ph_v_ref) * slope + offset;
        printf("[pH  task] pH=%.2f (tick=%lu)\n", ph_value, (unsigned long)xTaskGetTickCount());

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));  // every 2s
    }
}

static void tds_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        int raw = read_adc_filtered(channels[1]);
        float ec_from_tds = adc_raw_to_voltage(raw) * 1000.0f;
        float tds_ppm = tds_calculate_ppm(ec_from_tds, water_temp_c);
        printf("[TDS task] TDS=%.0f ppm (tick=%lu)\n", tds_ppm, (unsigned long)xTaskGetTickCount());

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));  // every 2s
    }
}

static void turbidity_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        int raw = read_adc_filtered(channels[2]);
        float turb_ntu = turbidity_calculate(adc_raw_to_voltage(raw));
        printf("[NTU task] Turbidity=%.1f NTU (tick=%lu)\n", turb_ntu, (unsigned long)xTaskGetTickCount());

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000));  // every 5s
    }
}

static void ec_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        int raw = read_adc_filtered(channels[3]);
        float ec_value = adc_raw_to_voltage(raw) * 1000.0f;
        printf("[EC  task] EC=%.0f (tick=%lu)\n", ec_value, (unsigned long)xTaskGetTickCount());

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000));  // every 5s
    }
}

static void temp_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    uint8_t scratchpad[9] = {0x88, 0x01, 0,0,0,0,0,0,0};  // placeholder, real 1-wire read = later week
    while (1) {
        xSemaphoreTake(calibration_mutex, portMAX_DELAY);
        float offset = calibration.ds18b20_offset_c;
        xSemaphoreGive(calibration_mutex);

        water_temp_c = ds18b20_decode_temperature(scratchpad) + offset;
        printf("[TMP task] Temp=%.1f C (tick=%lu)\n", water_temp_c, (unsigned long)xTaskGetTickCount());

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));  // every 1s
    }
}

// =====================================================
// -------------------- MAIN --------------------------
// =====================================================
void app_main(void)
{
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    i2c_master_init();
    ssd1306_init();
    ssd1306_clear();
    ssd1306_draw_string(0, 0, "TASK TEST");
    ssd1306_display();

    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < NUM_CHANNELS; i++) adc1_config_channel_atten(channels[i], ADC_ATTEN_DB_11);

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    calibration_mutex = xSemaphoreCreateMutex();

    if (!calibration_load_from_nvs(&calibration)) {
        calibration_load_defaults(&calibration);
        calibration_save_to_nvs(&calibration);
        printf("Startup: FACTORY DEFAULTS\n");
    } else {
        printf("Startup: LOADED FROM NVS\n");
    }

// Now each sensor runs as its own independent task here.
// No task waits for another — the scheduler automatically
// manages which task gets CPU time and when.
    xTaskCreate(ph_task,        "ph_task",        PH_TASK_STACK,       NULL, SENSOR_TASK_PRIORITY, NULL);
    xTaskCreate(tds_task,       "tds_task",       TDS_TASK_STACK,      NULL, SENSOR_TASK_PRIORITY, NULL);
    xTaskCreate(turbidity_task, "turbidity_task", TURBIDITY_TASK_STACK,NULL, SENSOR_TASK_PRIORITY, NULL);
    xTaskCreate(ec_task,        "ec_task",        EC_TASK_STACK,       NULL, SENSOR_TASK_PRIORITY, NULL);
    xTaskCreate(temp_task,      "temp_task",      TEMP_TASK_STACK,     NULL, SENSOR_TASK_PRIORITY, NULL);

// app_main itself is also a FreeRTOS task (running slightly above idle priority).
// We keep it alive to check for button presses, just like we did
// in Week 3 — it will continue to run this way in Day 2+ as well.
    TickType_t press_start = 0;
    bool button_was_down = false;
    while (1) {
        if (button_pressed && !button_was_down) {
            button_was_down = true;
            press_start = xTaskGetTickCount();
        }
        if (!button_pressed && button_was_down) {
            button_was_down = false;
            TickType_t held_ticks = xTaskGetTickCount() - press_start;
            if (held_ticks > pdMS_TO_TICKS(LONG_PRESS_MS)) {
                printf("Long press -> esp_restart()\n");
                esp_restart();
            } else {
                xSemaphoreTake(calibration_mutex, portMAX_DELAY);
                calibration.ph_cal.slope += 0.05f;
                calibration_save_to_nvs(&calibration);
                xSemaphoreGive(calibration_mutex);
                printf("Short press -> calibration updated & saved\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));  // just polling the button, no fixed sensor timing here
    }
}
