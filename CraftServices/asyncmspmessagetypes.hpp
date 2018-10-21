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

#ifndef ASYNCMSPMESSAGETGYPES_HPP
#define ASYNCMSPMESSAGETGYPES_HPP

#include <vector>
#include <stdint.h>
#include "CraftServicesMspID.hpp"
#include "BitOperators.hpp"

namespace CraftServices 
{

/**
 * @brief ByteVector vector of bytes
 */
typedef std::vector<uint8_t> ByteVector;

/////////////////////////////////////////////////////////////////////
/// Generic message types

// A convenience structure we use to build up a message
// during one-byte-at-a-time reading. This is a scratch pad
// about the message, not the message itself.
struct MspMessageScratchPad
{
	uint8_t MessageDirectionCharacter = 0;
	uint8_t MessageIDLowByte = 0;
	uint8_t MessageIDHighByte = 0;
	uint16_t MessageID = 0;
	uint8_t DataPayloadLengthLowByte = 0;
	uint8_t DataPayloadLengthHighByte = 0;
	uint16_t DataPayloadLength = 0;
	uint16_t DataPayloadBytesRead = 0;
	uint8_t * pDataPayload = NULL;
	uint8_t CrcByte = 0;
	
	void ClearValue()
	{
		MessageDirectionCharacter = 0;
		MessageIDLowByte = 0;
		MessageIDHighByte = 0;
		MessageID = 0;
		DataPayloadLengthLowByte = 0;
		DataPayloadLengthHighByte = 0;
		DataPayloadLength = 0;
		DataPayloadBytesRead = 0;
		if (pDataPayload != NULL)
		{
			delete pDataPayload;
			pDataPayload = NULL;
		}
		CrcByte = 0;
	}

	void InitMessageID()
	{
		MessageID = BitOperators::MakeUint16(MessageIDLowByte, MessageIDHighByte);
	}

	void InitDataPayload()
	{
		DataPayloadLength = BitOperators::MakeUint16(DataPayloadLengthLowByte, DataPayloadLengthHighByte);
		assert(pDataPayload == NULL);
		pDataPayload = new uint8_t[DataPayloadLength];
	}

	std::vector<uint8_t> GetPayloadDataAsVector() const
	{
		return std::vector<uint8_t>(pDataPayload, pDataPayload + DataPayloadLength);
	}
};


struct MspMessageAsync 
{
    virtual ID MessageID() const = 0;

	// TODO: A FC-Relative direction indicatior. "ToFc", "FromFC"??
	// This is probably better than polymorphism, since the same message can be tranmitted in both directions and it
	// will just lead to more obtrusive switching at a higher level if you have to make whole objects instead of just
	// 'setting the polarity' so to speak.

    // Destructor
    virtual ~MspMessageAsync() { }
 
    // Encode this message's payload from its current values
	virtual ByteVector EncodePayload() const = 0;

	// Decode message payload, setting this message's appropriate values
	virtual void DecodePayload(const ByteVector & payloadData) = 0;
};



} // namespace CraftServices

#endif // ASYNCMSPMESSAGETGYPES_HPP
