#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

// -------------------- I2C CONFIG --------------------
#define I2C_MASTER_SDA_IO   8
#define I2C_MASTER_SCL_IO   9
#define I2C_MASTER_FREQ_HZ  400000
#define I2C_MASTER_PORT     I2C_NUM_0
#define OLED_I2C_ADDR       0x3C

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   (OLED_HEIGHT / 8)   // 8 pages

static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES]; // 1 byte = 1 column of 8 pixels

// =====================================================
// ================  I2C MASTER INIT  =================
// =====================================================
// Ye function ESP32 ko "I2C master" bana deta hai — SDA/SCL pins
// set karta hai aur communication speed (400kHz) define karta hai.
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

// Chhota helper: OLED ko ek ya zyada bytes bhejta hai, saath mein
// control byte (0x00=command, 0x40=data/pixels) bhi bhejta hai.
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

// =====================================================
// ================  SSD1306 INIT  ====================
// =====================================================
// Ye standard init sequence hai jo har SSD1306 datasheet mein hoti hai —
// display off karke, clock/multiplex/charge-pump set karke, aakhir mein
// display on karta hai.
void ssd1306_init(void)
{
    uint8_t init_cmds[] = {
        0xAE,             // display OFF
        0xD5, 0x80,       // clock divide ratio
        0xA8, 0x3F,       // multiplex ratio (64-1)
        0xD3, 0x00,       // display offset = 0
        0x40,             // start line = 0
        0x8D, 0x14,       // enable charge pump
        0x20, 0x00,       // memory addressing mode = horizontal
        0xA1,             // segment remap
        0xC8,             // COM scan direction reversed
        0xDA, 0x12,       // COM pins config
        0x81, 0xCF,       // contrast
        0xD9, 0xF1,       // pre-charge period
        0xDB, 0x40,       // VCOMH deselect level
        0xA4,             // resume to RAM content
        0xA6,             // normal display (not inverted)
        0xAF              // display ON
    };
    ssd1306_send(0x00, init_cmds, sizeof(init_cmds));
}

// Poora framebuffer OLED ko bhejta hai (sirf jab hum screen update
// karna chahte hain tabhi call hoga — taake I2C traffic kam rahe).
void ssd1306_display(void)
{
    uint8_t col_cmd[] = { 0x21, 0, OLED_WIDTH - 1 };   // column range
    uint8_t page_cmd[] = { 0x22, 0, OLED_PAGES - 1 };  // page range
    ssd1306_send(0x00, col_cmd, sizeof(col_cmd));
    ssd1306_send(0x00, page_cmd, sizeof(page_cmd));
    ssd1306_send(0x40, framebuffer, sizeof(framebuffer));
}

void ssd1306_clear(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

// =====================================================
// ==============  MINIMAL 5x7 FONT  ==================
// =====================================================
// Har character 5 bytes ka hota hai (5 columns), har byte ke 7 bits
// use hote hain (bit0 = sabse upar wala pixel, bit6 = sabse neeche).
// Sirf wahi characters define kiye hain jo hume chahiye.
typedef struct { char c; uint8_t col[5]; } glyph_t;

static const glyph_t font_table[] = {
    {'0', {0x3E,0x51,0x49,0x45,0x3E}},
    {'1', {0x00,0x42,0x7F,0x40,0x00}},
    {'2', {0x62,0x51,0x49,0x49,0x46}},
    {'3', {0x22,0x41,0x49,0x49,0x36}},
    {'4', {0x18,0x14,0x12,0x7F,0x10}},
    {'5', {0x27,0x45,0x45,0x45,0x39}},
    {'6', {0x3C,0x4A,0x49,0x49,0x30}},
    {'7', {0x01,0x71,0x09,0x05,0x03}},
    {'8', {0x36,0x49,0x49,0x49,0x36}},
    {'9', {0x06,0x49,0x49,0x29,0x1E}},
    {'A', {0x7E,0x11,0x11,0x11,0x7E}},   
    {'P', {0x7F,0x09,0x09,0x09,0x06}},
    {'D', {0x7F,0x41,0x41,0x22,0x1C}},
    {'N', {0x7F,0x04,0x08,0x10,0x7F}},
    {'E', {0x7F,0x49,0x49,0x49,0x41}},
    {'T', {0x01,0x01,0x7F,0x01,0x01}},
    {'O', {0x3E,0x41,0x41,0x41,0x3E}},
    {'K', {0x7F,0x08,0x14,0x22,0x41}},
    {'R', {0x7F,0x09,0x19,0x29,0x46}},
    {'Q', {0x3E,0x41,0x51,0x21,0x5E}},   
    {'U', {0x3F,0x40,0x40,0x40,0x3F}},   
    {'L', {0x7F,0x40,0x40,0x40,0x40}},   
    {'S', {0x46,0x49,0x49,0x49,0x31}},   
    {'.', {0x00,0x00,0x40,0x40,0x00}},
    {':', {0x00,0x00,0x14,0x00,0x00}},
    {'-', {0x00,0x08,0x08,0x08,0x00}},
    {' ', {0x00,0x00,0x00,0x00,0x00}},
};
#define FONT_COUNT (sizeof(font_table)/sizeof(font_table[0]))

static const uint8_t *get_glyph(char c)
{
    for (size_t i = 0; i < FONT_COUNT; i++)
        if (font_table[i].c == c) return font_table[i].col;
    return font_table[FONT_COUNT - 1].col; // fallback: space
}

// Ek character ko given page (0-7) aur column position pe draw karta hai
void ssd1306_draw_char(int page, int col, char c)
{
    const uint8_t *glyph = get_glyph(c);
    for (int i = 0; i < 5; i++) {
        if (col + i < OLED_WIDTH)
            framebuffer[page * OLED_WIDTH + col + i] = glyph[i];
    }
}

// Poori string draw karta hai (har char ke baad 1 pixel gap)
void ssd1306_draw_string(int page, int col, const char *str)
{
    int x = col;
    while (*str) {
        ssd1306_draw_char(page, x, *str);
        x += 6; // 5 pixels char + 1 pixel gap
        str++;
    }
}

// -------------------- MAIN --------------------
void app_main(void)
{
    i2c_master_init();
    ssd1306_init();

    ssd1306_clear();
    ssd1306_draw_string(0, 0, "AQUAPULSE");
    ssd1306_draw_string(2, 0, "T-2 OK");   // I2C test confirmation line
    ssd1306_display();

    printf("SSD1306 init + I2C test string sent.\n");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
