#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "system_state.h"
#include "temp_humidity.h"
#include "dht11.h"
#include "flame_sensor.h"
#include "obstacle_sensor.h"
#include "pir_sensor.h"

static void test_alarm_priority(void)
{
    system_state_t state;
    system_state_init(&state);

    state.pir_detected = true;
    system_state_update_alarm(&state);
    assert(state.current_alarm == ALARM_INTRUSION);
    assert(strcmp(state.current_alarm_msg, "人员入侵报警") == 0);

    state.fire_detected = true;
    system_state_update_alarm(&state);
    assert(state.current_alarm == ALARM_FIRE);
    assert(strcmp(state.current_alarm_msg, "火灾报警") == 0);

    state.capsize_detected = true;
    system_state_update_alarm(&state);
    assert(state.current_alarm == ALARM_CAPSIZE);
    assert(strcmp(state.current_alarm_msg, "船体侧翻危险") == 0);
}

static void test_alarm_output_commands(void)
{
    system_state_t state;
    char response[128];
    system_state_init(&state);

    assert(state.alarm_output_enabled == true);
    assert(system_state_handle_command(&state, "ALARM_OFF", response, sizeof(response)) == true);
    assert(state.alarm_output_enabled == false);
    assert(strstr(response, "声光报警已关闭") != NULL);

    assert(system_state_handle_command(&state, "ALARM_ON", response, sizeof(response)) == true);
    assert(state.alarm_output_enabled == true);
    assert(strstr(response, "声光报警已恢复") != NULL);
}

static void test_status_json_contains_web_fields(void)
{
    system_state_t state;
    char json[512];
    system_state_init(&state);

    state.temperature_c = 26.5f;
    state.humidity_percent = 61.0f;
    state.gas_ppm = 120;
    state.flame_detected = true;
    state.pir_detected = true;
    state.obstacle_detected = false;
    state.angle_x = 2.5f;
    state.angle_y = -1.0f;
    state.fire_detected = true;
    system_state_update_alarm(&state);

    system_state_build_json(&state, json, sizeof(json));
    assert(strstr(json, "\"temp\":\"26.5\"") != NULL);
    assert(strstr(json, "\"humi\":\"61.0\"") != NULL);
    assert(strstr(json, "\"gas\":\"120\"") != NULL);
    assert(strstr(json, "\"flame\":\"检测到火焰\"") != NULL);
    assert(strstr(json, "\"pir\":\"有人\"") != NULL);
    assert(strstr(json, "\"obstacle\":\"无障碍物\"") != NULL);
    assert(strstr(json, "\"alarm\":\"火灾报警\"") != NULL);
    assert(strstr(json, "\"triggered\":1") != NULL);
}

static void test_temp_humidity_serial_frame(void)
{
    temp_humidity_reading_t reading;
    const unsigned char frame[5] = {0x23, 0x00, 0x55, 0x00, 0x78};

    assert(temp_humidity_parse_serial_frame(frame, sizeof(frame), &reading) == true);
    assert(reading.temperature_c == 35.0f);
    assert(reading.humidity_percent == 85.0f);
    assert(reading.over_limit == true);
}

static void test_temp_humidity_rejects_bad_checksum(void)
{
    temp_humidity_reading_t reading;
    const unsigned char frame[5] = {0x1A, 0x00, 0x3C, 0x00, 0x00};

    assert(temp_humidity_parse_serial_frame(frame, sizeof(frame), &reading) == false);
}

static void test_dht11_parse_bytes(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;
    const unsigned char bytes[5] = {60, 0, 28, 0, 88};

    assert(dht11_parse_bytes(bytes, &temperature, &humidity) == true);
    assert(temperature == 28.0f);
    assert(humidity == 60.0f);
}

static void test_dht11_rejects_bad_checksum(void)
{
    float temperature = 0.0f;
    float humidity = 0.0f;
    const unsigned char bytes[5] = {60, 0, 28, 0, 0};

    assert(dht11_parse_bytes(bytes, &temperature, &humidity) == false);
}

static void test_obstacle_low_level_means_detected(void)
{
    assert(obstacle_sensor_level_to_detected(0) == true);
    assert(obstacle_sensor_level_to_detected(1) == false);
}

static void test_flame_low_level_means_detected(void)
{
    assert(flame_sensor_level_to_detected(0) == true);
    assert(flame_sensor_level_to_detected(1) == false);
}

static void test_pir_high_level_means_detected(void)
{
    assert(pir_sensor_level_to_detected(1) == true);
    assert(pir_sensor_level_to_detected(0) == false);
}

int main(void)
{
    test_alarm_priority();
    test_alarm_output_commands();
    test_status_json_contains_web_fields();
    test_temp_humidity_serial_frame();
    test_temp_humidity_rejects_bad_checksum();
    test_dht11_parse_bytes();
    test_dht11_rejects_bad_checksum();
    test_obstacle_low_level_means_detected();
    test_flame_low_level_means_detected();
    test_pir_high_level_means_detected();
    return 0;
}
