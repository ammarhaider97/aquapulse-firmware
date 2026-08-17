#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "cJSON.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_sleep.h"   // needed for esp_sleep_enable_timer_wakeup()

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
static all_calibration_t calibration;
static SemaphoreHandle_t calibration_mutex;
static float ph_v_ref = 1.65f;
static float water_temp_c = 25.0f;

// =====================================================
// ====  BUS ACTIVITY MUTEX (Week 7 fix)  ==============
// =====================================================
// Guards I2C (OLED) and WiFi/HTTP operations from overlapping with
// light sleep. Light sleep pauses the whole chip, so if it triggers
// while a driver is mid-transaction, the driver's state can get
// corrupted and crash on wakeup. Any code that touches the I2C bus or
// performs an HTTP POST takes this mutex; the power management task
// only sleeps when it can also take it (proving nothing else is busy).
static SemaphoreHandle_t bus_activity_mutex;

// =====================================================
// =========  QUEUE + AGGREGATOR (Week 4 - Day 2)  ====
// =====================================================
typedef enum { SENSOR_PH = 0, SENSOR_TDS, SENSOR_TURBIDITY, SENSOR_EC, SENSOR_TEMP, SENSOR_COUNT } sensor_id_t;

typedef struct {
    sensor_id_t id;
    float value;
} sensor_reading_t;

#define SENSOR_QUEUE_LENGTH 10
static QueueHandle_t sensor_data_queue;

// =====================================================
// ============  FREERTOS TASKS (Week 4 - Day 1) ======
// =====================================================
#define PH_TASK_STACK        3072
#define TDS_TASK_STACK        3072
#define TURBIDITY_TASK_STACK  2560
#define EC_TASK_STACK         2560
#define TEMP_TASK_STACK       2048
#define SENSOR_TASK_PRIORITY  2

static void ph_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);   // register this task with the watchdog
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        esp_task_wdt_reset();  // tell the watchdog "I'm alive" every cycle

        int raw = read_adc_filtered(channels[0]);
        float voltage = adc_raw_to_voltage(raw);

        xSemaphoreTake(calibration_mutex, portMAX_DELAY);
        float slope = calibration.ph_cal.slope;
        float offset = calibration.ph_cal.offset;
        xSemaphoreGive(calibration_mutex);

        float ph_value = ph_calculate_ideal(voltage, ph_v_ref) * slope + offset;

        sensor_reading_t reading = { .id = SENSOR_PH, .value = ph_value };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));
    }
}

static void tds_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);   // register this task with the watchdog
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        esp_task_wdt_reset();  // tell the watchdog "I'm alive" every cycle

        int raw = read_adc_filtered(channels[1]);
        float ec_from_tds = adc_raw_to_voltage(raw) * 1000.0f;
        float tds_ppm = tds_calculate_ppm(ec_from_tds, water_temp_c);

        sensor_reading_t reading = { .id = SENSOR_TDS, .value = tds_ppm };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));
    }
}

static void turbidity_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);   // register this task with the watchdog
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        esp_task_wdt_reset();  // tell the watchdog "I'm alive" every cycle

        int raw = read_adc_filtered(channels[2]);
        float turb_ntu = turbidity_calculate(adc_raw_to_voltage(raw));

        sensor_reading_t reading = { .id = SENSOR_TURBIDITY, .value = turb_ntu };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000));
    }
}

static void ec_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);   // register this task with the watchdog
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        esp_task_wdt_reset();  // tell the watchdog "I'm alive" every cycle

        int raw = read_adc_filtered(channels[3]);
        float ec_value = adc_raw_to_voltage(raw) * 1000.0f;

        sensor_reading_t reading = { .id = SENSOR_EC, .value = ec_value };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000));
    }
}

static void temp_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);   // register this task with the watchdog
    TickType_t last_wake = xTaskGetTickCount();
    uint8_t scratchpad[9] = {0x88, 0x01, 0,0,0,0,0,0,0};
    while (1) {
        esp_task_wdt_reset();  // tell the watchdog "I'm alive" every cycle

        xSemaphoreTake(calibration_mutex, portMAX_DELAY);
        float offset = calibration.ds18b20_offset_c;
        xSemaphoreGive(calibration_mutex);

        water_temp_c = ds18b20_decode_temperature(scratchpad) + offset;

        sensor_reading_t reading = { .id = SENSOR_TEMP, .value = water_temp_c };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));
    }
}

// =====================================================
// ======  SHARED SENSOR SNAPSHOT (Week 5 - Day 1)  ====
// =====================================================
typedef struct {
    float ph;
    float tds_ppm;
    float turbidity_ntu;
    float ec_us_cm;
    float temp_c;
} system_readings_t;

static system_readings_t g_readings = {0};
static SemaphoreHandle_t readings_mutex;

// =====================================================
// ==============  AGGREGATOR TASK (Week 4 - Day 2)  ==
// =====================================================
#define AGGREGATOR_STACK    4096
#define AGGREGATOR_PRIORITY 3

static const char *sensor_names[SENSOR_COUNT] = { "pH", "TDS", "NTU", "EC", "TMP" };

// =====================================================
// ===========  TASK WATCHDOG TIMER (Week 4 - Day 3) ==
// =====================================================
#define WDT_TIMEOUT_MS 8000   // Fix: bumped from 5000 to give a real HTTP POST (DNS + connect
                              // + send, once Day 3's real URL is in) enough headroom under the
                              // shared bus_activity_mutex without nuisance-tripping the watchdog

static TaskHandle_t aggregator_handle = NULL;
static TaskHandle_t ph_handle = NULL;
static TaskHandle_t tds_handle = NULL;
static TaskHandle_t turbidity_handle = NULL;
static TaskHandle_t ec_handle = NULL;
static TaskHandle_t temp_handle = NULL;

// =====================================================
// ========  STACK HIGH-WATER MARK CHECK (Week 4 - Day 4) =
// =====================================================
typedef struct {
    const char *name;
    TaskHandle_t *handle;
    uint32_t allocated_bytes;
} stack_watch_entry_t;

static void print_stack_usage(void)
{
    stack_watch_entry_t entries[] = {
        { "ph_task",        &ph_handle,        PH_TASK_STACK },
        { "tds_task",       &tds_handle,       TDS_TASK_STACK },
        { "turbidity_task", &turbidity_handle, TURBIDITY_TASK_STACK },
        { "ec_task",        &ec_handle,        EC_TASK_STACK },
        { "temp_task",      &temp_handle,      TEMP_TASK_STACK },
        { "aggregator_task",&aggregator_handle,AGGREGATOR_STACK },
    };

    printf("---- Stack high-water mark report ----\n");
    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
        if (*entries[i].handle == NULL) continue;
        UBaseType_t free_words = uxTaskGetStackHighWaterMark(*entries[i].handle);
        uint32_t free_bytes = free_words * sizeof(StackType_t);
        float free_percent = 100.0f * (float)free_bytes / (float)entries[i].allocated_bytes;
        const char *flag = (free_percent < 20.0f) ? "  <-- WARNING: within 20% of limit!" : "";
        printf("  %-16s free=%4u bytes / %4u bytes (%.1f%% free)%s\n",
               entries[i].name, (unsigned)free_bytes, (unsigned)entries[i].allocated_bytes,
               free_percent, flag);
    }
    printf("---------------------------------------\n");
}

#define STACK_MONITOR_STACK    4096
#define STACK_MONITOR_PRIORITY 1

static void stack_monitor_task(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        print_stack_usage();
    }
}

static void aggregator_task(void *pvParameters)
{
    float latest[SENSOR_COUNT] = {0};
    sensor_reading_t reading;

    esp_task_wdt_add(NULL);

    while (1) {
        esp_task_wdt_reset();

        if (xQueueReceive(sensor_data_queue, &reading, pdMS_TO_TICKS(1000)) == pdTRUE) {
            latest[reading.id] = reading.value;
            printf("[AGG] updated %s = %.2f (tick=%lu)\n",
                   sensor_names[reading.id], reading.value, (unsigned long)xTaskGetTickCount());
        }

        while (xQueueReceive(sensor_data_queue, &reading, 0) == pdTRUE) {
            latest[reading.id] = reading.value;
            printf("[AGG] updated %s = %.2f (tick=%lu)\n",
                   sensor_names[reading.id], reading.value, (unsigned long)xTaskGetTickCount());
        }

        xSemaphoreTake(readings_mutex, portMAX_DELAY);
        g_readings.ph            = latest[SENSOR_PH];
        g_readings.tds_ppm       = latest[SENSOR_TDS];
        g_readings.turbidity_ntu = latest[SENSOR_TURBIDITY];
        g_readings.ec_us_cm      = latest[SENSOR_EC];
        g_readings.temp_c        = latest[SENSOR_TEMP];
        xSemaphoreGive(readings_mutex);

        // Bus activity mutex (fix): protects the I2C write below from
        // overlapping with a light-sleep window in power_mgmt_task.
        xSemaphoreTake(bus_activity_mutex, portMAX_DELAY);
        char line[16];
        ssd1306_clear();
        snprintf(line, sizeof(line), "P%.2f", latest[SENSOR_PH]);       ssd1306_draw_string(0, 0, line);
        snprintf(line, sizeof(line), "D%.0f", latest[SENSOR_TDS]);      ssd1306_draw_string(1, 0, line);
        snprintf(line, sizeof(line), "N%.1f", latest[SENSOR_TURBIDITY]);ssd1306_draw_string(2, 0, line);
        snprintf(line, sizeof(line), "E%.0f", latest[SENSOR_EC]);       ssd1306_draw_string(3, 0, line);
        snprintf(line, sizeof(line), "T%.1f", latest[SENSOR_TEMP]);     ssd1306_draw_string(4, 0, line);
        ssd1306_display();
        xSemaphoreGive(bus_activity_mutex);

        printf("[AGG] pH=%.2f TDS=%.0f NTU=%.1f EC=%.0f Temp=%.1f\n",
               latest[SENSOR_PH], latest[SENSOR_TDS], latest[SENSOR_TURBIDITY],
               latest[SENSOR_EC], latest[SENSOR_TEMP]);
    }
}

// =====================================================
// ============  FAULT FLAGS (Week 5 - Day 1)  =========
// =====================================================
#define FAULT_PH_RANGE        (1 << 0)
#define FAULT_TDS_RANGE       (1 << 1)
#define FAULT_TURBIDITY_RANGE (1 << 2)
#define FAULT_EC_RANGE        (1 << 3)
#define FAULT_TEMP_RANGE      (1 << 4)

#define PH_MIN         0.0f
#define PH_MAX         14.0f
#define TDS_MIN        0.0f
#define TDS_MAX        5000.0f
#define TURBIDITY_MIN  0.0f
#define TURBIDITY_MAX  3000.0f
#define EC_MIN         0.0f
#define EC_MAX         5000.0f
#define TEMP_MIN       -10.0f
#define TEMP_MAX       60.0f

#define DEVICE_ID         "AQUAPULSE-001"
#define FIRMWARE_VERSION  "1.5.0"

// Set to 1 to force a fake out-of-range pH reading every 3rd network
// cycle -- this is the fault-injection deliverable test. Set to 0 for
// normal operation once the test has been demonstrated/recorded.
#define TEST_FAULT_INJECTION 1

static bool in_range(float v, float lo, float hi) { return v >= lo && v <= hi; }

char *build_json_payload(const system_readings_t *r, uint32_t *fault_flags_out)
{
    uint32_t fault_flags = 0;
    float ph = r->ph, tds = r->tds_ppm, ntu = r->turbidity_ntu;
    float ec = r->ec_us_cm, temp = r->temp_c;

    if (!in_range(ph, PH_MIN, PH_MAX))                { fault_flags |= FAULT_PH_RANGE;        ph   = 0.0f; }
    if (!in_range(tds, TDS_MIN, TDS_MAX))             { fault_flags |= FAULT_TDS_RANGE;       tds  = 0.0f; }
    if (!in_range(ntu, TURBIDITY_MIN, TURBIDITY_MAX)) { fault_flags |= FAULT_TURBIDITY_RANGE; ntu  = 0.0f; }
    if (!in_range(ec, EC_MIN, EC_MAX))                { fault_flags |= FAULT_EC_RANGE;        ec   = 0.0f; }
    if (!in_range(temp, TEMP_MIN, TEMP_MAX))          { fault_flags |= FAULT_TEMP_RANGE;      temp = 0.0f; }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
    cJSON_AddStringToObject(root, "firmware_version", FIRMWARE_VERSION);
    cJSON_AddNumberToObject(root, "timestamp",
        (double)(xTaskGetTickCount() * portTICK_PERIOD_MS) / 1000.0);

    cJSON *readings = cJSON_CreateObject();
    cJSON_AddNumberToObject(readings, "ph", ph);
    cJSON_AddNumberToObject(readings, "tds_ppm", tds);
    cJSON_AddNumberToObject(readings, "turbidity_ntu", ntu);
    cJSON_AddNumberToObject(readings, "temp_c", temp);
    cJSON_AddNumberToObject(readings, "ec_us_cm", ec);
    cJSON_AddItemToObject(root, "readings", readings);

    cJSON_AddNumberToObject(root, "fault_flags", fault_flags);

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (fault_flags_out) *fault_flags_out = fault_flags;
    return json_str;
}

// =====================================================
// ===========  WIFI STA + BACKOFF (Week 5 - Day 2)  ==
// =====================================================
#define WIFI_SSID  "Wokwi-GUEST"
#define WIFI_PASS  ""

#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_RETRY_MIN_MS      1000
#define WIFI_RETRY_MAX_MS      60000

static EventGroupHandle_t wifi_event_group;
static esp_timer_handle_t reconnect_timer;
static int s_retry_delay_ms = WIFI_RETRY_MIN_MS;

static void reconnect_timer_cb(void *arg)
{
    printf("[WiFi] Retrying connection now...\n");
    esp_wifi_connect();
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        printf("[WiFi] Disconnected. Reconnecting in %d ms (backoff)\n", s_retry_delay_ms);

        esp_timer_start_once(reconnect_timer, (uint64_t)s_retry_delay_ms * 1000);

        s_retry_delay_ms *= 2;
        if (s_retry_delay_ms > WIFI_RETRY_MAX_MS) s_retry_delay_ms = WIFI_RETRY_MAX_MS;

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        printf("[WiFi] Connected, IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        s_retry_delay_ms = WIFI_RETRY_MIN_MS;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    esp_timer_create_args_t timer_args = {
        .callback = &reconnect_timer_cb,
        .name = "wifi_reconnect",
    };
    esp_timer_create(&timer_args, &reconnect_timer);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    printf("[WiFi] STA init done, connecting to \"%s\"...\n", WIFI_SSID);
}

// =====================================================
// ============  HTTP POST CLIENT (Week 5 - Day 3)  ===
// =====================================================
// Replace this with your real webhook.site (or other) test URL to
// verify JSON delivery end-to-end. Left as a placeholder here since
// none was provided -- http_post_json() will print
// "[HTTP] POST failed: ..." until a real URL is set, which is
// expected and does not affect any other part of the system.
#define MOCK_SERVER_URL "http://<mock-server-host>:5000/api/readings"
#define HTTP_TIMEOUT_MS 5000

esp_err_t http_post_json(const char *json_str)
{
    esp_http_client_config_t config = {
        .url = MOCK_SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = HTTP_TIMEOUT_MS,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        // Fix: if the URL was unparseable, init can hand back a client
        // that isn't safe to use further -- bail out cleanly instead of
        // calling set_header/set_post_field/perform on it.
        printf("[HTTP] Client init failed (check URL) -- skipping this POST\n");
        return ESP_FAIL;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_str, strlen(json_str));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        printf("[HTTP] POST complete, status = %d\n", status);
    } else {
        printf("[HTTP] POST failed: %s\n", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

// =====================================================
// ============  NETWORK TASK (Week 5 - Day 4)  ========
// =====================================================
// Runs every 10s: takes a sensor snapshot, validates + builds JSON,
// POSTs it if WiFi is up. Also runs the fault-injection demo when
// TEST_FAULT_INJECTION is 1.
#define NETWORK_TASK_STACK    4096
#define NETWORK_TASK_PRIORITY 2
#define NETWORK_PERIOD_MS     10000

static void network_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);   // Fix: register so a stuck network call trips the watchdog
                              // instead of silently deadlocking the whole system forever
    TickType_t last_wake = xTaskGetTickCount();
    int cycle = 0;

    while (1) {
        esp_task_wdt_reset();  // Fix: tell the watchdog "I'm alive" every cycle
        system_readings_t snapshot;
        xSemaphoreTake(readings_mutex, portMAX_DELAY);
        snapshot = g_readings;
        xSemaphoreGive(readings_mutex);

#if TEST_FAULT_INJECTION
        // Deliberately corrupt a LOCAL COPY every 3rd cycle to prove the
        // fault-injection deliverable. Real g_readings is never touched,
        // so the actual sensor pipeline stays valid throughout.
        cycle++;
        if (cycle % 3 == 0) {
            printf("[TEST] Fault injection: forcing pH = 20.0 (out of range)\n");
            snapshot.ph = 20.0f;
        }
#endif

        uint32_t fault_flags;
        char *json = build_json_payload(&snapshot, &fault_flags);

        if (fault_flags != 0) {
            printf("[NET] fault_flags = 0x%02lX -- rejected field(s) present\n",
                   (unsigned long)fault_flags);
        }

        EventBits_t bits = xEventGroupGetBits(wifi_event_group);

        // Fix: MOCK_SERVER_URL still has the "<mock-server-host>" placeholder
        // text -- calling esp_http_client_perform() on that malformed host
        // can hang indefinitely instead of failing with a normal timeout.
        // Skip the POST attempt entirely until a real URL (e.g. webhook.site)
        // is set on Day 3, so the rest of the system never gets stuck
        // waiting on it.
        bool url_is_placeholder = (strstr(MOCK_SERVER_URL, "<mock-server-host>") != NULL);

        if (url_is_placeholder) {
            printf("[NET] Skipping POST -- MOCK_SERVER_URL is still a placeholder: %s\n", json);
        } else if (bits & WIFI_CONNECTED_BIT) {
            // Bus activity mutex: protects the HTTP/WiFi call below from
            // overlapping with a light-sleep window in power_mgmt_task.
            // Safe now that a hung/malformed URL can no longer reach this
            // point and hold the mutex forever.
            xSemaphoreTake(bus_activity_mutex, portMAX_DELAY);
            printf("[NET] Posting: %s\n", json);
            http_post_json(json);
            xSemaphoreGive(bus_activity_mutex);
        } else {
            printf("[NET] WiFi not connected -- skipping this cycle: %s\n", json);
        }

        free(json);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(NETWORK_PERIOD_MS));
    }
}

// =====================================================
// =========  POWER MANAGEMENT TASK (Week 7 - Day 1)  ==
// =====================================================
// Demonstrates the light-sleep pattern required by the roadmap.
// NOTE: light sleep pauses the ENTIRE chip (all tasks), not just this
// one -- that is how the hardware works. If it fires while the OLED
// (I2C) or WiFi/HTTP driver is mid-transaction, the driver's state can
// get corrupted and crash on wakeup (this is a known ESP-IDF failure
// mode, confirmed during testing on this project).
//
// Fix: this task only enters light sleep when it can immediately grab
// bus_activity_mutex (i.e. nothing is currently using I2C or WiFi/HTTP).
// If the bus is busy, it skips sleeping this cycle instead of risking
// a crash. aggregator_task and network_task hold the same mutex while
// they touch the OLED / perform the HTTP POST.
#define POWER_MGMT_STACK     2560
#define POWER_MGMT_PRIORITY  1
#define LIGHT_SLEEP_US        (200 * 1000)   // 200 ms light sleep
#define POWER_MGMT_CYCLE_MS   5000            // attempt this once every 5 s

static void power_mgmt_task(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(POWER_MGMT_CYCLE_MS - (LIGHT_SLEEP_US / 1000)));

        // Try to take the bus mutex with NO wait -- if it's held by the
        // OLED or HTTP code right now, just skip sleeping this cycle.
        if (xSemaphoreTake(bus_activity_mutex, 0) == pdTRUE) {
            esp_sleep_enable_timer_wakeup(LIGHT_SLEEP_US);
            printf("[PWR] Entering light sleep for %d ms\n", LIGHT_SLEEP_US / 1000);
            esp_light_sleep_start();   // chip halts here until the timer fires
            printf("[PWR] Woke up from light sleep\n");
            xSemaphoreGive(bus_activity_mutex);
        } else {
            printf("[PWR] Bus busy (I2C/WiFi) -- skipping light sleep this cycle\n");
        }
    }
}

// =====================================================
// =============  UPTIME LOGGER (Day 2)  ===============
// =====================================================
// Purely for the 60-minute soak test evidence -- prints elapsed
// minutes so the serial log clearly shows continuous operation.
static void uptime_logger_task(void *pvParameters)
{
    int minutes = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000));
        minutes++;
        printf("[UPTIME] System running for %d minute(s), no watchdog reset so far\n", minutes);
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
    ssd1306_display();
    wifi_init_sta();

    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < NUM_CHANNELS; i++) adc1_config_channel_atten(channels[i], ADC_ATTEN_DB_11);

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    calibration_mutex = xSemaphoreCreateMutex();
    readings_mutex = xSemaphoreCreateMutex();
    bus_activity_mutex = xSemaphoreCreateMutex();   // Week 7 fix: guards I2C/WiFi vs light sleep
    sensor_data_queue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(sensor_reading_t));

    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WDT_TIMEOUT_MS,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_err_t wdt_err = esp_task_wdt_init(&twdt_config);
    if (wdt_err == ESP_ERR_INVALID_STATE) {
        printf("TWDT already initialized by IDF defaults -- continuing\n");
    } else if (wdt_err != ESP_OK) {
        printf("TWDT init failed: %d\n", wdt_err);
    }

    if (!calibration_load_from_nvs(&calibration)) {
        calibration_load_defaults(&calibration);
        calibration_save_to_nvs(&calibration);
        printf("Startup: FACTORY DEFAULTS\n");
    } else {
        printf("Startup: LOADED FROM NVS\n");
    }

    xTaskCreate(ph_task,        "ph_task",        PH_TASK_STACK,       NULL, SENSOR_TASK_PRIORITY, &ph_handle);
    xTaskCreate(tds_task,       "tds_task",       TDS_TASK_STACK,      NULL, SENSOR_TASK_PRIORITY, &tds_handle);
    xTaskCreate(turbidity_task, "turbidity_task", TURBIDITY_TASK_STACK,NULL, SENSOR_TASK_PRIORITY, &turbidity_handle);
    xTaskCreate(ec_task,        "ec_task",        EC_TASK_STACK,       NULL, SENSOR_TASK_PRIORITY, &ec_handle);
    xTaskCreate(temp_task,      "temp_task",      TEMP_TASK_STACK,     NULL, SENSOR_TASK_PRIORITY, &temp_handle);
    xTaskCreate(aggregator_task,"aggregator_task",AGGREGATOR_STACK,    NULL, AGGREGATOR_PRIORITY,  &aggregator_handle);
    xTaskCreate(stack_monitor_task, "stack_monitor_task", STACK_MONITOR_STACK, NULL, STACK_MONITOR_PRIORITY, NULL);

    xTaskCreate(network_task, "network_task", NETWORK_TASK_STACK, NULL, NETWORK_TASK_PRIORITY, NULL);

    // Power management demo task -- low priority so it never competes
    // with sensor/aggregator/network tasks for CPU time.
    xTaskCreate(power_mgmt_task, "power_mgmt_task", POWER_MGMT_STACK, NULL, POWER_MGMT_PRIORITY, NULL);

    // NEW (Day 2): logs elapsed minutes for the 60-min soak test evidence
    xTaskCreate(uptime_logger_task, "uptime_logger_task", 2048, NULL, 1, NULL);

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
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
