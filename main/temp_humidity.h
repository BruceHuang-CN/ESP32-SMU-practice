#ifndef TEMP_HUMIDITY_H
#define TEMP_HUMIDITY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float temperature_c;
    float humidity_percent;
    bool over_limit;
} temp_humidity_reading_t;

bool temp_humidity_parse_serial_frame(const unsigned char *frame, size_t length, temp_humidity_reading_t *reading);

#endif
