/*
 * This file is part of CraftServices.
 *
 * CraftServices is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CraftServices is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CraftServices. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRAFTSERVICES_HPP
#define CRAFTSERVICES_HPP

#include <string>
#include <array>
#include <sstream>
#include <set>
#include <climits>
#include <cassert>

//#include "types.hpp"

#include "deserialise.hpp"
#include "AsyncMspMessageTypes.hpp"
#include "MspFlightControllerAsync.hpp"
#include "CraftInfoAndPosition.hpp"

#define CRAFT_SERVICES_DEFAULT_LOG_LEVEL spdlog::level::info

void StartFlightControllerRefreshTimerFiring(uint32_t refreshIntervalInMillisecondsForAllPorts);
void ScheduleNextFlightControllerTimerToFireImmediately();
void RefreshFlightControllers(const boost::system::error_code & error);
bool IsShutdownInProgressOrComplete();
void DoCleanupAndShutdown(bool playBeepsAndShowExitLogging = true);
void OutputCraftServicesTotalRuntime();


#endif // CRAFTSERVICES_HPP