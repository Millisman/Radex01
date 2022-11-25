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

uint32_t protons = 0;
uint16_t protons_on_min = 0;
void proton_inc() { ++protons; }

uint16_t batt_vcc;

void setup_sensors() {
    pinMode(INTERRUPTPIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), proton_inc, FALLING);
    pinMode(HEATER, OUTPUT);
    digitalWrite(HEATER, LOW);
    batt_vcc = ReadVcc();
    
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

float ahtValue;
float ahtTemperature = -99.0;
float ahtHumidity = 0;
uint32_t time_aht10 = 0;
uint8_t ath_state = 0;
uint8_t ath_errors = 0;

AirQualitySensor_state aqss = UNK;
uint32_t time_airsens = 0;
bool     init_airsens = false;

uint32_t time_60sec = 0;
uint32_t protons_old_60s = 0;


uint32_t time_1sec = 0;

uint8_t MP503_air = 0; // FORCE_SIGNAL

uint16_t amin = 1023;
uint16_t amax = 0;
int aval;
uint16_t samples = 0;

uint16_t GM_TUBE_HV = 0;

void do_sensors() {
    
    aval = analogRead(PIN_A6);
    if (aval > amax) amax = aval;
    if (aval < amin) amin = aval;
    ++samples;
    

    if((millis() - time_1sec) > 1000) { // sec
        time_1sec = millis();
        batt_vcc = ReadVcc();
        
//         uint16_t Adc_HV = analogRead(PIN_A6);
        
         //(Adc_HV/1024)
        Serial.print(F("batt_vcc...: "));
        Serial.println(batt_vcc);
        
        uint16_t avg_v = (amax + amin) / 2;

        uint16_t Mv_HV_ADC = ((uint32_t)avg_v * (uint32_t)batt_vcc) / (1024 * 22 ); // (22 = 220K)
        GM_TUBE_HV = ((uint32_t)5022 * (uint32_t)Mv_HV_ADC) / 1000; // 5022 = 50M + 220K
        Serial.print(F("GM_TUBE_HV...: "));
        Serial.println(GM_TUBE_HV);
        
        
        
        amin = 1023;
        amax = 0;
        samples = 0;
        
    }

    if((millis() - time_60sec) > 60000) { // min
        time_60sec = millis();
        protons_on_min = protons - protons_old_60s;
        protons_old_60s = protons;
    }

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
            if (quality != -1) { MP503_air = quality; }
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


uint16_t ReadVcc() {
    // analogReference(INTERNAL); // INTERNAL or EXTERNAL
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // Read 1.1V reference against AVcc
    _delay_us(500);
    uint8_t old = ADCSRA;
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADSC);
//     _delay_us(10);
    while (bit_is_set(ADCSRA,ADSC));
    uint16_t mV = ADCL;
    mV |= ADCH<<8;
    mV = 1126400L / mV; // 1100 * 1024
    ADCSRA = old;
    return mV;
}
