/*
 * mec-policy.cpp
 *
 *  Created on: 13 Jun 2017
 *      Author: thiagogenez
 */

#include "ns3/assert.h"

#include "ns3/mec-utils-helper.h"
#include "mec-policy-container.h"

namespace ns3 {

MecPolicyContainer::MecPolicyContainer() {
}

MecPolicyContainer::~MecPolicyContainer() {
}

MecPolicyContainer::iterator MecPolicyContainer::Begin() {
	return m_policyList.begin();
}

MecPolicyContainer::iterator MecPolicyContainer::End() {
	return m_policyList.end();
}

MecPolicyContainer::const_iterator MecPolicyContainer::CBegin() const {
	return m_policyList.cbegin();
}

MecPolicyContainer::const_iterator MecPolicyContainer::CEnd() const {
	return m_policyList.cend();
}

uint32_t MecPolicyContainer::GetSize() const {
	return m_policyList.size();
}

Ptr<MecPolicy> MecPolicyContainer::GetPolicyById(uint32_t i) {

	for (MecPolicyContainer::iterator it = Begin(); it != End(); it++) {
		if ((*it)->GetId() == i) {
			return *it;
		}
	}

	NS_ASSERT_MSG(false,
			"MecPolicyContainer::GetPolicyById: policy id = " << i << "does not exist!");

	return NULL;
}

bool MecPolicyContainer::Contains(Ptr<MecPolicy> policy) {

	for (MecPolicyContainer::const_iterator it = CBegin(); it != CEnd(); it++) {
		if ((*it)->GetId() == policy->GetId()) {
			return true;
		}
	}
	return false;
}

void MecPolicyContainer::Add(Ptr<MecPolicy> policy) {
	m_policyList.push_back(policy);
}

void MecPolicyContainer::Add(MecPolicyContainer other) {
	for (MecPolicyContainer::const_iterator it = other.CBegin();
			it != other.CEnd(); it++) {

		Ptr<MecPolicy> policy = (*it);

		bool exist = Contains(policy);

		if (!exist) {
			Add(policy);
		}
	}
}

Ptr<MecPolicy> MecPolicyContainer::CreatePolicyEntry(Ptr<MecFlow> flow) {

	uint32_t srcAddr, dstAddr;
	uint8_t protocol;
	uint16_t srcPort, dstPort;

	MecUtilsHelper::ExtractFlowNetworkNodeInformation(flow, &srcAddr, &dstAddr,
			&protocol, &srcPort, &dstPort);

	Ptr<MecPolicy> policy = CreatePolicyEntry(srcAddr, dstAddr, protocol,
			srcPort, dstPort);

	return policy;

}

Ptr<MecPolicy> MecPolicyContainer::CreatePolicyEntry(uint32_t srcAddr,
		uint32_t dstAddr, uint8_t protocol, uint16_t srcPort,
		uint16_t dstPort) {

	Ptr<MecPolicy> p = CreateObject<MecPolicy>();

	p->SetSrcAddr(srcAddr);
	p->SetDstAddr(dstAddr);

	p->SetSrcPort(srcPort);
	p->SetDstPort(dstPort);

	p->SetProtocol(protocol);
	p->SetIsOk(true);

	Add(p);

	return p;

}

Ptr<MecPolicy> MecPolicyContainer::Search(uint32_t srcAddr, uint32_t dstAddr,
		uint8_t protocol, uint16_t srcPort, uint16_t dstPort) {

	MecPolicyContainer::iterator it;
	for (it = Begin(); it != End(); it++) {
		if ((*it)->Match(srcAddr, dstAddr, protocol, srcPort, dstPort)) {
			return *it;
		}
	}
	return NULL;
}

Ptr<MecPolicy> MecPolicyContainer::GetPolicyByFlow(Ptr<MecFlow> flow) {

	uint32_t srcAddr, dstAddr;
	uint8_t protocol;
	uint16_t srcPort, dstPort;

	MecUtilsHelper::ExtractFlowNetworkNodeInformation(flow, &srcAddr, &dstAddr,
			&protocol, &srcPort, &dstPort);

	return Search(srcAddr, dstAddr, protocol, srcPort, dstPort);

}

bool MecPolicyContainer::IsValidPolicy(Ptr<MecPolicy> policy) {
	if (!policy)
		return false;
	return policy->IsIsOk();
}

Ptr<MecPolicy> MecPolicyContainer::GetPolicyByIndex(uint32_t i) const {

	NS_ASSERT_MSG(i < GetSize(),
			"MecPolicyContainer::GetPolicyByIndex index=" << i << "GetSize()="<<GetSize());

	return m_policyList[i];
}

//set the last percentage % policies  to the status
void MecPolicyContainer::SetPolicyStatus(uint32_t percentage, bool status) {

	uint32_t startIndex = (GetSize() * (100 - percentage) / 100) + 1;

	for (uint32_t i = startIndex; i < GetSize(); i++) {
		Ptr<MecPolicy> p = GetPolicyByIndex(i);
		p->SetIsOk(status);
	}
}

void MecPolicyContainer::Print(std::ostream& os) const {
	std::cout << "Total of policies: " << GetSize() << " Flows" << std::endl;
	for (MecPolicyContainer::const_iterator it = CBegin(); it != CEnd(); it++) {
		(*it)->Print(os);
	}
}

} /* namespace ns3 */
