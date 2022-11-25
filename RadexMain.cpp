#include "Arduino/Arduino.h"
#include "AppRadio/Radio.h"
#include "AppSensor/Sensor.h"
#include "AppUI/LiquidCrystal.h"
#include "AppUI/UI.h"
#include "AppRadio/printf.h"



void setup(void) {
    Serial.begin(115200);
    printf_begin();
    Serial.println(F("\n\rRadex"));
    srand(analogRead(PIN_A0));
    srand(analogRead(PIN_A1));
    srand(analogRead(PIN_A2));
    srand(analogRead(PIN_A3));
    srand(analogRead(PIN_A4));
    srand(analogRead(PIN_A5));
    srand(analogRead(PIN_A6));
    srand(analogRead(PIN_A7));
    srand(ReadVcc());
    //-----------------
    setup_UI();
    setup_radio();
    setup_sensors();
}

void loop(void) {
    do_radio();
    do_sensors();
    do_UI();
    // do sleep
    // TODO timer2 - sleep
}
