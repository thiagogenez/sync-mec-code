/*
 * mec-sdn-controller.cc
 *
 *  Created on: 14 Jun 2017
 *      Author: thiagogenez
 */

#include <algorithm>
#include <cmath>

#include "ns3/log.h"

//#include "ns3/assert.h"
//#include "ns3/log-macros-enabled.h"

#include "ns3/mec-constants-helper.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-logger.h"

#include "mec-sdn-controller.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecSdnController");
NS_OBJECT_ENSURE_REGISTERED(MecSdnController);

TypeId MecSdnController::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::MecSdnController").SetParent<Application>().AddConstructor<
					MecSdnController>();
	return tid;

}

MecSdnController::MecSdnController() {
	NS_LOG_FUNCTION(this);
}

MecSdnController::~MecSdnController() {
}

void MecSdnController::StopApplication() {

	std::ostringstream msg;
	msg << "policiesAnalysed=" << m_migrationStas.GetPoliciesAnalysed();
	msg << ";policiesMigrated=" << m_migrationStas.GetPolciesMigrated();
	msg << ";policyMigrationRatio="
			<< m_migrationStas.GetPolciesMigrated()
					/ (m_migrationStas.GetPoliciesAnalysed() * 1.0);

	msg << ";vmAnalysed=" << m_migrationStas.GetVmAnalysed();
	msg << ";vmMigrated=" << m_migrationStas.GetVmMigrated();
	msg << ";vmMigrationRatio="
			<< m_migrationStas.GetVmMigrated()
					/ (m_migrationStas.GetVmAnalysed() * 1.0);

	MecLogger::Logging(msg.str());

}

void MecSdnController::StartApplication() {
	NS_LOG_FUNCTION(this);

	switch (m_scheduleStrategy) {
	case 1:
		MecLogger::Logging("Simulation started using STATIC");
		break;
	case 2:
		MecLogger::Logging("Simulation started using SYNC");
		break;
	case 3:
		MecLogger::Logging("Simulation started using PRIMAL");
		break;
	default:
		MecLogger::Logging("Unknown schedule strategy, stopping simulator");
		Simulator::Stop();
		return;
	}

	std::ostringstream msg;
	msg << "Simulation finisth at time "
			<< MecConstantsHelper::SIMULATION_STOP_TIME;

	MecLogger::Logging(msg.str());

	switch (m_ueMobilityStrategy) {
	case 1:
		MecLogger::Logging("Simulation started using NO-UE-MOBILITY");
		break;
	case 2:
		MecLogger::Logging("Simulation started using ONE-TIME-MOBILITY");
		break;
	case 3: {

		std::ostringstream msg;
		msg << "Simulation started using MANY-TIMES-MOBILITY, every "
				<< MecConstantsHelper::UE_NEXT_MOBILITY_TIME << " ["
				<< MecConstantsHelper::UE_MOBILITY_MIN_PERCENTAGEM << ","
				<< MecConstantsHelper::UE_MOBILITY_MAX_PERCENTAGEM << "]% of "
				<< m_pairCommunicationContainer->GetSize() << " UEs will move";

		MecLogger::Logging(msg.str());
		break;
	}
	default:
		MecLogger::Logging("Unknown mobility strategy, stopping simulator");
		Simulator::Stop();
		return;
	}

	// schedule the procedure
	ScheduleNextPolicyCheck(
			MecConstantsHelper::SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME);

	// schedule the mobility
	ScheduleNextMobility(MecConstantsHelper::UE_NEXT_MOBILITY_TIME);

	// schedule the stop time
	Simulator::Schedule(
			MecConstantsHelper::SIMULATION_STOP_TIME - MicroSeconds(1),
			&MecSdnController::StopApplication, this);

}

void MecSdnController::ScheduleNextPolicyCheck(Time nextTime) {

	NS_LOG_FUNCTION(this);

	if (m_policyCheckEvent.IsRunning()) {

		std::cout
				<< "MecSdnController::ScheduleNextPolicyCheck Event will be canceled"
				<< std::endl;

		m_policyCheckEvent.Cancel();
	}

	switch (m_scheduleStrategy) {
	case 1:
		m_policyCheckEvent = Simulator::Schedule(nextTime,
				&MecSdnController::StaticStrategy, this);
		break;
	case 2:
		m_policyCheckEvent = Simulator::Schedule(nextTime,
				&MecSdnController::SyncMecStrategy, this);
		break;
	case 3:
		m_policyCheckEvent = Simulator::Schedule(nextTime,
				&MecSdnController::PrimalStrategy, this);
		break;
	}
}

void MecSdnController::ScheduleNextMobility(Time nextTime) {

	NS_LOG_FUNCTION(this);

//	if (!m_policyCheckEvent.IsRunning()) {

	if (m_ueMobilityEvent.IsRunning()) {

		std::cout
				<< "MecSdnController::ScheduleNextMobility Event will be canceled"
				<< std::endl;

		m_ueMobilityEvent.Cancel();
	}

	switch (m_ueMobilityStrategy) {
	case 1:
		// no-mobility, so do nothing
		break;
	case 2: {
		Time finish = MecConstantsHelper::SIMULATION_STOP_TIME;

		std::ostringstream msg;
		msg << "MOBILITY -> next UE mobility trying at " << (finish / 2);
		MecLogger::Logging(msg.str());

		m_ueMobilityEvent = Simulator::Schedule(((finish / 2)),
				&MecSdnController::RandomlyUeMobility, this);
	}

		break;
	case 3: {

		Time now = Simulator::Now();
		std::ostringstream msg;
		msg << "MOBILITY -> next UE mobility trying at " << (now + nextTime);
		MecLogger::Logging(msg.str());

		m_ueMobilityEvent = Simulator::Schedule(nextTime,
				&MecSdnController::RandomlyUeMobility, this);
	}
		break;
	}

}

void MecSdnController::ReleaseHelper(uint64_t helperId) {

	std::ostringstream msg;
	msg << "SDN -> Unlocking helperId =" << helperId;
	MecLogger::Logging(msg.str());

	m_busyHelper.erase(
			std::remove(m_busyHelper.begin(), m_busyHelper.end(), helperId),
			m_busyHelper.end());
}
void MecSdnController::AssignHelper(uint64_t helperId) {
	std::ostringstream msg;
	msg << "SDN -> Locking helperId =" << helperId;
	MecLogger::Logging(msg.str());

	m_busyHelper.push_back(helperId);
}

MecPairCommunicationContainer MecSdnController::GetNextPairs(
		uint32_t bsNumber) {

	Ptr<Node> bs = m_backhaulCoreHelper->HostNodes().Get(bsNumber);

	std::ostringstream msg;
	msg << "SDN -> policy check at base-station nodeId=" << bs->GetId();
	MecLogger::Logging(msg.str());

	MecPairCommunicationContainer next;

	for (MecPairCommunicationContainer::const_iterator itPair =
			m_pairCommunicationContainer->CBegin();
			itPair != m_pairCommunicationContainer->CEnd(); itPair++) {

		Ptr<MecPairCommunication> pair = (*itPair);

		if (pair->GetUe()->GetHostServerId() == bs->GetId()) {
			next.Add(pair);
		}
	}

	return next;
}

uint64_t MecSdnController::CreateMecSdnHelper(Ptr<MecPairCommunication> pair) {
	Ptr<MecSdnControllerHelper> helper = CreateObject<MecSdnControllerHelper>();
	m_helper.insert(
			std::pair<uint64_t, Ptr<MecSdnControllerHelper>>(helper->GetId(),
					helper));
	helper->SetPair(pair);
	return helper->GetId();
}

void MecSdnController::SyncMecStrategy() {

	if (m_busyHelper.empty()) {

		MecLogger::Logging("SDN -> Free, starting SYNC-MEC strategy... ");

		MecPairCommunicationContainer pairs = GetNextPairs(m_bsInAnalyse);

		m_bsInAnalyse++;
		if (m_bsInAnalyse >= m_backhaulCoreHelper->HostNodes().GetN()) {
			m_bsInAnalyse = 0;
		}

		for (uint32_t i = 0; i < pairs.GetSize(); i++) {

			Ptr<MecPairCommunication> pair = pairs.GetPairByIndex(i);

			uint64_t helperId = CreateMecSdnHelper(pair);

			// lock helper Id until finalisation
			AssignHelper(helperId);

			Simulator::Schedule(MicroSeconds(i), &MecSdnController::SyncPhase_I,
					this, helperId);

		}
	}

	std::string busyList = MecUtilsHelper::VectorToString(m_busyHelper.begin(),
			m_busyHelper.end());
	std::ostringstream msg;
	msg << "SDN -> busy checking helperIds=[" << busyList
			<< "],  trying at time..."
			<< Simulator::Now()
					+ MecConstantsHelper::SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME;
	MecLogger::Logging(msg.str());

	ScheduleNextPolicyCheck(
			MecConstantsHelper::SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME);
}

void MecSdnController::SyncPhase_I(uint64_t helperId) {

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];
	Ptr<MecPairCommunication> pair = helper->GetPair();

	// real time measurement
	helper->ResetTimer();

	std::ostringstream msg;
	msg << "Phase I -> started for helperId=" << helperId << " and pairId="
			<< pair->GetId();
	MecLogger::Logging(msg.str());

	Ptr<MecVm> ue = pair->GetUe();
	Ptr<MecVm> vm = pair->GetVm();

	// container that represents the valid nodes that may host VM
	NodeContainer validVmDagNodes;

	// container that represents the current node that "host" UE
	NodeContainer validUeDagNodes;

	uint32_t numberOfEndNodes = SyncPhase_I_GetCandidateEndDagNodes(ue, vm,
			&validVmDagNodes, &validUeDagNodes);

	// helper
	helper->AddtHostServersCandidatesToVm(validVmDagNodes);

	// trigger all pair's flows (upload and download) of phase_I at the same time
	std::vector<Ptr<MecPolicyMigrationHelper>> waitingList;

	// analyse download policy flows VM -> UE
	SyncPhase_I_Initialisation(vm, &validVmDagNodes, ue, &validUeDagNodes,
			numberOfEndNodes, pair->GetDownloadPolicyFlowMap(), true, helperId,
			waitingList);

	// analyse upload policy flows UE -> VM
	SyncPhase_I_Initialisation(vm, &validVmDagNodes, ue, &validUeDagNodes,
			numberOfEndNodes, pair->GetUploadPolicyFlowMap(), false, helperId,
			waitingList);

	// all flow of the current pairId associated with the helperId are policy free,
	// then realease helperId and return this function
	if (waitingList.empty()) {

		std::ostringstream msg;
		msg << "Phase I -> all flows related to helperId=" << helperId
				<< " and pairId=" << pair->GetId() << " are policy-free";
		MecLogger::Logging(msg.str());

		ReleaseHelper(helperId);
		return;
	}

	for (uint32_t i = 0; i < waitingList.size(); i++) {

		m_migrationStas.IncrementPolicyAnalyse();

		Simulator::Schedule(MicroSeconds(i),
				&MecSdnController::SyncPhase_I_MigratePolicies, this,
				waitingList[i]->GetId(), helperId);
	}

}

void MecSdnController::SyncPhase_I_Initialisation(Ptr<MecVm> vm,
		NodeContainer *validVmDagNodes, Ptr<MecVm> ue,
		NodeContainer *validUeDagNodes, uint32_t numberOfEndNodes,
		std::map<Ptr<MecFlow>, Ptr<MecPolicy>>& policyFlowMap,
		bool isDownloadFlow, uint64_t helperId,
		std::vector<Ptr<MecPolicyMigrationHelper>> &waitingList) {

	std::map<Ptr<MecFlow>, Ptr<MecPolicy>>::const_iterator it;

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];

	for (it = policyFlowMap.begin(); it != policyFlowMap.end(); it++) {

		Ptr<MecFlow> flow = it->first;
		Ptr<MecPolicy> policy = it->second;

		if (policy->IsIsOk()) {

			// containers that represent tge valid nodes for MBs
			std::vector<MecMiddleBoxContainer> validMiddleBoxDagNodes;

			// calculate number of DAG nodes
			uint32_t numberOfMiddleNodes =
					SyncPhase_I_GetCandidateMiddleDagNodes(flow, policy,
							validMiddleBoxDagNodes);

			// create a policy-flow migration helper
			Ptr<MecPolicyMigrationHelper> policyMigrationHelper = CreateObject<
					MecPolicyMigrationHelper>();

			// define initial values
			policyMigrationHelper->SetIsDownloadFlow(isDownloadFlow);
			policyMigrationHelper->SetIsPolicyMigrationDone(false);

			//store the policy and flow
			policyMigrationHelper->SetPolicy(policy);
			policyMigrationHelper->SetFlow(flow);

			// add policy to the helper
			helper->AddMecPolicyMigrationHelper(policyMigrationHelper);

			// building the DAG = Flow-Policy Network
			if (isDownloadFlow) {

				policyMigrationHelper->DagConstruction(
						(numberOfEndNodes + numberOfMiddleNodes), vm,
						validVmDagNodes, validMiddleBoxDagNodes,
						validUeDagNodes, ue);

			} else {
				policyMigrationHelper->DagConstruction(
						(numberOfEndNodes + numberOfMiddleNodes), ue,
						validUeDagNodes, validMiddleBoxDagNodes,
						validVmDagNodes, vm);
			}

			// wait list
			waitingList.push_back(policyMigrationHelper);
		}

		else {
			std::ostringstream msg;
			msg << "SYNC -> Policy FREE helperId=" << helperId << " pairId="
					<< helper->GetPairId() << " policyId=" << policy->GetId()
					<< " flowId=" << flow->GetId();
			MecLogger::Logging(msg.str());
		}
	}

}

void MecSdnController::SyncPhase_I_MigratePolicies(
		uint32_t policyMigrationHelperId, uint64_t helperId) {

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];
	Ptr<MecPolicyMigrationHelper> policyMigrationHelper =
			helper->GetPolicyMigrationHelper(policyMigrationHelperId);

	if (policyMigrationHelper->IsShortestPathValid()) {

		// get new middlebox for the policy
		MecMiddleBoxContainer newMiddleboxPath =
				policyMigrationHelper->GetNewMiddleBoxContainer();

		// get the policy to be migrated (update)
		Ptr<MecPolicy> policy = policyMigrationHelper->GetPolicy();

		// get the flow related to this policy
		Ptr<MecFlow> flow = policyMigrationHelper->GetFlow();

		bool someMigrationPerformed = false;
		for (uint32_t i = 0; i < policy->GetLen(); i++) {

			//get the old middlebox
			Ptr<MecMiddleBox> oldMb = policy->GetMiddleBoxByIndex(i);

			// get the new middlebox
			Ptr<MecMiddleBox> newMb = newMiddleboxPath.GetMiddleboxByIndex(i);

			// if it is not the same
			if (oldMb->GetId() != newMb->GetId()) {

				//finally perform the migration
				policy->SetMiddleBox(i, newMb);

				//update MB's capacity
				oldMb->FlowRelease(flow);
				newMb->FlowAssign(flow);

				// stats
				m_migrationStas.IncrementPolicyMigration();

				std::ostringstream msg;
				msg << "Phase I -> Migrating MB id=" << oldMb->GetId()
						<< " type=" << oldMb->GetMiddleBoxType() << " to MB id="
						<< newMb->GetId() << " type="
						<< newMb->GetMiddleBoxType() << " for helperId="
						<< helperId << " pairId=" << helper->GetPairId()
						<< " policyId=" << policy->GetId() << " flowId="
						<< flow->GetId() << ",\twaiting to start Phase II ";

				someMigrationPerformed = true;
				MecLogger::Logging(msg.str());
			}
		}

		if (!someMigrationPerformed) {
			std::ostringstream msg;
			msg << "Phase I -> No MB migration needed for helperId=" << helperId
					<< " pairId=" << helper->GetPairId() << " policyId="
					<< policy->GetId() << " flowId=" << flow->GetId()
					<< ",\twaiting to start Phase II ";
			MecLogger::Logging(msg.str());
		}

		policyMigrationHelper->SetIsPolicyMigrationDone(true);

		// check if the wait for start phase II has been trigged
		// to avoid loads of call for phase II
		if (!helper->IsPhase_II_InitiationTrigged()) {
			helper->TriggerPhase_II();
			SyncPhase_II_WaitingToStart(helperId);
		}
	}

	else {
		NS_ASSERT_MSG(false,
				"Phase I -> PONG FAILED for helperId=" << helperId << " and pairId=" << helper->GetPairId());
	}
}

void MecSdnController::SyncPhase_II_WaitingToStart(uint64_t helperId) {

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];

	if (helper->AllPolicyMigrationDone()) {
		std::ostringstream msg;
		msg << "Phase I -> completed for helperId=" << helperId
				<< " and pairId=" << helper->GetPairId()
				<< ", starting Phase II ....";
		MecLogger::Logging(msg.str());

		// real time measurement
		double duration = helper->ElapseTimer();
		MecLogger::LoggingRealTime(1, duration);
		helper->ResetTimer();

		Simulator::Schedule(MicroSeconds(1), &MecSdnController::SyncPhase_II,
				this, helperId);
	}

	// call again
	else {

		std::ostringstream msg;
		msg << "Phase I -> Waiting to start Phase II for helperId=" << helperId
				<< " and pairId=" << helper->GetPairId();
		MecLogger::Logging(msg.str());
		Simulator::Schedule(MicroSeconds(10),
				&MecSdnController::SyncPhase_II_WaitingToStart, this, helperId);
	}

}

void MecSdnController::SyncPhase_II(uint64_t helperId) {

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];
	Ptr<Node> newHostServer = helper->GetPreferibleHostServer();
	Ptr<MecPairCommunication> pair = helper->GetPair();
	Ptr<MecVm> vm = pair->GetVm();

	// stats
	m_migrationStas.IncrementVmAnalyse();

	if (vm->GetHostServerId() != newHostServer->GetId()) {

		// logging
		std::ostringstream msg;
		msg << "Phase II-> Requesting VM migration for SDN helperId="
				<< helperId << "; pairId=" << pair->GetId();
		MecLogger::Logging(msg.str());

		//stats
		m_migrationStas.IncrementVmMigration();

		// migrate
		MigrateEntity(pair, newHostServer, true);
	}

	else {

		//logging
		std::ostringstream msg;
		msg << "Phase II-> No VM Migration needed for helperId=" << helperId
				<< " : vmId= " << vm->GetId() << " running on serverId="
				<< vm->GetHostServerId();

		MecLogger::Logging(msg.str());

	}

	std::ostringstream msg;
	msg << "Phase II-> completed for helperId=" << helperId << " and pairId="
			<< helper->GetPairId();

	// real time measurement
	double duration = helper->ElapseTimer();
	MecLogger::LoggingRealTime(2, duration);

	MecLogger::Logging(msg.str());
	ReleaseHelper(helperId);
}

void MecSdnController::MigrateEntity(Ptr<MecPairCommunication> pair,
		Ptr<Node> newHostServer, bool migrateVm) {

	NS_LOG_FUNCTION(this << migrateVm);

	Ptr<MecVm> entity;

	if (migrateVm) {
		entity = pair->GetVm();

		std::ostringstream msg;
		msg << "SDN -> Migrating vmId= " << entity->GetId()
				<< "  from serverId=" << entity->GetHostServerId()
				<< " to serverId=" << newHostServer->GetId() << "; pairId="
				<< pair->GetId();
		MecLogger::Logging(msg.str());
	}

	else {
		entity = pair->GetUe();

		std::ostringstream msg;
		msg << "MOBILITY -> UE mobility ueId= " << entity->GetId()
				<< "  from bsId=" << entity->GetHostServerId() << " to bsId="
				<< newHostServer->GetId() << "; pairId=" << pair->GetId();
		MecLogger::Logging(msg.str());
	}

	// get entity's hypervisor
	Ptr<MecVmHypervisor> oldHypervisor = MecUtilsHelper::GetHyperVisor(
			entity->GetHostServerId());
	Ptr<MecVmHypervisor> newHypervisor = MecUtilsHelper::GetHyperVisor(
			newHostServer->GetId());

	// shut it down
	entity->Shutdown();

	if (migrateVm) {

		// sanity check
		NS_ASSERT_MSG(oldHypervisor->ContainsVm(entity),
				"MecSdnController::MigrateEntity, VM id=" << entity->GetId() << " not presented in the old_hypervisorId=" << oldHypervisor->GetId());

		// remove from the old hypervisor
		oldHypervisor->RemoveVm(entity);

	}

	else {

		// sanity check
		NS_ASSERT_MSG(oldHypervisor->ContainsUe(entity),
				"MecSdnController::MigrateEntity, UE id=" << entity->GetId() << " not presented in the old_bsId=" << oldHypervisor->GetId());

		// remove from the old hypervisor
		oldHypervisor->RemoveUe(entity);
	}

	// update flows
	UpdateFlowsBeforeVmMigration(entity, newHostServer,
			pair->GetUploadPolicyFlowMap());
	UpdateFlowsBeforeVmMigration(entity, newHostServer,
			pair->GetDownloadPolicyFlowMap());

	//migrate the entity
	entity->MigrateToServer(newHostServer->GetId());

	if (migrateVm) {

		// sanity check
		NS_ASSERT_MSG(newHypervisor->IsVmAcceptable(entity),
				"MecSdnController::MigrateVm, VM id=" << entity->GetId() << " is not acceptable in the newHypervisor=" << newHypervisor->GetId());

		// add to the new hypervisor
		newHypervisor->AddVm(entity);

	}

	else {
		// add to the new hypervisor
		newHypervisor->AddUe(entity);

	}

	// turn it on
	entity->TurnOn(true);

	std::ostringstream msg;
	msg << ""
			<< (entity->IsIsUe() ?
					"MOBILITY -> ueId=" : "SDN -> Migration done vmId=")
			<< entity->GetId() << "  from serverId="
			<< entity->GetHostServerId() << " to serverId="
			<< newHostServer->GetId() << "; pairId=" << pair->GetId();
	MecLogger::Logging(msg.str());
}

void MecSdnController::UpdateFlowsBeforeVmMigration(Ptr<MecVm> entity,
		Ptr<Node> newHostServer,
		std::map<Ptr<MecFlow>, Ptr<MecPolicy>>& policyFlowMap) {

	NS_ASSERT_MSG(entity->IsOff(),
			"ERROR: Trying to migrate a running VM, entityID=" << entity->GetId());

	//update all related flows and policies
	Ipv4Address ipv4addr = MecUtilsHelper::GetIpv4Address(newHostServer, 1, 0);
	uint32_t newServerHostAddress = ipv4addr.Get();

	std::map<Ptr<MecFlow>, Ptr<MecPolicy>>::const_iterator it;

	for (it = policyFlowMap.begin(); it != policyFlowMap.end(); it++) {

		Ptr<MecFlow> flow = it->first;
		Ptr<MecPolicy> policy = it->second;

		//update flow and policy address
		//update the src of a policy and flow
		if (flow->GetSrcEntityId() == entity->GetId()) {
			policy->SetSrcAddr(newServerHostAddress);
			flow->SetSrcNodeId(newHostServer->GetId());
		}
		//update the dst of a policy and flow
		else if (flow->GetDstEntityId() == entity->GetId()) {
			policy->SetDstAddr(newServerHostAddress);
			flow->SetDstNodeId(newHostServer->GetId());
		}
	}
}

uint32_t MecSdnController::SyncPhase_I_GetCandidateEndDagNodes(Ptr<MecVm> ue,
		Ptr<MecVm> vm, NodeContainer *validVmDagNodes,
		NodeContainer *validUeDagNodes) {
	// one node is for the UE itself
	// and another one is for the VM itself
	uint32_t nodes = 2;

	// get the unique node that "host" the UE
	validUeDagNodes->Add(MecUtilsHelper::GetNode(ue->GetHostServerId()));

	//update the number of nodes
	nodes += validUeDagNodes->GetN();

	// get the nodes that may host the VM instead the current one
	*validVmDagNodes = SyncPhase_I_GetCandidateServerHosts(vm);

	// sanity check
	if (validVmDagNodes->GetN() <= 0) {
		std::ostringstream msg;
		msg
				<< "MecSdnController::Phase_I_CalculateNumberOfGraphNodes --> no available host for vm="
				<< vm->GetId();
		MecLogger::LoggingError(msg.str());
	}

	// update the number of nodes
	nodes += validVmDagNodes->GetN();

	return nodes;
}

uint32_t MecSdnController::SyncPhase_I_GetCandidateMiddleDagNodes(
		Ptr<MecFlow> flow, Ptr<MecPolicy> policy,
		std::vector<MecMiddleBoxContainer> &validMiddleBoxesDagNodes) {

	uint32_t nodes = 0;

	//get the "nodes" that may be the new MBs for the current ones
	for (uint32_t i = 0; i < policy->GetLen(); i++) {

		// get the current middlebox type
		Ptr<MecMiddleBox> middlebox = policy->GetMiddleBoxByIndex(i);

		// get the available middleboxes
		MecMiddleBoxContainer candidateMiddleBoxes =
				SyncPhase_I_GetCandidatesMiddleBoxes(middlebox, flow);
		// sanity check
		if (candidateMiddleBoxes.GetSize() <= 0) {
			std::ostringstream msg;
			msg
					<< "MecSdnController::Phase_I_CalculateNumberOfGraphNodes --> no available middlebox for mb="
					<< middlebox->GetId();
			MecLogger::LoggingError(msg.str());
		}

		// update the number of nodes
		nodes += candidateMiddleBoxes.GetSize();

		//add to the vector
		validMiddleBoxesDagNodes.push_back(candidateMiddleBoxes);
	}

	return nodes;
}

MecMiddleBoxContainer MecSdnController::SyncPhase_I_GetCandidatesMiddleBoxes(
		Ptr<MecMiddleBox> currentMiddlebox, Ptr<MecFlow> flow) {

	MecMiddleBoxContainer candidates = m_middleBoxContainer->GetMiddleBoxByType(
			currentMiddlebox->GetMiddleBoxType());

	MecMiddleBoxContainer available;

	for (uint32_t i = 0; i < candidates.GetSize(); i++) {
		Ptr<MecMiddleBox> candidate = candidates.GetMiddleboxByIndex(i);

		// check if the middlebox can accept the current flow
		bool acceptable = candidate->IsFlowAcceptable(flow);

		//check if there is connectivity between the current  middlebox and candidate one
		bool reachable = m_backhaulCoreHelper->CheckConnectivity(
				candidate->GetAttachedNodeId(),
				currentMiddlebox->GetAttachedNodeId());

		// check if the current middlebox is the candidate one
		bool contains = (candidate->GetId() == currentMiddlebox->GetId());

		//the current MB is included or the candidate can accept
		if (acceptable || reachable || contains) {
			available.Add(candidate);
		}

		NS_ASSERT_MSG(acceptable || reachable || contains,
				"ERROR: No middlebox to substitute, acceptable=" <<acceptable << "; reachable="<<reachable << "; contains="<<contains);

	}

	return available;
}

//get available servers than can accept vm for migration, including the one it currently hosted on
NodeContainer MecSdnController::SyncPhase_I_GetCandidateServerHosts(
		Ptr<MecVm> vm) {

	NodeContainer allHosts = m_backhaulCoreHelper->HostNodes();
	NodeContainer availableServerHosts;

	for (uint32_t i = 0; i < allHosts.GetN(); i++) {
		Ptr<Node> node = allHosts.Get(i);

		//the current server is included (with migration cost=0)
		if (node->GetId() == vm->GetHostServerId()) {
			availableServerHosts.Add(node);
			continue;
		}

		//for others, check the connectivity
		// MAYBE THIS NEED TO BE REMOVED FOR MEC PROBLEM
//		if (!(m_backhaulCoreHelper->CheckConnectivity(vm->GetHostServerId(),
//				node->GetId()))) {
//			continue;
//		}

		//get hypervisor
		Ptr<MecVmHypervisor> hypervisors = MecUtilsHelper::GetHyperVisor(
				node->GetId());

		// check if it can
		if (hypervisors->IsVmAcceptable(vm)) {
			availableServerHosts.Add(node);
		}
	}

	return availableServerHosts;
}

void MecSdnController::PrimalStrategy() {

	if (m_busyHelper.empty()) {

		MecLogger::Logging("SDN -> Free, starting PRIMAL procedure... ");

		MecPairCommunicationContainer pairs = GetNextPairs(m_bsInAnalyse);

		m_bsInAnalyse++;
		if (m_bsInAnalyse >= m_backhaulCoreHelper->HostNodes().GetN()) {
			m_bsInAnalyse = 0;
		}

		for (uint32_t i = 0; i < pairs.GetSize(); i++) {

			Ptr<MecPairCommunication> pair = pairs.GetPairByIndex(i);

			uint64_t helperId = CreateMecSdnHelper(pair);

			// lock helper Id until finalisation
			AssignHelper(helperId);

			Simulator::Schedule(MicroSeconds(i), &MecSdnController::Primal,
					this, helperId);

		}
	}

	std::string busyList = MecUtilsHelper::VectorToString(m_busyHelper.begin(),
			m_busyHelper.end());
	std::ostringstream msg;
	msg << "SDN -> busy checking helperIds=[" << busyList
			<< "],  trying at time..."
			<< Simulator::Now()
					+ MecConstantsHelper::SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME;
	MecLogger::Logging(msg.str());

	ScheduleNextPolicyCheck(
			MecConstantsHelper::SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME);
}

void MecSdnController::Primal(uint64_t helperId) {

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];
	Ptr<MecPairCommunication> pair = helper->GetPair();

	// real time measurement
	helper->ResetTimer();

	std::ostringstream msg;
	msg << "Primal -> started for helperId=" << helperId << " and pairId="
			<< pair->GetId();
	MecLogger::Logging(msg.str());

	Ptr<MecVm> ue = pair->GetUe();
	Ptr<MecVm> vm = pair->GetVm();

	// container that represents the valid nodes that may host VM
	NodeContainer validVmDagNodes;

	// container that represents the current node that "host" UE
	NodeContainer validUeDagNodes;

	uint32_t numberOfEndNodes = PrimalGetCandidateEndDagNodes(ue, vm,
			&validVmDagNodes, &validUeDagNodes);

	// helper
	helper->AddtHostServersCandidatesToVm(validVmDagNodes);

	// analyse download policy flows VM -> UE
	PrimalInitialisation(vm, &validVmDagNodes, ue, &validUeDagNodes,
			numberOfEndNodes, pair->GetDownloadPolicyFlowMap(), true, helperId);

	// analyse upload policy flows UE -> VM
	PrimalInitialisation(vm, &validVmDagNodes, ue, &validUeDagNodes,
			numberOfEndNodes, pair->GetUploadPolicyFlowMap(), false, helperId);

	uint32_t choosen = MecUtilsHelper::GetRandomIntegerNumber(0,
			validVmDagNodes.GetN() - 1);
	Ptr<Node> node = validVmDagNodes.Get(choosen);

	Simulator::Schedule(MicroSeconds(1), &MecSdnController::PrimalMigrateVMs,
			this, helperId, node);

}

void MecSdnController::PrimalInitialisation(Ptr<MecVm> vm,
		NodeContainer *validVmDagNodes, Ptr<MecVm> ue,
		NodeContainer *validUeDagNodes, uint32_t numberOfEndNodes,
		std::map<Ptr<MecFlow>, Ptr<MecPolicy>>& policyFlowMap,
		bool isDownloadFlow, uint64_t helperId) {

	std::map<Ptr<MecFlow>, Ptr<MecPolicy>>::const_iterator it;

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];

	for (it = policyFlowMap.begin(); it != policyFlowMap.end(); it++) {

		Ptr<MecFlow> flow = it->first;
		Ptr<MecPolicy> policy = it->second;

		// containers that represent tge valid nodes for MBs
		std::vector<MecMiddleBoxContainer> validMiddleBoxDagNodes;

		// calculate number of DAG nodes
		uint32_t numberOfMiddleNodes = PrimalGetMiddleDagNodes(flow, policy,
				validMiddleBoxDagNodes);

		// create a policy-flow migration helper
		Ptr<MecPolicyMigrationHelper> policyMigrationHelper = CreateObject<
				MecPolicyMigrationHelper>();

		policyMigrationHelper->SetIsDownloadFlow(isDownloadFlow);
		policyMigrationHelper->SetIsPolicyMigrationDone(false);

		//store the policy and flow
		policyMigrationHelper->SetPolicy(policy);
		policyMigrationHelper->SetFlow(flow);

		helper->AddMecPolicyMigrationHelper(policyMigrationHelper);

		// building the DAG = Flow-Policy Network
		if (isDownloadFlow) {

			policyMigrationHelper->DagConstruction(
					(numberOfEndNodes + numberOfMiddleNodes), vm,
					validVmDagNodes, validMiddleBoxDagNodes, validUeDagNodes,
					ue);

		} else {
			policyMigrationHelper->DagConstruction(
					(numberOfEndNodes + numberOfMiddleNodes), ue,
					validUeDagNodes, validMiddleBoxDagNodes, validVmDagNodes,
					vm);
		}

	}

}

void MecSdnController::PrimalMigrateVMs(uint64_t helperId, Ptr<Node> node) {

	Ptr<MecSdnControllerHelper> helper = m_helper[helperId];
	Ptr<Node> newHostServer = helper->GetPreferibleHostServer();
	Ptr<MecPairCommunication> pair = helper->GetPair();
	Ptr<MecVm> vm = pair->GetVm();

	// stats
	m_migrationStas.IncrementVmAnalyse();

//	//bla
//	newHostServer = node;
//	uint32_t coin = MecUtilsHelper::GetRandomIntegerNumber(0, 100);
//
//	if (coin > 75) {

	if (vm->GetHostServerId() != newHostServer->GetId()) {

		std::ostringstream msg;
		msg << "Primal -> Requesting VM migration for SDN helperId=" << helperId
				<< "; pairId=" << pair->GetId();
		MecLogger::Logging(msg.str());

		//stats
		m_migrationStas.IncrementVmMigration();

		MigrateEntity(pair, newHostServer, true);
	}

	else {

		std::ostringstream msg;
		msg << "Primal -> No VM Migration needed for helperId=" << helperId
				<< " : vmId= " << vm->GetId() << " running on serverId="
				<< vm->GetHostServerId();

		MecLogger::Logging(msg.str());

	}
//	}

	std::ostringstream msg;
	msg << "Primal -> completed for helperId=" << helperId << " and pairId="
			<< helper->GetPairId();
	MecLogger::Logging(msg.str());

	// real time measurement
	double duration = helper->ElapseTimer();
	MecLogger::LoggingRealTime(1, duration);
	ReleaseHelper(helperId);
}

uint32_t MecSdnController::PrimalGetCandidateEndDagNodes(Ptr<MecVm> ue,
		Ptr<MecVm> vm, NodeContainer *validVmDagNodes,
		NodeContainer *validUeDagNodes) {

	// use the same function employed in SYNC
	return SyncPhase_I_GetCandidateEndDagNodes(ue, vm, validVmDagNodes,
			validUeDagNodes);

}

// re-using and adapting the function SyncPhase_I_GetCandidateMiddleDagNodes
// as Primal does not migrate MBs, this function basically create an array of MBcontainers
// where each container contains only one Middlebox defined in the policy
uint32_t MecSdnController::PrimalGetMiddleDagNodes(Ptr<MecFlow> flow,
		Ptr<MecPolicy> policy,
		std::vector<MecMiddleBoxContainer> &validMiddleBoxesDagNodes) {

	uint32_t nodes = 0;

	//get the "nodes" that may be the new MBs for the current ones
	for (uint32_t i = 0; i < policy->GetLen(); i++) {

		// get the current middlebox type
		Ptr<MecMiddleBox> middlebox = policy->GetMiddleBoxByIndex(i);

		// get the available middleboxes
		MecMiddleBoxContainer container;
		container.Add(middlebox);

		// update the number of nodes
		nodes += container.GetSize();

		//add to the vector
		validMiddleBoxesDagNodes.push_back(container);
	}

	return nodes;
}

void MecSdnController::StaticStrategy() {
	// do nothing

	MecLogger::Logging("SDN -> Free, starting STATIC strategy... ");
}

MecPairCommunicationContainer MecSdnController::GetNextUEsToMove() {
	NS_LOG_FUNCTION(this);
	MecPairCommunicationContainer toMove;

	std::vector<uint32_t> pairIds;
	uint32_t total = m_pairCommunicationContainer->GetSize();

	uint32_t percentagem = MecUtilsHelper::GetRandomIntegerNumber(
			MecConstantsHelper::UE_MOBILITY_MIN_PERCENTAGEM,
			MecConstantsHelper::UE_MOBILITY_MAX_PERCENTAGEM);

	uint32_t move = (uint32_t) ceil(total * (percentagem / 100.0));

	MecUtilsHelper::GenerateRandomVectorOfUniqueNumbers(pairIds, (uint32_t) 0,
			total - 1, std::min((uint32_t) total - 1, (uint32_t) move));

	std::ostringstream msg;
	msg << "MOBILITY -> PairIds to MOVE: ["
			<< MecUtilsHelper::VectorToString(pairIds.begin(), pairIds.end())
			<< "] moves=" << move << " that represents " << percentagem
			<< "% of " << total << " UEs";

	MecLogger::Logging(msg.str());

	for (uint32_t i = 0; i < pairIds.size(); i++) {
		uint32_t pairId = pairIds[i];

		Ptr<MecPairCommunication> pair =
				m_pairCommunicationContainer->GetPairByIndex(pairId);
		toMove.Add(pair);
	}

	return toMove;

}

void MecSdnController::RandomlyUeMobility() {
	NS_LOG_FUNCTION(this);

	MecLogger::Logging("MOBILITY -> Some UEs will move... ");

	NodeContainer bs = m_backhaulCoreHelper->HostNodes();

	MecPairCommunicationContainer toMove = GetNextUEsToMove();

	for (MecPairCommunicationContainer::const_iterator itPair = toMove.CBegin();
			itPair != toMove.CEnd(); itPair++) {

		Ptr<MecPairCommunication> pair = (*itPair);

		uint32_t newBsId = MecUtilsHelper::GetRandomIntegerNumber(0,
				bs.GetN() - 1);

		Ptr<Node> newBs = bs.Get(newBsId);

		Ptr<MecVm> ue = pair->GetUe();

		if (ue->GetHostServerId() != newBs->GetId()) {

			MigrateEntity(pair, newBs, false);
		}
	}

	switch (m_ueMobilityStrategy) {
	case 1:
		// do nothing
		break;
	case 2:
		// do nothing
		break;
	case 3:
		ScheduleNextMobility(MecConstantsHelper::UE_NEXT_MOBILITY_TIME);
		break;
	default:
		break;
	}

}

void MecSdnController::SetBackhaulCoreHelper(
		Ptr<MecBackhaulCoreHelper> backhaulCoreHelper) {
	m_backhaulCoreHelper = backhaulCoreHelper;
}

void MecSdnController::SetMiddleBoxContainer(
		MecMiddleBoxContainer* middleBoxContainer) {
	m_middleBoxContainer = middleBoxContainer;
}

void MecSdnController::SetUeContainer(MecVmContainer* ueContainer) {
	m_ueContainer = ueContainer;
}

void MecSdnController::SetUeToVmFlowContainer(
		MecFlowContainer* ueToVmFlowContainer) {
	m_ueToVmFlowContainer = ueToVmFlowContainer;
}

void MecSdnController::SetUeToVmPolicyContainer(
		MecPolicyContainer* ueToVmPolicyContainer) {
	m_ueToVmPolicyContainer = ueToVmPolicyContainer;
}

void MecSdnController::SetVmContainer(MecVmContainer* vmContainer) {
	m_vmContainer = vmContainer;
}

void MecSdnController::SetVmToUeFlowContainer(
		MecFlowContainer* vmToUeFlowContainer) {
	m_vmToUeFlowContainer = vmToUeFlowContainer;
}

void MecSdnController::SetVmToUePolicyContainer(
		MecPolicyContainer* vmToUePolicyContainer) {
	m_vmToUePolicyContainer = vmToUePolicyContainer;
}

void MecSdnController::SetHypervisorContainer(
		ApplicationContainer* hypervisorContainer) {
	m_hypervisorContainer = hypervisorContainer;
}

void MecSdnController::SetScheduleStrategy(uint32_t strategy) {
	m_scheduleStrategy = strategy;
}
void MecSdnController::SetUeMobilityStrategy(uint32_t strategy) {
	m_ueMobilityStrategy = strategy;
}

void MecSdnController::SetCommunicationContainer(
		MecPairCommunicationContainer *communicationContainer) {
	m_pairCommunicationContainer = communicationContainer;
}

} /* namespace ns3 */
