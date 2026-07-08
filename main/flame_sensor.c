#include "flame_sensor.h"

bool flame_sensor_level_to_detected(int level)
{
    return level == 0;
}

#if defined(ESP_PLATFORM)
void flame_sensor_init(gpio_num_t gpio)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
}

bool flame_sensor_read(gpio_num_t gpio)
{
    return flame_sensor_level_to_detected(gpio_get_level(gpio));
}
#else
void flame_sensor_init(gpio_num_t gpio)
{
    (void)gpio;
}

bool flame_sensor_read(gpio_num_t gpio)
{
    (void)gpio;
    return false;
}
#endif
