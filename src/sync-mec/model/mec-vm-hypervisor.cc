/*
 * mec-vm-hypervisor.cc
 *
 *  Created on: 14 Jun 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"

#include "ns3/mec-vm.h"

#include "mec-vm-hypervisor.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecVmHypervisor");
NS_OBJECT_ENSURE_REGISTERED(MecVmHypervisor);

TypeId MecVmHypervisor::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::MecVmHypervisor").SetParent<Application>().AddConstructor<
					MecVmHypervisor>();
	return tid;

}

//Id initialisation
uint32_t MecVmHypervisor::m_counter = 0;

MecVmHypervisor::MecVmHypervisor() {
	NS_LOG_FUNCTION(this);
	m_id = MecVmHypervisor::m_counter;
	MecVmHypervisor::m_counter++;
}

MecVmHypervisor::~MecVmHypervisor() {
}

void MecVmHypervisor::StartApplication() {
	isStarted = true;

	//starting VMs
	for (MecVmContainer::const_iterator it = m_hostedVMs.CBegin();
			it != m_hostedVMs.CEnd(); it++) {

		Ptr<MecVm> vm = *it;

		NS_ASSERT_MSG(vm->GetHostServerId() == GetNode()->GetId(),
				"VM hostServerId is wrong, VM id=" << vm->GetId() << ", hostedServer=" << vm->GetHostServerId() << ", current Node=" << GetNode()->GetId());

		//start a VM and the flows from this VM
		vm->TurnOn(false);
	}

	// starting UEs
	for (MecVmContainer::const_iterator it = m_hostedUEs.CBegin();
			it != m_hostedUEs.CEnd(); it++) {

		Ptr<MecVm> ue = *it;

		NS_ASSERT_MSG(ue->GetHostServerId() == GetNode()->GetId(),
				"UE hostServerId is wrong, VM id=" << ue->GetId() << ", hostedServer=" << ue->GetHostServerId() << ", current Node=" << GetNode()->GetId());

		//start a VM and the flows from this VM
		ue->TurnOn(false);
	}

	std::cout << "HyperVisor Application Node " << GetNode()->GetId()
			<< ": HyperVisor started" << std::endl;

}

void MecVmHypervisor::AddVm(Ptr<MecVm> vm) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT_MSG(vm->IsIsVm(),
			"MecVmHypervisor::AddVm => is an Ue and not a VM;  ueId="<< vm->GetId()<< " hpervisorId="<<GetId());

	NS_ASSERT_MSG(!ContainsVm(vm),
			"MecVmHypervisor::AddVm => vmId="<< vm->GetId()<< " already in this hpervisorId="<<GetId());

	IncreaseCpuUse(vm->GetCpu());
	IncreaseMemoryUse(vm->GetMemory());

	m_hostedVMs.Add(vm);

}

void MecVmHypervisor::AddUe(Ptr<MecVm> ue) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT_MSG(ue->IsIsUe(),
			"MecVmHypervisor::AddUe => is a VM and not an Ue;  vmId="<< ue->GetId()<< " hpervisorId="<<GetId());

	NS_ASSERT_MSG(!ContainsUe(ue),
			"MecVmHypervisor::AddUe => ueId="<< ue->GetId()<< " already in this hpervisorId="<<GetId());

	m_hostedUEs.Add(ue);
}

void MecVmHypervisor::RemoveVm(Ptr<MecVm> vm) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT_MSG(vm->IsIsVm(),
			"MecVmHypervisor::AddVm => is an Ue and not a VM;  ueId="<< vm->GetId()<< " hpervisorId="<<GetId());

	NS_ASSERT_MSG(ContainsVm(vm),
			"MecVmHypervisor::AddVm => vmId="<< vm->GetId()<< " not found in this hpervisorId="<<GetId());

	ReduceCpuUse(vm->GetCpu());
	ReduceMemoryUse(vm->GetMemory());

	m_hostedVMs.Remove(vm);

}

void MecVmHypervisor::RemoveUe(Ptr<MecVm> ue) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT_MSG(ue->IsIsUe(),
			"MecVmHypervisor::AddUe => is a VM and not an Ue;  vmId="<< ue->GetId()<< " hpervisorId="<<GetId());

	NS_ASSERT_MSG(ContainsUe(ue),
			"MecVmHypervisor::AddVm => vmId="<< ue->GetId()<< " not found in this hpervisorId="<<GetId());

	m_hostedUEs.Remove(ue);
}

void MecVmHypervisor::IncreaseCpuUse(uint32_t cpu) {
	NS_LOG_FUNCTION(this << cpu << GetUsedCpu() << GetTotalCpu());

	NS_ASSERT_MSG((cpu + GetUsedCpu() <= GetTotalCpu()),
			"MecVmHypervisor::IncreaseCpuUse, VM id=" << GetId() << " cpu + GetUsedCpu() > GetTotalCpu() <==> " << cpu << " + " << GetUsedCpu() << " > " << GetTotalCpu());

	m_useCPU += cpu;

	NS_LOG_FUNCTION(this << cpu << GetUsedCpu() << GetTotalCpu());
}

void MecVmHypervisor::IncreaseMemoryUse(uint32_t memory) {
	NS_LOG_FUNCTION(this << memory << GetUsedMemory() << GetTotalMemory());

	NS_ASSERT_MSG((memory + GetUsedMemory() <= GetTotalMemory()),
			"MecVmHypervisor::IncreaseMemoryUse, VM id=" << GetId() << " memory + GetUsedMemory() > GetTotalMemory() <==> " << memory << " + " << GetUsedMemory() << " > " << GetTotalMemory());

	m_useMemory += memory;

	NS_LOG_FUNCTION(this << memory << GetUsedMemory() << GetTotalMemory());
}

void MecVmHypervisor::ReduceCpuUse(uint32_t cpu) {
	NS_LOG_FUNCTION(this << cpu << GetUsedCpu() << GetTotalCpu());

	NS_ASSERT_MSG(GetUsedCpu() >= cpu,
			"MecVmHypervisor::ReduceCpuUse, VM id=" << GetId() << " GetUsedCpu() >= cpu  <==> " << GetUsedCpu() << " - " << cpu << " < 0");

	m_useCPU -= cpu;
	NS_LOG_FUNCTION(this << cpu << GetUsedCpu() << GetTotalCpu());
}

void MecVmHypervisor::ReduceMemoryUse(uint32_t memory) {

	NS_LOG_FUNCTION(this << memory << GetUsedMemory() << GetTotalMemory());

	NS_ASSERT_MSG(GetUsedMemory() >= memory,
			"MecVmHypervisor::ReduceMemoryUse, VM id=" << GetId() << " GetUsedMemory() >= memory  <==> " << GetUsedMemory() << " - " << memory << " < 0");

	m_useMemory -= memory;
	NS_LOG_FUNCTION(this << memory << GetUsedMemory() << GetTotalMemory());
}

void MecVmHypervisor::StopApplication() {
	isStarted = false;
}

uint32_t MecVmHypervisor::GetTotalCpu() const {
	return m_totalCpu;
}

void MecVmHypervisor::SetTotalCpu(uint32_t totalCpu) {
	NS_LOG_FUNCTION(this << totalCpu);
	m_totalCpu = totalCpu;
}

void MecVmHypervisor::SetTotalMemory(uint32_t totalMemory) {
	NS_LOG_FUNCTION(this << totalMemory);
	m_totalMemory = totalMemory;
}

uint32_t MecVmHypervisor::GetTotalMemory() const {
	return m_totalMemory;
}

uint32_t MecVmHypervisor::GetUsedCpu() const {
	return m_useCPU;
}

uint32_t MecVmHypervisor::GetUsedMemory() const {
	return m_useMemory;
}

uint32_t MecVmHypervisor::GetId() const {
	return m_id;
}

bool MecVmHypervisor::IsVmAcceptable(Ptr<MecVm> vm) const {

	NS_LOG_FUNCTION(this << vm->GetCpu() <<GetUsedCpu() << GetTotalCpu());

	bool cpu = (vm->GetCpu() + GetUsedCpu() <= GetTotalCpu());
	bool memory = (vm->GetMemory() + GetUsedMemory() <= GetTotalMemory());

	NS_LOG_FUNCTION(this << cpu << memory);

	return (cpu && memory);
}

bool MecVmHypervisor::ContainsVm(Ptr<MecVm> vm) const {
	return m_hostedVMs.Contain(vm->GetId());
}

bool MecVmHypervisor::ContainsUe(Ptr<MecVm> ue) const {
	return m_hostedUEs.Contain(ue->GetId());
}

} /* namespace ns3 */
