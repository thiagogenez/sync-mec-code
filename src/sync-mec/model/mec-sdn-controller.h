/*
 * mec-sdn-controller.h
 *
 *  Created on: 14 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_SDN_CONTROLLER_H_
#define MEC_SDN_CONTROLLER_H_

#include <map>
#include <vector>

#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/application-container.h"

#include "ns3/mec-vm-container.h"
#include "ns3/mec-backhaul-core-helper.h"
#include "ns3/mec-middle-box-container.h"
#include "ns3/mec-policy-container.h"
#include "ns3/mec-flow-container.h"
#include "ns3/mec-pair-communication-container.h"
#include "ns3/mec-policy-migration-helper.h"
#include "ns3/mec-sdn-controller-helper.h"
#include "ns3/mec-flow-monitor.h"
#include "ns3/mec-e2e-checker.h"

#include "ns3/mec-migrations-stats.h"

namespace ns3 {

class MecSdnController: public Application {
public:

	static TypeId GetTypeId(void);

	MecSdnController();
	virtual ~MecSdnController();

	//getters
	bool IsSync() const;

	//setters
	void SetBackhaulCoreHelper(Ptr<MecBackhaulCoreHelper> backhaulCoreHelper);
	void SetMiddleBoxContainer(MecMiddleBoxContainer* middleBoxContainer);
	void SetUeContainer(MecVmContainer* ueContainer);
	void SetUeToVmFlowContainer(MecFlowContainer* ueToVmFlowContainer);
	void SetUeToVmPolicyContainer(MecPolicyContainer* ueToVmPolicyContainer);
	void SetVmContainer(MecVmContainer* vmContainer);
	void SetVmToUeFlowContainer(MecFlowContainer* vmToUeFlowContainer);
	void SetVmToUePolicyContainer(MecPolicyContainer* vmToUePolicyContainer);
	void SetHypervisorContainer(ApplicationContainer* hypervisorContainer);
	void SetScheduleStrategy(uint32_t strategy);
	void SetUeMobilityStrategy(uint32_t strategy);
	void SetCommunicationContainer(
			MecPairCommunicationContainer *communicationContainer);

	// Inherited from Application base class.
	// Called at time specified by Start
	virtual void StartApplication();
	// Called at time specified by Stop
	virtual void StopApplication();

private:

	//----------------------Strategies---------------------------//
	void SyncMecStrategy();
	void StaticStrategy();
	void PrimalStrategy();
	//----------------------Strategies---------------------------//

	//----------------------SyncMEC---------------------------//
	// PHASE 1
	void SyncPhase_I(uint64_t helperId);
	void SyncPhase_I_Initialisation(Ptr<MecVm> vm,
			NodeContainer *validVmDagNodes, Ptr<MecVm> ue,
			NodeContainer *validUeDagNodes, uint32_t numberOfEndNodes,
			std::map<Ptr<MecFlow>, Ptr<MecPolicy>>& policyFlowMap,
			bool isDownloadFlow, uint64_t helperId,
			std::vector<Ptr<MecPolicyMigrationHelper>> &waitingList);
	void SyncPhase_I_MigratePolicies(uint32_t policyMigrationHelperId,
			uint64_t helperId);
	uint32_t SyncPhase_I_GetCandidateMiddleDagNodes(Ptr<MecFlow> flow,
			Ptr<MecPolicy> policy,
			std::vector<MecMiddleBoxContainer> &validMiddleBoxesDagNodes);
	uint32_t SyncPhase_I_GetCandidateEndDagNodes(Ptr<MecVm> ue, Ptr<MecVm> vm,
			NodeContainer *validVmDagNodes, NodeContainer *validUeDagNodes);
	NodeContainer SyncPhase_I_GetCandidateServerHosts(Ptr<MecVm> vm);
	MecMiddleBoxContainer SyncPhase_I_GetCandidatesMiddleBoxes(
			Ptr<MecMiddleBox> mb, Ptr<MecFlow> flow);

	// PHASE 2
	void SyncPhase_II(uint64_t helperId);
	void SyncPhase_II_WaitingToStart(uint64_t helperId);
	//----------------------SyncMEC---------------------------//

	//----------------------PRIMAL---------------------------//
	void Primal(uint64_t helperId);
	uint32_t PrimalGetMiddleDagNodes(Ptr<MecFlow> flow, Ptr<MecPolicy> policy,
			std::vector<MecMiddleBoxContainer> &validMiddleBoxesDagNodes);
	uint32_t PrimalGetCandidateEndDagNodes(Ptr<MecVm> ue, Ptr<MecVm> vm,
			NodeContainer *validVmDagNodes, NodeContainer *validUeDagNodes);

	void PrimalInitialisation(Ptr<MecVm> vm, NodeContainer *validVmDagNodes,
			Ptr<MecVm> ue, NodeContainer *validUeDagNodes,
			uint32_t numberOfEndNodes,
			std::map<Ptr<MecFlow>, Ptr<MecPolicy>>& policyFlowMap,
			bool isDownloadFlow, uint64_t helperId);
	void PrimalMigrateVMs(uint64_t helperId, Ptr<Node> node );
	//----------------------PRIMAL---------------------------//

	//----------------------UE MOBILITY---------------------------//
	// THIS FUNCTION SHOULD BE IN ANOTHER PLACE SINCE SDN IS NOT RESPONSIBLE FOR
	// UE's mobility. However, as an UE's structure is the same as an VM's structure,
	// an UE mobility funcionality is the same as an VM migration. Thus, I put this function here
	// only to simulate a Random UE mobility.
	void RandomlyUeMobility();
	MecPairCommunicationContainer GetNextUEsToMove();
	void ScheduleNextMobility(Time nextTime);
	//----------------------UE MOBILITY---------------------------//

	//----------------------General---------------------------//

	void MigrateEntity(Ptr<MecPairCommunication> pair, Ptr<Node> newHostServer,
			bool migrateVm);
	void UpdateFlowsBeforeVmMigration(Ptr<MecVm> vm, Ptr<Node> newHostServer,
			std::map<Ptr<MecFlow>, Ptr<MecPolicy>>& policyFlowMap);
	//----------------------General---------------------------//

	//------------Management of the next policy check event------------//
	MecPairCommunicationContainer GetNextPairs(uint32_t);
	void ReleaseHelper(uint64_t helperId);
	void AssignHelper(uint64_t helperId);
	uint64_t CreateMecSdnHelper(Ptr<MecPairCommunication> pair);
	void ScheduleNextPolicyCheck(Time nextTime);
	//------------Management of the next policy check event------------//

	MecVmContainer m_toBeCheckedVMs;

	// Global info
	Ptr<MecBackhaulCoreHelper> m_backhaulCoreHelper;
	MecMiddleBoxContainer *m_middleBoxContainer;

	// VMs and theirs output flows to UEs according to policies
	MecVmContainer *m_vmContainer;
	MecFlowContainer *m_vmToUeFlowContainer;
	MecPolicyContainer *m_vmToUePolicyContainer;

	//UEs and theris output flows to VMs
	MecVmContainer *m_ueContainer;
	MecFlowContainer *m_ueToVmFlowContainer;
	MecPolicyContainer *m_ueToVmPolicyContainer;

	//Mec Communication container
	MecPairCommunicationContainer *m_pairCommunicationContainer;

	//hypervisor container
	ApplicationContainer *m_hypervisorContainer;

	//whether policy-aware
	uint32_t m_scheduleStrategy = -1;

	// mobility strategy
	uint32_t m_ueMobilityStrategy = -1;

	std::map<uint64_t, Ptr<MecSdnControllerHelper>> m_helper;
	std::vector<uint64_t> m_busyHelper;

	// to control policy check event
	EventId m_policyCheckEvent;

	// to control policy check event
	EventId m_ueMobilityEvent;

	// RR Base station check
	uint32_t m_bsInAnalyse = 0;

	// migrations analysis
	MecMigrationsStats m_migrationStas;

};

} /* namespace ns3 */

#endif /* MEC_SDN_CONTROLLER_H_ */
