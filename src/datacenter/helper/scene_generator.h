/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SCENE_GENERATOR_H
#define SCENE_GENERATOR_H

#include "ns3/dc-fat-tree-helper.h"

//#include "ns3/dc-vm-hypervisor-helper.h"
#include "ns3/dc-vm-hypervisor.h"
//#include "ns3/policy-routing.h"

//#include "ns3/dc-traffic-helper.h"
#include "ns3/packet-sink-helper.h"

#include "ns3/dc-middlebox.h"
#include "ns3/dc-vm.h"

#include "ns3/dc-flow.h"

#include "ns3/policy_log.h"

#include "ns3/flow-monitor-module.h"

#define BW_BASE_FOR_MIGRATION 1/1000000
//#define BW_BASE 1/1000  //k

namespace ns3 {

/* ... */
class SceneGenerator {
private:

public:

	Ptr<UniformRandomVariable> m_uniformRand;

	Ptr<DcFatTreeHelper> fattree;
	MiddleBoxContainer * mbc;
	DcPolicy * policies;
	DcVmContainer * vmc;
	DcFlowContainer * fc;
	PolicyLogAll * policyLog;

	uint32_t sizeFatTreeK;

	bool isInitialPlacement;
	bool usePolicy;

//	uint32_t stopTime;
	double stopTime;

	uint32_t numVmOnServer_min; //number of VMs on one server
	uint32_t numVmOnServer_max;

	uint32_t numVmTenant_min; //number of VMs for each tennants in the DC, used to divide VMs groups
	uint32_t numVmTenant_max;

	uint32_t flowDataRate_min; //kbps
	uint32_t flowDataRate_max;

	uint32_t liveMig_VmSize_min;
	uint32_t liveMig_VmSize_max;
	uint32_t liveMig_DirtyRate_min; //%
	uint32_t liveMig_DirtyRate_max; //%
	uint32_t liveMig_L;
	uint32_t liveMig_X;
	uint32_t liveMig_T;

	bool m_changePolicy;
	bool m_changeFlow;

	//bool isTestAlgorithm;

	uint32_t maxFlowPerVM;

	uint16_t tcpPortSeed;
	uint16_t udpPortSeed;

	bool enableFlowMonitor;
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor;

	bool isTestScene; //use the scene in test(), or use the specify function for generating MB, VM, Policy, Flow

	SceneGenerator() {
		m_uniformRand = CreateObject<UniformRandomVariable>();

		sizeFatTreeK = 2;
		isInitialPlacement = false;
		usePolicy = true;
		stopTime = 100;

		m_changePolicy = false;
		m_changeFlow = false;

		//isTestAlgorithm = false;

		numVmOnServer_min = 2; //number of VMs on one server
		numVmOnServer_max = 10;

		numVmTenant_min = 4;
		numVmTenant_max = 10;

		flowDataRate_min = 50; //kbps
		flowDataRate_max = 1000;

		liveMig_VmSize_min = 50; //M
		liveMig_VmSize_max = 150; //M
		liveMig_DirtyRate_min = 0; //%
		liveMig_DirtyRate_max = 10; //%
		uint32_t averageVmSize = (liveMig_VmSize_min + liveMig_VmSize_max) / 2;
		liveMig_L = averageVmSize / 2;
		liveMig_X = averageVmSize / 5;
		liveMig_T = (averageVmSize / liveMig_L) / 10;

		maxFlowPerVM = 2;

		tcpPortSeed = 1000;
		udpPortSeed = 1000;
	}
	;

	~SceneGenerator() {
	}
	;

	void defaultConfiguration() {
		sizeFatTreeK = 4;
		isInitialPlacement = false;

		//stopTime = 200;
		stopTime = 3;

		//isInitialPlacement = true;
		usePolicy = true;
		//usePolicy = false;
		m_changePolicy = false;
		m_changeFlow = false;

		//isTestAlgorithm = false;

		numVmOnServer_min = 1; //2; //number of VMs on one server
		numVmOnServer_max = 5; //10;

		numVmTenant_min = 2; //4;
		numVmTenant_max = 8; //10;

		//Datarate Unit: kbps
		flowDataRate_min = 500; //1000;
		flowDataRate_max = 10000; //15000;

		maxFlowPerVM = 2;

		tcpPortSeed = 1000;
		udpPortSeed = 1000;

		//policyLog->initialLogFiles(usePolicy);

		liveMig_VmSize_min = 50 * 1000000 * BW_BASE_FOR_MIGRATION; //M
		liveMig_VmSize_max = 150 * 1000000 * BW_BASE_FOR_MIGRATION; //M
		uint32_t averageVmSize = (liveMig_VmSize_min + liveMig_VmSize_max) / 2;

		liveMig_DirtyRate_min = 0; //%0
		liveMig_DirtyRate_max = 3 * 1000000 * BW_BASE_FOR_MIGRATION; //averageVmSize*0.05;//%5

		liveMig_L = 10 * 1000000 * BW_BASE_FOR_MIGRATION; //averageVmSize/10; //MBytes
		liveMig_X = averageVmSize / 10;
		liveMig_T = (averageVmSize / liveMig_L) / 10;

		enableFlowMonitor = false; //MUST ENABLE FOR STATISTICS

		isTestScene = false;
	}
	;

	void generateMiddleboxes() {
		NodeContainer allNodes = fattree->AllNodes();

		NodeContainer hostNodes = fattree->HostNodes();
		NodeContainer switchNodes;
		//switchNodes.Add(fattree->CoreNodes());
		switchNodes.Add(fattree->AggrNodes());
		switchNodes.Add(fattree->EdgeNodes());

		//switchNodes.Add(fattree->AggrNodes());

		//each Aggr/Edge has one MB
		mbc->Create(switchNodes.GetN());  //both id and type are ready

		for (uint32_t i = 0; i < switchNodes.GetN(); ++i) {
			Ptr<Node> node = switchNodes.Get(i);
			Ptr<DcMiddleBox> mb = mbc->Get(i);

			//uint32_t rateValue = m_uniformRand->GetInteger(flowDataRate_min, flowDataRate_max);				
			//sprintf(rateStr,"%dkb/s", rateValue);
			DataRate rate = DataRate("5Gbps");// 1Gbps, related to the link capacity?
			mb->m_capacity = rate.GetBitRate() / 1000;			//kbps
			mb->m_capacityRemain = mb->m_capacity;

			mb->attachMiddleBox(node);
		}

		mbc->print();

		return;
	}
	;

	void generateVMs() {
		//uint32_t cpu;
		//uint32_t memory;

		uint32_t numVMonNode = 1;

		std::vector<uint32_t> numVMonNodeVector;
		uint32_t totalVMs = 0;

		NodeContainer allHosts = fattree->HostNodes();

		for (uint32_t i = 0; i < allHosts.GetN(); ++i) {
			//numVMonNode = RandomVariable(UniformVariable(5, 15)).GetInteger();
			numVMonNode = m_uniformRand->GetInteger(numVmOnServer_min,
					numVmOnServer_max);	//RandomVariable(UniformVariable(5, 10)).GetInteger();
			//numVMonNode = 1;  //TODOTODOTODO:
			numVMonNodeVector.push_back(numVMonNode);
			totalVMs += numVMonNode;

			//if(i==1)
			//	break;
		}

		vmc->Create(totalVMs);

		uint32_t vmIndex = 0;

		for (uint32_t i = 0; i < allHosts.GetN(); ++i) {
			Ptr<Node> node = allHosts.Get(i);

			Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node->GetId());
			hvapp->m_totalCPU = 200;
			hvapp->m_totalMemory = 200;

			numVMonNode = numVMonNodeVector[i];

			for (uint32_t j = 0; j < numVMonNode; j++) {
				Ptr<DcVm> vm = vmc->Get(vmIndex);

				hvapp->m_hostedVMs.Add(vm);

				vm->m_hostServer = node->GetId();

				vm->m_cpu = m_uniformRand->GetInteger(5, 10);
				vm->m_memory = m_uniformRand->GetInteger(5, 10);

				//for migration cost
				uint32_t liveMig_VmSize = m_uniformRand->GetInteger(
						liveMig_VmSize_min, liveMig_VmSize_max);//RandomVariable(UniformVariable(liveMig_VmSize_min, liveMig_VmSize_max)).GetInteger();
				uint32_t liveMig_DirtyRate = m_uniformRand->GetInteger(
						liveMig_DirtyRate_min, liveMig_DirtyRate_max);//RandomVariable(UniformVariable(liveMig_DirtyRate_min, liveMig_DirtyRate_max)).GetInteger();

				vm->liveMig_VmSize = liveMig_VmSize;
				vm->liveMig_DirtyRate = liveMig_DirtyRate;
				vm->liveMig_L = liveMig_L;
				vm->liveMig_T = liveMig_T;
				vm->liveMig_X = liveMig_X;

				vm->CalcMigrationTraffic();

				vmIndex++;
			}
		}
	}
	;

	void generateFlows() {
		uint32_t numVMs = vmc->GetN();

		//generating tenants for VMs
		uint32_t numTenant = numVMs
				/ ((numVmTenant_min + numVmOnServer_max) / 2); //number of Tenants
		DcVmContainer tennants[numTenant];
		DcVmContainer unAssignedVM;
		unAssignedVM.Add(*vmc);
		//std::cout<<"VMC size="<<vmc->GetN()<<", unAssignedVM size="<<unAssignedVM.GetN()<<std::endl;
		for (uint32_t i = 0; i < numTenant; i++) {
			if (i == numTenant - 1) {
				tennants[i].Add(unAssignedVM);
				break;
			}

			uint32_t numVMinTenant = m_uniformRand->GetInteger(numVmTenant_min,
					numVmOnServer_max);
			if (numVMinTenant > unAssignedVM.GetN()) {
				numVMinTenant = unAssignedVM.GetN();
			}
			for (uint32_t j = 0; j < numVMinTenant; j++) {
				uint32_t tempID = m_uniformRand->GetInteger(0,
						unAssignedVM.GetN() - 1);
				Ptr<DcVm> vm = unAssignedVM.Get(tempID);
				tennants[i].Add(vm);
				unAssignedVM.Remove(vm);
				//std::cout<<"tennant="<<i<<", vm id="<<vm->m_id<<std::endl;
			}

			if (i < numTenant - 1 && unAssignedVM.GetN() <= 2) {
				tennants[i].Add(unAssignedVM);
				numTenant = i + 1;
			}
		}

		//LogComponentEnable("DcFlowApplication", LOG_LEVEL_ALL);

		//install
		PacketSinkHelper sinkTcp("ns3::TcpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), DC_TCP_DST_PORT));
		ApplicationContainer sinkTcpApps;
		sinkTcpApps = sinkTcp.Install(fattree->HostNodes());

		PacketSinkHelper sinkUdp("ns3::UdpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), DC_UDP_DST_PORT));
		ApplicationContainer sinkUdpApps;
		sinkUdpApps = sinkUdp.Install(fattree->HostNodes());

		DataRate rate = DataRate("50kb/s");
		Ptr<DcFlowApplication> flow;

		uint32_t numFlow = 0;
		uint32_t dstVmID = 0;
		char rateStr[100];
		uint32_t rateValue = 0;

		std::vector<uint32_t> numFlowOnVmVector;
		uint32_t totalFlows = 0;

		for (uint32_t i = 0; i < numVMs; i++) {
			//numFlow = 2;
			numFlow = m_uniformRand->GetInteger(1, maxFlowPerVM);

			numFlowOnVmVector.push_back(numFlow);
			totalFlows += numFlow;
		}

		fc->Create(totalFlows);

		std::cout << "There are " << numTenant << " tenants, total Flows="
				<< totalFlows << std::endl;

		uint32_t totalAssignedVM = 0;
		uint32_t flowIndex = 0;
		for (uint32_t i = 0; i < numTenant; i++) {
			//std::cout<<"Tenant "<<i<<std::endl;
			//tennants[i].print(std::cout);
			for (uint32_t k = 0; k < tennants[i].GetN(); k++) {
				Ptr<DcVm> vm = tennants[i].Get(k);
				totalAssignedVM++;

				numFlow = numFlowOnVmVector[vm->m_id];

				//uint32_t count=0;
				for (uint32_t j = 0; j < numFlow; j++) {
					Ptr<DcFlowApplication> flow = fc->Get(flowIndex);
					flow->m_srcVmID = vm->m_id;
					flow->m_srcNodeID = vm->m_hostServer;
					vm->m_flowList.Add(flow);

					//Get a random dest VM
					uint32_t vmIndex = 0;
					Ptr<DcVm> dstVM;
					do {
						vmIndex = m_uniformRand->GetInteger(0,
								tennants[i].GetN() - 1);
						dstVM = tennants[i].Get(vmIndex);
						dstVmID = dstVM->m_id;
					} while (dstVmID == vm->m_id);

					flow->m_dstVmID = dstVmID;
					flow->m_dstNodeID = dstVM->m_hostServer;
					dstVM->m_flowList.Add(flow);

					//Get a rate
					//rateValue = RandomVariable(UniformVariable(50, 1000)).GetInteger();
					rateValue = m_uniformRand->GetInteger(flowDataRate_min,
							flowDataRate_max);
					sprintf(rateStr, "%dkb/s", rateValue);
					rate = DataRate(rateStr);

					bool isTCP = false;
					flow->m_dataRate = rate;
					flow->SetUdpPacketSize(1450);  //size of UDP data: 1500-20-8
					flow->useTCP = isTCP;
					if (isTCP) {
						flow->m_srcPort = tcpPortSeed++;
						flow->m_dstPort = DC_TCP_DST_PORT;  //tcpPortSeed++;
					} else {
						flow->m_srcPort = udpPortSeed++;
						flow->m_dstPort = DC_UDP_DST_PORT;  //udpPortSeed++;
					}

					//flow = dcVMs->createFlows(itVM->id,dstVmID,false,rate,0);
					//flow->m_nPackets=1;

					flowIndex++;
				}
			}
		}

		NS_ASSERT_MSG(totalAssignedVM == vmc->GetN(),
				"Error: Total VMs in all tenants is incorrect");

		std::cout << "Complete generating flows...total Flows=" << totalFlows
				<< std::endl;
		fc->print(std::cout);
	}
	;

	void generateFlows_withoutTenants() {
		//LogComponentEnable("DcFlowApplication", LOG_LEVEL_ALL);

		//install
		PacketSinkHelper sinkTcp("ns3::TcpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), DC_TCP_DST_PORT));
		ApplicationContainer sinkTcpApps;
		sinkTcpApps = sinkTcp.Install(fattree->HostNodes());

		PacketSinkHelper sinkUdp("ns3::UdpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), DC_UDP_DST_PORT));
		ApplicationContainer sinkUdpApps;
		sinkUdpApps = sinkUdp.Install(fattree->HostNodes());

		DataRate rate = DataRate("50kb/s");
		Ptr<DcFlowApplication> flow;

		uint32_t numVMs = vmc->GetN();

		uint32_t numFlow = 0;
		uint32_t dstVmID = 0;
		char rateStr[100];
		uint32_t rateValue = 0;

		std::vector<uint32_t> numFlowOnVmVector;
		uint32_t totalFlows = 0;

		for (uint32_t i = 0; i < numVMs; i++) {
			//numFlow = 2;
			numFlow = m_uniformRand->GetInteger(1, maxFlowPerVM);

			numFlowOnVmVector.push_back(numFlow);
			totalFlows += numFlow;
		}

		fc->Create(totalFlows);

		uint32_t flowIndex = 0;
		for (uint32_t i = 0; i < numVMs; i++) {
			Ptr<DcVm> vm = vmc->Get(i);

			numFlow = numFlowOnVmVector[i];

			//uint32_t count=0;
			for (uint32_t j = 0; j < numFlow; j++) {
				Ptr<DcFlowApplication> flow = fc->Get(flowIndex);

				flow->m_srcVmID = vm->m_id;
				flow->m_srcNodeID = vm->m_hostServer;
				vm->m_flowList.Add(flow);

				//Get a random dest VM
				do {
					dstVmID = m_uniformRand->GetInteger(0, numVMs - 1);
				} while (dstVmID == vm->m_id);

				Ptr<DcVm> dstVM = vmc->Get(dstVmID);
				flow->m_dstVmID = dstVmID;
				flow->m_dstNodeID = dstVM->m_hostServer;
				dstVM->m_flowList.Add(flow);

				//Get a rate
				//rateValue = RandomVariable(UniformVariable(50, 1000)).GetInteger();
				rateValue = m_uniformRand->GetInteger(flowDataRate_min,
						flowDataRate_max);
				sprintf(rateStr, "%dkb/s", rateValue);
				rate = DataRate(rateStr);

				bool isTCP = false;
				flow->m_dataRate = rate;
				flow->useTCP = isTCP;
				if (isTCP) {
					flow->m_srcPort = tcpPortSeed++;
					flow->m_dstPort = DC_TCP_DST_PORT;			//tcpPortSeed++;
				} else {
					flow->m_srcPort = udpPortSeed++;
					flow->m_dstPort = DC_UDP_DST_PORT;			//udpPortSeed++;
				}

				//flow = dcVMs->createFlows(itVM->id,dstVmID,false,rate,0);

				flowIndex++;
			}
		}

		std::cout << "Complete generating flows..." << std::endl;
	}
	;

	void generatePolicies() {
		for (DcFlowContainer::Iterator it = fc->Begin(); it != fc->End();
				it++) {
			Ptr<DcFlowApplication> flow = *it;
			Ptr<Node> srcNode = NodeList::GetNode(flow->m_srcNodeID);
			Ptr<Node> dstNode = NodeList::GetNode(flow->m_dstNodeID);
			Ipv4Address saddr =
					srcNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
			Ipv4Address daddr =
					dstNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

			uint8_t proto = flow->useTCP ? POLICY_TCP_PROTO : POLICY_UDP_PROTO;

			uint16_t sport = flow->m_srcPort;
			uint16_t dport = flow->m_dstPort;

			PolicyItemIterator itP = policies->createPolicyEntry(saddr.Get(),
					daddr.Get(), proto, sport, dport);
			if (policies->isValid(itP)) {
				itP->flowID = flow->m_flowID;//must set before generating random policies

				policies->generateRandomPolicy(itP, *mbc, fattree, *fc);
				policies->printPolicy(itP);
			} else
				std::cout << "create policy failed\n";
		}

		//Set MB capacity
		policies->initialMiddleboxCapacity(*fc);

		//policies->setPolicyStatus(9,false);
		policies->setPolicyStatus(20, false);

		return;
	}
	;

	void initialPlacement() {
		if (!isInitialPlacement)
			return;

		return;
	}

	void beforeStarted() {
		//Initialize log files
		policyLog->initialLogFiles(usePolicy, sizeFatTreeK);

		Ptr<Node> node1 = fattree->HostNodes().Get(0);
		Ptr<DcVmHypervisorApplication> hvapp1 = getHyperVisor(node1->GetId());
		hvapp1->LogPolicyViolation();

		if (enableFlowMonitor) {
			///NodeContainer allFatTreeNodes;
			//allFatTreeNodes.Add(fattree->CoreNodes());
			//allFatTreeNodes.Add(fattree->AggrNodes());
			//allFatTreeNodes.Add(fattree->EdgeNodes());
			//allFatTreeNodes.Add(fattree->HostNodes());

			monitor = flowmon.InstallAll();
			//monitor = flowmon.Install(allFatTreeNodes);

			Ptr<Node> node = fattree->HostNodes().Get(0);
			Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node->GetId());
			hvapp->monitor = monitor;
			hvapp->classifier = DynamicCast<Ipv4FlowClassifier>(
					flowmon.GetClassifier());

			//Ptr<FlowMonitor> monitor = flowmon.Install(fattree->HostNodes());
			std::cout << "Enable flow monitor.........." << std::endl;
		}
		return;

		Ptr<Node> node = fattree->HostNodes().Get(0);
		Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node->GetId());

		//set Policy and Flow change indicator, only for the first host
		hvapp->m_changeFlow = m_changeFlow;
		hvapp->m_changePolicy = m_changePolicy;
		/*
		 hvapp->policyLog->logVmCostMigration
		 <<"HostNodesNum\t"<<fattree->HostNodes().GetN()<<"\t"
		 <<"AllNodesNum\t"<<fattree->AllNodes().GetN()<<"\t"
		 <<"allFlow.size\t"<<dcVMs->m_allFlows.size()<<"\t"
		 <<"allVMs.size\t"<<dcVMs->m_allVMs.size()<<"\t"
		 <<"usePolicy\t"<<(usePolicy?1:0)<<"\t"
		 <<"\n";

		 uint32_t numLinks[3];
		 numLinks[0]= fattree->HostNodes().GetN(); //num of host-edge links
		 numLinks[1]= fattree->EdgeNodes().GetN() * fattree->GetFatTreeSize(); //num of edge-aggr links
		 numLinks[2]= fattree->CoreNodes().GetN() * 2 * fattree->GetFatTreeSize(); //num of aggr-core links
		 hvapp->policyLog->logLinkUtilization
		 <<"HostNodesNum\t"<<fattree->HostNodes().GetN()<<"\t"
		 <<"AllNodesNum\t"<<fattree->AllNodes().GetN()<<"\t"
		 <<"allFlow.size\t"<<dcVMs->m_allFlows.size()<<"\t"
		 <<"allVMs.size\t"<<dcVMs->m_allVMs.size()<<"\t"
		 <<"usePolicy\t"<<(usePolicy?1:0)<<"\t"
		 <<numLinks[0]<<"\t"<<numLinks[1]<<"\t"<<numLinks[2]<<"\t"
		 <<"\n";

		 hvapp->policyLog->logVmCountOnServer
		 <<"HostNodesNum\t"<<fattree->HostNodes().GetN()<<"\t"
		 <<"AllNodesNum\t"<<fattree->AllNodes().GetN()<<"\t"
		 <<"allFlow.size\t"<<dcVMs->m_allFlows.size()<<"\t"
		 <<"allVMs.size"<<dcVMs->m_allVMs.size()<<"\t"
		 <<"usePolicy\t"<<(usePolicy?1:0)<<"\t"
		 <<"\n";
		 */

		//Initialize log files
		//policyLog->initialLogFiles(usePolicy);
		//add log for the first state
		//hvapp->LogAllVmCost(0, 0, 0);
		//hvapp->LogAllLinkUtilization(false);
		std::cout << "Simulation Started for DC" << std::endl;
	}

	void enableFlowMonitorAnalysis() {
		return; //calc the statistics in hypervisor

		if (!enableFlowMonitor)
			return;

		std::cout << "Flow monitor result.........." << std::endl;

		char filename[] = "statistics/Fat-tree.xml"; // filename for Flow Monitor xml output file

		monitor->CheckForLostPackets();
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(
				flowmon.GetClassifier());
		std::map<FlowId, FlowMonitor::FlowStats> stats =
				monitor->GetFlowStats();
		std::map<FlowId, FlowMonitor::FlowStats> statsLast = stats;
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i =
				stats.begin(); i != stats.end(); ++i) {
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

			std::cout << "  Flow " << i->first << " (" << t.sourceAddress
					<< " -> " << t.destinationAddress << ")\n";
			std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
			std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
			std::cout << "  Throughput: "
					<< i->second.rxBytes * 8.0 / 1000 / 1000 << " Mbps\n";
		}
		monitor->SerializeToXmlFile(filename, true, true);
	}

	Ptr<DcVmHypervisorApplication> getHyperVisor(uint32_t NodeID) {
		//return hyperVisor;

		Ptr<DcVmHypervisorApplication> hvapp;
		Ptr<Application> app;

		uint32_t num = NodeList::GetNode(NodeID)->GetNApplications();

		for (uint32_t i = 0; i < num; i++) {
			app = NodeList::GetNode(NodeID)->GetApplication(i);
			//NS_ASSERT (app != 0);
			hvapp = app->GetObject<DcVmHypervisorApplication>();
			if (hvapp != 0)
				return hvapp;
			//NS_ASSERT (hvapp != 0);
		}

		NS_ASSERT_MSG(hvapp != 0, "Can't find the DcVmHypervisorApplication");

		return hvapp;
	}

	void testScene() {
		//fattree->Print();
		//fattree->Test();

		NodeContainer allHost = fattree->HostNodes();
		NodeContainer allCore = fattree->CoreNodes();
		NodeContainer allAggr = fattree->AggrNodes();
		NodeContainer allEdge = fattree->EdgeNodes();
		NodeContainer allNodes = fattree->AllNodes();

		mbc->Create(5);
		mbc->Get(0)->attachMiddleBox(allNodes.Get(5));
		mbc->Get(1)->attachMiddleBox(allNodes.Get(7));
		mbc->Get(2)->attachMiddleBox(allNodes.Get(3));
		mbc->Get(3)->attachMiddleBox(allNodes.Get(13));
		mbc->Get(4)->attachMiddleBox(allNodes.Get(12));

		mbc->print();

		Ptr<Node> clientNode = allHost.Get(0);
		Ptr<Node> serverNode = allHost.Get(4);
		Ipv4Address clientAddr =
				clientNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		Ipv4Address serverAddr =
				serverNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

		//creating a vm on host 1 and one vm on host2
		vmc->Create(3);

		for (uint32_t i = 0; i < allHost.GetN(); ++i) {
			Ptr<Node> node = allHost.Get(i);

			Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node->GetId());
			hvapp->m_totalCPU = 200;
			hvapp->m_totalMemory = 200;
		}

		//VM0
		Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(
				clientNode->GetId());
		Ptr<DcVm> vm = vmc->Get(0);
		hvapp->m_hostedVMs.Add(vm);
		vm->m_hostServer = clientNode->GetId();
		vm->m_cpu = m_uniformRand->GetInteger(5, 10);
		vm->m_memory = m_uniformRand->GetInteger(5, 10);

		//for migration cost
		uint32_t liveMig_VmSize = m_uniformRand->GetInteger(liveMig_VmSize_min,
				liveMig_VmSize_max);//RandomVariable(UniformVariable(liveMig_VmSize_min, liveMig_VmSize_max)).GetInteger();
		uint32_t liveMig_DirtyRate = m_uniformRand->GetInteger(
				liveMig_DirtyRate_min, liveMig_DirtyRate_max);//RandomVariable(UniformVariable(liveMig_DirtyRate_min, liveMig_DirtyRate_max)).GetInteger();

		vm->liveMig_VmSize = liveMig_VmSize;
		vm->liveMig_DirtyRate = liveMig_DirtyRate;
		vm->liveMig_L = liveMig_L;
		vm->liveMig_T = liveMig_T;
		vm->liveMig_X = liveMig_X;
		vm->CalcMigrationTraffic();

		//Vm1
		hvapp = getHyperVisor(serverNode->GetId());
		vm = vmc->Get(1);
		hvapp->m_hostedVMs.Add(vm);
		vm->m_hostServer = serverNode->GetId();
		vm->m_cpu = m_uniformRand->GetInteger(5, 10);
		vm->m_memory = m_uniformRand->GetInteger(5, 10);

		//for migration cost
		liveMig_VmSize = m_uniformRand->GetInteger(liveMig_VmSize_min,
				liveMig_VmSize_max);//RandomVariable(UniformVariable(liveMig_VmSize_min, liveMig_VmSize_max)).GetInteger();
		liveMig_DirtyRate = m_uniformRand->GetInteger(liveMig_DirtyRate_min,
				liveMig_DirtyRate_max);	//RandomVariable(UniformVariable(liveMig_DirtyRate_min, liveMig_DirtyRate_max)).GetInteger();

		vm->liveMig_VmSize = liveMig_VmSize;
		vm->liveMig_DirtyRate = liveMig_DirtyRate;
		vm->liveMig_L = liveMig_L;
		vm->liveMig_T = liveMig_T;
		vm->liveMig_X = liveMig_X;
		vm->CalcMigrationTraffic();

		//Vm3
		hvapp = getHyperVisor(serverNode->GetId());
		vm = vmc->Get(2);
		hvapp->m_hostedVMs.Add(vm);
		vm->m_hostServer = serverNode->GetId();
		vm->m_cpu = m_uniformRand->GetInteger(5, 10);
		vm->m_memory = m_uniformRand->GetInteger(5, 10);

		//for migration cost
		liveMig_VmSize = m_uniformRand->GetInteger(liveMig_VmSize_min,
				liveMig_VmSize_max);//RandomVariable(UniformVariable(liveMig_VmSize_min, liveMig_VmSize_max)).GetInteger();
		liveMig_DirtyRate = m_uniformRand->GetInteger(liveMig_DirtyRate_min,
				liveMig_DirtyRate_max);	//RandomVariable(UniformVariable(liveMig_DirtyRate_min, liveMig_DirtyRate_max)).GetInteger();

		vm->liveMig_VmSize = liveMig_VmSize;
		vm->liveMig_DirtyRate = liveMig_DirtyRate;
		vm->liveMig_L = liveMig_L;
		vm->liveMig_T = liveMig_T;
		vm->liveMig_X = liveMig_X;
		vm->CalcMigrationTraffic();

		vmc->print(std::cout);

		//Create Flows
		fc->Create(1);

		LogComponentEnable("DcFlowApplication", LOG_LEVEL_ALL);
		//LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
		//LogComponentEnable("UdpSocket", LOG_LEVEL_LOGIC);

		//install
		PacketSinkHelper sinkTcp("ns3::TcpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), 9));
		ApplicationContainer sinkTcpApps;
		sinkTcpApps = sinkTcp.Install(fattree->HostNodes());

		PacketSinkHelper sinkUdp("ns3::UdpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), 9));
		ApplicationContainer sinkUdpApps;
		sinkUdpApps = sinkUdp.Install(fattree->HostNodes());
		//sinkUdpApps.Start (Seconds (0.0));
		//sinkUdpApps.Stop (Seconds(stopTime+0.001));

		/*PacketSinkHelper sink ("ns3::PacketSocketFactory",
		 InetSocketAddress (Ipv4Address::GetAny(), DC_UDP_DST_PORT));
		 ApplicationContainer sinkApps;
		 sinkApps = sink.Install (fattree->HostNodes());

		 sinkApps.Start (Seconds (0.0));
		 sinkApps.Stop (Seconds(stopTime+0.001));*/

		//Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", 
		//					MakeCallback (&DcVmHypervisorApplication::FlowRxTrace)); 
		//hvapp = getHyperVisor(28);
		//Config::Connect ("/NodeList/28/ApplicationList/*/$ns3::PacketSink/Rx", 
		//					MakeCallback (&DcVmHypervisorApplication::FlowRxTrace, hvapp));
		vm = vmc->Get(0);
		Ptr<DcFlowApplication> flow = fc->Get(0);

		flow->m_srcVmID = vm->m_id;
		flow->m_srcNodeID = vm->m_hostServer;
		vm->m_flowList.Add(flow);

		uint32_t dstVmID = 1;
		Ptr<DcVm> dstVM = vmc->Get(dstVmID);
		flow->m_dstVmID = dstVmID;
		flow->m_dstNodeID = dstVM->m_hostServer;
		dstVM->m_flowList.Add(flow);

		DataRate rate = DataRate("500kb/s");

		bool isTCP = false;
		flow->m_dataRate = rate;
		flow->useTCP = isTCP;
		if (isTCP) {
			flow->m_srcPort = 49153;
			flow->m_dstPort = 9;
		} else {
			flow->m_srcPort = 49153;
			flow->m_dstPort = 9;
		}
		flow->m_nPackets = 1;
		flow->m_packetsSent = 1;
		fc->print(std::cout);

		//PolicyItemIterator itP = policies->createPolicyEntry(clientAddr.Get(),serverAddr.Get(),17,0,0);
		PolicyItemIterator itP = policies->createPolicyEntry(clientAddr.Get(),
				serverAddr.Get(), 17, 49153, 9);
		//policies->generateRandomPolicy(itP, *mbc);  //one MB
		//policies->generateRandomPolicy(itP, *mbc);
		//policies->generateRandomPolicy(itP, *mbc);
		//policies->generateRandomPolicy(itP, *mbc); //three MBs
		itP->len = 2;
		itP->Seq[0] = MB_FIREWALL;
		itP->Seq[1] = MB_IDS;
		itP->Seq[2] = MB_LB;
		itP->mbList.Add(mbc->Get(0));
		itP->mbList.Add(mbc->Get(1));
		//itP->mbList.Add(mbc->Get(2));
		policies->checkPolicy(itP);
		policies->printPolicy(itP);

		return;
	}

	void sceneForStressTest(uint32_t totalVMs, uint32_t totalFlows) {

		NodeContainer allHosts = fattree->HostNodes();

		generateMiddleboxes();

		std::cout << "totalVMs=" << totalVMs << std::endl;

		//VMs
		vmc->Create(totalVMs);
		uint32_t vmIndex = 0;
		for (uint32_t i = 0; i < allHosts.GetN(); ++i) {
			Ptr<Node> node = allHosts.Get(i);

			Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node->GetId());
			hvapp->m_totalCPU = 1000;
			hvapp->m_totalMemory = 1000;

			uint32_t numVMonNode = totalVMs / allHosts.GetN();
			if (i == allHosts.GetN() - 1)
				numVMonNode = totalVMs - vmIndex;

			for (uint32_t j = 0; j < numVMonNode; j++) {
				Ptr<DcVm> vm = vmc->Get(vmIndex);

				hvapp->m_hostedVMs.Add(vm);

				vm->m_hostServer = node->GetId();

				vm->m_cpu = m_uniformRand->GetInteger(1, 5);
				vm->m_memory = m_uniformRand->GetInteger(1, 5);

				//for migration cost
				uint32_t liveMig_VmSize = m_uniformRand->GetInteger(
						liveMig_VmSize_min, liveMig_VmSize_max);//RandomVariable(UniformVariable(liveMig_VmSize_min, liveMig_VmSize_max)).GetInteger();
				uint32_t liveMig_DirtyRate = m_uniformRand->GetInteger(
						liveMig_DirtyRate_min, liveMig_DirtyRate_max);//RandomVariable(UniformVariable(liveMig_DirtyRate_min, liveMig_DirtyRate_max)).GetInteger();

				vm->liveMig_VmSize = liveMig_VmSize;
				vm->liveMig_DirtyRate = liveMig_DirtyRate;
				vm->liveMig_L = liveMig_L;
				vm->liveMig_T = liveMig_T;
				vm->liveMig_X = liveMig_X;

				vm->CalcMigrationTraffic();

				vmIndex++;
			}
		}

		uint32_t numVMs = vmc->GetN();

		//Flows ======================
		//generating tenants for VMs
		uint32_t numTenant = numVMs
				/ ((numVmTenant_min + numVmOnServer_max) / 2); //number of Tenants
		std::cout << "Num of tenant: " << numTenant << ", numVMs=" << numVMs
				<< ",(numVmTenant_min+numVmOnServer_max)/2="
				<< (numVmTenant_min + numVmOnServer_max) / 2 << std::endl;
		DcVmContainer tennants[numTenant];
		DcVmContainer unAssignedVM;
		unAssignedVM.Add(*vmc);
		//std::cout<<"VMC size="<<vmc->GetN()<<", unAssignedVM size="<<unAssignedVM.GetN()<<std::endl;
		for (uint32_t i = 0; i < numTenant; i++) {
			if (i == numTenant - 1) {
				tennants[i].Add(unAssignedVM);
				break;
			}

			uint32_t numVMinTenant = m_uniformRand->GetInteger(numVmTenant_min,
					numVmOnServer_max);
			if (numVMinTenant > unAssignedVM.GetN()) {
				numVMinTenant = unAssignedVM.GetN();
			}
			for (uint32_t j = 0; j < numVMinTenant; j++) {
				uint32_t tempID = m_uniformRand->GetInteger(0,
						unAssignedVM.GetN() - 1);
				Ptr<DcVm> vm = unAssignedVM.Get(tempID);
				tennants[i].Add(vm);
				unAssignedVM.Remove(vm);
				//std::cout<<"tennant="<<i<<", vm id="<<vm->m_id<<std::endl;
			}

			if (i < numTenant - 1 && unAssignedVM.GetN() <= 2) {
				tennants[i].Add(unAssignedVM);
				numTenant = i + 1;
			}
		}

		//std::cout<<"Num of tenant: "<<numTenant<<std::endl;

		//LogComponentEnable("DcFlowApplication", LOG_LEVEL_ALL);

		//install
		PacketSinkHelper sinkTcp("ns3::TcpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), DC_TCP_DST_PORT));
		ApplicationContainer sinkTcpApps;
		sinkTcpApps = sinkTcp.Install(fattree->HostNodes());

		PacketSinkHelper sinkUdp("ns3::UdpSocketFactory",
				InetSocketAddress(Ipv4Address::GetAny(), DC_UDP_DST_PORT));
		ApplicationContainer sinkUdpApps;
		sinkUdpApps = sinkUdp.Install(fattree->HostNodes());

		DataRate rate = DataRate("50kb/s");
		Ptr<DcFlowApplication> flow;

		uint32_t numFlow = 0;
		uint32_t dstVmID = 0;
		char rateStr[100];
		uint32_t rateValue = 0;

		fc->Create(totalFlows);

		std::cout << "There are " << numTenant << " tenants, total Flows="
				<< totalFlows << std::endl;

		uint32_t totalAssignedVM = 0;
		uint32_t flowIndex = 0;
		for (uint32_t i = 0; i < numTenant; i++) {
			for (uint32_t k = 0; k < tennants[i].GetN(); k++) {
				Ptr<DcVm> vm = tennants[i].Get(k);
				totalAssignedVM++;

				numFlow = totalFlows / numVMs;
				if (i == numTenant - 1 && k == tennants[i].GetN() - 1)
					numFlow = totalFlows - flowIndex;

				//uint32_t count=0;
				for (uint32_t j = 0; j < numFlow; j++) {
					Ptr<DcFlowApplication> flow = fc->Get(flowIndex);
					flow->m_srcVmID = vm->m_id;
					flow->m_srcNodeID = vm->m_hostServer;
					vm->m_flowList.Add(flow);

					//Get a random dest VM
					uint32_t vmIndex = 0;
					Ptr<DcVm> dstVM;
					do {
						vmIndex = m_uniformRand->GetInteger(0,
								tennants[i].GetN() - 1);
						dstVM = tennants[i].Get(vmIndex);
						dstVmID = dstVM->m_id;
					} while (dstVmID == vm->m_id);

					flow->m_dstVmID = dstVmID;
					flow->m_dstNodeID = dstVM->m_hostServer;
					dstVM->m_flowList.Add(flow);

					//Get a rate
					//rateValue = RandomVariable(UniformVariable(50, 1000)).GetInteger();
					rateValue = m_uniformRand->GetInteger(1, 3);
					sprintf(rateStr, "%dkb/s", rateValue);
					rate = DataRate(rateStr);

					bool isTCP = false;
					flow->m_dataRate = rate;
					flow->SetUdpPacketSize(1450);  //size of UDP data: 1500-20-8
					flow->useTCP = isTCP;
					if (isTCP) {
						flow->m_srcPort = tcpPortSeed++;
						flow->m_dstPort = DC_TCP_DST_PORT;  //tcpPortSeed++;
					} else {
						flow->m_srcPort = udpPortSeed++;
						flow->m_dstPort = DC_UDP_DST_PORT;  //udpPortSeed++;
					}

					flowIndex++;
				}
			}
		}

		NS_ASSERT_MSG(totalAssignedVM == vmc->GetN(),
				"Error: Total VMs in all tenants is incorrect");

		//Policies  =================================
		generatePolicies();

	}

};

}

#endif /* SCENE_GENERATOR_H */

