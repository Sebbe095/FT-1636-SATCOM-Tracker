#pragma once

#include <Payload.h>
#include <CtcssTone.h>

class Repeater : public Payload
{
public:
    Repeater(unsigned long uplinkFrequency, unsigned long downlinkFrequeny, OperatingMode mode);
    Repeater(unsigned long uplinkFrequency, unsigned long downlinkFrequeny, OperatingMode mode, CtcssTone uplinkCtcssTone);

    void setUplinkFrequency(unsigned long frequency) override;
    void setDownlinkFrequency(unsigned long frequency) override;
    CtcssTone getUplinkCtcssTone() const;

private:
    CtcssTone uplinkCtcssTone;
};
