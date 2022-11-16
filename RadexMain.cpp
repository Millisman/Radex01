#include "Arduino/Arduino.h"
#include "Arduino/HardwareSerial.h"

#include "AppRadio/RF24.h"
#include "AppRadio/printf.h"
RF24 radio(PIN_A0, 10); //cepin, cspin

#include "AppUI/LiquidCrystal.h"
// rs, enable, d d d d
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);


#include "AppSensor/Wire.h"
#include "AppSensor/AHTxx.h"

float ahtValue;                               //to store T/RH result

AHTxx aht10(AHTXX_ADDRESS_X38, AHT1x_SENSOR); //sensor address, sensor type
#include "AppSensor/Air_Quality_Sensor.h"
AirQualitySensor sensor(A2);




const uint8_t num_channels = 126;
uint8_t values[num_channels];


void printStatus()
{
    switch (aht10.getStatus())
    {
        case AHTXX_NO_ERROR:
            Serial.println(F("no error"));
            break;
            
        case AHTXX_BUSY_ERROR:
            Serial.println(F("sensor busy, increase polling time"));
            break;
            
        case AHTXX_ACK_ERROR:
            Serial.println(F("sensor didn't return ACK, not connected, broken, long wires (reduce speed), bus locked by slave (increase stretch limit)"));
            break;
            
        case AHTXX_DATA_ERROR:
            Serial.println(F("received data smaller than expected, not connected, broken, long wires (reduce speed), bus locked by slave (increase stretch limit)"));
            break;
            
        case AHTXX_CRC8_ERROR:
            Serial.println(F("computed CRC8 not match received CRC8, this feature supported only by AHT2x sensors"));
            break;
            
        default:
            Serial.println(F("unknown status"));    
            break;
    }
}
const byte interruptPin = 2;


static uint32_t protons = 0;
void blink() {
    // Invertiere den Status: "Lass die LED blinken von HIGH auf LOW/ an auf aus"
    ++protons;
}

void setup(void) {
    Serial.begin(115200);
    printf_begin();
    Serial.println(F("\n\rRF24/examples/scanner/"));
    lcd.begin(20, 4);
    lcd.print("hello, world!");
    
    pinMode(interruptPin, INPUT);
    // Lege die ISR 'blink' auf den Interruptpin mit Modus 'CHANGE':
    // "Bei wechselnder Flanke auf dem Interruptpin" --> "Führe die ISR aus"
    attachInterrupt(digitalPinToInterrupt(interruptPin), blink, FALLING);
    
    
    
    
    //
    // Setup and configure rf radio
    //
    
    radio.begin();
    radio.setAutoAck(false);
    
    // Get into standby mode
    radio.startListening();
    radio.stopListening();
    radio.printDetails();
    
    //delay(1000);
    // Print out header, high then low digit
//     int i = 0;
//     while (i < num_channels) {
//         Serial.print(i >> 4, HEX);
//         ++i;
//     }
//     Serial.println();
//     i = 0;
//     while (i < num_channels) {
//         Serial.print(i & 0xf, HEX);
//         ++i;
//     }
//     Serial.println();
    //delay(1000);
    
    
    if (sensor.init()) {
        Serial.println("Sensor ready.");
    } else {
        Serial.println("Sensor ERROR!");
    }
    
    
    
    while (aht10.begin() != true)
    {
        Serial.println(F("AHT1x not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
        
        delay(1000);
        
        
    }
//     
    Serial.println(F("AHT10 OK"));
    
    
    pinMode(PIN_A1, OUTPUT);
    digitalWrite(PIN_A1, HIGH);
}

//
// Loop
//

const int num_reps = 100;
bool constCarrierMode = 0;

// int period = 1000;
uint32_t time_now = 0;
uint16_t amin = 1023;
uint16_t amax = 0;
int aval;
uint16_t samples = 0;
void loop(void) {
    
    if((millis() - time_now) > 1000){
        time_now = millis();

        Serial.print(F("HV ADC min: "));
        Serial.print(amin);
        Serial.print(F(" max "));
        Serial.print(amax);
        Serial.print(F(" samples "));
        Serial.println(samples);
        lcd.setCursor(0, 1);
        lcd.print(F("HV min "));
        lcd.print(amin);
        lcd.print(F(" max "));
        lcd.print(amax);
        lcd.setCursor(0, 2);
        lcd.print(F(" samples "));
        lcd.print(millis());
        amin = 1023;
        amax = 0;
        samples =0;
        
        int quality = sensor.slope();
        
        Serial.print("Sensor value: ");
        Serial.println(sensor.getValue());
        
        if (quality == AirQualitySensor::FORCE_SIGNAL) {
            Serial.println("High pollution! Force signal active.");
        } else if (quality == AirQualitySensor::HIGH_POLLUTION) {
            Serial.println("High pollution!");
        } else if (quality == AirQualitySensor::LOW_POLLUTION) {
            Serial.println("Low pollution!");
        } else if (quality == AirQualitySensor::FRESH_AIR) {
            Serial.println("Fresh air.");
        }
        
    }

    aval = analogRead(PIN_A6);
    if (aval > amax) amax = aval;
    if (aval < amin) amin = aval;
    ++samples;
        lcd.setCursor(0, 3);
//     Serial.print(F("Protons: "));
//     Serial.println(protons);
    
    
    

    
    
    
    
    
    
    
    
//     lcd.setCursor(0, 1);
//     print the number of seconds since reset:
//     lcd.print(millis() / 1000);
//     lcd.setCursor(0, 2);
//     lcd.print(protons);
    return;
    
    /****************************************/
    // Send g over Serial to begin CCW output
    // Configure the channel and power level below
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'g') {
            constCarrierMode = 1;
            radio.stopListening();
            delay(2);
            Serial.println("Starting Carrier Out");
            radio.startConstCarrier(RF24_PA_LOW, 40);
        } else if (c == 'e') {
            constCarrierMode = 0;
            radio.stopConstCarrier();
            Serial.println("Stopping Carrier Out");
        }
    }
    /****************************************/
    
    

    
    
    
    
    Serial.println(F("DEMO 1: read 12-bytes"));
    
    ahtValue = aht10.readTemperature(); //read 6-bytes via I2C, takes 80 milliseconds
    
    Serial.print(F("Temperature...: "));
    
    if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
    {
        Serial.print(ahtValue);
        Serial.println(F(" +-0.3C"));
    }
    else
    {
        printStatus(); //print temperature command status
        
        if   (aht10.softReset() == true) Serial.println(F("reset success")); //as the last chance to make it alive
        else                             Serial.println(F("reset failed"));
    }
    
    delay(2000); //measurement with high frequency leads to heating of the sensor, see NOTE
    
    ahtValue = aht10.readHumidity(); //read another 6-bytes via I2C, takes 80 milliseconds
    
    Serial.print(F("Humidity......: "));
    
    if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
    {
        Serial.print(ahtValue);
        Serial.println(F(" +-2%"));
    }
    else
    {
        printStatus(); //print humidity command status
    }
    
    delay(2000); //measurement with high frequency leads to heating of the sensor, see NOTE
    
    // DEMO - 2, temperature call will read 6-bytes via I2C, humidity will use same 6-bytes/
    Serial.println();
    Serial.println(F("DEMO 2: read 6-byte"));
    
    ahtValue = aht10.readTemperature(); //read 6-bytes via I2C, takes 80 milliseconds
    
    Serial.print(F("Temperature: "));
    
    if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
    {
        Serial.print(ahtValue);
        Serial.println(F(" +-0.3C"));
    }
    else
    {
        printStatus(); //print temperature command status
    }
    
    ahtValue = aht10.readHumidity(AHTXX_USE_READ_DATA); //use 6-bytes from temperature reading, takes zero milliseconds!!!
    
    Serial.print(F("Humidity...: "));
    
    if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
    {
        Serial.print(ahtValue);
        Serial.println(F(" +-2%"));
    }
    else
    {
        printStatus(); //print temperature command status not humidity!!! RH measurement use same 6-bytes from T measurement
    }
    
    Serial.print(F("HV ADC 6: "));
    Serial.println(analogRead(PIN_A6));
    Serial.print(F("Protons: "));
    Serial.println(protons);

//     Vref = 4550mV
//     ADC = 301 (0..1023)

//     HVDC = 393V

    
    
    
    if (constCarrierMode == 0) {
        // Clear measurement values
        memset(values, 0, sizeof(values));
        
        // Scan all channels num_reps times
        int rep_counter = num_reps;
        while (rep_counter--) {
            int i = num_channels;
            while (i--) {
                // Select this channel
                radio.setChannel(i);
                
                // Listen for a little
                radio.startListening();
                delayMicroseconds(128);
                radio.stopListening();
                
                // Did we get a carrier?
                if (radio.testCarrier()) {
                    ++values[i];
                }
            }
        }
        
        
        // Print out channel measurements, clamped to a single hex digit
        int i = 0;
        while (i < num_channels) {
            if (values[i])
                Serial.print(min(0xf, values[i]), HEX);
            else
                Serial.print(F("-"));
            
            ++i;
        }
        Serial.println();
        
    }  //If constCarrierMode == 0
}
