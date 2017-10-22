/*
 * mec-vm.h
 *
 *  Created on: 12 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_VM_H_
#define MEC_VM_H_


#include "ns3/mec-flow-container.h"
#include "ns3/mec-policy-container.h"
#include "ns3/mec-policy.h"

namespace ns3 {

class MecVm: public Object {
public:
	static TypeId GetTypeId();

	MecVm();
	virtual ~MecVm();

	bool TurnOn(bool allFlow);
	bool Shutdown();
	uint64_t CalculateMigrationTraffic();

	bool MigrateToServer(uint32_t newNodeID);

	void AddFlow(Ptr<MecFlow> flow);

	//getters
	uint32_t GetId() const;
	uint32_t GetBindId() const;
	uint32_t GetHostServerId() const;
	uint32_t GetCpu() const;
	uint32_t GetMemory() const;
	uint32_t GetLiveMigDirtyRate() const;
	uint32_t GetLiveMigL() const;
	uint32_t GetLiveMigT() const;
	uint32_t GetLiveMigVmSize() const;
	uint32_t GetLiveMigX() const;
	bool IsIsUe() const;
	bool IsIsVm() const;
	bool IsOn() const;
	bool IsOff() const;

	//setters
	void SetHostServerId(uint32_t vmId);
	void SetCpu(uint32_t cpu);
	void SetMemory(uint32_t memory);
	void SetLiveMigDirtyRate(uint32_t liveMigDirtyRate);
	void SetLiveMigL(uint32_t liveMigL);
	void SetLiveMigT(uint32_t liveMigT);
	void SetLiveMigVmSize(uint32_t liveMigVmSize);
	void SetLiveMigX(uint32_t liveMigX);
	void SetIsUe(bool isUe);
	void SetBindId(uint32_t bindId);
	void Print(std::ostream& os) const;

private:

	static uint32_t m_counter;

	//VM's ID
	uint32_t m_id;

	//Binded ID
	uint32_t m_bindId;

	// VM's cpu capacity
	uint32_t m_cpu = 0;

	//VM's memory capacity
	uint32_t m_memory = 0;

	// current host server
	uint32_t m_hostServerId = 0;

	//Is VM started
	bool m_isStart = false;

	//flows start and to this vm
	MecFlowContainer m_flowList;

	//For migration cost
	uint32_t m_liveMig_VmSize = 0;
	uint32_t m_liveMig_DirtyRate = 0; //%
	uint32_t m_liveMig_L = 0;
	uint32_t m_liveMig_X = 0;
	uint32_t m_liveMig_T = 0;
	uint64_t m_migrationTraffic = 0;

	bool m_isUE = false;

};

} /* namespace ns3 */

#endif /* MEC_VM_H_ */
