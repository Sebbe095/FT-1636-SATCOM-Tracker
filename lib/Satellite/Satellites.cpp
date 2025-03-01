#include <Satellites.h>
#include <Repeater.h>
#include <Transponder.h>

Transponder rs44Transponder({14593500, 14599500}, OperatingMode::LSB, {43561000, 43567000}, OperatingMode::USB, true);
Satellite rs44("RS-44",
               "1 44909U 19096E   25059.62330329  .00000046  00000-0  13769-3 0  9999",
               "2 44909  82.5200 294.3755 0217544 171.3671 189.1271 12.79738713241823",
               &rs44Transponder);

Repeater so50Repeater(14585000, 43679500, OperatingMode::FM);
Satellite so50("SO-50",
               "1 27607U 02058C   25059.56165239  .00003578  00000-0  47061-3 0  9999",
               "2 27607  64.5551 152.2403 0044449 359.1048   0.9969 14.81049216194476",
               &so50Repeater);

Repeater issRepeater(14599000, 43780000, OperatingMode::FM);
Satellite iss("ISS",
              "1 25544U 98067A   25059.54152977  .00022226  00000-0  40176-3 0  9996",
              "2 25544  51.6353 125.1519 0006135 326.8326  33.2278 15.49597200498316",
              &issRepeater);