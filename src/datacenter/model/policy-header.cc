/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "policy-header.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(PolicyHeader);

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
PolicyHeader::PolicyHeader() :
		m_type(POLICY_HEADER_TYPE), m_index(0), m_policyID(0) {
}
PolicyHeader::~PolicyHeader() {

}

void PolicyHeader::SetPolicyID(uint16_t id) {
	m_policyID = id;
}

uint16_t PolicyHeader::GetPolicyID(void) const {
	return m_policyID;
}

void PolicyHeader::SetIndex(uint8_t i) {
	m_index = i;
}

uint8_t PolicyHeader::GetIndex(void) const {
	return m_index;
}

TypeId PolicyHeader::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::PolicyHeader").SetParent<Header>().AddConstructor<
					PolicyHeader>();
	return tid;
}
TypeId PolicyHeader::GetInstanceTypeId(void) const {
	return GetTypeId();
}
void PolicyHeader::Print(std::ostream &os) const {
	os << "Index:" << (uint32_t) m_index << ",PolicyID: " << m_policyID;
}

uint32_t PolicyHeader::GetSerializedSize(void) const {
	return 4; // 1 byte type, 1byte index, 2 byte policyID
}

void PolicyHeader::Serialize(Buffer::Iterator start) const {
	Buffer::Iterator it = start;

	it.WriteU8(POLICY_HEADER_TYPE); //0x6868
	it.WriteU8(m_index);
	it.WriteU16(m_policyID);
}

uint32_t PolicyHeader::Deserialize(Buffer::Iterator start) {
	Buffer::Iterator it = start;

	m_type = it.ReadU8();

	//0x6868 //Not a Policy Header
	if (m_type != POLICY_HEADER_TYPE) {
		return 0;
	}

	m_index = it.ReadU8();
	m_policyID = it.ReadU16();
	return GetSerializedSize();
}

bool PolicyHeader::IsPolicyHeader() {

	//0x6868
	if (m_type == POLICY_HEADER_TYPE) {
		return true;
	}
	return false;
}

} // namespace ns3
