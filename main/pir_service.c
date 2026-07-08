#include "pir_service.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"
#include "pir_sensor.h"

static system_state_t *s_state;

static void pir_task(void *arg)
{
    (void)arg;
    bool last_detected = false;

    pir_sensor_init(PIN_PIR);

    while (1) {
        bool detected = pir_sensor_read(PIN_PIR);
        s_state->pir_detected = detected;
        system_state_update_alarm(s_state);

        if (detected != last_detected) {
            printf("[人体热释电] %s\n", detected ? "有人" : "无人");
            last_detected = detected;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void pir_service_start(system_state_t *state)
{
    s_state = state;
    xTaskCreate(pir_task, "pir_task", 2048, NULL, 5, NULL);
}
