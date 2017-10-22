/*
 * scenario-generator.cc
 *
 *  Created on: 8 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-scenario-generator-helper.h"

#include <sstream>      // std::ostringstream

#include "ns3/assert.h"
#include "ns3/packet-sink-helper.h"

#include "ns3/mec-vm-hypervisor-helper.h"
#include "ns3/mec-constants-helper.h"
#include "ns3/mec-utils-helper.h"

namespace ns3 {

MecScenarioGeneratorHelper::MecScenarioGeneratorHelper() {

}

MecScenarioGeneratorHelper::~MecScenarioGeneratorHelper() {
}

void MecScenarioGeneratorHelper::GeneratingHypervisors(NodeContainer *allHosts,
		ApplicationContainer *applicationContainer) {

	MecVmHypervisorHelper hypervisorHelper;
	ApplicationContainer hypervisors = hypervisorHelper.Install(*allHosts);
	*applicationContainer = hypervisors;

	for (uint32_t i = 0; i < allHosts->GetN(); ++i) {
		Ptr<Node> hostNode = allHosts->Get(i);
		Ptr<Application> hostHypervisor = hostNode->GetApplication(0);

		//sanity check
		NS_ASSERT(hostHypervisor != 0);
		Ptr<MecVmHypervisor> hvapp =
				hostHypervisor->GetObject<MecVmHypervisor>();
		NS_ASSERT(hvapp != 0);

		hvapp->SetTotalCpu(MecConstantsHelper::TOTAL_HOST_SERVER_CPU);
		hvapp->SetTotalMemory(MecConstantsHelper::TOTAL_HOST_SERVER_MEMORY);

	}

}

void MecScenarioGeneratorHelper::GenerateFlows(NodeContainer *allHosts,
		MecVmContainer *vmContainer, MecVmContainer *ueContainer,
		MecFlowContainer *vmToUeFlowContainer,
		MecFlowContainer *ueToVmFlowContainer) {

	// install TCP and UDPs sockets on hosts
	InstallSocketsOnServerHosts(allHosts);

	// sanity check
	NS_ASSERT_MSG(vmContainer->GetSize() == ueContainer->GetSize(),
			"MecScenarioGeneratorHelper::GenerateFlows --> Number of VMs=" << vmContainer->GetSize()<<" must be equal to the number of UEs="<< ueContainer->GetSize());
	uint32_t numberOfPairs = vmContainer->GetSize();

	std::vector<uint32_t> numberOfFlowsVmToUe;
	std::vector<uint32_t> numberOfFlowsUeToVm;
	uint32_t totalFlowsVmToUe = 0;
	uint32_t totalFlowsUeToVm = 0;

	// generating random number of flows among each pair UE <-> VM
	for (uint32_t i = 0; i < numberOfPairs; i++) {

		// VM -> UE
		uint32_t numFlow = MecUtilsHelper::GetRandomIntegerNumber(
				MecConstantsHelper::MIN_FLOW_PER_VM,
				MecConstantsHelper::MAX_FLOW_PER_VM);

		numberOfFlowsVmToUe.push_back(numFlow);
		totalFlowsVmToUe += numFlow;

		//  UE -> VM
		numFlow = MecUtilsHelper::GetRandomIntegerNumber(
				MecConstantsHelper::MIN_FLOW_PER_VM,
				MecConstantsHelper::MAX_FLOW_PER_VM);
		numberOfFlowsUeToVm.push_back(numFlow);
		totalFlowsUeToVm += numFlow;
	}

	// creating Flows VM -> UE
	vmToUeFlowContainer->CreateFlowContainer(totalFlowsVmToUe);

	// creating Flows UE -> VM
	ueToVmFlowContainer->CreateFlowContainer(totalFlowsUeToVm);

	//  Configure all flows from VM to UE
	AssignFlows(vmToUeFlowContainer, vmContainer, ueContainer,
			numberOfFlowsVmToUe, numberOfPairs, true);

	//  Configure all flows from UE to VM
	AssignFlows(ueToVmFlowContainer, ueContainer, vmContainer,
			numberOfFlowsUeToVm, numberOfPairs, false);

	std::cout << "\nComplete generating flows...total VM->UE Flows="
			<< MecUtilsHelper::ToString(totalFlowsVmToUe)
			<< "\t total UE->VM flows="
			<< MecUtilsHelper::ToString(totalFlowsUeToVm) << std::endl;
}

void MecScenarioGeneratorHelper::AssignFlows(MecFlowContainer *flowContainer,
		MecVmContainer* from, MecVmContainer* to,
		std::vector<uint32_t> &numberOfFlows, uint32_t numberOfPairs,
		bool isDownloadFlow) {

	uint32_t flowIndex = 0;

	for (uint32_t index = 0; index < numberOfPairs; index++) {

		Ptr<MecVm> src = from->GetVmByIndex(index);
		Ptr<MecVm> dst = to->GetVmByIndex(index);

		uint32_t numberOfFlowsAmongSrcDst = numberOfFlows[index];

		for (uint32_t j = 0; j < numberOfFlowsAmongSrcDst; j++) {

			// sanity check
			NS_ASSERT_MSG(flowIndex < flowContainer->GetSize(),
					"MecScenarioGeneratorHelper::AssignFlows --> Accessing non-exist flow flowIndex="<<flowIndex << " flowContainer->GetSize()="<<flowContainer->GetSize());

			Ptr<MecFlow> flow = flowContainer->GetFlowByIndex(flowIndex);

			// Setup flow's src
			flow->SetSrcEntity(src);
			flow->SetSrcNodeId(src->GetHostServerId());
			src->AddFlow(flow);

			// Set flow's dst
			flow->SetDstEntity(dst);
			flow->SetDstNodeId(dst->GetHostServerId());
			dst->AddFlow(flow);

			// Setup flow's data rate
			SetupFlow(flow, MecConstantsHelper::USE_TCP, isDownloadFlow);

			flowIndex++;
		}
	}
}

void MecScenarioGeneratorHelper::SetupFlow(Ptr<MecFlow> flow, bool useTCP,
		bool isDownloadFlow) {

	uint32_t rateRandomValue;

	if (isDownloadFlow) {
		rateRandomValue = MecUtilsHelper::GetRandomIntegerNumber(
				MecConstantsHelper::MIN_DOWNLOAD_FLOW_DATA_RATE,
				MecConstantsHelper::MAX_DOWNLOAD_FLOW_DATA_RATE);
	} else {
		rateRandomValue = MecUtilsHelper::GetRandomIntegerNumber(
				MecConstantsHelper::MIN_UPLOAD_FLOW_DATA_RATE,
				MecConstantsHelper::MAX_UPLOAD_FLOW_DATA_RATE);
	}

	std::ostringstream stringStream;
	stringStream << rateRandomValue << MecConstantsHelper::FLOW_DATE_RATE_UNITS;

	flow->SetDataRate(stringStream.str());
	flow->SetUseTCP(useTCP);

	if (useTCP) {
		flow->SetPacketSize(MecConstantsHelper::TCP_PACKET_SIZE);
		flow->SetSrcPort(MecConstantsHelper::TCP_PORT_SEED++);
		flow->SetDstPort(MecConstantsHelper::MEC_TCP_DST_PORT);
	} else {
		flow->SetPacketSize(MecConstantsHelper::UDP_PACKET_SIZE);
		flow->SetSrcPort(MecConstantsHelper::UDP_PORT_SEED++);
		flow->SetDstPort(MecConstantsHelper::MEC_UDP_DST_PORT);
	}

}

void MecScenarioGeneratorHelper::InstallSocketsOnServerHosts(
		NodeContainer *allHosts) {

	//	PacketSinkHelper sinkTcp("ns3::TcpSocketFactory",
	//			InetSocketAddress(Ipv4Address::GetAny(),
	//					MecConstantsHelper::MEC_TCP_DST_PORT));
	//	ApplicationContainer sinkTcpApps;
	//	sinkTcpApps = sinkTcp.Install(*allHosts);

	PacketSinkHelper sinkUdp("ns3::UdpSocketFactory",
			InetSocketAddress(Ipv4Address::GetAny(),
					MecConstantsHelper::MEC_UDP_DST_PORT));
	ApplicationContainer sinkUdpApps;
	sinkUdpApps = sinkUdp.Install(*allHosts);
}

void MecScenarioGeneratorHelper::GenerateMiddleboxes(NodeContainer *aggrNodes,
		NodeContainer *edgeNodes, MecMiddleBoxContainer *middleboxContainer) {

	NodeContainer switchNodes;
	switchNodes.Add(*aggrNodes);
	switchNodes.Add(*edgeNodes);

	std::vector<uint16_t> types;

	//generating middlebox types
	for (uint32_t i = 0; i < switchNodes.GetN(); ++i) {
		types.push_back(
				MecUtilsHelper::GetRandomIntegerNumber(1,
						MecConstantsHelper::AMOUNT_OF_MIDDLEBOX_TYPE));
	}

	// Each Aggr/Edge has one MB
	middleboxContainer->CreateMiddleBoxContainer(switchNodes.GetN(),
			MecConstantsHelper::MIDDLEBOX_CAPACITY, types);

	for (uint32_t i = 0; i < switchNodes.GetN(); ++i) {
		Ptr<Node> node = switchNodes.Get(i);
		Ptr<MecMiddleBox> mb = middleboxContainer->GetMiddleboxByIndex(i);

		mb->AttachMiddleBox(node);
	}

}

void MecScenarioGeneratorHelper::GenerateVMsAndUEs(NodeContainer *allHosts,
		MecVmContainer *vmContainer, MecVmContainer *ueContainer) {

	std::vector<uint32_t> numHostedVmAtBaseStation;
	std::vector<uint32_t> numHostedUEAtBaseStation;

	std::vector<uint32_t> suffledBaseStationIdsVM;
	// this should be a copy of above array
	std::vector<uint32_t> suffledBaseStationIdsUE;

	uint32_t totalPairsUE_VM = 0;

	// generate total VMs
	for (uint32_t i = 0; i < allHosts->GetN(); ++i) {
		uint32_t numbersOfVmsOnHostServer =
				MecUtilsHelper::GetRandomIntegerNumber(
						MecConstantsHelper::MIN_NUMBER_VMS_ON_HOST_SERVER,
						MecConstantsHelper::MAX_NUMBER_VMS_ON_HOST_SERVER);

		numHostedVmAtBaseStation.push_back(numbersOfVmsOnHostServer);
		numHostedUEAtBaseStation.push_back(numbersOfVmsOnHostServer);

		totalPairsUE_VM += numbersOfVmsOnHostServer;

		for (uint32_t j = 0; j < numbersOfVmsOnHostServer; j++) {
			suffledBaseStationIdsVM.push_back(i);
			suffledBaseStationIdsUE.push_back(i);
		}

		std::cout << "Base Station: " << allHosts->Get(i)->GetId()
				<< "   vm/ues: " << numbersOfVmsOnHostServer << std::endl;
	}

	//suffle
	MecUtilsHelper::Suffle(suffledBaseStationIdsVM.begin(),
			suffledBaseStationIdsVM.end());
	MecUtilsHelper::Suffle(suffledBaseStationIdsUE.begin(),
			suffledBaseStationIdsUE.end());

	// create a containers of VMs
	vmContainer->CreateVmContainer(totalPairsUE_VM, false);

	// create a containers of UEs (simulated as VM)
	ueContainer->CreateVmContainer(totalPairsUE_VM, true);

	NS_ASSERT_MSG(vmContainer->GetSize() == ueContainer->GetSize(),
			"MecScenarioGeneratorHelper::GenerateVMsAndUEs -> Number of VMs="<< vmContainer->GetSize() <<" must be equal to the number of UEs="<< ueContainer->GetSize());

	//bind IDs
	//vmContainer.GetVm(1) communicates only with ueContainer.GetVm(1);
	for (uint32_t i = 0; i < vmContainer->GetSize(); i++) {
		Ptr<MecVm> vm = vmContainer->GetVmByIndex(i);
		Ptr<MecVm> ue = ueContainer->GetVmByIndex(i);

		vm->SetBindId(ue->GetId());
		ue->SetBindId(vm->GetId());
	}

	//Deploy VMs on Server Hosts acting as Base Stations
	DeployOnBaseStation(vmContainer, allHosts, numHostedVmAtBaseStation,
			suffledBaseStationIdsVM);

	//Deploy UEs on Server Hosts acting as Base Stations
	DeployOnBaseStation(ueContainer, allHosts, numHostedUEAtBaseStation,
			suffledBaseStationIdsUE);

	std::cout << "Complete generating VMs and UEs...total VMs="
			<< vmContainer->GetSize() << "\ttotal UEs="
			<< ueContainer->GetSize() << std::endl;
}

void MecScenarioGeneratorHelper::DeployOnBaseStation(MecVmContainer *container,
		NodeContainer *allHosts, std::vector<uint32_t> &numHostedVmAtServer,
		std::vector<uint32_t> &suffledServerHostIds) {

	for (uint32_t i = 0; i < container->GetSize(); ++i) {

		// Get a VM
		Ptr<MecVm> entity = container->GetVmByIndex(i);

		// Get a Random Server to host the VM vm
		Ptr<Node> hostServerNode = GetRandomHostServerForVm(allHosts,
				numHostedVmAtServer, suffledServerHostIds);

		// Get the hypervisor of the host server
		Ptr<MecVmHypervisor> hostHypervisor = MecUtilsHelper::GetHyperVisor(
				hostServerNode->GetId());

		// bind vm/ue <=> host server
		entity->SetHostServerId(hostServerNode->GetId());

		// setup vm
		if (entity->IsIsVm()) {

			// define the confs const values
			SetupVm(entity);

			NS_ASSERT_MSG(hostHypervisor->IsVmAcceptable(entity) == true,
					"MecScenarioGeneratorHelper::DeployOnBaseStation --> hypervisorId="<<hostHypervisor->GetId() <<" can't accept vmId="<<entity->GetId() << "; vm->GetCpu()="<<entity->GetCpu() << "; vm->GetMemory()="<<entity->GetMemory() << "; hostHypervisor->GetUsedCpu()="<< hostHypervisor->GetUsedCpu()<< "; hostHypervisor->GetUsedMemory()="<< hostHypervisor->GetUsedMemory() << "; hostHypervisor->GetTotalCpu()="<<hostHypervisor->GetTotalCpu() <<"; hostHypervisor->GetTotalMemory()="<<hostHypervisor->GetTotalMemory());

			// Add the VM on the hypervisor
			hostHypervisor->AddVm(entity);
		}

		else {
			hostHypervisor->AddUe(entity);
		}

	}
}

Ptr<Node> MecScenarioGeneratorHelper::GetRandomHostServerForVm(
		NodeContainer *allHosts, std::vector<uint32_t>& numHostedVmAtServer,
		std::vector<uint32_t>& suffledServerHostIds) {

	NS_ASSERT_MSG(!suffledServerHostIds.empty(),
			"MecScenarioGeneratorHelper::GetRandomHostServerForVm --> suffledServerHostIds should not be empty during this call");

	uint32_t nodeIndex = suffledServerHostIds.back();

	NS_ASSERT_MSG(numHostedVmAtServer[nodeIndex] != 0,
			"MecScenarioGeneratorHelper::GetRandomHostServerForV --> Number of deployed VMs exceeded, index="<<nodeIndex<<" vector="<< MecUtilsHelper::VectorToString(numHostedVmAtServer.begin(), numHostedVmAtServer.end()));

	suffledServerHostIds.pop_back();

	numHostedVmAtServer[nodeIndex]--;

	return allHosts->Get(nodeIndex);

}

void MecScenarioGeneratorHelper::SetupVm(Ptr<MecVm> vm) {

	vm->SetCpu(
			MecUtilsHelper::GetRandomIntegerNumber(
					MecConstantsHelper::MIN_TOTAL_VM_CPU,
					MecConstantsHelper::MAX_TOTAL_VM_CPU));
	vm->SetMemory(
			MecUtilsHelper::GetRandomIntegerNumber(
					MecConstantsHelper::MIN_TOTAL_VM_MEMORY,
					MecConstantsHelper::MAX_TOTAL_VM_MEMORY));

	//for migration cost
	uint32_t liveMig_VmSize = MecUtilsHelper::GetRandomIntegerNumber(
			MecConstantsHelper::MIN_LIVE_MIGRATION_VM_SIZE,
			MecConstantsHelper::MAX_LIVE_MIGRATION_VM_SIZE);
	uint32_t liveMig_DirtyRate = MecUtilsHelper::GetRandomIntegerNumber(
			MecConstantsHelper::MIN_LIVE_MIGRATION_DIRTY_RATE_PAGE,
			MecConstantsHelper::MAX_LIVE_MIGRATION_DIRTY_RATE_PAGE);

	vm->SetLiveMigVmSize(liveMig_VmSize);
	vm->SetLiveMigDirtyRate(liveMig_DirtyRate);
	vm->SetLiveMigL(MecConstantsHelper::LIVE_MIGRATION_L);
	vm->SetLiveMigT(MecConstantsHelper::LIVE_MIGRATION_T);
	vm->SetLiveMigX(MecConstantsHelper::LIVE_MIGRATION_X);

	vm->CalculateMigrationTraffic();
}

void MecScenarioGeneratorHelper::GeneratePolicies(
		MecFlowContainer *vmToUeFlowContainer,
		MecFlowContainer *ueToVmFlowContainer,
		MecPolicyContainer *vmToUePolicyContainer,
		MecPolicyContainer *ueToVmPolicyContainer,
		MecMiddleBoxContainer *middleboxContainer,
		Ptr<MecBackhaulCoreHelper> backhaulCoreHelper) {

	//Setup policies VM->UE
	SetupPolicies(vmToUeFlowContainer, vmToUePolicyContainer,
			middleboxContainer, backhaulCoreHelper);

	//Setup policies UE->VM
	SetupPolicies(ueToVmFlowContainer, ueToVmPolicyContainer,
			middleboxContainer, backhaulCoreHelper);

}

void MecScenarioGeneratorHelper::SetupPolicies(MecFlowContainer *flowContainer,
		MecPolicyContainer *mecPolicyContainer,
		MecMiddleBoxContainer *middleboxContainer,
		Ptr<MecBackhaulCoreHelper> backhaulCoreHelper) {

	for (MecFlowContainer::const_iterator it = flowContainer->CBegin();
			it != flowContainer->CEnd(); it++) {

		Ptr<MecFlow> flow = *it;
		Ptr<MecPolicy> policy = mecPolicyContainer->CreatePolicyEntry(flow);

		bool constructed = false;

		// BE CAREFULL
		uint32_t attempt = 0;
		while (!constructed) {
			constructed = ConstructPolicies(flow, policy, middleboxContainer,
					backhaulCoreHelper);
			std::cout << "attempt: " << attempt << std::endl;
			NS_ASSERT_MSG(attempt < 1000,
					"MecScenarioGeneratorHelper::SetupPolicies non-stop loop!!!");

			attempt++;
		}
	}

	//Set % of policy-free
	mecPolicyContainer->SetPolicyStatus(
			MecConstantsHelper::PERCENTAGE_POLICY_FREE, false);

}

bool MecScenarioGeneratorHelper::ConstructPolicies(Ptr<MecFlow> flow,
		Ptr<MecPolicy> policy, MecMiddleBoxContainer *middleboxContainer,
		Ptr<MecBackhaulCoreHelper> backhaulCoreHelper) {

	std::vector<uint16_t> types = middleboxContainer->GetTypes();

	uint32_t len = MecUtilsHelper::GetRandomIntegerNumber(
			std::min((uint32_t) 1,
					(uint32_t) MecConstantsHelper::MIN_POLICY_LEN),
			std::min((uint32_t) MecConstantsHelper::MAX_POLICY_LEN,
					(uint32_t) types.size()));

	// Clear the middlebox list just in case
	policy->Clear();

	//setting the amount of MB that the flow has to across
	policy->SetLen(len);

	//setting the sequence of n = len MBs

	std::vector<uint16_t> seq(types);


	MecUtilsHelper::GenerateRandomVectorOfUniqueNumbers(seq, len);

	//fulfilling sequence of middlebox type
	while (!seq.empty()) {
		uint16_t typeValue = (uint16_t) seq.back();
		policy->AddMiddleBoxType(typeValue);
		seq.pop_back();
	}

	uint32_t previousNode = flow->GetSrcNodeId();
	bool isEgress = false;

	//Generate middlebox list according to policy->m_seq
	for (uint8_t i = 0; i < len; i++) {

		uint16_t type = policy->GetMiddleBoxType(i);
		MecMiddleBoxContainer candidates =
				middleboxContainer->GetMiddleBoxByType(type);

		if (i == len - 1) {
			isEgress = true;
		}

		// select one middlebox from the middlebox containers
		Ptr<MecMiddleBox> selected = GetMiddlebox(flow, policy, &candidates,
				backhaulCoreHelper, previousNode, isEgress);

		if (!selected) {
			return false;
		}

		// Store the current node Id for the connectivity step of the
		// GetMiddlebox function in the next loop
		previousNode = selected->GetId();

		//add middlebox into the policy list
		policy->AddMiddlebox(selected);

		//assign flow to the selected middlebox
		selected->FlowAssign(flow);
	}

	return true;
}

Ptr<MecMiddleBox> MecScenarioGeneratorHelper::GetMiddlebox(Ptr<MecFlow> flow,
		Ptr<MecPolicy> policy, MecMiddleBoxContainer *candidates,
		Ptr<MecBackhaulCoreHelper> backhaulCoreHelper, uint32_t previousNodeId,
		bool isEgress) {

	std::vector<uint32_t> suffleMiddleboxIndexes;
	uint32_t n = candidates->GetSize();

	MecUtilsHelper::GenerateRandomVectorOfUniqueNumbers(suffleMiddleboxIndexes,
			(uint32_t) 0, (uint32_t) candidates->GetSize() - 1, n);

	NS_ASSERT_MSG(suffleMiddleboxIndexes.size() == candidates->GetSize(),
			"ScenarioGenerator::GetMiddlebox some candidate index is missing, suffleMiddleboxIndexes.size()="<<suffleMiddleboxIndexes.size() << "candidates.GetSize()="<<candidates->GetSize());

	for (uint32_t i = 0; i < suffleMiddleboxIndexes.size(); i++) {

		Ptr<MecMiddleBox> middlebox = candidates->GetMiddleboxByIndex(
				suffleMiddleboxIndexes[i]);

		// check if the middlebox can accept the current flow
		bool acceptable = middlebox->IsFlowAcceptable(flow);

		//check if there is connectivity among the previous middlebox or flow.GetSrcNodeId
		bool reachable = backhaulCoreHelper->CheckConnectivity(previousNodeId,
				middlebox->GetAttachedNodeId());

		// check if the current middlebox already has been used by the policy
		bool contains = policy->ContainsMiddlebox(middlebox);

		//if the current middlebox selection is for egress node, then
		// check the connectivity with the flow.GetDstNodeId as well
		if (isEgress && reachable) {
			reachable = backhaulCoreHelper->CheckConnectivity(
					middlebox->GetAttachedNodeId(), flow->GetDstNodeId());

		}

		//everything is fine, then return the middlebox
		if (acceptable && reachable && !contains) {

			return middlebox;
		}
	}

	return NULL;

}

} /* namespace ns3 */
