#include "Payload.h"

class Transponder : public Payload
{

public:
    struct FrequencyRange
    {
        unsigned long lowerFrequency; // daHz
        unsigned long upperFrequency; // daHz
    };

    Transponder(FrequencyRange uplinkRange, Mode uplinkMode, FrequencyRange downlinkRange, Mode downlinkMode, bool inverting);

    void setUplinkFrequency(unsigned long frequency) override;
    void setDownlinkFrequency(unsigned long frequency) override;

private:
    FrequencyRange uplinkRange;
    FrequencyRange downlinkRange;
    Mode uplinkMode;
    Mode downlinkMode;
    bool inverting;
};
