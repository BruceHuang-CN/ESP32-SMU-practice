#include "dht11.h"

#if defined(ESP_PLATFORM)
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

bool dht11_parse_bytes(const unsigned char bytes[5], float *temperature_c, float *humidity_percent)
{
    unsigned char checksum = (unsigned char)(bytes[0] + bytes[1] + bytes[2] + bytes[3]);
    if (checksum != bytes[4]) {
        return false;
    }

    *humidity_percent = (float)bytes[0] + ((float)bytes[1] / 10.0f);
    *temperature_c = (float)bytes[2] + ((float)bytes[3] / 10.0f);
    return true;
}

#if defined(ESP_PLATFORM)
static bool wait_for_level(gpio_num_t gpio, int level, int timeout_us, int *duration_us)
{
    int elapsed = 0;
    while (gpio_get_level(gpio) != level) {
        if (++elapsed >= timeout_us) {
            return false;
        }
        esp_rom_delay_us(1);
    }

    if (duration_us != 0) {
        *duration_us = elapsed;
    }
    return true;
}

void dht11_init(gpio_num_t gpio)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(gpio, 1);
}

bool dht11_read(gpio_num_t gpio, dht11_reading_t *reading)
{
    unsigned char bytes[5] = {0};

    gpio_set_direction(gpio, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(gpio, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(gpio, GPIO_MODE_INPUT);

    if (!wait_for_level(gpio, 0, 100, 0)) {
        return false;
    }
    if (!wait_for_level(gpio, 1, 100, 0)) {
        return false;
    }
    if (!wait_for_level(gpio, 0, 100, 0)) {
        return false;
    }

    for (int bit = 0; bit < 40; bit++) {
        int high_us = 0;
        if (!wait_for_level(gpio, 1, 80, 0)) {
            return false;
        }
        if (!wait_for_level(gpio, 0, 120, &high_us)) {
            return false;
        }

        bytes[bit / 8] <<= 1;
        if (high_us > 40) {
            bytes[bit / 8] |= 1;
        }
    }

    return dht11_parse_bytes(bytes, &reading->temperature_c, &reading->humidity_percent);
}
#else
void dht11_init(gpio_num_t gpio)
{
    (void)gpio;
}

bool dht11_read(gpio_num_t gpio, dht11_reading_t *reading)
{
    (void)gpio;
    (void)reading;
    return false;
}
#endif
