#pragma GCC diagnostic ignored "-Wunknown-attributes"

#include "Arduino/Arduino.h"
#include "Arduino/HardwareSerial.h"
#include "RF24.h"
#include "Radio.h"
#include "AppRadio/Radio.h"
// struct Serial_Num_Struct;

RF24 radio(PIN_A0, 10); //cepin, cspin

uint8_t address[][6] = {"xMain", "xRemo"};

Radio_Packet_Type pkt;

uint32_t fake_serial = 1;



void do_packet(Radio_Packet_Type *p) { // TODO fake data

    extern Serial_Num_Struct sn_my; 
    p->DevSerNum = sn_my.DevSerNum;
    extern uint16_t batt_vcc;
    p->Batt_mV = batt_vcc;
    p->Batt_mA = 0;
    p->Solar_mV = 0;
    p-> Solar_mA = 0;
    p-> Period_AirQ_s = 500;
    p-> Period_GM_s = 1000;
    p-> Duty_GM_s = 240;
    p-> Period_Temp_s = 120;
    p-> Period_Hum_s = 240;
    p-> Err_Power = 0;
    p-> Err_HVDC = 1;
    p-> Err_Geiger = 1;
    p-> Err_AirSense= 0;
    p-> Err_TempSense= 0;
    p-> Err_Humidity= 0;
    p-> Err_XReady= 0;
    extern uint16_t protons_on_min;
    p->Sen_GM_Cnt_Min=protons_on_min;
    extern uint8_t MP503_air;
    p->Sen_Air_Q = MP503_air;
    extern float ahtTemperature;
    p->Sen_Temp = ahtTemperature;
    extern float ahtHumidity;
    p->Sen_Humidity = ahtHumidity;
    p->Sen_GM_Zivert = (float)protons_on_min / 1000.0f; // TODO coeff not corrected
}

Serial_Num_Struct EEMEM sn_eem;
Serial_Num_Struct   sn_my;

void setup_radio() {
    eeprom_read_block(&sn_my, &sn_eem, sizeof(Serial_Num_Struct));
    if (sn_my.DevSerNum == 0 || sn_my.DevSerNum == __UINT32_MAX__) {
        sn_my.DevSerNum = rand();
        eeprom_write_block(&sn_my, &sn_eem, sizeof(Serial_Num_Struct));
    }
    Serial.print(F("My SN: "));
    Serial.println(sn_my.DevSerNum, DEC);
    
    
    memset(&pkt, 0, sizeof(pkt));
    if (!radio.begin()) {
        Serial.println(F("RF24 is not responding!!"));
        pkt.Err_XReady = 1;
    } else {
        radio.setPALevel(RF24_PA_LOW);
        radio.setChannel(5);
        radio.setPayloadSize(sizeof(Radio_Packet_Type)); // default value is the maximum 32 bytes
        radio.openWritingPipe(address[1]);      // set the TX address of the RX node into the TX pipe always uses pipe 0
        radio.openReadingPipe(1, address[0]);   // using pipe 1
        radio.stopListening(); // put radio in TX mode
        // radio.startListening();   // put radio in RX mode
        // For debugging info
        radio.printDetails();
        do_packet(&pkt);
        pkt.Err_XReady = 0;
    }
}

void do_radio() {
    if (pkt.Err_XReady == 0) {
        do_packet(&pkt);
        if (!radio.writeFast(&pkt, sizeof(Radio_Packet_Type))) {
            //Serial.println(F("writeFast fail!!"));
            radio.reUseTX();
        }
    }
}
