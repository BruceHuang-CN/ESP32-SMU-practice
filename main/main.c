#include <stdio.h>
#include <string.h>

#include "alarm_control.h"
#include "flame_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "obstacle_service.h"
#include "pir_service.h"
#include "system_state.h"
#include "temp_humidity.h"
#include "temperature_service.h"
#include "web_server.h"

static system_state_t g_state;

static void handle_temp_humidity_byte(unsigned char byte, unsigned char *window, size_t *count)
{
    temp_humidity_reading_t reading;

    if (*count < 5) {
        window[*count] = byte;
        (*count)++;
    } else {
        memmove(window, window + 1, 4);
        window[4] = byte;
    }

    if (*count == 5 && temp_humidity_parse_serial_frame(window, 5, &reading)) {
        g_state.temperature_c = reading.temperature_c;
        g_state.humidity_percent = reading.humidity_percent;
        g_state.environment_over_limit = reading.over_limit;
        system_state_update_alarm(&g_state);
        printf("[温湿度] 温度: %.1f C, 湿度: %.1f %%, %s\n",
               reading.temperature_c,
               reading.humidity_percent,
               reading.over_limit ? "环境参数超标" : "正常");
        *count = 0;
    }
}

static void serial_command_task(void *arg)
{
    (void)arg;
    char line[64];
    size_t len = 0;
    unsigned char sensor_window[5];
    size_t sensor_count = 0;

    printf("串口命令: STATUS, ALARM_OFF, ALARM_ON, HELP\n");
    printf("温湿度串口帧: 温度高8位 温度低8位 湿度高8位 湿度低8位 校验8位\n");

    while (1) {
        int ch = getchar();
        if (ch == EOF) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        handle_temp_humidity_byte((unsigned char)ch, sensor_window, &sensor_count);

        if (ch == '\r' || ch == '\n') {
            if (len > 0) {
                line[len] = '\0';
                char response[256];
                system_state_handle_command(&g_state, line, response, sizeof(response));
                printf("%s\n", response);
                len = 0;
            }
            continue;
        }

        if (len < sizeof(line) - 1) {
            line[len++] = (char)ch;
        }
    }
}

static void status_report_task(void *arg)
{
    (void)arg;
    char status[512];

    while (1) {
        system_state_update_alarm(&g_state);
        system_state_build_status_text(&g_state, status, sizeof(status));
        printf("[状态] %s\n", status);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    system_state_init(&g_state);

    alarm_control_start(&g_state);
    web_server_start(&g_state);
    temperature_service_start(&g_state);
    obstacle_service_start(&g_state);
    flame_service_start(&g_state);
    pir_service_start(&g_state);

    xTaskCreate(serial_command_task, "serial_command_task", 4096, NULL, 5, NULL);
    xTaskCreate(status_report_task, "status_report_task", 4096, NULL, 4, NULL);
}
