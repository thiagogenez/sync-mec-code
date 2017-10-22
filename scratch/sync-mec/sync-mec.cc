#include "ns3/core-module.h"
#include "ns3/mec-backhaul-core-helper.h"
#include "ns3/global-router-interface.h"
#include "ns3/rng-seed-manager.h"

#include "ns3/mec-logger.h"
#include "ns3/mec-middle-box-container.h"
#include "ns3/mec-policy-container.h"
#include "ns3/mec-policy-routing.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-constants-helper.h"
#include "ns3/mec-scenario-generator-helper.h"
#include "ns3/mec-sdn-controller.h"
#include "ns3/mec-pair-communication-container.h"
#include "ns3/mec-flow-monitor.h"
#include "ns3/mec-e2e-checker.h"

#include <ctime>
#include <cstdlib>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("sync-mec");

void SetupPolicyRouting(NodeContainer *allNodes,
		Ptr<MecBackhaulCoreHelper> mecBackhaulCoreHelper,
		MecPolicyContainer *policyContainer, bool policyEnable) {

	for (NodeContainer::Iterator i = allNodes->Begin(); i != allNodes->End();
			++i) {

		Ptr<Node> node = *i;
		Ptr<GlobalRouter> globalRouter = node->GetObject<GlobalRouter>();
		Ptr<Ipv4GlobalRouting> globalRoutingP =
				globalRouter->GetRoutingProtocol();
		Ptr<MecPolicyRouting> mecPolicyRouting = globalRoutingP->GetObject<
				MecPolicyRouting>();

		mecPolicyRouting->SetMecBackHaulCoreHelper(mecBackhaulCoreHelper);
		mecPolicyRouting->SetPolicyEnable(policyEnable);
		mecPolicyRouting->SetMecPolicyContainer(policyContainer);
	}
}

void BindFlowPolicy(Ptr<MecPairCommunication> pair,
		MecFlowContainer *flowContainer, MecPolicyContainer *policyContainer) {

	for (MecFlowContainer::const_iterator itFlow = flowContainer->CBegin();
			itFlow != flowContainer->CEnd(); itFlow++) {

		Ptr<MecFlow> flow = (*itFlow);

		// upload flow
		if (flow->GetSrcEntityId() == pair->GetUe()->GetId()
				&& flow->GetDstEntityId() == pair->GetVm()->GetId()) {

			//get policy for this flow
			Ptr<MecPolicy> policy = policyContainer->GetPolicyByFlow(flow);

			pair->AddUploadPolicyFlow(flow, policy);
		}

		// download flow
		else if (flow->GetSrcEntityId() == pair->GetVm()->GetId()
				&& flow->GetDstEntityId() == pair->GetUe()->GetId()) {

			//get policy for this flow
			Ptr<MecPolicy> policy = policyContainer->GetPolicyByFlow(flow);

			pair->AddDownloadPolicyFlow(flow, policy);
		}
	}
}

void OrganasingPairCommunications(
		MecPairCommunicationContainer *pairCommunicationContainer,
		MecVmContainer *vmContainer, MecFlowContainer *vmToUeFlowContainer,
		MecPolicyContainer *vmToUePolicyContainer, MecVmContainer *ueContainer,
		MecFlowContainer *ueToVmFlowContainer,
		MecPolicyContainer *ueToVmPolicyContainer) {

	for (MecVmContainer::const_iterator itVm = vmContainer->CBegin();
			itVm != vmContainer->CEnd(); itVm++) {

		// creating a pair
		Ptr<MecPairCommunication> pair = CreateObject<MecPairCommunication>();

		// get the current VM
		Ptr<MecVm> vm = *itVm;

		// sanity check
		if (!vm->IsIsVm()) {
			NS_LOG_ERROR(
					"OrganasingPairCommunications -> Vm="<< vm->GetId() << "is not seen as a VM");
		}

		// sanity check
		if (!ueContainer->Contain(vm->GetBindId())) {
			NS_LOG_ERROR(
					"OrganasingPairCommunications -> Vm="<< vm->GetId() << "is not binded with any UE");
		}

		// get the UE
		Ptr<MecVm> ue = *(ueContainer->GetVmById(vm->GetBindId()));

		// sanity check
		if (!ue->IsIsUe()) {
			NS_LOG_ERROR(
					"OrganasingPairCommunications -> Ue="<< vm->GetId() << "is not seen as an ue");
		}

		// add VM and UE to the pair
		pair->AddVm(vm);
		pair->AddUe(ue);

		// add download flows VM->UE
		BindFlowPolicy(pair, vmToUeFlowContainer, vmToUePolicyContainer);

		// add upload flows UE->VM
		BindFlowPolicy(pair, ueToVmFlowContainer, ueToVmPolicyContainer);

		// add pair to the container
		pairCommunicationContainer->Add(pair);
	}

}

int main(int argc, char *argv[]) {

	CommandLine cmd;

	uint32_t baseStation = 3;
	uint32_t minUser = MecConstantsHelper::MIN_NUMBER_VMS_ON_HOST_SERVER;
	uint32_t maxUser = MecConstantsHelper::MAX_NUMBER_VMS_ON_HOST_SERVER;

	uint32_t minUpFlowRate = MecConstantsHelper::MIN_UPLOAD_FLOW_DATA_RATE;
	uint32_t maxUpFlowRate = MecConstantsHelper::MAX_UPLOAD_FLOW_DATA_RATE;

	uint32_t minDownFlowRate = MecConstantsHelper::MIN_DOWNLOAD_FLOW_DATA_RATE;
	uint32_t maxDownFlowRate = MecConstantsHelper::MAX_DOWNLOAD_FLOW_DATA_RATE;

	uint32_t simulationStopTime = 30;
	uint32_t strategy = 1;
	uint32_t mobility = 1;

	uint64_t rngSeed = SeedManager::GetSeed();
	uint64_t rngRun = SeedManager::GetRun();

	std::string dataRate = MecConstantsHelper::FLOW_DATE_RATE_UNITS;
	std::string outputDir = "results-default";

	srand(SeedManager::GetSeed());

	cmd.AddValue("base-station", "number of BS", baseStation);
	cmd.AddValue("min-user-bs", "min user per BS", minUser);
	cmd.AddValue("max-user-bs", "max user per BS", maxUser);
	cmd.AddValue("stop", "simulation stop time", simulationStopTime);
	cmd.AddValue("strategy", "schedule strategy", strategy);
	cmd.AddValue("mobility", "ue mobility strategy", mobility);

	cmd.AddValue("min-up-flow-rate", "min upload flow rate in Mbps",
			minUpFlowRate);
	cmd.AddValue("max-up-flow-rate", "min upload flow rate in Mbps",
			maxUpFlowRate);

	cmd.AddValue("min-down-flow-rate", "min download flow rate in Mbps",
			minDownFlowRate);
	cmd.AddValue("max-down-flow-rate", "min download flow rate in Mbps",
			maxDownFlowRate);

	cmd.AddValue("rngRun", "random number for rngrun", rngRun);
	cmd.AddValue("rngSeed", "random number for rngseed", rngSeed);

	cmd.AddValue("data-rate", "Mbps or kbps?", dataRate);
	cmd.AddValue("outputDir", "results default foulder name", outputDir);

	cmd.Parse(argc, argv);

	SeedManager::SetRun(rngRun);
	SeedManager::SetSeed(rngSeed);

	MecConstantsHelper::MIN_NUMBER_VMS_ON_HOST_SERVER = minUser;
	MecConstantsHelper::MAX_NUMBER_VMS_ON_HOST_SERVER = maxUser;
	MecConstantsHelper::SIMULATION_STOP_TIME = Seconds(simulationStopTime + 1);

	MecConstantsHelper::MIN_UPLOAD_FLOW_DATA_RATE = minUpFlowRate;
	MecConstantsHelper::MAX_UPLOAD_FLOW_DATA_RATE = maxUpFlowRate;

	MecConstantsHelper::MIN_DOWNLOAD_FLOW_DATA_RATE = minDownFlowRate;
	MecConstantsHelper::MAX_DOWNLOAD_FLOW_DATA_RATE = maxDownFlowRate;
	MecConstantsHelper::FLOW_DATE_RATE_UNITS = dataRate;


	std::ostringstream filename;
	filename << outputDir << "/";
	filename << "bs-" << baseStation << "_minUBS-" << minUser << "_maxUBS-"
			<< maxUser << "_nstime-" << simulationStopTime << "_strategy-"
			<< strategy << "_mobility-" << mobility << "_minUp"
			<< minUpFlowRate <<dataRate<< "_maxUp" << maxUpFlowRate <<dataRate
			<< "_minDown"  << minDownFlowRate<<dataRate << "_maxDown"
			<< maxDownFlowRate <<dataRate <<"_rngSeed-" << rngSeed << "_rngRun-"
			<< rngRun;

	std::cout << "Starting simulation..." << std::endl;

	//----------BEGIN: CREATING BACKHAUL AND CORE  ----------//

	Ptr<MecBackhaulCoreHelper> backhaulCoreHelper = CreateObject<
			MecBackhaulCoreHelper>();
	backhaulCoreHelper->Create(baseStation);
	backhaulCoreHelper->Print(std::cout);

	NodeContainer allHost = backhaulCoreHelper->HostNodes();
	NodeContainer allCore = backhaulCoreHelper->CoreNodes();
	NodeContainer allAggr = backhaulCoreHelper->AggrNodes();
	NodeContainer allEdge = backhaulCoreHelper->EdgeNodes();
	NodeContainer allNodes = backhaulCoreHelper->AllNodes();

	//----------END: CREATING BACKHAUL AND CORE  ----------//

	//----------BEGIN: CREATING CONTAINERS  ---------------//

	//Middlebox Container
	MecMiddleBoxContainer middleboxContainer;

	// VMs and theirs output flows to UEs according to policies
	MecVmContainer vmContainer;
	MecFlowContainer vmToUeFlowContainer;
	MecPolicyContainer vmToUePolicyContainer;

	//UEs and theris output flows to VMs
	MecVmContainer ueContainer;
	MecFlowContainer ueToVmFlowContainer;
	MecPolicyContainer ueToVmPolicyContainer;

	// Hypervisor container
	ApplicationContainer hypervisorContainer;

	//----------END: CREATING CONTAINERS  ----------------//

	//----------BEGIN: CREATING SCENARIO  ----------------//

	MecScenarioGeneratorHelper scenarioGenerator;

	//Install hypervisor in all server hosts nodes
	// that works as base stations
	scenarioGenerator.GeneratingHypervisors(&allHost, &hypervisorContainer);

	// Generating middlebox
	scenarioGenerator.GenerateMiddleboxes(&allAggr, &allEdge,
			&middleboxContainer);
	middleboxContainer.Print(std::cout);

	// Generating UEs and VMs
	scenarioGenerator.GenerateVMsAndUEs(&allHost, &vmContainer, &ueContainer);
	MecVmContainer all; // <<< JUST TO PRINT
	all.Add(vmContainer);
	all.Add(ueContainer);
	all.Print(std::cout);

	//Generating Flows UE -> VM and VM -> UE
	scenarioGenerator.GenerateFlows(&allHost, &vmContainer, &ueContainer,
			&vmToUeFlowContainer, &ueToVmFlowContainer);

	std::cout << "VM->UE flows:" << std::endl;
	vmToUeFlowContainer.Print(std::cout);

	std::cout << "UE->VM flows:" << std::endl;
	ueToVmFlowContainer.Print(std::cout);

	//Generating Policies
	scenarioGenerator.GeneratePolicies(&vmToUeFlowContainer,
			&ueToVmFlowContainer, &vmToUePolicyContainer,
			&ueToVmPolicyContainer, &middleboxContainer, backhaulCoreHelper);

	std::cout << "VM->UE policies:" << std::endl;
	vmToUePolicyContainer.Print(std::cout);

	std::cout << "UE->VM policies:" << std::endl;
	ueToVmPolicyContainer.Print(std::cout);

	//----------END: CREATING SCENARIO    ----------------//

	//---------BEGIN: ORGANASING VM-UE PAIRS -----------//

	MecPairCommunicationContainer pairCommunicationContainer;

	OrganasingPairCommunications(&pairCommunicationContainer, &vmContainer,
			&vmToUeFlowContainer, &vmToUePolicyContainer, &ueContainer,
			&ueToVmFlowContainer, &ueToVmPolicyContainer);

	std::cout << "Print pairs: " << std::endl;
	pairCommunicationContainer.Print(std::cout);

	//---------END: ORGANASING VM-UE PAIRS -----------//

	//---------BEGIN: SETUP POLIY IPV4 ROUTING------------//

	bool policyEnable = true;

	//joing policies
	MecPolicyContainer policyContainer;
	policyContainer.Add(vmToUePolicyContainer);
	policyContainer.Add(ueToVmPolicyContainer);

	//sanity check
	std::cout << "ALL VM<->UE policies:" << std::endl;
	policyContainer.Print(std::cout);

	SetupPolicyRouting(&allNodes, backhaulCoreHelper, &policyContainer,
			policyEnable);

	//---------END: SETUP POLIY IPV4 ROUTING -----------//

	//---------BEGIN: E2E LATENCY CHECKER  ------------//

	//MecE2EChecker e2eChecker;
	MecE2EChecker::Install(backhaulCoreHelper,
			MecConstantsHelper::E2E_CHECKER_START_TIME,
			MecConstantsHelper::E2E_CHECKER_STOP_TIME);

	//---------END: E2E LATENCY CHECKER  -------------//

	//---------BEGIN: CREATING SDN CONTROLLER AND SETUP----------//

	Ptr<MecSdnController> sdnController = CreateObject<MecSdnController>();

	sdnController->SetBackhaulCoreHelper(backhaulCoreHelper);
	sdnController->SetMiddleBoxContainer(&middleboxContainer);

	sdnController->SetUeContainer(&ueContainer);
	sdnController->SetUeToVmFlowContainer(&ueToVmFlowContainer);
	sdnController->SetUeToVmPolicyContainer(&ueToVmPolicyContainer);

	sdnController->SetVmContainer(&vmContainer);
	sdnController->SetVmToUeFlowContainer(&vmToUeFlowContainer);
	sdnController->SetVmToUePolicyContainer(&vmToUePolicyContainer);

	sdnController->SetHypervisorContainer(&hypervisorContainer);
	sdnController->SetCommunicationContainer(&pairCommunicationContainer);
	//---------END: CREATING SDN CONTROLLER AND SETUP------------//

	//---------BEGIN: CREATING FLOW MONITOR --------------//
	MecFlowMonitor flowMonitor(allHost, pairCommunicationContainer);
	//---------END: CREATING FLOW MONITOR --------------//

	//---------BEGIN: LOGGER --------------//

	std::string file = filename.str();
	std::string ext;

	ext = ".log.simulation";
	MecLogger::VerboseToFile(file, ext);

	ext = ".log.realtime";
	MecLogger::RealTimeToFile(file, ext);

	ext = ".log.up";
	MecLogger::UploadFlowMonitorToFile(file, ext);

	ext = ".log.down";
	MecLogger::DownloadFlowMonitorToFile(file, ext);

	//---------END: LOGGER --------------//

	//---------BEGIN: CALL SIMULATION -------------------//
	flowMonitor.StartApplication();
	sdnController->SetScheduleStrategy(strategy);
	sdnController->SetUeMobilityStrategy(mobility);
	sdnController->StartApplication();

	hypervisorContainer.Start(MecConstantsHelper::HYPERVISOR_START_TIME);
	hypervisorContainer.Stop(
			MecConstantsHelper::SIMULATION_STOP_TIME - MicroSeconds(2));

	NS_LOG_INFO("Run Simulation.");
	Simulator::Stop(MecConstantsHelper::SIMULATION_STOP_TIME);
	Simulator::Run();

	std::cout << "Simulation finished " << "\n";
	Simulator::Destroy();

	NS_LOG_INFO("Done.");

	//---------END: CALL SIMULATION ---------------------//

	return 0;

}

