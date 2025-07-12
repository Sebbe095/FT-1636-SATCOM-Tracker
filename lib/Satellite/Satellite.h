#pragma once

#include <libsgp4/SGP4.h>
#include <Payload.h>

class Satellite : public libsgp4::SGP4
{
private:
    std::string name;
    Payload *payload;

public:
    Satellite(std::string name, std::string line1, std::string line2, Payload *payload);

    std::string getName() const;
    Payload *getPayload() const;
};
