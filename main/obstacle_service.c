#include "obstacle_service.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "obstacle_sensor.h"
#include "pin_config.h"

static system_state_t *s_state;

static void obstacle_task(void *arg)
{
    (void)arg;
    bool last_detected = false;

    obstacle_sensor_init(PIN_OBSTACLE_SIGNAL);

    while (1) {
        bool detected = obstacle_sensor_read(PIN_OBSTACLE_SIGNAL);
        s_state->obstacle_detected = detected;

        if (detected != last_detected) {
            printf("[红外避障] %s\n", detected ? "有障碍物" : "无障碍物");
            last_detected = detected;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void obstacle_service_start(system_state_t *state)
{
    s_state = state;
    xTaskCreate(obstacle_task, "obstacle_task", 2048, NULL, 5, NULL);
}
