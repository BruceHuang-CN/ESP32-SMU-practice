#include "pir_sensor.h"

bool pir_sensor_level_to_detected(int level)
{
    return level == 1;
}

#if defined(ESP_PLATFORM)
void pir_sensor_init(gpio_num_t gpio)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
}

bool pir_sensor_read(gpio_num_t gpio)
{
    return pir_sensor_level_to_detected(gpio_get_level(gpio));
}
#else
void pir_sensor_init(gpio_num_t gpio)
{
    (void)gpio;
}

bool pir_sensor_read(gpio_num_t gpio)
{
    (void)gpio;
    return false;
}
#endif
