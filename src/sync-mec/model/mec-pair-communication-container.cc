/*
 * mec-pair-communication-container.cc
 *
 *  Created on: 30 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-pair-communication-container.h"

namespace ns3 {

MecPairCommunicationContainer::MecPairCommunicationContainer() {

}

MecPairCommunicationContainer::~MecPairCommunicationContainer() {
}

MecPairCommunicationContainer::const_iterator MecPairCommunicationContainer::CBegin() const {
	return m_pairList.cbegin();
}

MecPairCommunicationContainer::const_iterator MecPairCommunicationContainer::CEnd() const {
	return m_pairList.cend();
}

void MecPairCommunicationContainer::Add(Ptr<MecPairCommunication> pair) {
	m_pairList.push_back(pair);
}

Ptr<MecPairCommunication> MecPairCommunicationContainer::GetPairByIndex(uint32_t i) const {

	NS_ASSERT_MSG(i < GetSize(),
			"MecPairCommunicationContainer::GetFlowByIndex index=" << i << "GetSize()="<<GetSize());

	return m_pairList[i];
}

bool MecPairCommunicationContainer::Empty() const {
	return m_pairList.empty();
}

uint32_t MecPairCommunicationContainer::GetSize() const {
	return m_pairList.size();
}

void MecPairCommunicationContainer::Print(std::ostream& os) const {

	for (MecPairCommunicationContainer::const_iterator it = CBegin();
			it != CEnd(); it++) {
		(*it)->Print(os);
	}

}
} /* namespace ns3 */
