#include <Satellite.h>

Satellite::Satellite(std::string name, std::string line1, std::string line2, Payload *payload) : libsgp4::SGP4(libsgp4::Tle(name, line1, line2)), name(name), payload(payload)
{
}

std::string Satellite::getName() const
{
    return name;
}

Payload *Satellite::getPayload() const
{
    return payload;
}
