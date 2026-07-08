idf_component_register(SRCS "main.c" "system_state.c" "web_server.c" "alarm_control.c" "temp_humidity.c" "dht11.c" "temperature_service.c" "obstacle_sensor.c" "obstacle_service.c" "flame_sensor.c" "flame_service.c" "pir_sensor.c" "pir_service.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_http_server esp_netif esp_event nvs_flash driver)
