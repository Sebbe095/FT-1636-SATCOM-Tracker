#pragma once

#include <Arduino.h>
#include <vector>

// Note: All frequencies in daHz (decahertz)

class FT818
{
private:
    const unsigned long VHF_MIN = 14400000; // VHF minimum frequency
    const unsigned long VHF_MAX = 14600000; // VHF maximum frequency
    const unsigned long UHF_MIN = 43000000; // UHF minimum frequency
    const unsigned long UHF_MAX = 44000000; // UHF maximum frequency
    const unsigned int CAT_RATE = 38400; // FT-818 CAT baud rate (bps)
    const byte CMD_READ_FREQ_MODE = 0x03;
    const byte CMD_SET_FREQ = 0x01;

    HardwareSerial &port;

    bool sendCommand(byte opcode, std::array<byte, 4> parameters = {0x00, 0x00, 0x00, 0x00});
    bool readData(std::vector<byte> &buffer);
    bool isFrequencyValid(unsigned long frequency);

public:
    FT818(HardwareSerial &serialPort) : port(serialPort) {}

    void begin(int rxPin, int txPin);
    void end();
    bool getFrequency(unsigned long &frequency);
    bool setFrequency(unsigned long frequency);
};
