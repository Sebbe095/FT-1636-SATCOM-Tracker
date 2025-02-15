#include "Repeater.h"

Repeater::Repeater(unsigned long uplinkFrequency, unsigned long downlinkFrequency, OperatingMode mode)
{
    this->uplinkFrequency = uplinkFrequency;
    this->downlinkFrequency = downlinkFrequency;
    uplinkMode = mode;
    downlinkMode = mode;
}

void Repeater::setUplinkFrequency(unsigned long frequency)
{
    uplinkFrequency = frequency; // TODO: sync downlink frequency?
}

void Repeater::setDownlinkFrequency(unsigned long frequency)
{
    downlinkFrequency = frequency; // TODO: sync uplink frequency?
}
