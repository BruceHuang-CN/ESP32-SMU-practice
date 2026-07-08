#include "alarm_control.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"

#define LED_ACTIVE_LEVEL 1
#define BUZZER_ACTIVE_LEVEL 0

#define LED_INACTIVE_LEVEL (1 - LED_ACTIVE_LEVEL)
#define BUZZER_INACTIVE_LEVEL (1 - BUZZER_ACTIVE_LEVEL)

static system_state_t *s_state;

static int alarm_on_ms(alarm_level_t alarm)
{
    switch (alarm) {
    case ALARM_CAPSIZE:
        return 100;
    case ALARM_FIRE:
        return 150;
    case ALARM_INTRUSION:
        return 300;
    case ALARM_NONE:
    case ALARM_ENVIRONMENT:
    default:
        return 0;
    }
}

static int alarm_off_ms(alarm_level_t alarm)
{
    switch (alarm) {
    case ALARM_CAPSIZE:
        return 100;
    case ALARM_FIRE:
        return 150;
    case ALARM_INTRUSION:
        return 500;
    case ALARM_NONE:
    case ALARM_ENVIRONMENT:
    default:
        return 500;
    }
}

static void alarm_task(void *arg)
{
    (void)arg;
    while (1) {
        system_state_update_alarm(s_state);
        if (!s_state->alarm_output_enabled ||
            s_state->current_alarm == ALARM_NONE ||
            s_state->current_alarm == ALARM_ENVIRONMENT) {
            gpio_set_level(PIN_ALARM_LED, LED_INACTIVE_LEVEL);
            gpio_set_level(PIN_BUZZER, BUZZER_INACTIVE_LEVEL);
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        int on_ms = alarm_on_ms(s_state->current_alarm);
        int off_ms = alarm_off_ms(s_state->current_alarm);
        gpio_set_level(PIN_ALARM_LED, LED_ACTIVE_LEVEL);
        gpio_set_level(PIN_BUZZER, BUZZER_ACTIVE_LEVEL);
        vTaskDelay(pdMS_TO_TICKS(on_ms));
        gpio_set_level(PIN_ALARM_LED, LED_INACTIVE_LEVEL);
        gpio_set_level(PIN_BUZZER, BUZZER_INACTIVE_LEVEL);
        vTaskDelay(pdMS_TO_TICKS(off_ms));
    }
}

void alarm_control_start(system_state_t *state)
{
    s_state = state;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_ALARM_LED) | (1ULL << PIN_BUZZER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_ALARM_LED, LED_INACTIVE_LEVEL);
    gpio_set_level(PIN_BUZZER, BUZZER_INACTIVE_LEVEL);

    xTaskCreate(alarm_task, "alarm_task", 2048, NULL, 5, NULL);
}
