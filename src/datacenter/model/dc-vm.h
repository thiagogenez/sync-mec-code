#ifndef DC_VM_H
#define DC_VM_H

#include <stdint.h>
#include <string>
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"

//#include "ns3/data-rate.h"
#include "ns3/node-list.h"

#include "ns3/dc-fat-tree-helper.h"
#include "ns3/dc-flow.h"
#include "ns3/dc-policy.h"
//#include "ns3/dc-vm-hypervisor.h"

#include "ns3/random-variable-stream.h"

namespace ns3 {

//#define POLICY_UDP_PROTO	17
//#define POLICY_TCP_PROTO	6

#define DC_UDP_DST_PORT 999
#define DC_TCP_DST_PORT 999

class DataRate;
class DcVmHypervisorApplication;

class DcVm: public Object {
private:
	Ptr<UniformRandomVariable> m_uniformRand;

public:
	//std::vector<DcVmNewItem> m_allVMs;
	//std::vector<Ptr<DcFlowApplication> > m_allFlows;

	//uint32_t vmSeedID;
	//uint32_t flowSeedID;

	uint16_t tcpPortSeed;
	uint16_t udpPortSeed;

public:

	Ptr<DcFatTreeHelper> fattree;

	DcVm();
	~DcVm();
	static TypeId GetTypeId(void);
	bool startVM(bool allFlow);
	bool stopVM();
	uint64_t CalcMigrationTraffic();
	bool MigrateToServer(uint32_t newNodeID, DcPolicy *policies,
			bool allowTempOverload);
	uint64_t CalcMigrationCost(uint32_t toServerID);

	Ptr<DcFlowApplication> createFlows(uint32_t srcVmID, uint32_t dstVmID,
			bool isTCP, DataRate rate, uint32_t numPackets);

	bool createVM(Ptr<Node> node, uint32_t cpu, uint32_t memory,
			uint32_t liveMig_VmSize, uint32_t liveMig_DirtyRate,
			uint32_t liveMig_L, uint32_t liveMig_T, uint32_t liveMig_X);

//DcVmNewItemI getVM(uint32_t id);
//bool isValid(DcVmNewItemI it);

	void setFirstNFlow(uint32_t num, bool status);

private:

	Ptr<DcVmHypervisorApplication> getHyperVisor(uint32_t NodeID);

public:
	uint32_t m_id;
	uint32_t m_hostServer; //current host server

	uint32_t m_cpu;
	uint32_t m_memory;

	bool isStart;  //Is VM started

	Time nextTime; //Abandoned

	uint64_t CommCost;

	//std::map<uint32_t,Ptr<DcFlowApplication> > flows;

	DcFlowContainer m_flowList; //flows start and to this vm

	//For migration cost
	uint32_t liveMig_VmSize;
	uint32_t liveMig_DirtyRate; //%
	uint32_t liveMig_L;
	uint32_t liveMig_X;
	uint32_t liveMig_T;

	uint64_t migrationTraffic;

};

class DcVmContainer {
public:
	typedef std::vector<Ptr<DcVm> >::const_iterator Iterator;
	//typedef std::vector< Ptr<DcVm> >::iterator Iterator;

	DcVmContainer();
	Iterator Begin(void) const;
	Iterator End(void) const;
	uint32_t GetN(void) const;
	Ptr<DcVm> Get(uint32_t i) const;
	DcVmContainer::Iterator GetVmByID(uint32_t vmID) const;
	bool IsVmExist(uint32_t vmID) const;

	void Add(Ptr<DcVm> vm); //do not allow to add, use create for centrallized management
	void Add(DcVmContainer other);
	void Remove(Ptr<DcVm> vm);

	void Create(uint32_t n);
	void print(std::ostream& os);
private:
	std::vector<Ptr<DcVm> > m_VmList;
};

} // namespace ns3

#endif
