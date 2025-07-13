/*
 * Copyright 2013 Daniel Warner <contact@danrw.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Modified by Sebbe Vandenborre (2025)
 */

#pragma once

#include <list>
#include <libsgp4/SGP4.h>
#include <libsgp4/CoordGeodetic.h>

struct PassDetails
{
    libsgp4::DateTime aos;
    libsgp4::DateTime los;
    double max_elevation;
};

std::list<struct PassDetails> GeneratePassList(
    const libsgp4::CoordGeodetic &user_geo,
    libsgp4::SGP4 &sgp4,
    const libsgp4::DateTime &start_time,
    const libsgp4::DateTime &end_time,
    const int time_step);