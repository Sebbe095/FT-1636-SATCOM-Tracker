#include <Satellites.h>
#include <Repeater.h>
#include <Transponder.h>

std::vector<Satellite *> satellites{&rs44, &ao7, &mo122, &so50, &so124, &iss};

Transponder rs44Transponder({14593500, 14599500}, OperatingMode::LSB, {43561000, 43567000}, OperatingMode::USB, true);
Satellite rs44("RS-44",
               "1 44909U 19096E   25066.34698064  .00000013  00000-0  45198-5 0  9996",
               "2 44909  82.5187 289.9066 0217845 155.6762 205.4811 12.79738883242632",
               &rs44Transponder);

Transponder ao7Transponder({43212500, 43217500}, OperatingMode::LSB, {14592500, 14597500}, OperatingMode::USB, true);
Satellite ao7("AO-7",
              "1 07530U 74089B   25066.65507859 -.00000017  00000-0  18448-3 0  9994",
              "2 07530 101.9950  69.9146 0011895 302.1475 172.1848 12.53686842302295",
              &ao7Transponder);

Transponder mo122Transponder({14591000, 14594000}, OperatingMode::LSB, {43581000, 43584000}, OperatingMode::USB, true);
Satellite mo122("MO-122",
               "1 60209U 24125G   25066.38155875  .00020685  00000-0  68104-3 0  9995",
               "2 60209  97.2929 279.2658 0021937 113.4771 246.8779 15.31282194 37517",
               &mo122Transponder);

Repeater so50Repeater(14585000, 43679500, OperatingMode::FM);
Satellite so50("SO-50",
               "1 27607U 02058C   25066.65149993  .00002096  00000-0  28316-3 0  9992",
               "2 27607  64.5536 130.3759 0045394 357.1248   2.9589 14.81084589195529",
               &so50Repeater);

Repeater so124Repeater(14592500, 43688500, OperatingMode::FM);
Satellite so124("SO-124",
                "1 62690U 25009CK  25066.65489536  .00037219  00000-0  17255-2 0  9998",
                "2 62690  97.4169 148.3751 0002416  77.6604 282.4901 15.19964967  7834",
                &so124Repeater);

Repeater issRepeater(14599000, 43780000, OperatingMode::FM);
Satellite iss("ISS",
              "1 25544U 98067A   25066.89290769  .00010600  00000-0  19475-3 0  9995",
              "2 25544  51.6354  88.7424 0006375 355.6958   4.3975 15.49796787499453",
              &issRepeater);