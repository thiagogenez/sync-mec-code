/*
 * mec-middle-box-container.cc
 *
 *  Created on: 7 Jun 2017
 *      Author: thiagogenez
 */

#include <set>

#include "mec-middle-box-container.h"

namespace ns3 {

MecMiddleBoxContainer::MecMiddleBoxContainer() {

}

MecMiddleBoxContainer::~MecMiddleBoxContainer() {
}

MecMiddleBoxContainer::const_iterator MecMiddleBoxContainer::CBegin() const {
	return m_mbList.cbegin();
}

MecMiddleBoxContainer::const_iterator MecMiddleBoxContainer::CEnd() const {
	return m_mbList.cend();
}

uint32_t MecMiddleBoxContainer::GetSize() const {
	return m_mbList.size();
}

Ptr<MecMiddleBox> MecMiddleBoxContainer::GetMiddleboxByIndex(uint32_t i) const {

	NS_ASSERT_MSG(i < GetSize(),
			"MecMiddleBoxContainer::GetMiddleboxByIndex index=" << i << "GetSize()="<<GetSize());

	return m_mbList[i];
}

std::vector<uint16_t> MecMiddleBoxContainer::GetTypes() const {

	std::set<uint16_t> myset;

	for (MecMiddleBoxContainer::const_iterator it = m_mbList.begin();
			it != m_mbList.end(); it++) {

		uint16_t type = (*it)->GetMiddleBoxType();
		myset.insert(type);

	}

	return std::vector<uint16_t>(myset.begin(), myset.end());
}

MecMiddleBoxContainer MecMiddleBoxContainer::GetMiddleBoxByType(uint16_t type) {

	MecMiddleBoxContainer mbc;
	for (MecMiddleBoxContainer::const_iterator it = m_mbList.begin();
			it != m_mbList.end(); it++) {
		Ptr<MecMiddleBox> mb = *it;

		if (type == mb->GetMiddleBoxType()) {
			mbc.Add(mb);
		}
	}

	return mbc;
}

void MecMiddleBoxContainer::SetMiddleBoxAt(uint32_t index, Ptr<MecMiddleBox> newMB) {
	m_mbList[index] = newMB;
}

void MecMiddleBoxContainer::Add(Ptr<MecMiddleBox> middlebox) {
	m_mbList.push_back(middlebox);
}

void MecMiddleBoxContainer::CreateMiddleBoxContainer(uint32_t n,
		std::string dataRate, std::vector<uint16_t> types) {

	NS_ASSERT_MSG(n == types.size(),
			"MecMiddleBoxContainer::CreateMiddleBoxContainer number of MBs="<< n << " is different of the number of types's vector size="<< types.size());

	for (uint32_t i = 0; i < n; i++) {

		Ptr<MecMiddleBox> mb = CreateObject<MecMiddleBox>();
		mb->SetType(types[i]);
		mb->SetCapacity(dataRate);

		Add(mb);
	}
}

void MecMiddleBoxContainer::Remove(Ptr<MecMiddleBox> mb) {

	for (MecMiddleBoxContainer::iterator it = m_mbList.begin();
			it != m_mbList.end(); it++) {

		Ptr<MecMiddleBox> mbTemp = *it;

		if (mbTemp->GetId() == mb->GetId()) {
			m_mbList.erase(it);
			return;
		}
	}

	std::cout << "ERROR: Can not find MB in current container" << std::endl;
}

bool MecMiddleBoxContainer::Contains(Ptr<MecMiddleBox> middlebox) {

	for (MecMiddleBoxContainer::const_iterator it = CBegin(); it < CEnd();
			it++) {

		Ptr<MecMiddleBox> mb = *it;
		if (middlebox->GetId() == mb->GetId()) {
			return true;
		}
	}

	return false;
}

void MecMiddleBoxContainer::Clear() {
	m_mbList.clear();
}

void MecMiddleBoxContainer::Print(std::ostream& os) {
	os << "Total MBs: " << GetSize() << std::endl;

	for (MecMiddleBoxContainer::const_iterator it = m_mbList.begin();
			it != m_mbList.end(); it++) {
		(*it)->Print(os);
	}
}

bool MecMiddleBoxContainer::Empty() const {
	return m_mbList.empty();
}

} /* namespace ns3 */
