#pragma once

// Note: All frequencies in daHz (decahertz)

class Payload
{
public:
    enum class Mode
    {
        USB,
        LSB,
        CW,
        FM
    };

    virtual unsigned long getUplinkFrequency() const
    {
        return uplinkFrequency;
    };

    virtual void setUplinkFrequency(unsigned long frequency) = 0;

    virtual unsigned long getDownlinkFrequency() const
    {
        return downlinkFrequency;
    };

    virtual void setDownlinkFrequency(unsigned long frequency) = 0;

    Mode getUplinkMode() const
    {
        return uplinkMode;
    };

    Mode getDownlinkMode() const
    {
        return downlinkMode;
    };

protected:
    unsigned long uplinkFrequency;
    unsigned long downlinkFrequency;
    Mode uplinkMode;
    Mode downlinkMode;
};
