#include "FT818.h"

bool FT818::sendCommand(byte opcode, std::array<byte, 4> parameters)
{
    // No bytes should be available in the serial buffer before sending a command
    if (port.available() > 0)
    {
        byte buffer[port.available()];
        port.readBytes(buffer, port.available());
    }

    for (byte parameter : parameters)
    {
        if (!port.write(parameter))
        {
            return false;
        }
    }
    if (!port.write(opcode))
    {
        return false;
    }

    port.flush();

    return true;
}

bool FT818::readData(std::vector<byte> &buffer)
{
    return port.readBytes(buffer.data(), buffer.size()) == buffer.size();
}

bool FT818::readAck()
{
    std::vector<byte> ack(1);
    if (!readData(ack))
    {
        return false;
    }
    return ack.at(0) == 0;
}

bool FT818::isFrequencyValid(unsigned long frequency)
{
    return (frequency >= VHF_MIN && frequency <= VHF_MAX) || (frequency >= UHF_MIN && frequency <= UHF_MAX);
}

void FT818::begin(int rxPin, int txPin)
{
    port.begin(CAT_RATE, SERIAL_8N2, rxPin, txPin, false, 20000UL, 112U);
}

void FT818::end()
{
    port.end();
}

bool FT818::getFrequency(unsigned long &frequency)
{
    if (!sendCommand(CMD_READ_FREQ_MODE))
    {
        return false;
    }

    std::vector<byte> buffer(5);
    if (!readData(buffer))
    {
        return false;
    }

    unsigned long freq = 0;
    for (int i = 0; i < 4; i++)
    {
        freq *= 100;
        freq += (buffer.at(i) >> 4) * 10 + (buffer.at(i) & 0x0F);
    }

    if (!isFrequencyValid(freq))
    {
        return false;
    }

    frequency = freq;

    return true;
}

bool FT818::setFrequency(unsigned long frequency)
{
    if (!isFrequencyValid(frequency))
    {
        return false;
    }

    std::array<byte, 4> frequencyBytes;
    for (int i = frequencyBytes.size() - 1; i >= 0; --i)
    {
        frequencyBytes.at(i) = (frequency / 10 % 10) << 4 | frequency % 10;
        frequency /= 100;
    }

    if (!sendCommand(CMD_SET_FREQ, frequencyBytes))
    {
        return false;
    }

    return readAck();
}

bool FT818::setOperatingMode(OperatingMode operatingMode)
{
    byte operatingModeByte;
    switch (operatingMode)
    {
    case OperatingMode::LSB:
        operatingModeByte = 0x00;
        break;
    case OperatingMode::USB:
        operatingModeByte = 0x01;
        break;
    case OperatingMode::FM:
        operatingModeByte = 0x08;
        break;
    default:
        return false; // Unsupported operating mode
    }

    if (!sendCommand(CMD_SET_OP_MODE, {operatingModeByte, 0x00, 0x00, 0x00}))
    {
        return false;
    }

    return readAck();
}

bool FT818::setCtcssDcsMode(CtcssDcsMode mode)
{
    byte modeByte;
    switch (mode)
    {
    case CtcssDcsMode::DCS_ON:
        modeByte = 0x0A;
        break;
    case CtcssDcsMode::CTCSS_ON:
        modeByte = 0x2A;
        break;
    case CtcssDcsMode::ENCODER_ON:
        modeByte = 0x4A;
        break;
    case CtcssDcsMode::OFF:
        modeByte = 0x8A;
        break;
    default:
        return false; // Unsupported CTCSS/DCS mode
    }

    if (!sendCommand(CMD_SET_CTCSS_DCS_MODE, {modeByte, 0x00, 0x00, 0x00}))
    {
        return false;
    }

    return readAck();
}

bool FT818::setCtcssTone(CtcssTone tone)
{
    int toneValue = static_cast<int>(tone);
    if (!isValidCtcssTone(toneValue))
    {
        return false;
    }

    std::array<byte, 4> toneBytes = {0x00, 0x00, 0x00, 0x00};
    for (int i = 1; i >= 0; --i)
    {
        toneBytes.at(i) = (toneValue / 10 % 10) << 4 | toneValue % 10;
        toneValue /= 100;
    }

    if (!sendCommand(CMD_SET_CTCSS_TONE, toneBytes))
    {
        return false;
    }

    return readAck();
}
