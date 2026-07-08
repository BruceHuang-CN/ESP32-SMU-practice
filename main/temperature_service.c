#include "temperature_service.h"

#include <stdio.h>

#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"

#define TEMP_LIMIT_C 35.0f
#define HUMIDITY_LIMIT_PERCENT 85.0f

static system_state_t *s_state;

static void dht11_task(void *arg)
{
    (void)arg;
    dht11_reading_t reading;

    dht11_init(PIN_DHT11);

    while (1) {
        if (dht11_read(PIN_DHT11, &reading)) {
            s_state->temperature_c = reading.temperature_c;
            s_state->humidity_percent = reading.humidity_percent;
            s_state->environment_over_limit = reading.temperature_c >= TEMP_LIMIT_C ||
                                              reading.humidity_percent >= HUMIDITY_LIMIT_PERCENT;
            system_state_update_alarm(s_state);
            printf("[DHT11] 温度: %.1f C, 湿度: %.1f %%, %s\n",
                   reading.temperature_c,
                   reading.humidity_percent,
                   s_state->environment_over_limit ? "环境参数超标" : "正常");
        } else {
            printf("[DHT11] 读取失败，请检查 GPIO4 接线、上拉电阻和供电\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void temperature_service_start(system_state_t *state)
{
    s_state = state;
    xTaskCreate(dht11_task, "dht11_task", 4096, NULL, 5, NULL);
}
