#include "system_state.h"

#include <stdio.h>
#include <string.h>

static const char *alarm_message(alarm_level_t alarm)
{
    switch (alarm) {
    case ALARM_CAPSIZE:
        return "船体侧翻危险";
    case ALARM_FIRE:
        return "火灾报警";
    case ALARM_INTRUSION:
        return "人员入侵报警";
    case ALARM_ENVIRONMENT:
        return "环境参数超标报警";
    case ALARM_NONE:
    default:
        return "系统正常";
    }
}

void system_state_init(system_state_t *state)
{
    memset(state, 0, sizeof(*state));
    state->alarm_output_enabled = true;
    state->current_alarm = ALARM_NONE;
    state->current_alarm_msg = alarm_message(ALARM_NONE);
}

void system_state_update_alarm(system_state_t *state)
{
    if (state->capsize_detected) {
        state->current_alarm = ALARM_CAPSIZE;
    } else if (state->fire_detected || state->flame_detected || state->gas_ppm >= 300) {
        state->current_alarm = ALARM_FIRE;
    } else if (state->pir_detected) {
        state->current_alarm = ALARM_INTRUSION;
    } else if (state->environment_over_limit || state->temperature_c >= 35.0f || state->humidity_percent >= 85.0f) {
        state->current_alarm = ALARM_ENVIRONMENT;
    } else {
        state->current_alarm = ALARM_NONE;
    }

    state->current_alarm_msg = alarm_message(state->current_alarm);
}

bool system_state_handle_command(system_state_t *state, const char *command, char *response, size_t response_size)
{
    if (strcmp(command, "ALARM_OFF") == 0) {
        state->alarm_output_enabled = false;
        snprintf(response, response_size, "声光报警已关闭");
        return true;
    }

    if (strcmp(command, "ALARM_ON") == 0) {
        state->alarm_output_enabled = true;
        snprintf(response, response_size, "声光报警已恢复");
        return true;
    }

    if (strcmp(command, "STATUS") == 0) {
        system_state_build_status_text(state, response, response_size);
        return true;
    }

    if (strcmp(command, "HELP") == 0) {
        snprintf(response, response_size, "支持命令: STATUS, ALARM_OFF, ALARM_ON, HELP");
        return true;
    }

    snprintf(response, response_size, "未知命令: %s", command);
    return false;
}

void system_state_build_json(const system_state_t *state, char *buffer, size_t buffer_size)
{
    snprintf(buffer, buffer_size,
             "{\"temp\":\"%.1f\",\"humi\":\"%.1f\",\"gas\":\"%d\"," 
             "\"flame\":\"%s\",\"pir\":\"%s\",\"obstacle\":\"%s\"," 
             "\"angleX\":\"%.1f\",\"angleY\":\"%.1f\",\"alarm\":\"%s\",\"triggered\":%d}",
             state->temperature_c,
             state->humidity_percent,
             state->gas_ppm,
             state->flame_detected ? "检测到火焰" : "正常",
             state->pir_detected ? "有人" : "无人",
             state->obstacle_detected ? "有障碍物" : "无障碍物",
             state->angle_x,
             state->angle_y,
             state->current_alarm_msg,
             state->current_alarm != ALARM_NONE ? 1 : 0);
}

void system_state_build_status_text(const system_state_t *state, char *buffer, size_t buffer_size)
{
    snprintf(buffer, buffer_size,
             "温度: %.1f C, 湿度: %.1f %%, MQ-2: %d ppm, 火焰: %s, 人体: %s, 障碍物: %s, "
             "X倾角: %.1f deg, Y倾角: %.1f deg, 当前报警: %s, 声光输出: %s",
             state->temperature_c,
             state->humidity_percent,
             state->gas_ppm,
             state->flame_detected ? "检测到火焰" : "正常",
             state->pir_detected ? "有人" : "无人",
             state->obstacle_detected ? "有障碍物" : "无障碍物",
             state->angle_x,
             state->angle_y,
             state->current_alarm_msg,
             state->alarm_output_enabled ? "开启" : "关闭");
}
