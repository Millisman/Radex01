#include "Arduino/Arduino.h"
#include "AppRadio/Radio.h"
#include "AppSensor/Sensor.h"
#include "AppUI/LiquidCrystal.h"
#include "AppUI/UI.h"



void setup(void) {
    Serial.begin(115200);
    Serial.println(F("\n\rRadex"));
    setup_UI();
    //-----------------
    setup_radio();
    setup_sensors();
    //-----------------
    do_radio();
    do_sensors();
}

void loop(void) {
    do_UI();
    do_radio();
    do_sensors();
    // do sleep
    // TODO timer2 - sleep
}
