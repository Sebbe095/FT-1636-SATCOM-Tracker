#pragma once

#include <OperatingMode.h>

// Note: All frequencies in daHz (decahertz)

class Payload
{
public:
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

    OperatingMode getUplinkMode() const
    {
        return uplinkMode;
    };

    OperatingMode getDownlinkMode() const
    {
        return downlinkMode;
    };

protected:
    unsigned long uplinkFrequency;
    unsigned long downlinkFrequency;
    OperatingMode uplinkMode;
    OperatingMode downlinkMode;
};
