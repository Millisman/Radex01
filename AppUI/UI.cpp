#pragma GCC diagnostic ignored "-Wunknown-attributes"

#include "UI.h"
#include "LiquidCrystal.h"
#include "AppRadio/Radio.h"

LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

#define REDRAW_UI   2000

// int period = 1000;

// uint16_t amin = 1023;
// uint16_t amax = 0;
// int aval;
// uint16_t samples = 0;

static FILE file_lcd0;

int putc_lcd0(char ch, [[gnu::unused]] FILE *stream) {
    lcd.write(ch);
    return 0;
}

void setup_UI() {
    lcd.begin(20, 4);
    lcd.setCursor(0, 0);
    fdev_setup_stream( &file_lcd0, putc_lcd0,   NULL, _FDEV_SETUP_WRITE);
    stderr = &file_lcd0;
    lcd.print(F("Radex firmware v0.1"));
//     delay(500);
    
}

static uint32_t time_lcd_redraw = 0;

void do_UI() {

    if((millis() - time_lcd_redraw) > REDRAW_UI) {
        time_lcd_redraw = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        
        extern Serial_Num_Struct sn_my; 
        fprintf_P(stderr, PSTR("%6u"), sn_my.DevSerNum);
        
        extern uint16_t GM_TUBE_HV;
        fprintf_P(stderr, PSTR("TUBE DC%4uv"), GM_TUBE_HV);
        lcd.setCursor(0, 1);
        
        fprintf_P(stderr, PSTR("AIR "));
        
        extern uint8_t MP503_air;
        
        switch (MP503_air) {
            case 0: fprintf_P(stderr, PSTR("VERY BAD! [UNK]")); break;
            case 1: fprintf_P(stderr, PSTR("HIGH POLLUTION!")); break;
            case 2: fprintf_P(stderr, PSTR("LOW POLLUTION")); break;
            case 3: fprintf_P(stderr, PSTR("FRESH! GOOD!")); break;
        }
        
        lcd.setCursor(0, 2);
        extern float ahtTemperature;
//         p->Sen_Temp = ahtTemperature;
        extern float ahtHumidity;
        fprintf_P(stderr, PSTR("Tmp %3.1f Hum %3.1f %"), ahtTemperature, ahtHumidity);
        
        lcd.setCursor(0, 3);
        extern uint16_t batt_vcc;
        extern uint16_t protons_on_min;
        fprintf_P(stderr, PSTR("B %4umV GMM %4u"), batt_vcc, protons_on_min);
//         amin = 1023;
//         amax = 0;
//         samples = 0;
    }
/*
    aval = analogRead(PIN_A6);
    if (aval > amax) amax = aval;
    if (aval < amin) amin = aval;
    ++samples;*/
}
