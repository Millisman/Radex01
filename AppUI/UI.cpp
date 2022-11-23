#include "UI.h"
#include "LiquidCrystal.h"

LiquidCrystal lcd(3, 4, 5, 6, 7, 8);



// int period = 1000;

uint16_t amin = 1023;
uint16_t amax = 0;
int aval;
uint16_t samples = 0;



void setup_UI() {
    lcd.begin(20, 4);
}

void do_UI() {
    lcd.setCursor(0, 1);
    lcd.print(F("HV min "));
    lcd.print(amin);
    lcd.print(F(" max "));
    lcd.print(amax);
    lcd.setCursor(0, 2);
    lcd.print(F(" samples "));
    lcd.print(millis());
    Serial.print(F("HV ADC min: "));
    Serial.print(amin);
    Serial.print(F(" max "));
    Serial.print(amax);
    Serial.print(F(" samples "));
    Serial.println(samples);
    
    amin = 1023;
    amax = 0;
    samples = 0;
    
    Serial.print(F("HV ADC 6: "));
    Serial.println(analogRead(PIN_A6));
    Serial.print(F("Protons: "));
//     Serial.println(protons);
    //     Vref = 4550mV
    //     ADC = 301 (0..1023)
    //     HVDC = 393V
    
    aval = analogRead(PIN_A6);
    if (aval > amax) amax = aval;
    if (aval < amin) amin = aval;
    ++samples;
    lcd.setCursor(0, 3);
}
