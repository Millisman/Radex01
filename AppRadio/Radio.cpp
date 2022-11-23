#include "Arduino/Arduino.h"
#include "Arduino/HardwareSerial.h"
#include "RF24.h"
#include "printf.h"
#include "Radio.h"

RF24 radio(PIN_A0, 10); //cepin, cspin
bool constCarrierMode = 0;
const uint8_t num_channels = 126;
uint8_t values[num_channels];
const int num_reps = 100;

void setup_radio() {
    printf_begin();
    radio.begin();
    radio.setAutoAck(false);
    
    // Get into standby mode
    radio.startListening();
    radio.stopListening();
    radio.printDetails();
}

void do_radio() {
    /****************************************/
    // Send g over Serial to begin CCW output
    // Configure the channel and power level below
//     if (Serial.available()) {
//         char c = Serial.read();
//         if (c == 'g') {
//             constCarrierMode = 1;
//             radio.stopListening();
//             delay(2);
//             Serial.println("Starting Carrier Out");
//             radio.startConstCarrier(RF24_PA_LOW, 40);
//         } else if (c == 'e') {
//             constCarrierMode = 0;
//             radio.stopConstCarrier();
//             Serial.println("Stopping Carrier Out");
//         }
//     }
    /****************************************/
    
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
