#pragma once

#include <Payload.h>

class Repeater : public Payload
{
public:
    Repeater(unsigned long uplinkFrequency, unsigned long downlinkFrequeny, Mode mode);
    
    void setUplinkFrequency(unsigned long frequency) override;
    void setDownlinkFrequency(unsigned long frequency) override;
};
