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

// =====================================================
// ============  I2C / OLED CONFIG (Week 3)  ==========
// =====================================================
#define I2C_MASTER_SDA_IO   21
#define I2C_MASTER_SCL_IO   22
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
// Now 5 separate tasks will read this "calibration" struct,
// and a button press will write to it. When two or more tasks
// access the same variable simultaneously, a race condition can occur
// (one task might read incomplete data while another is writing).
// Therefore, we use a "mutex" (lock): whichever task accesses the
// calibration data must acquire the lock first, perform its operations,
// and then release the lock.
static all_calibration_t calibration;
static SemaphoreHandle_t calibration_mutex;
static float ph_v_ref = 1.65f;
static float water_temp_c = 25.0f;  // updated by temp_task every cycle

// =====================================================
// =========  QUEUE + AGGREGATOR (Week 4 - Day 2)  ====
// =====================================================
// A queue acts like a "mailbox" that safely transfers data between
// two tasks (FreeRTOS itself prevents race conditions when using queues,
// so a mutex is not needed here). Each sensor task packages its reading
// into a struct and posts it to the queue (xQueueSend). On the receiving
// end, a single "aggregator" task fetches items from the queue
// (xQueueReceive) and combines them to render on the OLED + serial console.
//
// We use a single shared queue where each reading includes its sensor_id tag.
// While creating 5 separate queues was an option, a single queue is simpler
// and sufficient for this scale (5 tasks).
typedef enum { SENSOR_PH = 0, SENSOR_TDS, SENSOR_TURBIDITY, SENSOR_EC, SENSOR_TEMP, SENSOR_COUNT } sensor_id_t;

typedef struct {
    sensor_id_t id;
    float value;
} sensor_reading_t;

#define SENSOR_QUEUE_LENGTH 10  // room for a couple of readings per sensor before aggregator catches up
static QueueHandle_t sensor_data_queue;

// =====================================================
// ============  FREERTOS TASKS (Week 4 - Day 1) ======
// =====================================================
// A FreeRTOS "task" is an independent function running on its own stack,
// functioning like a mini-program inside the system. The scheduler allocates
// CPU time based on priority. The parameters for xTaskCreate() are:
//   1) Task function pointer
//   2) Name (for debugging)
//   3) Stack size (in bytes for ESP-IDF)
//   4) Parameter pointer (passing NULL here)
//   5) Priority (higher number = higher priority)
//   6) Task handle (reference for later control, used in Day 3/4)
//
// Each task maintains its timing using vTaskDelayUntil(), which is preferred
// over vTaskDelay() because the next wake-up time is calculated from
// "last wake time + period", preventing timing drift even if processing takes extra time.

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

        sensor_reading_t reading = { .id = SENSOR_PH, .value = ph_value };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

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

        sensor_reading_t reading = { .id = SENSOR_TDS, .value = tds_ppm };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));  // every 2s
    }
}

static void turbidity_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        int raw = read_adc_filtered(channels[2]);
        float turb_ntu = turbidity_calculate(adc_raw_to_voltage(raw));

        sensor_reading_t reading = { .id = SENSOR_TURBIDITY, .value = turb_ntu };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(5000));  // every 5s
    }
}

static void ec_task(void *pvParameters)
{
    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        int raw = read_adc_filtered(channels[3]);
        float ec_value = adc_raw_to_voltage(raw) * 1000.0f;

        sensor_reading_t reading = { .id = SENSOR_EC, .value = ec_value };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

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

        sensor_reading_t reading = { .id = SENSOR_TEMP, .value = water_temp_c };
        xQueueSend(sensor_data_queue, &reading, pdMS_TO_TICKS(100));

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));  // every 1s
    }
}

// =====================================================
// ==============  AGGREGATOR TASK (Week 4 - Day 2)  ==
// =====================================================
// This is the single task that fetches readings from the queue.
// Whenever a new reading arrives, it updates the corresponding sensor
// value in the local "latest[]" array, then writes the complete combined
// snapshot to the OLED and serial log. This ensures the OLED (a shared resource)
// is accessed by only this task, eliminating the need for a mutex here.
#define AGGREGATOR_STACK    4096  // bumped from 3072 -- it was sitting at only ~34.5% free, too close to the 20% margin
#define AGGREGATOR_PRIORITY 3   // slightly higher than sensor tasks (priority 2) to drain the queue quickly

static const char *sensor_names[SENSOR_COUNT] = { "pH", "TDS", "NTU", "EC", "TMP" };

// =====================================================
// ===========  TASK WATCHDOG TIMER (Week 4 - Day 3) ==
// =====================================================
// The watchdog is a safety timer: if a monitored task does not send a
// "still alive" signal (esp_task_wdt_reset) within X seconds, the watchdog
// assumes the task has hung or deadlocked and forces a system reboot
// (rebooting is preferable to an indefinite freeze). We monitor only the
// AGGREGATOR task since it is the central node—if it freezes, all display
// and logging functionality stops.
#define WDT_TIMEOUT_MS 5000  // aggregator loops every ~1s, leaving ample buffer time

static TaskHandle_t aggregator_handle = NULL;
static TaskHandle_t ph_handle = NULL;
static TaskHandle_t tds_handle = NULL;
static TaskHandle_t turbidity_handle = NULL;
static TaskHandle_t ec_handle = NULL;
static TaskHandle_t temp_handle = NULL;

// =====================================================
// ========  STACK HIGH-WATER MARK CHECK (Week 4 - Day 4) =
// =====================================================
// The stack space allocated to a task (3rd parameter in xTaskCreate)
// is rarely fully used at all times. FreeRTOS tracks the minimum amount of
// remaining unused stack memory during execution—this is called the
// "high water mark" (indicating how close memory usage came to the limit).
//
// uxTaskGetStackHighWaterMark(handle) returns the value in WORDS
// (1 word = 4 bytes on ESP32, as it is a 32-bit architecture).
// We convert this to bytes and evaluate it against the total allocated stack size.
// If the remaining free stack falls below 20% of the allocated allocation,
// a warning is issued as it indicates proximity to a stack overflow,
// which could lead to crashes or memory corruption.
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

// A separate, lightweight task that prints the memory usage report every 10 seconds.
// Kept separate so it does not interfere with sensor or aggregator execution timing.
#define STACK_MONITOR_STACK    4096  // bumped from 2048 -- printf() with %.1f floats needs more room than expected
#define STACK_MONITOR_PRIORITY 1  // lowest priority, dedicated strictly to periodic reporting

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
    bool has_value[SENSOR_COUNT] = {false};
    sensor_reading_t reading;

    // Subscribe THIS task to the watchdog right at the start, before
    // anything else runs. NULL means "the task calling this function" --
    // using our own handle here removes any dependency on app_main having
    // already called esp_task_wdt_add() by the time we get scheduled.
    esp_task_wdt_add(NULL);

    while (1) {
        esp_task_wdt_reset();  // "I'm alive" signal — must be called well within WDT_TIMEOUT_MS

        // Block until at least one reading arrives (max 1s wait so the
        // OLED still refreshes even if a slow sensor hasn't reported yet).
        if (xQueueReceive(sensor_data_queue, &reading, pdMS_TO_TICKS(1000)) == pdTRUE) {
            latest[reading.id] = reading.value;
            has_value[reading.id] = true;
            printf("[AGG] updated %s = %.2f (tick=%lu)\n",
                   sensor_names[reading.id], reading.value, (unsigned long)xTaskGetTickCount());
        }

        // Drain any extra readings already waiting, so the queue never backs up.
        while (xQueueReceive(sensor_data_queue, &reading, 0) == pdTRUE) {
            latest[reading.id] = reading.value;
            has_value[reading.id] = true;
            printf("[AGG] updated %s = %.2f (tick=%lu)\n",
                   sensor_names[reading.id], reading.value, (unsigned long)xTaskGetTickCount());
        }

        char line[16];
        ssd1306_clear();
        snprintf(line, sizeof(line), "P%.2f", latest[SENSOR_PH]);       ssd1306_draw_string(0, 0, line);
        snprintf(line, sizeof(line), "D%.0f", latest[SENSOR_TDS]);      ssd1306_draw_string(1, 0, line);
        snprintf(line, sizeof(line), "N%.1f", latest[SENSOR_TURBIDITY]);ssd1306_draw_string(2, 0, line);
        snprintf(line, sizeof(line), "E%.0f", latest[SENSOR_EC]);       ssd1306_draw_string(3, 0, line);
        snprintf(line, sizeof(line), "T%.1f", latest[SENSOR_TEMP]);     ssd1306_draw_string(4, 0, line);
        ssd1306_display();

        printf("[AGG] pH=%.2f TDS=%.0f NTU=%.1f EC=%.0f Temp=%.1f\n",
               latest[SENSOR_PH], latest[SENSOR_TDS], latest[SENSOR_TURBIDITY],
               latest[SENSOR_EC], latest[SENSOR_TEMP]);
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

    adc1_config_width(ADC_WIDTH_BIT_12);
    for (int i = 0; i < NUM_CHANNELS; i++) adc1_config_channel_atten(channels[i], ADC_ATTEN_DB_11);

    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

    calibration_mutex = xSemaphoreCreateMutex();
    sensor_data_queue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(sensor_reading_t));

    // Watchdog must be initialized BEFORE any task that will call
    // esp_task_wdt_reset() is created — otherwise a high-priority task
    // (like aggregator_task) can preempt app_main and call reset() before
    // it's even registered, giving a "task not found" error.
    //
    // Newer ESP-IDF (v5.x) uses esp_task_wdt_config_t. If your Wokwi
    // project pulls an older IDF (v4.x), replace this block with:
    //     esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, true);
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = WDT_TIMEOUT_MS,
        .idle_core_mask = 0,
        .trigger_panic = true,   // reboot on timeout instead of just logging a warning
    };
    esp_err_t wdt_err = esp_task_wdt_init(&twdt_config);
    if (wdt_err == ESP_ERR_INVALID_STATE) {
        // Some ESP-IDF targets (e.g. ESP32-S3 with default sdkconfig)
        // already auto-initialize the TWDT at boot. That's fine — we
        // just skip re-init and go straight to subscribing our task.
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

    // Every sensor is now executed in its own independent task.
    // No single task waits for another — the FreeRTOS scheduler
    // dynamically handles CPU time allocation across tasks.
    xTaskCreate(ph_task,        "ph_task",        PH_TASK_STACK,       NULL, SENSOR_TASK_PRIORITY, &ph_handle);
    xTaskCreate(tds_task,       "tds_task",       TDS_TASK_STACK,      NULL, SENSOR_TASK_PRIORITY, &tds_handle);
    xTaskCreate(turbidity_task, "turbidity_task", TURBIDITY_TASK_STACK,NULL, SENSOR_TASK_PRIORITY, &turbidity_handle);
    xTaskCreate(ec_task,        "ec_task",        EC_TASK_STACK,       NULL, SENSOR_TASK_PRIORITY, &ec_handle);
    xTaskCreate(temp_task,      "temp_task",      TEMP_TASK_STACK,     NULL, SENSOR_TASK_PRIORITY, &temp_handle);
    xTaskCreate(aggregator_task,"aggregator_task",AGGREGATOR_STACK,    NULL, AGGREGATOR_PRIORITY,  &aggregator_handle);
    xTaskCreate(stack_monitor_task, "stack_monitor_task", STACK_MONITOR_STACK, NULL, STACK_MONITOR_PRIORITY, NULL);

    // app_main itself runs as a FreeRTOS task (slightly above idle priority).
    // We keep it active to process button interactions, maintaining the same
    // architecture structure introduced in Week 3 and used through Day 2+.
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
