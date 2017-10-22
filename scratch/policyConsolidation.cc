#include "ns3/core-module.h"
//#include "ns3/network-module.h"
//#include "ns3/internet-module.h"
//#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
//#include "ns3/ipv4-static-routing-helper.h"
//#include "ns3/ipv4-list-routing-helper.h"

#include "ns3/policy-routing-helper.h"
#include "ns3/dc-fat-tree-helper.h"
#include "ns3/dc-middlebox.h"
#include "ns3/dc-policy.h"
#include "ns3/dc-flow.h"
#include "ns3/policy-routing.h"
#include "ns3/dc-vm-hypervisor-helper.h"
#include "ns3/dc-vm.h"

#include "ns3/scene_generator.h"
#include "ns3/policy_log.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("PolicyConsolidation");

int main(int argc, char *argv[]) {
	PolicyLogAll policyLog;
	SceneGenerator sg;

	sg.defaultConfiguration();

	//for command line augment of scenegenerator
	CommandLine cmd;
	bool isPolicy = true;
	bool isChangeFlow = false;
	bool isChangePolicy = false;
	cmd.AddValue("isPolicy", "whether use policy aware or not", isPolicy);
	cmd.AddValue("isChangeFlow",
			"whether change the flow settings during sim or not", isChangeFlow);
	cmd.AddValue("isChangePolicy",
			"whether change the polich settings during sim or not",
			isChangePolicy);
	cmd.Parse(argc, argv);

	if (argc > 1) {
		sg.usePolicy = isPolicy;
		sg.m_changePolicy = isChangePolicy;
		sg.m_changeFlow = isChangeFlow;
	}

	std::cout << "Settings in sg: isPolicy:" << sg.usePolicy
			<< ", isChangeFlow:" << sg.m_changePolicy << ", isChangePolicy:"
			<< sg.m_changeFlow
			//<<"\n argc="<<argc
			//<<"\n argv="<<argv
			<< std::endl;
	//end of for command line augment of scenegenerator

	//std::cout<<"Start Test OK"<<std::endl;
	Ptr<DcFatTreeHelper> fattree = CreateObject<DcFatTreeHelper>(
			sg.sizeFatTreeK);

	//Other parameters if any

	fattree->Create();

	NodeContainer allHost = fattree->HostNodes();
	NodeContainer allCore = fattree->CoreNodes();
	NodeContainer allAggr = fattree->AggrNodes();
	NodeContainer allEdge = fattree->EdgeNodes();
	NodeContainer allNodes = fattree->AllNodes();

	//Middlebox
	MiddleBoxContainer mbc;
	DcPolicy policies;

	for (NodeContainer::Iterator i = allNodes.Begin(); i != allNodes.End();
			++i) {
		Ptr<Node> node = *i;
		Ptr<GlobalRouter> globalRouter = node->GetObject<GlobalRouter>();
		Ptr<Ipv4GlobalRouting> globalRoutingP =
				globalRouter->GetRoutingProtocol();
		Ptr<PolicyRouting> pr = globalRoutingP->GetObject<PolicyRouting>();
		pr->fattree = fattree;
		pr->policies = &policies;
		pr->mbc = &mbc;
		pr->m_policyEnable = true;		//Always true for policyRouting protocol
		//std::cout<<"Setting PolicyRouting OK"<<std::endl;
	}

	// Hypervisor and VMs
	DcVmContainer vmc;
	DcFlowContainer fc;

	//Ptr<FlowMonitor> monitor;  //for flow monitor statistics

	DcVmHypervisorApplicationHelper hyperVisor;
	ApplicationContainer hyperVisorApps = hyperVisor.Install(allHost);

	for (NodeContainer::Iterator i = allHost.Begin(); i != allHost.End(); ++i) {
		Ptr<Node> node = *i;

		Ptr<Application> app = node->GetApplication(0);
		//NS_ASSERT (app != 0);
		Ptr<DcVmHypervisorApplication> hvapp = app->GetObject<
				DcVmHypervisorApplication>();
		NS_ASSERT(hvapp != 0);

		hvapp->fattree = fattree;
		hvapp->mbc = &mbc;
		hvapp->policies = &policies;
		hvapp->vmc = &vmc;
		hvapp->fc = &fc;
		hvapp->policyLog = &policyLog;

		//hvapp->monitor = monitor;

		hvapp->m_usePolicy = sg.usePolicy;
	}
	hyperVisorApps.Start(Seconds(0.1));
	hyperVisorApps.Stop(Seconds(sg.stopTime));

	sg.fattree = fattree;
	sg.mbc = &mbc;
	sg.policies = &policies;
	sg.vmc = &vmc;
	sg.fc = &fc;
	sg.policyLog = &policyLog;

	//sg.monitor = monitor;

	if (sg.isTestScene) {
		std::cout << "In Test Scene....." << std::endl;
		sg.testScene();
	} else {
		sg.generateMiddleboxes();
		sg.generateVMs();
		sg.generateFlows();
		sg.generatePolicies();
	}

	sg.initialPlacement();
	sg.beforeStarted();

	NS_LOG_INFO("Run Simulation.");
	Simulator::Stop(Seconds(sg.stopTime + 0.001));
	Simulator::Run();

	sg.enableFlowMonitorAnalysis();

	std::cout << "Simulation finished " << "\n";

	Simulator::Destroy();
	NS_LOG_INFO("Done.");
	return 0;
}

