#ifndef __SENSOR_H__
#define __SENSOR_H__

void setup_sensors();
void do_sensors();

void aht10_printStatus();


typedef enum {
    UNK = 0,
    HEATING,
    MEASURE,
    WAIT_NEXT,
    INIT_FAIL
} AirQualitySensor_state;


#endif // __SENSOR_H__
