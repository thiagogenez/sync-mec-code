/*
 * mec-vm-container.cc
 *
 *  Created on: 12 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-vm-container.h"

namespace ns3 {

MecVmContainer::MecVmContainer() {

}

MecVmContainer::~MecVmContainer() {
}

MecVmContainer::const_iterator MecVmContainer::CBegin() const {
	return m_VmList.cbegin();
}

MecVmContainer::const_iterator MecVmContainer::CEnd() const {
	return m_VmList.cend();
}

MecVmContainer::iterator MecVmContainer::Begin() {
	return m_VmList.begin();
}

MecVmContainer::iterator MecVmContainer::End() {
	return m_VmList.end();
}

uint32_t MecVmContainer::GetSize() const {
	return m_VmList.size();
}

Ptr<MecVm> MecVmContainer::GetVmByIndex(uint32_t i) const {

	NS_ASSERT_MSG(i < GetSize(),
			"MecVmContainer::GetVmByIndex index=" << i << "GetSize()="<<GetSize());

	return m_VmList[i];
}

MecVmContainer::const_iterator MecVmContainer::GetVmById(uint32_t vmId) const {
	for (MecVmContainer::const_iterator it = CBegin(); it != CEnd(); it++) {
		if ((*it)->GetId() == vmId)
			return it;
	}
	return CEnd();
}

bool MecVmContainer::Contain(uint32_t vmId) const {
	MecVmContainer::const_iterator it = GetVmById(vmId);
	if (it == CEnd())
		return false;
	return true;
}

void MecVmContainer::Add(Ptr<MecVm> vm) {
	m_VmList.push_back(vm);
}

void MecVmContainer::Add(MecVmContainer other) {
	for (MecVmContainer::const_iterator it = other.CBegin(); it != other.CEnd();
			it++) {
		Ptr<MecVm> vm = *it;
		if (Contain(vm->GetId())) //if VM already exist in current container, passed
			continue;

		Add(*it);
	}
}

void MecVmContainer::Remove(Ptr<MecVm> vm) {
	MecVmContainer::iterator it;

	for (it = Begin(); it != End(); it++) {
		if ((*it)->GetId() == vm->GetId()) {
			m_VmList.erase(it);
			return;
		}
	}
	std::cout << "Can not find Vm in current server, vm id=" << vm->GetId()
			<< std::endl;
}

void MecVmContainer::CreateVmContainer(uint32_t n) {
	CreateVmContainer(n, false);
}

void MecVmContainer::CreateVmContainer(uint32_t n, bool isUe) {

	for (uint32_t i = 0; i < n; i++) {
		Ptr<MecVm> vm = CreateObject<MecVm>();
		vm->SetIsUe(isUe);
		m_VmList.push_back(vm);
	}
}

bool MecVmContainer::Empty() const {
	return m_VmList.empty();
}

void MecVmContainer::Print(std::ostream& os) const {

	for (MecVmContainer::const_iterator it = CBegin(); it != CEnd(); it++) {
		Ptr<MecVm> vm = *it;
		vm->Print(os);
	}
}

} /* namespace ns3 */
