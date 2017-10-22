/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef DC_STATS_TAG_H
#define DC_STATS_TAG_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include <iostream>

using namespace ns3;

// define this class in a public header
class DcStatsTag: public Tag {
public:
	static TypeId GetTypeId(void) {
		static TypeId tid =
				TypeId("ns3::DcStatsTag").SetParent<Tag>().AddConstructor<
						DcStatsTag>().AddAttribute("numForward", "m_numForward",
						EmptyAttributeValue(),
						MakeUintegerAccessor(&DcStatsTag::GetNumForward),
						MakeUintegerChecker<uint8_t>()).AddAttribute(
						"Timestamp", "Some momentous point in time!",
						EmptyAttributeValue(),
						MakeTimeAccessor(&DcStatsTag::GetTimestamp),
						MakeTimeChecker());
		return tid;
	}
	;

	virtual TypeId GetInstanceTypeId(void) const {
		return GetTypeId();
	}
	;

	virtual uint32_t GetSerializedSize(void) const {
		return 1 + 8 + 4;
	}
	;

	virtual void Serialize(TagBuffer i) const {
		i.WriteU8(m_numForward);

		int64_t t = m_timestamp.GetNanoSeconds();
		i.Write((const uint8_t *) &t, 8);

		i.WriteU32(m_flowID);
	}
	;

	virtual void Deserialize(TagBuffer i) {
		m_numForward = i.ReadU8();

		int64_t t;
		i.Read((uint8_t *) &t, 8);
		m_timestamp = NanoSeconds(t);

		m_flowID = i.ReadU32();
	}
	;

	virtual void Print(std::ostream &os) const {
		os << "m_numForward=" << (uint32_t) m_numForward << ", m_timestamp="
				<< m_timestamp << ", m_flowID=" << m_flowID << "\n";
	}
	;

	// these are our accessors to our tag structure
	void SetNumForward(uint8_t value) {
		m_numForward = value;
	}
	;

	void increaseNumForward() {
		m_numForward++;
	}
	;

	uint8_t GetNumForward(void) const {
		return m_numForward;
	}
	;

	void SetTimestamp(Time time) {
		m_timestamp = time;
	}
	;

	Time GetTimestamp(void) const {
		return m_timestamp;
	}
	;

	void SetFlowID(uint32_t id) {
		m_flowID = id;
	}
	;

	uint32_t GetFlowID(void) const {
		return m_flowID;
	}
	;

private:
	uint8_t m_numForward;
	Time m_timestamp;
	; //!< Timestamp
	uint32_t m_flowID;
};

/*
 int main (int argc, char *argv[])
 {
 // create a tag.
 MyTag tag;
 tag.SetSimpleValue (0x56);

 // store the tag in a packet.
 Ptr<Packet> p = Create<Packet> (100);
 p->AddPacketTag (tag);

 // create a copy of the packet
 Ptr<Packet> aCopy = p->Copy ();

 // read the tag from the packet copy
 MyTag tagCopy;
 p->PeekPacketTag (tagCopy);

 // the copy and the original are the same !
 NS_ASSERT (tagCopy.GetSimpleValue () == tag.GetSimpleValue ());

 aCopy->PrintPacketTags (std::cout);
 std::cout << std::endl;

 return 0;
 }*/

#endif

