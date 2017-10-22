/*
 * mec-vm.cc
 *
 *  Created on: 12 Jun 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/object-base.h"

#include "ns3/mec-flow.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-logger.h"
#include "mec-vm.h"

namespace ns3 {

//Id initialisation
uint32_t MecVm::m_counter = 0;

MecVm::MecVm() {
	m_id = MecVm::m_counter;
	MecVm::m_counter++;
}

MecVm::~MecVm() {
}

NS_LOG_COMPONENT_DEFINE("MecVm");
NS_OBJECT_ENSURE_REGISTERED(MecVm);

TypeId MecVm::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::MecVm").SetParent<Object>().AddConstructor<
			MecVm>();
	return tid;
}

bool MecVm::TurnOn(bool allFlow) {

	std::ostringstream msg;

	msg << (IsIsUe() ? "ueId=" : "vmId=") << m_id << " starting on nodeId="
			<< GetHostServerId();
	MecLogger::Logging(msg.str());

	msg.clear();
	for (MecFlowContainer::const_iterator it = m_flowList.CBegin();
			it != m_flowList.CEnd(); ++it) {

		Ptr<MecFlow> flow = *it;

		//Only start flows from local VM
		if (!allFlow && flow->GetSrcEntityId() != m_id) {
			continue;
		}

		flow->StartApplication();

	}
	std::ostringstream msg2;
	m_isStart = true;
	msg2 << (IsIsUe() ? "ueId=" : "vm=id") << m_id << " ON on nodeId="
			<< GetHostServerId();
	MecLogger::Logging(msg2.str());

	return true;
}

bool MecVm::Shutdown() {

	std::ostringstream msg;

	msg << (IsIsUe() ? "ueId=" : "vmId=") << m_id
			<< "  shutting down on nodeId=" << GetHostServerId();
	MecLogger::Logging(msg.str());

	for (MecFlowContainer::const_iterator it = m_flowList.CBegin();
			it != m_flowList.CEnd(); ++it) {

		Ptr<MecFlow> flow = *it;
		flow->StopApplication();

	}

	std::ostringstream msg2;
	m_isStart = false;
	msg2 << (IsIsUe() ? "ueId=" : "vmId=") << m_id << " OFF on nodeId="
			<< GetHostServerId();
	MecLogger::Logging(msg2.str());

	return true;
}

uint64_t MecVm::CalculateMigrationTraffic() {

	uint32_t numCycle = 0;
	uint64_t totalTraffic = 0;

	uint32_t numCondition1 = ceil(
			log((1.0 * m_liveMig_T * m_liveMig_L) / m_liveMig_VmSize)
					/ log(1.0 * m_liveMig_DirtyRate / m_liveMig_L));
	uint32_t numCondition2 = ceil(
			log(
					(m_liveMig_X * m_liveMig_DirtyRate)
							/ (m_liveMig_VmSize
									* (m_liveMig_L - m_liveMig_DirtyRate)))
					/ log(1.0 * m_liveMig_DirtyRate / m_liveMig_L));

	if (numCondition1 > numCondition2) {
		numCycle = numCondition2;
	} else {
		numCycle = numCondition1;
	}

	//sanity check
	if (numCycle < 0) {
		numCycle = 0;
	}

	totalTraffic = m_liveMig_VmSize
			* (1 - pow(1.0 * m_liveMig_DirtyRate / m_liveMig_L, numCycle + 1)) //1-(R/L)^(n+1)
	/ (1 - 1.0 * m_liveMig_DirtyRate / m_liveMig_L);

	m_migrationTraffic = totalTraffic;
	return totalTraffic;
}

bool MecVm::MigrateToServer(uint32_t newNodeID) {

	NS_ASSERT_MSG(!m_isStart,
			"ERROR: Trying to migrate a running VM, vmID=" << m_id);

//	//updating both src and dst server resources
//	Ptr<MecVmHypervisor> srcHvapp = MecUtilsHelper::GetHyperVisor(
//			m_hostServerId);
//	Ptr<MecVmHypervisor> dstHvapp = MecUtilsHelper::GetHyperVisor(newNodeID);

//	dstHvapp->IncreaseCpuUse(m_cpu);
//	dstHvapp->IncreaseMemoryUse(m_memory);
//
//	srcHvapp->ReduceCpuUse(m_cpu);
//	srcHvapp->ReduceMemoryUse(m_memory);
//
//	if (dstHvapp->GetUseCpu() <= dstHvapp->GetTotalCpu()
//			&& dstHvapp->GetUseMemory() <= dstHvapp->GetTotalMemory()) {
//
//		dstHvapp->SetOverloaded(false);
//	}
//
//	else if (allowTempOverload) {
//		dstHvapp->SetOverloaded(true);
//	}
//
//	else {
//		NS_ASSERT_MSG(
//				dstHvapp->GetUseCpu() <= dstHvapp->GetTotalCpu()
//						&& dstHvapp->GetUseMemory()
//								<= dstHvapp->GetTotalMemory(),
//				"Trying to migrate a VM to a server without enough resource, vm id=" << m_id << ", from server " << m_hostServerId << " to server " << newNodeID);
//	}

	//update VM hostserver
	m_hostServerId = newNodeID;

	return true;
}

void MecVm::AddFlow(Ptr<MecFlow> flow) {
	m_flowList.Add(flow);
}

uint32_t MecVm::GetId() const {
	return m_id;
}

uint32_t MecVm::GetHostServerId() const {
	return m_hostServerId;
}

void MecVm::SetHostServerId(uint32_t vmId) {
	m_hostServerId = vmId;
}

uint32_t MecVm::GetCpu() const {
	return m_cpu;
}

void MecVm::SetCpu(uint32_t cpu) {
	m_cpu = cpu;
}

uint32_t MecVm::GetMemory() const {
	return m_memory;
}

void MecVm::SetMemory(uint32_t memory) {
	m_memory = memory;
}

uint32_t MecVm::GetLiveMigDirtyRate() const {
	return m_liveMig_DirtyRate;
}

void MecVm::SetLiveMigDirtyRate(uint32_t liveMigDirtyRate) {
	m_liveMig_DirtyRate = liveMigDirtyRate;
}

uint32_t MecVm::GetLiveMigL() const {
	return m_liveMig_L;
}

void MecVm::SetLiveMigL(uint32_t liveMigL) {
	m_liveMig_L = liveMigL;
}

uint32_t MecVm::GetLiveMigT() const {
	return m_liveMig_T;
}

void MecVm::SetLiveMigT(uint32_t liveMigT) {
	m_liveMig_T = liveMigT;
}

uint32_t MecVm::GetLiveMigVmSize() const {
	return m_liveMig_VmSize;
}

void MecVm::SetLiveMigVmSize(uint32_t liveMigVmSize) {
	m_liveMig_VmSize = liveMigVmSize;
}

uint32_t MecVm::GetLiveMigX() const {
	return m_liveMig_X;
}

void MecVm::SetLiveMigX(uint32_t liveMigX) {
	m_liveMig_X = liveMigX;
}

void MecVm::SetIsUe(bool isUe) {
	m_isUE = isUe;
}

bool MecVm::IsIsUe() const {
	return m_isUE;
}

bool MecVm::IsIsVm() const {
	return !m_isUE;
}

void MecVm::SetBindId(uint32_t bindId) {
	m_bindId = bindId;
}

uint32_t MecVm::GetBindId() const {
	return m_bindId;
}

bool MecVm::IsOn() const {
	return m_isStart;
}
bool MecVm::IsOff() const {
	return !IsOn();
}

void MecVm::Print(std::ostream& os) const {

	// getting string
	if (IsIsUe()) {
		os << "UE=";
	} else {
		os << "VM=";
	}

	os << GetId() << " Base-Station=" << GetHostServerId() << "\tIP="
			<< MecUtilsHelper::GetIpv4Address(GetHostServerId(), 1, 0)
			<< std::endl;
}

} /* namespace ns3 */
