#ifndef __RADIO_H__
#define __RADIO_H__

void setup_radio();
void do_radio();

typedef union __attribute__ ((packed)) {
    struct __attribute__ ((packed)) {
        uint32_t DevSerNum      : 24;
        uint16_t Batt_mV        : 13; // 0..8V
        int16_t  Batt_mA        : 13; // +/- 4A
        uint16_t Solar_mV       : 13; // 0..8V
        uint16_t Solar_mA       : 13; // +/- 4A
        uint16_t Period_AirQ_s  : 13; // sec
        uint16_t Period_GM_s    : 13; // sec
        uint16_t Duty_GM_s      : 10; // sec
        uint16_t Period_Temp_s  : 13; // sec
        uint16_t Period_Hum_s   : 13; // sec
        bool Err_Power          : 1;
        bool Err_HVDC           : 1;
        bool Err_Geiger         : 1;
        bool Err_AirSense       : 1;
        bool Err_TempSense      : 1;
        bool Err_Humidity       : 1;
        bool Err_XReady         : 1;
        uint16_t Sen_GM_Cnt_Min : 13;
        uint8_t Sen_Air_Q       : 2;
        float   Sen_Temp;
        float   Sen_Humidity;
        float   Sen_GM_Zivert;
    };
    uint8_t body[32];
} Radio_Packet_Type;




#endif // __RADIO_H__
