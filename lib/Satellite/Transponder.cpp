#include "Transponder.h"

Transponder::Transponder(FrequencyRange uplinkRange, Mode uplinkMode, FrequencyRange downlinkRange, Mode downlinkMode, bool inverting)
    : uplinkRange(uplinkRange), uplinkMode(uplinkMode), downlinkRange(downlinkRange), downlinkMode(downlinkMode), inverting(inverting)
{
    uplinkFrequency = uplinkRange.lowerFrequency + (uplinkRange.upperFrequency - uplinkRange.lowerFrequency) / 2;         // Default to the center frequency
    downlinkFrequency = downlinkRange.lowerFrequency + (downlinkRange.upperFrequency - downlinkRange.lowerFrequency) / 2; // Default to the center frequency
}

void Transponder::setUplinkFrequency(unsigned long frequency)
{
    if (frequency >= uplinkRange.lowerFrequency && frequency <= uplinkRange.upperFrequency)
    {
        uplinkFrequency = frequency;
        downlinkFrequency = inverting
                                ? downlinkRange.upperFrequency - (uplinkFrequency - uplinkRange.lowerFrequency)
                                : downlinkRange.lowerFrequency + (uplinkFrequency - uplinkRange.lowerFrequency);
    }
}

void Transponder::setDownlinkFrequency(unsigned long frequency)
{
    if (frequency >= downlinkRange.lowerFrequency && frequency <= downlinkRange.upperFrequency)
    {
        downlinkFrequency = frequency;
        uplinkFrequency = inverting
                              ? uplinkRange.upperFrequency - (downlinkFrequency - downlinkRange.lowerFrequency)
                              : uplinkRange.lowerFrequency + (downlinkFrequency - downlinkRange.lowerFrequency);
    }
}
