#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    ALARM_NONE = 0,
    ALARM_ENVIRONMENT,
    ALARM_INTRUSION,
    ALARM_FIRE,
    ALARM_CAPSIZE,
} alarm_level_t;

typedef struct {
    float temperature_c;
    float humidity_percent;
    int gas_ppm;
    bool flame_detected;
    bool pir_detected;
    bool obstacle_detected;
    float angle_x;
    float angle_y;

    bool environment_over_limit;
    bool fire_detected;
    bool capsize_detected;

    bool alarm_output_enabled;
    alarm_level_t current_alarm;
    const char *current_alarm_msg;
} system_state_t;

void system_state_init(system_state_t *state);
void system_state_update_alarm(system_state_t *state);
bool system_state_handle_command(system_state_t *state, const char *command, char *response, size_t response_size);
void system_state_build_json(const system_state_t *state, char *buffer, size_t buffer_size);
void system_state_build_status_text(const system_state_t *state, char *buffer, size_t buffer_size);

#endif
