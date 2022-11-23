#include "Arduino/Arduino.h"
#include "AppRadio/Radio.h"
#include "AppSensor/Sensor.h"
#include "AppUI/LiquidCrystal.h"



void setup(void) {
    Serial.begin(115200);
    Serial.println(F("\n\rRadex"));
    //-----------------
    setup_radio();
    setup_sensors();
    //-----------------
    do_radio();
    do_sensors();
}

// static uint32_t time_now = 0;

void loop(void) {

//     if((millis() - time_now) > 1000) {
//         time_now = millis();
//         do_radio();
//         do_sensors();
//     }
    
    do_radio();
    do_sensors();
    
    
    
    // do sleep
    // TODO timer2 - sleep
}
