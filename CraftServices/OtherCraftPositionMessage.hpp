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

#ifndef OTHERCRAFTPOSITIONMESSAGE_HPP
#define OTHERCRAFTPOSITIONMESSAGE_HPP

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

namespace CraftServices 
{
namespace msg 
{
		// A position message for a single other craft.
		//
		// MSP2_INAV_OTHER_CRAFT_POSITION = 0x201B
		struct OtherCraftPositionMessage : public CraftServices::MspMessageAsync
		{
		public:
			ID MessageID() const
			{
				return ID::MSP2_INAV_OTHER_CRAFT_POSITION;
			}

			CraftServices::CraftInfoAndPosition MessageCraftInfoAndPosition;

			// TODO: Craft Type (Quad, Plane, Tricopter, etc.)

			// Default constructor
			OtherCraftPositionMessage() : MspMessageAsync()
			{
				MessageCraftInfoAndPosition.ClearValues();
			}

			// Payload constructor
			OtherCraftPositionMessage(const ByteVector & payloadData)
			{
				DecodePayload(payloadData);
			}
			
			// Constructor from MSP Flight Controller
			OtherCraftPositionMessage(const MSPFlightControllerAsync & mspFlightController)
			{
				MessageCraftInfoAndPosition.U_ID_0 = mspFlightController.MspFcInfo.UID_0;
				MessageCraftInfoAndPosition.U_ID_1 = mspFlightController.MspFcInfo.UID_1;
				MessageCraftInfoAndPosition.U_ID_2 = mspFlightController.MspFcInfo.UID_2;

				MessageCraftInfoAndPosition.FixType = mspFlightController.CurrentPosition.FixType;

				MessageCraftInfoAndPosition.NumSat = mspFlightController.CurrentPosition.NumSat;
				MessageCraftInfoAndPosition.MspLat = mspFlightController.CurrentPosition.MspLat;
				MessageCraftInfoAndPosition.MspLon = mspFlightController.CurrentPosition.MspLon;
				MessageCraftInfoAndPosition.AltitudeInMeters = mspFlightController.CurrentPosition.AltitudeInMeters;
				MessageCraftInfoAndPosition.Speed = mspFlightController.CurrentPosition.Speed;
				MessageCraftInfoAndPosition.GroundCourseInDecidegrees = mspFlightController.CurrentPosition.GroundCourseInDecidegrees;

				MessageCraftInfoAndPosition.CraftName = mspFlightController.MspFcInfo.CraftName;
			}

			// Constructor from Phantom Test Craft
			OtherCraftPositionMessage(PhantomTestCraft & phantomTestCraft)
			{
				// Phantom Crafts may appear to move on their own, so we need to retrieve the 
				// current Phantom Craft position each time a position is about to be sent.
				CraftInfoAndPosition currentCraftInfoAndPosition = phantomTestCraft.GetCurrentCraftPosition();

				MessageCraftInfoAndPosition.U_ID_0 = phantomTestCraft.UID_0;
				MessageCraftInfoAndPosition.U_ID_1 = phantomTestCraft.UID_1;
				MessageCraftInfoAndPosition.U_ID_2 = phantomTestCraft.UID_2;

				MessageCraftInfoAndPosition.FixType = currentCraftInfoAndPosition.FixType;

				MessageCraftInfoAndPosition.NumSat = currentCraftInfoAndPosition.NumSat;
				MessageCraftInfoAndPosition.MspLat = currentCraftInfoAndPosition.MspLat;
				MessageCraftInfoAndPosition.MspLon = currentCraftInfoAndPosition.MspLon;
				MessageCraftInfoAndPosition.AltitudeInMeters = currentCraftInfoAndPosition.AltitudeInMeters;
				MessageCraftInfoAndPosition.Speed = currentCraftInfoAndPosition.Speed;
				MessageCraftInfoAndPosition.GroundCourseInDecidegrees = currentCraftInfoAndPosition.GroundCourseInDecidegrees;

				MessageCraftInfoAndPosition.CraftName = currentCraftInfoAndPosition.CraftName;
			}

			ByteVector EncodePayload() const
			{
				ByteVector encodedPayload;

				serialize_uint32(MessageCraftInfoAndPosition.U_ID_0, encodedPayload);
				serialize_uint32(MessageCraftInfoAndPosition.U_ID_1, encodedPayload);
				serialize_uint32(MessageCraftInfoAndPosition.U_ID_2, encodedPayload);

				serialize_uint8(MessageCraftInfoAndPosition.FixType, encodedPayload);
				serialize_uint8(MessageCraftInfoAndPosition.NumSat, encodedPayload);
				serialize_uint32(MessageCraftInfoAndPosition.MspLat, encodedPayload);
				serialize_uint32(MessageCraftInfoAndPosition.MspLon, encodedPayload);
				serialize_uint16(MessageCraftInfoAndPosition.AltitudeInMeters, encodedPayload);
				serialize_uint16(MessageCraftInfoAndPosition.Speed, encodedPayload);
				serialize_uint16(MessageCraftInfoAndPosition.GroundCourseInDecidegrees, encodedPayload);

				// Put last since it is variable in size
				encodedPayload.insert(encodedPayload.end(), MessageCraftInfoAndPosition.CraftName.begin(), MessageCraftInfoAndPosition.CraftName.end());

				return encodedPayload;
			}

			void DecodePayload(const ByteVector & payloadData)
			{
				if (payloadData.size() == 0)
				{
					throw new exception("No payload -- 0 bytes");
				}
				MessageCraftInfoAndPosition.ClearValues();				

				MessageCraftInfoAndPosition.U_ID_0 = CraftServices::deserialize_uint32(payloadData, 0);
				MessageCraftInfoAndPosition.U_ID_1 = CraftServices::deserialize_uint32(payloadData, 4);
				MessageCraftInfoAndPosition.U_ID_2 = CraftServices::deserialize_uint32(payloadData, 8);

				MessageCraftInfoAndPosition.FixType = CraftServices::deserialize_uint8(payloadData, 0);
				MessageCraftInfoAndPosition.NumSat = CraftServices::deserialize_uint8(payloadData, 1);
				MessageCraftInfoAndPosition.MspLat = CraftServices::deserialize_uint32(payloadData, 2);
				MessageCraftInfoAndPosition.MspLon = CraftServices::deserialize_uint32(payloadData, 6);
				MessageCraftInfoAndPosition.AltitudeInMeters = CraftServices::deserialize_uint16(payloadData, 10);
				MessageCraftInfoAndPosition.Speed = CraftServices::deserialize_uint16(payloadData, 12);
				MessageCraftInfoAndPosition.GroundCourseInDecidegrees = CraftServices::deserialize_uint16(payloadData, 14);

				// Put last since it is variable in size
				MessageCraftInfoAndPosition.CraftName = std::string(payloadData.begin() + 16, payloadData.end());
			}
		};

	} // Namespace msg
} // Namespace CraftServices

#endif // OTHERCRAFTPOSITIONMESSAGE_HPP