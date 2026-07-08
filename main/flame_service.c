#include "flame_service.h"

#include <stdio.h>

#include "flame_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"

static system_state_t *s_state;

static void flame_task(void *arg)
{
    (void)arg;
    bool last_detected = false;

    flame_sensor_init(PIN_FLAME);

    while (1) {
        bool detected = flame_sensor_read(PIN_FLAME);
        s_state->flame_detected = detected;
        s_state->fire_detected = detected;
        system_state_update_alarm(s_state);

        if (detected != last_detected) {
            printf("[火焰] %s\n", detected ? "检测到火焰" : "正常");
            last_detected = detected;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void flame_service_start(system_state_t *state)
{
    s_state = state;
    xTaskCreate(flame_task, "flame_task", 2048, NULL, 5, NULL);
}
