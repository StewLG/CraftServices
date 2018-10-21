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

#ifndef PHANTOMTESTCRAFT_HPP
#define PHANTOMTESTCRAFT_HPP

#include <string>

#include <boost/algorithm/string.hpp>
#include "UidUtil.hpp"
#include "NotImplementedException.hpp"
#include "CraftInfoAndPosition.hpp"


// Base class for a fake Craft used for testing.
//
// This can be used to test message traffic and position display without
// actually having another aircraft to test against.

namespace CraftServices
{
	class PhantomTestCraft
	{
		public:

			// A deliberately, ridiculously high value of satellites
			static const int VeryHighSatelliteCount = 50;

			// All Test Craft will have these variables
			uint32_t UID_0;
			uint32_t UID_1;
			uint32_t UID_2;

			std::string CraftName;

			// Constructor
			PhantomTestCraft(std::string craftName) : CraftName(craftName)
			{
				MakeUID();
			}

			// Destructor
			virtual ~PhantomTestCraft() { }

			// Some Phantom crafts may set their position in reference to another craft. All of them
			// support this interface, but it may do nothing, depending on the type of Phantom Craft.
			virtual void UpdateReferenceCraftPosition(CraftInfoAndPosition & referenceCraftInfoAndPosition, bool craftPositionIsStale) = 0;

			// Is this PhantomTestCraft currently eligible to be sent to the given port?
			virtual bool IsEligibleToBeSent(const std::string & portName, std::string & eligibilityMessage) = 0;

			// Get the current position information for the craft
			// (A PhantomTestCraft may move in a regular or random pattern according to internal logic.)
			virtual CraftInfoAndPosition GetCurrentCraftPosition() = 0;

			// Get a dump of the parameters as a string
			virtual std::string GetParametersAsString() = 0;


		private:

			void MakeUID()
			{
				CraftServices::UidUtil::MakeUID(UID_0, UID_1, UID_2);
			}
	};

} // Namespace CraftServices

#endif // PHANTOMTESTCRAFT_HPP




