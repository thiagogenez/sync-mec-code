/*
 * mec-vm-hypervisor.h
 *
 *  Created on: 14 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_VM_HYPERVISOR_H_
#define MEC_VM_HYPERVISOR_H_

#include "ns3/application.h"

#include "ns3/mec-vm.h"
#include "ns3/mec-vm-container.h"

namespace ns3 {

class MecVmHypervisor: public Application {
public:

	static TypeId GetTypeId(void);

	MecVmHypervisor();
	virtual ~MecVmHypervisor();

	void AddVm(Ptr<MecVm> vm);
	void RemoveVm(Ptr<MecVm> vm);

	void AddUe(Ptr<MecVm> vm);
	void RemoveUe(Ptr<MecVm> vm);

	bool IsVmAcceptable(Ptr<MecVm> vm) const;
	bool ContainsUe(Ptr<MecVm> ue) const;
	bool ContainsVm(Ptr<MecVm> vm) const;

	//getters
	uint32_t GetTotalCpu() const;
	uint32_t GetTotalMemory() const;
	uint32_t GetUsedCpu() const;
	uint32_t GetUsedMemory() const;
	uint32_t GetId() const;

	//setters
	void SetTotalCpu(uint32_t totalCpu);
	void SetTotalMemory(uint32_t totalMemory);

private:

	static uint32_t m_counter;

	//Hypervisor's ID
	uint32_t m_id;

	// Has been this started by StartApplication()?
	bool isStarted = false;

	//VMs hosted by current server
	MecVmContainer m_hostedVMs;

	MecVmContainer m_hostedUEs;

	uint32_t m_totalCpu = 0;
	uint32_t m_totalMemory = 0;
	uint32_t m_useCPU = 0;
	uint32_t m_useMemory = 0;

	void IncreaseCpuUse(uint32_t cpu);
	void IncreaseMemoryUse(uint32_t memory);
	void ReduceCpuUse(uint32_t cpu);
	void ReduceMemoryUse(uint32_t memory);

protected:

	// Inherited from Application base class.
	// Called at time specified by Start
	virtual void StartApplication();
	// Called at time specified by Stop
	virtual void StopApplication();

};

} /* namespace ns3 */

#endif /* MEC_VM_HYPERVISOR_H_ */
