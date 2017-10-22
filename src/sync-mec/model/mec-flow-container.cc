/*
 * mec-flow-container.cc
 *
 *  Created on: 9 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-flow-container.h"

namespace ns3 {

MecFlowContainer::MecFlowContainer() {

}

MecFlowContainer::~MecFlowContainer() {
}

MecFlowContainer::const_iterator MecFlowContainer::CBegin() const {
	return m_flowList.cbegin();
}

MecFlowContainer::const_iterator MecFlowContainer::CEnd() const {
	return m_flowList.cend();
}

uint32_t MecFlowContainer::GetSize() const {
	return m_flowList.size();
}

Ptr<MecFlow> MecFlowContainer::GetFlowByIndex(uint32_t i) const {

	NS_ASSERT_MSG(i < GetSize(),
			"MecFlowContainer::GetFlowByIndex index=" << i << "GetSize()="<<GetSize());

	return m_flowList[i];
}

MecFlowContainer::const_iterator MecFlowContainer::GetFlowById(
		uint32_t flowID) const {

	for (MecFlowContainer::const_iterator it = CBegin(); it != CEnd(); it++) {

		if ((*it)->GetId() == flowID) {
			return it;
		}
	}

	return CEnd();
}

bool MecFlowContainer::IsFlowExist(uint32_t flowID) const {
	MecFlowContainer::const_iterator it = GetFlowById(flowID);
	if (it == CEnd()) {
		return false;
	}
	return true;
}

void MecFlowContainer::Add(Ptr<MecFlow> flow) {
	m_flowList.push_back(flow);
}

void MecFlowContainer::Add(MecFlowContainer other) {
	for (MecFlowContainer::const_iterator i = other.CBegin(); i != other.CEnd();
			i++) {
		if (IsFlowExist((*i)->GetId())) {
			continue;
		}

		Add(*i);
	}
}

void MecFlowContainer::Remove(Ptr<MecFlow> flow) {

	for (MecFlowContainer::iterator it = m_flowList.begin();
			it != m_flowList.end(); it++) {

		if ((*it)->GetId() == flow->GetId()) {
			m_flowList.erase(it);
			return;
		}
	}
	std::cout << "Can not find Flow in current container" << std::endl;
}

void MecFlowContainer::CreateFlowContainer(uint32_t n) {

	for (uint32_t i = 0; i < n; i++) {
		Ptr<MecFlow> flow = CreateObject<MecFlow>();
		Add(flow);
	}
}

bool MecFlowContainer::Empty() const {
	return m_flowList.empty();
}

void MecFlowContainer::Print(std::ostream& os) const {
	std::cout << "Total of flows: " << GetSize() << " Flows" << std::endl;
	for (MecFlowContainer::const_iterator it = CBegin(); it != CEnd(); it++) {
		Ptr<MecFlow> flow = *it;
		flow->Print(os);
	}
}

}
/* namespace ns3 */
