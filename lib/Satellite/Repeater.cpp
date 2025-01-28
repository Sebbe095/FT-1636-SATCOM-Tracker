#include "Repeater.h"

Repeater::Repeater(unsigned long uplinkFrequency, unsigned long downlinkFrequency, Mode mode)
{
    this->uplinkFrequency = uplinkFrequency;
    this->downlinkFrequency = downlinkFrequency;
    uplinkMode = mode;
    downlinkMode = mode;
}

void Repeater::setUplinkFrequency(unsigned long frequency)
{
    uplinkFrequency = frequency;
}

void Repeater::setDownlinkFrequency(unsigned long frequency)
{
    downlinkFrequency = frequency;
}
