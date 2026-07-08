#include "temp_humidity.h"

#define TEMP_LIMIT_C 35.0f
#define HUMIDITY_LIMIT_PERCENT 85.0f

bool temp_humidity_parse_serial_frame(const unsigned char *frame, size_t length, temp_humidity_reading_t *reading)
{
    if (frame == 0 || reading == 0 || length != 5) {
        return false;
    }

    unsigned char checksum = (unsigned char)(frame[0] + frame[1] + frame[2] + frame[3]);
    if (checksum != frame[4]) {
        return false;
    }

    reading->temperature_c = (float)frame[0] + ((float)frame[1] / 10.0f);
    reading->humidity_percent = (float)frame[2] + ((float)frame[3] / 10.0f);
    reading->over_limit = reading->temperature_c >= TEMP_LIMIT_C || reading->humidity_percent >= HUMIDITY_LIMIT_PERCENT;
    return true;
}
