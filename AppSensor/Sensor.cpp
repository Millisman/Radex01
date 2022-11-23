#pragma GCC diagnostic ignored "-Wunknown-attributes"

#include "Arduino/Arduino.h"
#include "Wire.h"
#include "AHTxx.h"
#include "Air_Quality_Sensor.h" // MP503

#include "Sensor.h"

#define HEATER PIN_A1

AirQualitySensor AirSensor(A7);
AHTxx aht10(AHTXX_ADDRESS_X38, AHT1x_SENSOR); //sensor address, sensor type

                              //to store T/RH result

#define INTERRUPTPIN 2

static uint32_t protons = 0;
void proton_inc() { ++protons; }

void setup_sensors() {
    pinMode(INTERRUPTPIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), proton_inc, FALLING);
    pinMode(HEATER, OUTPUT);
    digitalWrite(HEATER, LOW);

    uint8_t tryes = 5;
    while (aht10.begin() != true) {
        Serial.println(F("AHT1x init FAIL!"));
        --tryes;
        delay(1000);
        if (tryes == 0) break;
    }
}

#define AHT10_INTERVAL       4000
#define AIR_SENSOR_INTERVAL  600000

static float ahtValue;
static float ahtTemperature = -99.0;
static float ahtHumidity = 0;
static uint32_t time_aht10 = 0;
static uint8_t ath_state = 0;
static uint8_t ath_errors = 0;

static AirQualitySensor_state aqss = UNK;
static uint32_t time_airsens = 0;
static bool     init_airsens = false;


void do_sensors() {

    if((millis() - time_aht10) > AHT10_INTERVAL) {
        time_aht10 = millis();
        switch (ath_state) {

            case 0:
                ahtValue = aht10.readTemperature(); //read 6-bytes via I2C, takes 80 milliseconds
                if (ahtValue == AHTXX_ERROR) {
                    ath_state = 2; // reset state
                } else {
                    Serial.print(F("Temperature...: "));
                    Serial.print(ahtValue);
                    Serial.println(F(" +-0.3C"));
                    ahtTemperature = ahtValue;
                    ath_state = 1;
                }
                break;

            case 1:
                ahtValue = aht10.readHumidity(); //read 6-bytes via I2C, takes 80 milliseconds
                if (ahtValue == AHTXX_ERROR) {
                    ath_state = 2; // reset state
                } else {
                    Serial.print(F("Humidity...: "));
                    Serial.print(ahtValue);
                    Serial.println(F(" +-2%"));
                    ahtHumidity = ahtValue;
                    ath_state = 0;
                }
                break;

            case 2:
                aht10_printStatus(); //print command status
                ++ath_errors;
                if (ath_errors > 5) {
                    ath_errors = 0;
                    if (aht10.softReset() == true) {
                        Serial.println(F("aht10 reset success")); //as the last chance to make it alive
                    } else {
                        Serial.println(F("aht10 reset failed"));
                    }
                }
                ath_state = 0;
                break;
        }
    }




    switch (aqss) {
        case UNK:
            Serial.println(F("HEATER ON"));
            digitalWrite(HEATER, HIGH);
            aqss = HEATING;
            time_airsens = millis();
            break;

        case HEATING:
            if((millis() - time_airsens) > 60000) { // 1min
                time_airsens = millis();
                Serial.println(F("HEATER DONE"));

                if (init_airsens == false) {
                    Serial.print(F("Air Sensor init "));
                    if (AirSensor.init() != true) {
                        Serial.println(F("FAIL!"));
                        aqss = INIT_FAIL;
                    } else {
                        Serial.println(F("OK!"));
                        init_airsens = true;
                        aqss = MEASURE;
                    }
                } else {
                    aqss = MEASURE;
                }
            }
            break;

        case INIT_FAIL:
            break;

        case WAIT_NEXT:
            if((millis() - time_airsens) > AIR_SENSOR_INTERVAL) {
                time_airsens = millis();
                aqss = MEASURE;
            }
            break;

        case MEASURE:
            int quality = AirSensor.slope();
            Serial.print(F("Sensor value: "));
            Serial.println(AirSensor.getValue());
            if (quality == AirQualitySensor::FORCE_SIGNAL) {
                Serial.println(F("High pollution! Force signal active."));
            } else if (quality == AirQualitySensor::HIGH_POLLUTION) {
                Serial.println(F("High pollution!"));
            } else if (quality == AirQualitySensor::LOW_POLLUTION) {
                Serial.println(F("Low pollution!"));
            } else if (quality == AirQualitySensor::FRESH_AIR) {
                Serial.println(F("Fresh air."));
            }
            Serial.println(F("HEATER OFF"));
            digitalWrite(HEATER, LOW);
            time_airsens = millis();
            aqss = WAIT_NEXT;
            break;
    }
}

void aht10_printStatus() {
    switch (aht10.getStatus()) {
        case AHTXX_NO_ERROR:
            Serial.println(F("no error"));
            break;
        case AHTXX_BUSY_ERROR:
            Serial.println(F("sensor busy, increase polling time"));
            break;
        case AHTXX_ACK_ERROR:
            Serial.println(F("sensor didn't return ACK, not connected, broken..."));
            break;
        case AHTXX_DATA_ERROR:
            Serial.println(F("received data smaller than expected..."));
            break;
        case AHTXX_CRC8_ERROR:
            Serial.println(F("computed CRC8 not match received CRC8, this feature supported only by AHT2x sensors"));
            break;
        default:
            Serial.println(F("unknown status"));
            break;
    }
}
