#include <Satellites.h>
#include <Repeater.h>
#include <Transponder.h>

Transponder rs44Transponder({14593500, 14599500}, OperatingMode::LSB, {43561000, 43567000}, OperatingMode::USB, true);
Satellite rs44("RS-44",
               "1 44909U 19096E   25024.28504031  .00000004  00000-0 -20149-4 0  9992",
               "2 44909  82.5247 317.8537 0216334 254.2445 103.4704 12.79737924237254",
               &rs44Transponder);

Repeater so50Repeater(14585000, 43679500, OperatingMode::FM);
Satellite so50("SO-50",
               "1 27607U 02058C   25024.78444537  .00003594  00000-0  47659-3 0  9997",
               "2 27607  64.5540 259.4674 0039790   9.3518 350.8319 14.80830306189324",
               &so50Repeater);

Repeater issRepeater(14599000, 43780000, OperatingMode::FM);
Satellite iss("ISS",
              "1 25544U 98067A   25024.67510457  .00025956  00000-0  45306-3 0  9996",
              "2 25544  51.6377 297.8978 0002089 131.4270 306.8981 15.50509808492908",
              &issRepeater);