/*
 * mec-policy.cc
 *
 *  Created on: 13 Jun 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"

#include "ns3/mec-constants-helper.h"
#include "mec-policy.h"

namespace ns3 {

//Id initialisation
uint32_t MecPolicy::m_counter = 0;

NS_LOG_COMPONENT_DEFINE("MecPolicy");
NS_OBJECT_ENSURE_REGISTERED(MecPolicy);

TypeId MecPolicy::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecPolicy").SetParent<Object>().SetGroupName(
					"sync-mec");
	return tid;
}

MecPolicy::MecPolicy() {
	m_id = MecPolicy::m_counter;
	MecPolicy::m_counter++;
}

MecPolicy::~MecPolicy() {
}

bool MecPolicy::IsIsOk() const {
	return m_isOK;
}

void MecPolicy::SetIsOk(bool isOk) {
	m_isOK = isOk;
}

uint32_t MecPolicy::GetDstAddr() const {
	return m_dstAddr;
}

void MecPolicy::SetDstAddr(uint32_t dstAddr) {
	m_dstAddr = dstAddr;
}

uint16_t MecPolicy::GetDstPort() const {
	return m_dstPort;
}

void MecPolicy::SetDstPort(uint16_t dstPort) {
	m_dstPort = dstPort;
}

uint32_t MecPolicy::GetId() const {
	return m_id;
}

uint8_t MecPolicy::GetProtocol() const {
	return m_protocol;
}

void MecPolicy::SetProtocol(uint8_t protocol) {
	m_protocol = protocol;
}

uint32_t MecPolicy::GetSrcAddr() const {
	return m_srcAddr;
}

void MecPolicy::SetSrcAddr(uint32_t srcAddr) {
	m_srcAddr = srcAddr;
}

uint16_t MecPolicy::GetSrcPort() const {
	return m_srcPort;
}

void MecPolicy::SetSrcPort(uint16_t srcPort) {
	m_srcPort = srcPort;
}

uint8_t MecPolicy::GetLen() const {
	return m_len;
}

void MecPolicy::SetLen(uint8_t len) {
	m_len = len;
}

uint16_t MecPolicy::GetMiddleBoxType(uint32_t index) const {

	NS_ASSERT_MSG(index < m_len,
			"MecPolicy::GetMiddleBoxType: Wrong index="<< index << " m_seq.size()=" << m_seq.size());

	return m_seq[index];
}

void MecPolicy::AddMiddleBoxType(uint16_t type) {
	m_seq.push_back(type);
}

Ptr<MecMiddleBox> MecPolicy::GetMiddleBoxByIndex(uint32_t index) const {
	return m_mbList.GetMiddleboxByIndex(index);
}

void MecPolicy::SetMiddleBox(uint32_t index, Ptr<MecMiddleBox> middlebox) {
	m_mbList.SetMiddleBoxAt(index, middlebox);
}
void MecPolicy::AddMiddlebox(Ptr<MecMiddleBox> middlebox) {
	m_mbList.Add(middlebox);
}

bool MecPolicy::Match(uint32_t srcAddr, uint32_t dstAddr, uint8_t protocol,
		uint16_t srcPort, uint16_t dstPort) {

	return (srcAddr == GetSrcAddr() && dstAddr == GetDstAddr()
			&& protocol == GetProtocol()
			&& (srcPort == GetSrcPort() || GetSrcPort() == 0)
			&& (dstPort == GetDstPort() || GetSrcPort() == 0));

}

bool MecPolicy::Match(Ptr<MecPolicy> other) {
	return Match(other->GetSrcAddr(), other->GetDstAddr(), other->GetProtocol(),
			other->GetSrcPort(), other->GetDstPort());
}

bool MecPolicy::ContainsMiddlebox(Ptr<MecMiddleBox> middlebox) {
	return m_mbList.Contains(middlebox);
}

void MecPolicy::Print(std::ostream& os) {

	os << "Policy " << GetId() << ":[" << Ipv4Address(GetSrcAddr()) << ":"
			<< GetSrcPort() << "]-->" << "[" << Ipv4Address(GetDstAddr()) << ":"
			<< GetDstPort() << "], "
			<< (GetProtocol() == MecConstantsHelper::POLICY_TCP_PROTOCOL ?
					"TCP" : "UDP") << ", isOK:" << (IsIsOk() ? "true" : "false")
			<< ", Seq: {";
	for (uint8_t i = 0; i < GetLen(); i++) {
		os << "[" << GetMiddleBoxType(i) << "]";
	}

	os << ", MB list:{ ";
	for (uint8_t i = 0; i < GetLen(); i++) {
		os << "[" << GetMiddleBoxByIndex(i)->GetId() << "]";
	}

	os << " }" << std::endl;
}

void MecPolicy::Clear() {
	m_len = 0;
	m_seq.clear();
	m_mbList.Clear();
}

uint32_t MecPolicy::GetMiddleBoxListSize() const {
	return m_mbList.GetSize();
}

} /* namespace ns3 */
