#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include "ns3/dc-vm-hypervisor.h"

#include <cmath> 

#include "Bellman-Ford.h"

#include "ns3/dc-stats-tag.h"

//#define BW_BASE 1/1000000 
#define BW_BASE 1/1000  //kbps
#define TIME_PERIOD_LAMDA 20

NS_LOG_COMPONENT_DEFINE("DcVmHypervisorApplication");

namespace ns3 {

//commentted by cuilin
//using namespace stdcomb;

NS_OBJECT_ENSURE_REGISTERED(DcVmHypervisorApplication);

TypeId DcVmHypervisorApplication::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::DcVmHypervisorApplication").SetParent<
			Object>().AddConstructor<DcVmHypervisorApplication>();
	return tid;
}

DcVmHypervisorApplication::DcVmHypervisorApplication() {

	m_uniformRand = CreateObject<UniformRandomVariable>();

	m_linkcost[0] = 3; //e^1
	m_linkcost[1] = 7; //e^2
	m_linkcost[2] = 20; //e^3
	m_linkcost[3] = 54; //e^4

	/*
	 m_linkcost[0] = 1; //e^1
	 m_linkcost[1] = 2; //e^2
	 m_linkcost[2] = 4; //e^3
	 m_linkcost[3] = 8; //e^4
	 //*/

	m_usePolicy = true;
	isStarted = false;
	m_changePolicy = false;
	m_changeFlow = false;

	m_overloaded = false;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutexLog, NULL);
}

DcVmHypervisorApplication::~DcVmHypervisorApplication() {
	// TODO Auto-generated destructor stub
}

void DcVmHypervisorApplication::StopApplication() {

	isStarted = false;
}

void DcVmHypervisorApplication::FlowRxTrace(std::string context,
		Ptr<const Packet> p, const Address &address) {
	Time currentTime = Simulator::Now();
	DcStatsTag statsTag;

	p->PeekPacketTag(statsTag);
	//std::cout<<"RECEIVE: ======== At the receiver: "<<GetNode()->GetId()<<"\n";
	//statsTag.Print( std::cout);
	uint8_t timeForwards = statsTag.GetNumForward();
	Time tx = statsTag.GetTimestamp();
	uint32_t flowID = statsTag.GetFlowID();

	//uint32_t numNodes = NodeList::GetNNodes();
	//Ptr<Application> app =	NodeList::GetNode(numNodes-1)->GetApplication (0);
	//NS_ASSERT (app != 0);
	//Ptr<DcVmHypervisorApplication> hvapp = app->GetObject<DcVmHypervisorApplication> ();

	Ptr<DcFlowApplication> flow = fc->Get(flowID);

	flow->m_stats.timesForwarded += (timeForwards + 1);
	flow->m_stats.rxPackets++;
	flow->m_stats.rxBytes += p->GetSize();

	Time delay = currentTime - tx;
	flow->m_stats.delaySum += delay;

	if (flow->m_stats.rxPackets <= 1)
		flow->m_stats.lastDelay = delay;
	else {
		Time jitter = delay - flow->m_stats.lastDelay;
		flow->m_stats.jitterSum += jitter;
	}
	//flow->printFlow( std::cout);
	return;
}

void DcVmHypervisorApplication::StartApplication() {
	isStarted = true;

	//For measurement: only one node need to do
	if (fattree->HostNodes().Get(0)->GetId() == GetNode()->GetId()) {
		Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
				MakeCallback(&DcVmHypervisorApplication::FlowRxTrace, this));
		std::cout << "HOOK to the $ns3::PacketSink/Rx\n";
	}

	for (DcVmContainer::Iterator it = m_hostedVMs.Begin();
			it != m_hostedVMs.End(); it++) {
		Ptr<DcVm> vm = *it;
		NS_ASSERT_MSG(vm->m_hostServer == GetNode()->GetId(),
				"VM hostServer is wrong, VM id="<<vm->m_id<<", hostedServer="<<vm->m_hostServer<<", current Node="<<GetNode()->GetId());

		vm->startVM(false); //start a VM and the flows from this VM
	}

	//NS_LOG_INFO("Node "<<GetNode()->GetId()<<": HyperVisor started");
	std::cout << "Node " << GetNode()->GetId()
			<< ": HyperVisor started, isPolicy=" << m_usePolicy << std::endl;

	//On the first host, perform as central controller
	if (fattree->HostNodes().Get(0)->GetId() == GetNode()->GetId()) {
		if (m_usePolicy) //for PolicyConsolidation
		{
			m_toBeCheckedVMs.Add(*vmc); //initially, all VMs need to be checked
			//ScheduleNextPolicyCheck (MilliSeconds(50));
			ScheduleNextPolicyCheck(Seconds(1));
			//Ptr<DcMiddleBox> newMB = mbc->Get(10);
			//std::cout<<"CHECK: MB id=10:, capacityRemain="<<newMB->m_capacityRemain<<std::endl;			
		} else  //not policy-aware, e.g., S-CORE
		{
			//ScheduleSCORE(MilliSeconds(50), 0);
			ScheduleSCORE(Seconds(1), 0);
		}
	}

//return;
	//For measurement: only one node need to do
	if (fattree->HostNodes().Get(0)->GetId() == GetNode()->GetId()) {
		//Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", 
		//					MakeCallback (&DcVmHypervisorApplication::FlowRxTrace, this));

		SchedulePeriodLog();
	}

	return;

	//ScheduleNextCalc (Seconds(1));
	ScheduleNextCalc(MilliSeconds(1));

	//ScheduleVmCount ();

	//the first host node responsible for changing the policies
	if (m_changePolicy && //false && //m_usePolicy &&
			fattree->HostNodes().Get(0)->GetId() == GetNode()->GetId()) {
		Time nextTime(Seconds(50));
		Simulator::Schedule(nextTime,
				&DcVmHypervisorApplication::globalChangePolicy, this, 0, false);

		Time nextTime1(Seconds(100));
		Simulator::Schedule(nextTime1,
				&DcVmHypervisorApplication::globalChangePolicy, this, 0, true);

		Time nextTime2(Seconds(150));
		Simulator::Schedule(nextTime2,
				&DcVmHypervisorApplication::globalChangePolicy, this, 1, true);

		std::cout << "SETUP for policy changing" << std::endl;
	}

	//the first host node responsible for changing the flows
	if (m_changeFlow && //false && //m_usePolicy &&
			fattree->HostNodes().Get(0)->GetId() == GetNode()->GetId()) {
		Time nextTime2(Seconds(50));
		Simulator::Schedule(nextTime2,
				&DcVmHypervisorApplication::globalChangeFlow, this, 0, false);

		Time nextTime3(Seconds(100));
		Simulator::Schedule(nextTime3,
				&DcVmHypervisorApplication::globalChangeFlow, this, 0, true);

		Time nextTime4(Seconds(150));
		Simulator::Schedule(nextTime4,
				&DcVmHypervisorApplication::globalChangeFlow, this, 1, true);
		std::cout << "SETUP for policy changing" << std::endl;
	}
}

//ONLY ON FIRST HOST, work as a SDN Controller
void DcVmHypervisorApplication::ScheduleNextPolicyCheck(Time nextTime) {
	if (m_policyCheckEvent.IsRunning()) {
		m_policyCheckEvent.Cancel();
	}

	//Time nextTime (MilliSeconds(interval));
	m_policyCheckEvent = Simulator::Schedule(nextTime,
			&DcVmHypervisorApplication::policyCheck, this);
}
void DcVmHypervisorApplication::policyCheck() {
	DcVmContainer groupVMs;
	//get next isolated group
	groupVMs = getNextIsolatedGroup();
	std::cout << "In PolicyCheck: get next isolated group:";
	groupVMs.print(std::cout);

	if (groupVMs.GetN() >= 1) {
		//Preference Matrix: server-vm
		//std::map<uint32_t, PreferenceList> preferenceMatrix; //<hostID, preferenceList>
		PreferenceMatrix preferenceMatrixServer;

		struct timeval start, phase1, phase2;
		gettimeofday(&start, NULL);

		//Phase I: calc SPF for policy migration
		policyPhase_I(groupVMs, preferenceMatrixServer);

		gettimeofday(&phase1, NULL);

		//std::cout <<"Preference Matrix for all servers:\n";
		//preferenceMatrix.Print();

		std::cout << "Phase I complete, start Phase II..." << std::endl;

		//Phase II: matching for VM migration
		policyPhase_II(groupVMs, preferenceMatrixServer);

		gettimeofday(&phase2, NULL);

		LogAlgPerformance(start, phase1, phase2);

		std::cout << "Phase II complete" << std::endl;
	}

	//Schedule next time
	if (m_toBeCheckedVMs.GetN() > 0) {
		//ScheduleNextPolicyCheck (MilliSeconds(1));
		//ScheduleNextPolicyCheck (MicroSeconds(50));
		ScheduleNextPolicyCheck(MicroSeconds(1));
		std::cout << "More VMs to be checked, set next timer..." << std::endl;
	} else {
		//check and log policy violation
		LogPolicyViolation();
	}
}

DcVmContainer DcVmHypervisorApplication::getNextIsolatedGroup() {
	DcVmContainer group;
	DcFlowContainer flowGroup;

	std::cout << "getNextIsolatedGroup()\n";

	if (m_toBeCheckedVMs.GetN() == 0) //All VMs have been policyChecked
			{
		return group;
	}

	//Otherwise, add the first VM in the unchecked VMs
	Ptr<DcVm> vm = m_toBeCheckedVMs.Get(0);
	m_toBeCheckedVMs.Remove(vm);
	group.Add(vm);
	flowGroup.Add(vm->m_flowList);

	Ptr<DcVm> otherVm;

	while (flowGroup.GetN() > 0) {
		Ptr<DcFlowApplication> flow = flowGroup.Get(0);
		if (!group.IsVmExist(flow->m_srcVmID)) // flow.srcVmID is not in the group
				{
			NS_ASSERT_MSG(m_toBeCheckedVMs.IsVmExist(flow->m_srcVmID),
					"VM is neither in checked or unchecked list. VM id="<<flow->m_srcVmID);
			otherVm = *(m_toBeCheckedVMs.GetVmByID(flow->m_srcVmID));
		} else if (!group.IsVmExist(flow->m_dstVmID)) {
			NS_ASSERT_MSG(m_toBeCheckedVMs.IsVmExist(flow->m_dstVmID),
					"VM is neither in checked or unchecked list. VM id="<<flow->m_srcVmID);
			otherVm = *(m_toBeCheckedVMs.GetVmByID(flow->m_dstVmID));
		} else {
			flowGroup.Remove(flow);
			continue; //both src and dst of the flow is in the group, go to next flow
		}

		//if either src or dst of the flow is NOT in the group, add to the group
		m_toBeCheckedVMs.Remove(otherVm);
		group.Add(otherVm);
		flowGroup.Add(otherVm->m_flowList);
		flowGroup.Remove(flow);

		//std::cout<<"FlowGroup size="<<flowGroup.GetN()<<std::endl;
	}

	//m_checkedVMs.Add(group); //add the group to the checkedVMs
	return group;
}

void DcVmHypervisorApplication::policyPhase_I(DcVmContainer groupVMs,
		PreferenceMatrix &preferenceMatrix) {
	DcFlowContainer flowGroup;

	std::vector<uint32_t> mappingGraph; //map from vm/server/mb new id(in the graph) to original id

	//Get all related flows
	for (uint32_t i = 0; i < groupVMs.GetN(); i++) {
		Ptr<DcVm> vm = groupVMs.Get(i);
		flowGroup.Add(vm->m_flowList);
	}

	//===================================================
	//for each flow, construct cost matrix, calc SPF
	for (uint32_t fi = 0; fi < flowGroup.GetN(); fi++) {
		Ptr<DcFlowApplication> flow = flowGroup.Get(fi);
		PolicyItemIterator itPolicy = policies->getPolicyByFlow(flow);
		Ptr<DcVm> srcVM = *(vmc->GetVmByID(flow->m_srcVmID));
		Ptr<DcVm> dstVM = *(vmc->GetVmByID(flow->m_dstVmID));

		bool hasPolicy = false;
		if (policies->isOK(itPolicy))
			hasPolicy = true;

		if (flow->m_flowID == 25) {
			int a;
			a = 1;
		}

		//construct cost matrix
		Graph * graph;
		graph = ConstructCostMatrix(flow, mappingGraph);

		//-----------------------------------------------------------
		//calc SPF
		std::vector<int> path;
		//path={srcVM, srcServer, [MB list], dstServer, dstVM}
		//std::cout<<"mappingGraph.size = "<<mappingGraph.size()<<std::endl;
		BellmanFord(graph, 0, 1, path);

		printPath(path);

		NS_ASSERT_MSG(path.size() >= 4,
				"ERROR: incorrect shortest path length: len="<<path.size());

		//std::cout<<"mappingGraph size = "<<mappingGraph.size()<<" : ";
		//for(uint32_t j=0;j<mappingGraph.size();j++)
		//{
		//	std::cout<<mappingGraph[j]<<",";
		//}
		//std::cout<<std::endl;

		//---------------------------------------------------------
		//Check if any migration needed

		//Get new MB list
		if (hasPolicy) {
			MiddleBoxContainer newMBList;
			NS_ASSERT_MSG(path.size() == (uint32_t )itPolicy->len + 4,
					"ERROR: incorrect shortest path length: len="<<path.size()<<", 4+MblistLen="<<itPolicy->len+4);
			for (uint32_t j = 0; j < itPolicy->len; j++) {
				uint32_t newMBID = mappingGraph[path[2 + j]];
				Ptr<DcMiddleBox> mb = mbc->Get(newMBID);
				//std::cout<<"newMB id="<<newMBID<<", type="<<mb->m_type<<"itPolicy->Seq[j]="<<itPolicy->Seq[j]<<std::endl;
				NS_ASSERT_MSG(mb != 0 && mb->m_type == itPolicy->Seq[j],
						"ERROR: get a wrong MB: either no such MB or the type is incorrect");
				//std::cout<<j<<" MB: id="<<mb->m_id;
				newMBList.Add(mb);
			}
			//std::cout<<"Old MB list: ";
			//itPolicy->mbList.print();

			//std::cout<<"New MB list: ";
			//newMBList.print();

			//Perform Policy Migration
			for (uint32_t j = 0; j < itPolicy->len; j++) {
				if (itPolicy->mbList.Get(j)->m_id != newMBList.Get(j)->m_id) {
					//for logging
					int64_t utility = CalcUtilityOfPolicyMigration(itPolicy, j,
							newMBList.Get(j)->m_id, flow);
					LogPolicyMigration(itPolicy->id, flow->m_flowID, j,
							itPolicy->mbList.Get(j)->m_id,
							newMBList.Get(j)->m_id, utility);

					//itPolicy->mbList.setMB(j, newMBList.Get(j));
					Ptr<DcMiddleBox> newMB = newMBList.Get(j);
					//std::cout<<"CHECK: MB id"<<newMB->m_id<<":, capacityRemain="<<newMB->m_capacityRemain<<std::endl;		
					policies->migratePolicy(itPolicy, j, newMB, *fc);
					NS_ASSERT_MSG(newMB->m_capacityRemain >= 0,
							"Error: MB capacity error, MB id="<<newMB->m_id<<", capacityRemain="<<newMB->m_capacityRemain);
				}
			}
			policies->printPolicy(itPolicy);
		}

		//Get new srcServer and dstServer: path[1], path[n-2]
		uint32_t newSrcServerID = mappingGraph[path[1]];
		uint32_t newDstServerID = mappingGraph[path[path.size() - 2]];

		//std::cout<<"Old flow: from server "<<srcVM->m_hostServer<<" to server "<<dstVM->m_hostServer<<std::endl;
		//std::cout<<"New flow: from server "<<newSrcServerID<<" to server "<<newDstServerID<<std::endl;

		//Set preference matrix for server-vm
		PreferenceList * plist;
		PreferenceListItemIterator itItem;

		plist = preferenceMatrix.GetPrefereceList(newSrcServerID);//(PreferenceList) preferenceMatrix[newSrcServerID];
		itItem = plist->GetItem(srcVM->m_id);
		itItem->value += 1;

		//std::cout<<"Update preference matrix for srcVM:\n";
		//plist->Print();

		plist = preferenceMatrix.GetPrefereceList(newDstServerID);//(PreferenceList) preferenceMatrix[newDstServerID];
		itItem = plist->GetItem(dstVM->m_id);
		itItem->value += 1;

		//std::cout<<"Update preference matrix for dstVM:\n";
		//plist->Print();

		//preferenceMatrix.Print();
		std::cout << "Complete for on flow: " << flow->m_flowID << std::endl;
	}
}

void DcVmHypervisorApplication::policyPhase_II(DcVmContainer groupVMs,
		PreferenceMatrix &preferenceMatrixServer) {
	VmServerMatching matching;

	//uint32_t numVMs = groupVMs.GetN();

	//preferenceMatrixServer.Print();
	ConstructingPreferenceList_Server2Vm(preferenceMatrixServer, groupVMs);
	//preferenceMatrixServer.Print();

	//Constructing the preference list for all VMs, it is equal to using blacklist
	//in descending order by utility (utility>=0), each VM has at least one server in the list, i.e., the current hostedServer
	PreferenceMatrix preferenceMatrixVm;
	preferenceMatrixVm = ConstructingPreferenceList_Vm2Server(groupVMs);
	//preferenceMatrixVm.Print();

	DcVmContainer unProcessedVMs;
	unProcessedVMs.Add(groupVMs);
	while (unProcessedVMs.GetN() > 0) {
		Ptr<DcVm> vm = unProcessedVMs.Get(0);
		unProcessedVMs.Remove(vm);

		//Get the Server with largest utility
		PreferenceList *listVm = preferenceMatrixVm.GetPrefereceList(vm->m_id);
		NS_ASSERT_MSG(listVm->Size() > 0,
				"ERROR: preference list for vm is empty! vmID="<<vm->m_id);
		PrefrerenceListItem item = listVm->PopItem();

		uint32_t serverID = item.id;
		NS_ASSERT_MSG(fattree->IsHost(serverID),
				"ERROR: item in the preference list is NOT a host!");

		matching.Add(vm->m_id, serverID);

		if (!checkServerCapacity(matching, serverID, groupVMs)) {
			uint32_t lastVmID;
			do {
				lastVmID = getLastMatchedVmForServer(matching, serverID,
						preferenceMatrixServer);
				matching.Erase(lastVmID);

				Ptr<DcVm> vmTemp = vmc->Get(lastVmID);
				unProcessedVMs.Add(vmTemp);
			} while (!checkServerCapacity(matching, serverID, groupVMs));

			//remove serverID from all these VM's preference 
			PreferenceList *listServer =
					preferenceMatrixServer.GetPrefereceList(serverID);
			for (PreferenceListItemIterator itItem = listServer->Get(lastVmID);
					itItem != listServer->End(); itItem++) {
				uint32_t vmIdTemp = itItem->id;
				PreferenceList *listVmTemp =
						preferenceMatrixVm.GetPrefereceList(vmIdTemp);
				listVmTemp->Erase(serverID);
			}
			//remove all vm after lastVmId in server's preference list
			PreferenceListItemIterator itItem = listServer->Get(lastVmID);
			listServer->Erase(itItem, listServer->End());
		}
	}
	CheckVmMigration(matching);
}

Graph * DcVmHypervisorApplication::ConstructCostMatrix(
		Ptr<DcFlowApplication> flow, std::vector<uint32_t> & mappingGraph) {
	//Ptr<DcFlowApplication> flow = flowGroup.Get(fi);
	PolicyItemIterator itPolicy = policies->getPolicyByFlow(flow);
	Ptr<DcVm> srcVM = *(vmc->GetVmByID(flow->m_srcVmID));
	Ptr<DcVm> dstVM = *(vmc->GetVmByID(flow->m_dstVmID));

	bool hasPolicy = false;

	std::vector<uint32_t> numGroup;

	//-------------------------------------------------------------
	//get total nodes in the cost matrix
	int totalNode = 2; //src and dst
	int totalEdge = 0;
	numGroup.push_back(2); //first group: src and dst

	NodeContainer freeServerSrc = getAvailableServer(srcVM);
	int numFreeServerSrc = freeServerSrc.GetN();

	totalNode += numFreeServerSrc;
	totalEdge += numFreeServerSrc; //srcVM to each srcServer
	numGroup.push_back(numFreeServerSrc); //second group: all servers for srcVM

	if (policies->isOK(itPolicy)) {
		hasPolicy = true;
		for (uint32_t j = 0; j < itPolicy->len; j++) {
			//MiddleboxType type = itPolicy->Seq[j];
			Ptr<DcMiddleBox> mb = itPolicy->mbList.Get(j);
			MiddleBoxContainer freeMB = getAvailableMB(mb, flow);
			int numMB = freeMB.GetN();
			totalNode += numMB;

			//std::cout<<"MBs with type="<<mb->m_type<<":";
			//freeMB.print();

			totalEdge += numMB * numGroup[numGroup.size() - 1]; //each srcServer to each MB, or each previous type MB to each current type MB
			numGroup.push_back(numMB); //a new group: all possible MBs for mbtype
		}
	}

	NodeContainer freeServerDst = getAvailableServer(dstVM);
	int numFreeServerDst = freeServerDst.GetN();

	totalNode += numFreeServerDst;
	totalEdge += numFreeServerDst * numGroup[numGroup.size() - 1]; //each previous type MB to each current dstServer
	numGroup.push_back(numFreeServerDst); //a new group: all possible MBs for mbtype

	totalEdge += numFreeServerDst; //each dstServer to dstVM

	//-------------------------------------------------------
	//Construct graph
	struct Graph* graph = createGraph(totalNode, totalEdge);
	std::cout << "Creating Graph, numNode=" << totalNode << ", numEdge="
			<< totalEdge << std::endl;

	int groupIndex = 0;
	int edgeIndex = 0;
	uint32_t vertexIndex = 0;

	//group 1: srcVm and dstVM
	mappingGraph.clear();
	mappingGraph.push_back(0);	//srcVM, 0
	mappingGraph.push_back(1);	//dstVM, 1
	vertexIndex = 1;
	edgeIndex = -1;

	//group 2: srcServer
	groupIndex = 1;	// 2nd group is srcServers
	//add edge from srcVM to srcServers
	for (uint32_t si = 0; si < numGroup[groupIndex]; si++) {
		Ptr<Node> srcNode = freeServerSrc.Get(si);
		vertexIndex++;
		int vertexID = vertexIndex;

		mappingGraph.push_back(srcNode->GetId());

		edgeIndex++;
		graph->edge[edgeIndex].src = 0;  //src is the srcVM
		graph->edge[edgeIndex].dest = vertexID;

		//cost = migration cost
		graph->edge[edgeIndex].weight = srcVM->CalcMigrationCost(
				srcNode->GetId());
	}
	uint32_t lenBeforeStart = 2;
	uint32_t lenBeforeEnd = 2 + numGroup[groupIndex] - 1;

	//group 3--...: for MBs if any
	if (hasPolicy) {
		//add each tier of MBs
		for (uint32_t j = 0; j < itPolicy->len; j++) {
			groupIndex++;  //from 3rd group, MBs

			//MiddleboxType type = itPolicy->Seq[j];
			Ptr<DcMiddleBox> mb = itPolicy->mbList.Get(j);
			MiddleBoxContainer freeMB = getAvailableMB(mb, flow);

			NS_ASSERT_MSG(numGroup[groupIndex] == freeMB.GetN(),
					"ERROR: MB number is not equal");

			for (uint32_t mi = 0; mi < numGroup[groupIndex]; mi++) {
				Ptr<DcMiddleBox> mbCurrent = freeMB.Get(mi);
				uint32_t nodeCurrent = mbCurrent->m_nodeID;
				uint32_t nodePrevious;

				mappingGraph.push_back(mbCurrent->m_id);
				//std::cout<<"numGroup[groupIndex]="<<numGroup[groupIndex]<<", Add MB "<<mbCurrent->m_id<<std::endl;

				vertexIndex++;
				NS_ASSERT_MSG((mappingGraph.size() - 1) == vertexIndex,
						"ERROR: vertex id error when creating graph");

				for (uint32_t pi = lenBeforeStart; pi <= lenBeforeEnd; pi++) //previous index in the mappingGraph
						{
					if (j == 0) //first MB in the seq, connected from srcServer
							{
						nodePrevious = mappingGraph[pi];
					} else //previous is MB
					{
						Ptr<DcMiddleBox> mb = mbc->Get(mappingGraph[pi]);
						nodePrevious = mb->m_nodeID;
					}

					//Check connectivity
					if (!(fattree->checkConnectivity(nodePrevious, nodeCurrent))) {
						//if unreachable, continue to next
						totalEdge--;
						continue;
					}

					edgeIndex++;

					graph->edge[edgeIndex].src = pi; //previous vertex id in the graph
					graph->edge[edgeIndex].dest = vertexIndex;

					//cost = migration cost
					graph->edge[edgeIndex].weight = CalcPathCost_Node2Node(
							flow->m_dataRate, nodePrevious, nodeCurrent);
				}

			}

			lenBeforeStart = lenBeforeEnd + 1;
			lenBeforeEnd = lenBeforeStart + numGroup[groupIndex] - 1;
		}

		//add last tier of MB to dstServer
		groupIndex++;
		NS_ASSERT_MSG(numGroup[groupIndex] == freeServerDst.GetN(),
				"ERROR: MB number is not equal");

		for (uint32_t si = 0; si < freeServerDst.GetN(); si++) {
			Ptr<Node> dstNode = freeServerDst.Get(si);
			uint32_t nodePrevious;

			mappingGraph.push_back(dstNode->GetId());

			vertexIndex++;
			NS_ASSERT_MSG((mappingGraph.size() - 1) == vertexIndex,
					"ERROR: vertex id error when creating graph");

			for (uint32_t pi = lenBeforeStart; pi <= lenBeforeEnd; pi++) //previous index in the mappingGraph
					{
				Ptr<DcMiddleBox> mb = mbc->Get(mappingGraph[pi]);
				nodePrevious = mb->m_nodeID;

				//Check connectivity
				if (!(fattree->checkConnectivity(nodePrevious, dstNode->GetId()))) {
					//if unreachable, continue to next
					totalEdge--;
					continue;
				}

				edgeIndex++;

				graph->edge[edgeIndex].src = pi; //previous vertex id in the graph
				graph->edge[edgeIndex].dest = vertexIndex;

				//cost = migration cost
				graph->edge[edgeIndex].weight = CalcPathCost_Node2Node(
						flow->m_dataRate, nodePrevious, dstNode->GetId());
			}
		}
		lenBeforeStart = lenBeforeEnd + 1;
		lenBeforeEnd = lenBeforeStart + numGroup[groupIndex] - 1;
	} else  //no policy, srcServer to dstServer
	{
		//lenBeforeStart = 2 + 1; //start of srcServer in mappingGraph
		//lenBeforeEnd = lenBeforeStart + freeServerSrc.GetN()- 1;

		//NS_ASSERT_MSG(numGroup[2]==freeServerSrc.GetN(),"ERROR: Server number is not equal");
		NS_ASSERT_MSG(numGroup[1] == freeServerSrc.GetN(),
				"ERROR: Server number is not equal");

		for (uint32_t si = 0; si < freeServerDst.GetN(); si++) {
			Ptr<Node> dstNode = freeServerDst.Get(si);
			uint32_t nodeSrc;

			mappingGraph.push_back(dstNode->GetId());

			vertexIndex++;
			NS_ASSERT_MSG((mappingGraph.size() - 1) == vertexIndex,
					"ERROR: vertex id error when creating graph");

			for (uint32_t pi = lenBeforeStart; pi <= lenBeforeEnd; pi++) //previous index in the mappingGraph
					{
				nodeSrc = mappingGraph[pi];

				NS_ASSERT_MSG(
						nodeSrc
								== freeServerSrc.Get(pi - lenBeforeStart)->GetId(),
						"ERROR: vertex id (srcServer) error when creating graph");

				edgeIndex++;
				graph->edge[edgeIndex].src = pi; //previous vertex id in the graph
				graph->edge[edgeIndex].dest = vertexIndex;

				//cost = migration cost
				graph->edge[edgeIndex].weight = CalcPathCost_Node2Node(
						flow->m_dataRate, nodeSrc, dstNode->GetId());
			}
		}
		lenBeforeStart = lenBeforeEnd + 1;
		//lenBeforeEnd = lenBeforeStart + numGroup[1] - 1;	
		lenBeforeEnd = lenBeforeStart + freeServerDst.GetN() - 1;
	}

	//add dstServers to dstVM
	//add edge from dstServers to dstVm
	for (uint32_t pi = lenBeforeStart; pi <= lenBeforeEnd; pi++) {
		uint32_t dstNodeID = mappingGraph[pi];
		NS_ASSERT_MSG(
				freeServerDst.Get(pi - lenBeforeStart)->GetId() == dstNodeID,
				"ERROR: dstNodeID is not equal");

		edgeIndex++;
		graph->edge[edgeIndex].src = pi;
		graph->edge[edgeIndex].dest = 1; //dst is the dstVM

		//cost = migration cost
		graph->edge[edgeIndex].weight = dstVM->CalcMigrationCost(dstNodeID);

		//std::cout<<"add Edge "<<edgeIndex<<": src="<<pi<<", dst=1, weight="<<graph->edge[edgeIndex].weight<<std::endl;
	}

	//since the number of edge might be reduced due to unreachablity, update totalEdge
	graph->E = totalEdge;

	//printGraph(graph);
	return graph;
}

void DcVmHypervisorApplication::CheckVmMigration(VmServerMatching & matching) {
	for (MatchingIterator it = matching.Begin(); it != matching.End(); it++) {
		uint32_t vmID = it->first;
		uint32_t serverID = it->second;

		Ptr<DcVm> vm = vmc->Get(vmID);

		if (vm->m_hostServer != serverID) {
			//for logging
			int64_t utility = CalcUtilityOfMigrationToServer(vm, serverID);
			LogVmMigration(vm->m_id, vm->m_hostServer, serverID, utility);

			std::cout << "MIGRATION: VM " << vm->m_id
					<< " migrated from server " << vm->m_hostServer
					<< " to server " << serverID << std::endl;

			vm->stopVM();
			vm->MigrateToServer(serverID, policies, true);
			vm->startVM(true);	//start all flow related to this VM		
		}
	}
}

void DcVmHypervisorApplication::ConstructingPreferenceList_Server2Vm(
		PreferenceMatrix & preferenceMatrixServer, DcVmContainer groupVMs) {
	//Sort preference matrix
	std::cout << "Sorting all preference list... " << std::endl;
	preferenceMatrixServer.SortEachList(true); //in descending order	
	//preferenceMatrixServer.Print();

	//get a ordered list of all groupVMs
	PreferenceList listVMs;
	for (uint32_t i = 0; i < groupVMs.GetN(); i++) {
		Ptr<DcVm> vm = groupVMs.Get(i);
		PreferenceListItemIterator item = listVMs.GetItem(vm->m_id);
		item->value = vm->m_cpu + vm->m_memory; //TODO:
	}
	listVMs.Sort(false);

	for (uint32_t i = 0; i < fattree->HostNodes().GetN(); i++) {
		Ptr<Node> host = fattree->HostNodes().Get(i);
		PreferenceList *list = preferenceMatrixServer.GetPrefereceList(
				host->GetId());

		//if(list->Size()==0)
		//{
		//	*list = listVMs;
		//	continue;
		//}

		for (PreferenceListItemIterator it = listVMs.Begin();
				it != listVMs.End(); it++) {
			//uint32_t vmIdTemp = it->id;
			if (!(list->IsExist(it->id))) //not in the preference list, add at the end
			{
				list->AddAtEnd(it->id, it->value);
			}
		}
	}
}

PreferenceMatrix DcVmHypervisorApplication::ConstructingPreferenceList_Vm2Server(
		DcVmContainer groupVMs) {
	uint32_t numVMs = groupVMs.GetN();
	int64_t utility = 0;

	//Constructing the preference list for all VMs, it is equal to using blacklist
	PreferenceMatrix preferenceMatrixVm;
	for (uint32_t i = 0; i < numVMs; i++) {
		Ptr<DcVm> vm = groupVMs.Get(i);
		NodeContainer freeServer = getAvailableServer(vm);
		PreferenceList *list = preferenceMatrixVm.GetPrefereceList(vm->m_id);

		//add all available servers to the preference list
		for (uint32_t j = 0; j < freeServer.GetN(); j++) {
			Ptr<Node> server = freeServer.Get(j);

			//check connectivity
			if (!IsOkVmOnServer(vm, server->GetId())) {
				continue;
			}

			utility = CalcUtilityOfMigrationToServer(vm, server->GetId());
			if (utility >= 0) //Only consier utility>0
					{
				PreferenceListItemIterator item = list->GetItem(
						server->GetId());
				//get the utility of migration			
				item->value = utility;
			}

		}
	}
	preferenceMatrixVm.SortEachList(true); //in descending order	

	return preferenceMatrixVm;
}

bool DcVmHypervisorApplication::checkServerCapacity(VmServerMatching & matching,
		uint32_t serverID, DcVmContainer groupVMs) {
	uint32_t totalCPU = 0;
	uint32_t totalMemory = 0;

	uint32_t serverRemainCPU = 0;
	uint32_t serverRemainMemory = 0;

	Ptr<DcVmHypervisorApplication> hvapp;
	hvapp = getHyperVisor(serverID);
	serverRemainCPU = hvapp->m_totalCPU - hvapp->m_useCPU;
	serverRemainMemory = hvapp->m_totalMemory - hvapp->m_useMemory;

	//Get totoal resources of server, excluding VMs to be matched in this phase
	for (uint32_t i = 0; i < groupVMs.GetN(); i++) {
		Ptr<DcVm> vm = groupVMs.Get(i);
		if (hvapp->m_hostedVMs.IsVmExist(vm->m_id)) //in the groupVMs,
				{
			serverRemainCPU += vm->m_cpu;
			serverRemainMemory += vm->m_memory;
		}
	}

	//get the total resources of current matched VMs
	std::vector<uint32_t> listVmIDs = matching.GetMatchedVmOnServer(serverID);
	for (uint32_t i = 0; i < listVmIDs.size(); i++) {
		uint32_t vmID = listVmIDs[i];
		Ptr<DcVm> vm = vmc->Get(vmID);
		totalCPU += vm->m_cpu;
		totalMemory += vm->m_memory;
	}

	if (serverRemainCPU >= totalCPU && serverRemainMemory >= totalMemory)
		return true;
	else
		return false;
}

uint32_t DcVmHypervisorApplication::getLastMatchedVmForServer(
		VmServerMatching & matching, uint32_t serverID,
		PreferenceMatrix & preferenceMatrixServer) {
	PreferenceList *list = preferenceMatrixServer.GetPrefereceList(serverID);

	//for each vm in list(from back to front), if find one in listVmIDs, return;
	std::vector<uint32_t> listVmIDs = matching.GetMatchedVmOnServer(serverID);
	for (PreferenceListItemReverseIterator it = list->rBegin();
			it != list->rEnd(); it++) {
		uint32_t vmID = it->id;
		std::vector<uint32_t>::iterator itTemp = std::find(listVmIDs.begin(),
				listVmIDs.end(), vmID);
		if (itTemp != listVmIDs.end()) //found one
				{
			return vmID;
		}
	}

	NS_ABORT_MSG(
			"Can't find a matched VM in the preference list, serverID="<<serverID);
	return MAX_NODE_ID;
}

bool DcVmHypervisorApplication::IsOkVmOnServer(Ptr<DcVm> vm,
		uint32_t serverID) {
	bool ok = true;
	uint32_t node2;
	Ptr<DcMiddleBox> mb;

	for (uint32_t i = 0; i < vm->m_flowList.GetN(); i++) {
		Ptr<DcFlowApplication> flow = vm->m_flowList.Get(i);
		PolicyItemIterator itPolicy = policies->getPolicyByFlow(flow);

		if (policies->isOK(itPolicy)) {
			if (flow->m_srcVmID == vm->m_id) //is src, get the first MB
					{
				mb = itPolicy->mbList.Get(0);
				node2 = mb->m_nodeID;
			} else //is dst, get the last MB
			{
				mb = itPolicy->mbList.Get(itPolicy->len - 1);
				node2 = mb->m_nodeID;
			}

			if (!(fattree->checkConnectivity(serverID, node2))) {
				return false;
			}
		}
	}
	return ok;
}

int64_t DcVmHypervisorApplication::CalcUtilityOfMigrationToServer(Ptr<DcVm> vm,
		uint32_t serverID) {
	uint64_t totalCostOld = 0;
	uint64_t totalCostNew = 0;
	int64_t utility = 0;

	//utility=0, if dst is current server
	if (vm->m_hostServer == serverID)
		return 0;

	totalCostOld = CalcCostofVmOnServer(vm, vm->m_hostServer);
	totalCostNew = CalcCostofVmOnServer(vm, serverID);

	//if (totalCostOld >= totalCostNew)
	utility = totalCostOld - totalCostNew - vm->CalcMigrationCost(serverID);
	//else
	//	utility = -1 * (totalCostNew - totalCostOld);

	//utility = utility ;

	//std::cout<<"Utilty of VM Migration: totalCostOld="<<totalCostOld<<", totalCostNew="<<totalCostNew
	//		<<", utility="<<utility<<std::endl;
	return utility;
}

int64_t DcVmHypervisorApplication::CalcUtilityOfPolicyMigration(
		PolicyItemIterator itPolicy, uint8_t seqIndex, uint32_t newMbID,
		Ptr<DcFlowApplication> flow) {
	uint64_t totalCostOld = 0;
	uint64_t totalCostNew = 0;
	int64_t utility = 0;

	//utility=0, no migration
	if (itPolicy->mbList.Get(seqIndex)->m_id == newMbID)
		return 0;

	Ptr<DcMiddleBox> mb = mbc->Get(itPolicy->mbList.Get(seqIndex)->m_id);
	Ptr<DcMiddleBox> mbNew = mbc->Get(newMbID);

	if (seqIndex == 0)	//first MB
			{
		Ptr<DcVm> vm1 = vmc->Get(flow->m_srcVmID);
		totalCostOld = CalcPathCost_Node2Node(flow->m_dataRate,
				vm1->m_hostServer, mb->m_nodeID);
		totalCostNew = CalcPathCost_Node2Node(flow->m_dataRate,
				vm1->m_hostServer, mbNew->m_nodeID);
	} else {
		Ptr<DcMiddleBox> mbPrev = mbc->Get(
				itPolicy->mbList.Get(seqIndex - 1)->m_id);
		totalCostOld = CalcPathCost_Node2Node(flow->m_dataRate,
				mbPrev->m_nodeID, mb->m_nodeID);
		totalCostNew = CalcPathCost_Node2Node(flow->m_dataRate,
				mbPrev->m_nodeID, mbNew->m_nodeID);
	}

	if (seqIndex == itPolicy->len - 1)	//last MB
			{
		Ptr<DcVm> vm2 = vmc->Get(flow->m_dstVmID);
		totalCostOld += CalcPathCost_Node2Node(flow->m_dataRate, mb->m_nodeID,
				vm2->m_hostServer);
		totalCostNew += CalcPathCost_Node2Node(flow->m_dataRate,
				mbNew->m_nodeID, vm2->m_hostServer);
	} else {
		Ptr<DcMiddleBox> mbNext = mbc->Get(
				itPolicy->mbList.Get(seqIndex + 1)->m_id);
		totalCostOld += CalcPathCost_Node2Node(flow->m_dataRate, mb->m_nodeID,
				mbNext->m_nodeID);
		totalCostNew += CalcPathCost_Node2Node(flow->m_dataRate,
				mbNew->m_nodeID, mbNext->m_nodeID);
	}

	utility = totalCostOld - totalCostNew;
	return utility;
}

uint64_t DcVmHypervisorApplication::CalcCostofVmOnServer(Ptr<DcVm> vm,
		uint32_t serverID) {
	uint64_t totalCost = 0;

	Ptr<DcVm> vm2;
	Ptr<DcMiddleBox> mb2;
	uint32_t vmID2;

	uint32_t nodeID2;

	for (uint32_t i = 0; i < vm->m_flowList.GetN(); i++) {
		Ptr<DcFlowApplication> flow = vm->m_flowList.Get(i);
		PolicyItemIterator itPolicy = policies->getPolicyByFlow(flow);

		if (policies->isOK(itPolicy)) {
			if (flow->m_srcVmID == vm->m_id)	//is src, get the first MB
				mb2 = itPolicy->mbList.Get(0);
			else
				//is dst, get the last MB
				mb2 = itPolicy->mbList.Get(itPolicy->len - 1);

			nodeID2 = mb2->m_nodeID;
		} else {
			if (flow->m_srcVmID == vm->m_id)
				vmID2 = flow->m_dstVmID;
			else
				vmID2 = flow->m_srcVmID;
			vm2 = vmc->Get(vmID2);
			nodeID2 = vm2->m_hostServer;
		}
		totalCost += CalcPathCost_Node2Node(flow->m_dataRate, serverID,
				nodeID2);
	}

	return totalCost;
}

//get availabe servers than can accept vm for migration, including the one it currently hosted on
NodeContainer DcVmHypervisorApplication::getAvailableServer(Ptr<DcVm> vm) {
	Ptr<Node> node;
	Ptr<DcVmHypervisorApplication> hvapp;
	NodeContainer freeHosts;
	NodeContainer allHosts = fattree->HostNodes();

	for (uint32_t i = 0; i < allHosts.GetN(); i++) {
		node = allHosts.Get(i);

		//the current server is included (with migration cost=0)
		if (node->GetId() == vm->m_hostServer) {
			freeHosts.Add(node);
			continue;
		}

		//for others, check the connectivity
		if (!(fattree->checkConnectivity(vm->m_hostServer, node->GetId()))) {
			continue;
		}

		//for others, check the availble resource
		hvapp = getHyperVisor(node->GetId());
		if ((hvapp->m_useCPU + vm->m_cpu <= hvapp->m_totalCPU)
				&& (hvapp->m_useMemory + vm->m_memory <= hvapp->m_totalMemory)) {
			freeHosts.Add(node);
			continue;
		}
	}
	return freeHosts;
}

MiddleBoxContainer DcVmHypervisorApplication::getAvailableMB(
		Ptr<DcMiddleBox> mb, Ptr<DcFlowApplication> flow) {
	MiddleBoxContainer freeMB;

	freeMB = mbc->getMiddleBoxByType(mb->m_type);

	for (uint32_t i = 0; i < freeMB.GetN(); i++) {
		Ptr<DcMiddleBox> m = freeMB.Get(i);

		//the current MB is included (with migration cost=0)
		if (m->m_id == mb->m_id)
			continue;

		//check avalilable capacity
		if (m->isFlowAcceptable(flow))
			continue;

		std::cout << "MB doesn't have enough space, mbID=" << m->m_id
				<< "remainCapaciy: " << m->m_capacityRemain << "Flow rate:"
				<< flow->m_dataRate.GetBitRate() / 1000 << std::endl;

		//Otherwise, remove
		freeMB.Remove(m);
		i--;
	}

	return freeMB;
}

uint64_t DcVmHypervisorApplication::CalcPathCost_Node2Node(DataRate rate,
		uint32_t nodeID1, uint32_t nodeID2) {
	uint64_t totalCost = 0;
	uint64_t lamda = 0;
	uint32_t sumWeight = 1;

	//computer link cost for this flow
	lamda = rate.GetBitRate() / 8 * BW_BASE; //using KB/s, otherwise, the number would be too large to use uint64_t
	//lamda = lamda*TIME_PERIOD_LAMDA;  //measured within a period

	sumWeight = CalcPathWeight_Node2Node(nodeID1, nodeID2);

	totalCost = lamda * sumWeight;

	//std::cout<<"Node "<<nodeID1<<" to Node "<<nodeID2<<" Cost: "<<totalCost<<", lamda="<<lamda<<"KB/s, total weight="<<sumWeight<<std::endl;

	return totalCost;
}

//Return the sum of all links weights on shortest path from any two nodes
uint32_t DcVmHypervisorApplication::CalcPathWeight_Node2Node(uint32_t nodeID1,
		uint32_t nodeID2) {
	uint32_t linkCost = 0;

	//zero if the same node
	if (nodeID1 == nodeID2) {
		return 0;
	}

	//Host - Host
	if (fattree->IsHost(nodeID1) && fattree->IsHost(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) {
			if (fattree->GetIDinPod(nodeID1) == fattree->GetIDinPod(nodeID2)) {	//host from the same pod and same edge: host-edge-host
				linkCost = m_linkcost[0] + m_linkcost[0];
			} else {//host from the same pod but different edge: host-edge-aggr-edge-host
				linkCost = m_linkcost[0] + m_linkcost[1] + m_linkcost[1]
						+ m_linkcost[0];
			}
		} else	//host from different pod: host-edge-aggr-core-aggr-edge-host
		{
			linkCost = m_linkcost[0] + m_linkcost[1] + m_linkcost[2]
					+ m_linkcost[2] + m_linkcost[1] + m_linkcost[0];
		}
	}

	//Host - Edge
	else if (fattree->IsHost(nodeID1) && fattree->IsEdge(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) {
			if (fattree->GetIDinPod(nodeID1) == fattree->GetIDinPod(nodeID2)) {	//from the same pod and same edge: host-edge
				linkCost = m_linkcost[0];
			} else {//from the same pod but different edge: host-edge-aggr-edge
				linkCost = m_linkcost[0] + m_linkcost[1] + m_linkcost[1];
			}
		} else	//from different pod: host-edge-aggr-core-aggr-edge
		{
			linkCost = m_linkcost[0] + m_linkcost[1] + m_linkcost[2]
					+ m_linkcost[2] + m_linkcost[1];
		}
	}

	//Host - Aggr
	else if (fattree->IsHost(nodeID1) && fattree->IsAggr(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) {	//from the same pod: host-edge-aggr
			linkCost = m_linkcost[0] + m_linkcost[1];
		} else	//from different pod: host-edge-aggr-core-aggr
		{
			linkCost = m_linkcost[0] + m_linkcost[1] + m_linkcost[2]
					+ m_linkcost[2];
		}
	}

	//Host -Core
	else if (fattree->IsHost(nodeID1) && fattree->IsCore(nodeID2)) { //host-edge-aggr-core
		linkCost = m_linkcost[0] + m_linkcost[1] + m_linkcost[2];
	}

	//Edge - Host
	else if (fattree->IsEdge(nodeID1) && fattree->IsHost(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) {
			if (fattree->GetIDinPod(nodeID1) == fattree->GetIDinPod(nodeID2)) { //from the same pod and same edge: edge-host
				linkCost = m_linkcost[0];
			} else { //from the same pod but different edge: edge-aggr-edge-host
				linkCost = m_linkcost[1] + m_linkcost[1] + m_linkcost[0];
			}
		} else //from different pod: edge-aggr-core-aggr-edge-host
		{
			linkCost = m_linkcost[1] + m_linkcost[2] + m_linkcost[2]
					+ m_linkcost[1] + m_linkcost[0];
		}
	}

	//Edge-Edge
	else if (fattree->IsEdge(nodeID1) && fattree->IsEdge(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) { //same pod: edge-aggr-edge
			linkCost = m_linkcost[1] + m_linkcost[1];
		} else { //different pod: edge-aggr-core-aggr-edge
			linkCost = m_linkcost[1] + m_linkcost[2] + m_linkcost[2]
					+ m_linkcost[1];
		}
	}

	//Edge-Aggr
	else if (fattree->IsEdge(nodeID1) && fattree->IsAggr(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) { //same pod: edge-aggr
			linkCost = m_linkcost[1];
		} else { //different pod: edge-aggr-core-aggr
			linkCost = m_linkcost[1] + m_linkcost[2] + m_linkcost[2];
		}
	}

	//Edge-Core
	else if (fattree->IsEdge(nodeID1) && fattree->IsCore(nodeID2)) { //edge-aggr-core
		linkCost = m_linkcost[1] + m_linkcost[2];
	}

	//Aggr - Host
	else if (fattree->IsAggr(nodeID1) && fattree->IsHost(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) { //from the same pod: aggr-edge-host
			linkCost = m_linkcost[1] + m_linkcost[0];
		} else //from different pod: aggr-core-aggr-edge-host
		{
			linkCost = m_linkcost[2] + m_linkcost[2] + m_linkcost[1]
					+ m_linkcost[0];
		}
	}

	//Aggr-Edge
	else if (fattree->IsAggr(nodeID1) && fattree->IsEdge(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) { //same pod: aggr-edge
			linkCost = m_linkcost[1];
		} else { //different pod: aggr-core-edge
			linkCost = m_linkcost[2] + m_linkcost[2] + m_linkcost[1];
		}
	}

	//Aggr-Aggr
	else if (fattree->IsAggr(nodeID1) && fattree->IsAggr(nodeID2)) {
		if (fattree->GetPodID(nodeID1) == fattree->GetPodID(nodeID2)) { //same pod: aggr-edge-aggr
			linkCost = m_linkcost[1] + m_linkcost[1];
		} else { //different pod
			if (fattree->GetIDinPod(nodeID1) == fattree->GetIDinPod(nodeID2)) { //have the same id in pod, connected to the same core group: aggr-core-aggr
				linkCost = m_linkcost[2] + m_linkcost[2];
			} else { //have different id in pod, connected to different core group: aggr-core-aggr-edge-aggr
				linkCost = m_linkcost[2] + m_linkcost[2] + m_linkcost[1]
						+ m_linkcost[1];
			}
		}
	}

	//Aggr-Core
	else if (fattree->IsAggr(nodeID1) && fattree->IsCore(nodeID2)) {
		if (fattree->GetIDinPod(nodeID1) == fattree->GetCoreGroupID(nodeID2)) { //have the same id in pod and core group id: aggr-core
			linkCost = m_linkcost[2];
		} else { //have different id in pod (and core group id), connected to different core group: aggr-edge-aggr-core
			linkCost = m_linkcost[1] + m_linkcost[1] + m_linkcost[2];
		}
	}

	//Core - Host
	else if (fattree->IsCore(nodeID1) && fattree->IsHost(nodeID2)) { //core-aggr-edge-host
		linkCost = m_linkcost[2] + m_linkcost[1] + m_linkcost[0];
	}

	//Core-Edge
	else if (fattree->IsCore(nodeID1) && fattree->IsEdge(nodeID2)) { //core-aggr-edge
		linkCost = m_linkcost[2] + m_linkcost[1];
	}

	//Core-Aggr
	else if (fattree->IsCore(nodeID1) && fattree->IsAggr(nodeID2)) {
		if (fattree->GetCoreGroupID(nodeID1) == fattree->GetIDinPod(nodeID2)) { //have the same id in pod and core group id: aggr-core
			linkCost = m_linkcost[2];
		} else { //have different id in pod (and core group id), connected to different core group: core-aggr-edge-aggr
			linkCost = m_linkcost[2] + m_linkcost[1] + m_linkcost[1];
		}
	}

	//Core-Core
	else if (fattree->IsCore(nodeID1) && fattree->IsCore(nodeID2)) {
		if (fattree->GetCoreGroupID(nodeID1)
				== fattree->GetCoreGroupID(nodeID2)) { //have the same core group id, connected to the same aggr: core-aggr-core
			linkCost = m_linkcost[2] + m_linkcost[2];
		} else { //have different core group id, connected to different Aggrs: core-aggr-edge-aggr-core
			linkCost = m_linkcost[2] + m_linkcost[1] + m_linkcost[1]
					+ m_linkcost[2];
		}
	} else
		NS_ABORT_MSG(
				"Node ID is wrong, nodeID1="<<nodeID1<<", nodeID2="<<nodeID2);

	return linkCost;
}

//ONLY ON FIRST HOST
void DcVmHypervisorApplication::ScheduleSCORE(Time nextTime,
		uint32_t vmID = 0) {
	if (m_scoreScheduleEvent.IsRunning()) {
		m_scoreScheduleEvent.Cancel();
	}

	m_scoreScheduleEvent = Simulator::Schedule(nextTime,
			&DcVmHypervisorApplication::ScoreCheck, this, vmID);
}
void DcVmHypervisorApplication::ScoreCheck(uint32_t vmID = 0) {
	Ptr<DcVm> vm = vmc->Get(vmID);
	Ptr<Node> nodeCurrent = NodeList::GetNode(vm->m_hostServer);

	NodeContainer hostNodes = fattree->HostNodes();

	Ptr<DcVmHypervisorApplication> hvapp;

	uint64_t costMin = 0;
	uint32_t serverMinID;

	uint64_t costTemp = 0;
	bool isFirst = true;
	// find a server with most cost reduction
	for (uint32_t i = 0; i < hostNodes.GetN(); i++) {
		Ptr<Node> nodeDst = hostNodes.Get(i);

		if (nodeCurrent->GetId() == nodeDst->GetId())
			continue;

		//for others, check the availble resource
		hvapp = getHyperVisor(nodeDst->GetId());
		if ((hvapp->m_useCPU + vm->m_cpu <= hvapp->m_totalCPU)
				&& (hvapp->m_useMemory + vm->m_memory <= hvapp->m_totalMemory)) {

			costTemp = CalcCostofVmOnServer_SCORE(vm, nodeDst->GetId());
			if (isFirst || costTemp < costMin) {
				costMin = costTemp;
				serverMinID = nodeDst->GetId();
				isFirst = false;
			}
		}
	}

	if (serverMinID == vm->m_hostServer)  //current server is the best
		return;

	uint64_t costOriginal = CalcCostofVmOnServer_SCORE(vm, vm->m_hostServer);
	uint64_t costMigration = vm->CalcMigrationCost(serverMinID);
	if (costOriginal - costMin > costMigration) //utility>0, perform migration
			{
		//for logging
		int64_t utility = CalcUtilityOfMigrationToServer(vm, serverMinID);
		LogVmMigration(vm->m_id, vm->m_hostServer, serverMinID, utility);

		std::cout << "MIGRATION: VM " << vm->m_id << " migrated from server "
				<< vm->m_hostServer << " to server " << serverMinID
				<< std::endl;
		vm->stopVM();
		vm->MigrateToServer(serverMinID, policies, false);
		vm->startVM(true); //start all flow related to this VM		
	}

	//pass the ticket to next Vm
	vmID++;
	if (vmID == vmc->GetN()) //All VMs have been checked
			{
		//check and log policy violation
		LogPolicyViolation();

		return;
	}

	//ScheduleSCORE(MilliSeconds(1), vmID);
	ScheduleSCORE(MicroSeconds(50), vmID);
}

uint64_t DcVmHypervisorApplication::CalcCostofVmOnServer_SCORE(Ptr<DcVm> vm,
		uint32_t serverID) {
	uint64_t totalCost = 0;

	Ptr<DcVm> vm2;
	uint32_t vmID2;

	uint32_t nodeID2;

	for (uint32_t i = 0; i < vm->m_flowList.GetN(); i++) {
		Ptr<DcFlowApplication> flow = vm->m_flowList.Get(i);

		if (flow->m_srcVmID == vm->m_id)
			vmID2 = flow->m_dstVmID;
		else
			vmID2 = flow->m_srcVmID;
		vm2 = vmc->Get(vmID2);
		nodeID2 = vm2->m_hostServer;

		totalCost += CalcPathCost_Node2Node(flow->m_dataRate, serverID,
				nodeID2);
	}

	return totalCost;
}

uint64_t DcVmHypervisorApplication::CalcCostofFlow(
		Ptr<DcFlowApplication> flow) {
	uint64_t totalCost = 0;

	PolicyItemIterator itPolicy = policies->getPolicyByFlow(flow);
	Ptr<DcVm> srcVM = *(vmc->GetVmByID(flow->m_srcVmID));
	Ptr<DcVm> dstVM = *(vmc->GetVmByID(flow->m_dstVmID));

	if (policies->isOK(itPolicy)) {
		//Ptr<DcMiddleBox> mb = mbc->Get(itPolicy->mbList.Get(0)->m_id); //first mb
		Ptr<DcMiddleBox> mb = itPolicy->mbList.Get(0); //first mb
		totalCost = CalcPathCost_Node2Node(flow->m_dataRate,
				srcVM->m_hostServer, mb->m_nodeID);

		for (uint32_t i = 0; i < (uint32_t) itPolicy->len - 1; i++) {
			//mb = mbc->Get(itPolicy->mbList.Get(i)->m_id);
			//Ptr<DcMiddleBox> mb2 = mbc->Get(itPolicy->mbList.Get(i+1)->m_id);
			mb = itPolicy->mbList.Get(i);
			Ptr<DcMiddleBox> mb2 = itPolicy->mbList.Get(i + 1);
			totalCost += CalcPathCost_Node2Node(flow->m_dataRate, mb->m_nodeID,
					mb2->m_nodeID);
		}

		//mb = mbc->Get(itPolicy->mbList.Get(itPolicy->len-1)->m_id); //last mb
		mb = itPolicy->mbList.Get(itPolicy->len - 1); //last mb
		totalCost += CalcPathCost_Node2Node(flow->m_dataRate, mb->m_nodeID,
				dstVM->m_hostServer);
	} else //No policy, just host-host
	{
		totalCost = CalcPathCost_Node2Node(flow->m_dataRate, flow->m_srcNodeID,
				flow->m_dstNodeID);
	}

	return totalCost;
}

void DcVmHypervisorApplication::SchedulePeriodLog() {
	if (!isStarted)
		return;

	LogPeriodAllFlowCost();

	LogPeriodStats();

	Time nextTime(MilliSeconds(300));
	//if(Simulator::Now().GetMilliSeconds()<=4000)// within 4 sec, not start yet
	//	nextTime = MilliSeconds(500);
	//else if(Simulator::Now().GetMilliSeconds()<=8000)// within 8 sec, should be in the migration
	//	nextTime = MilliSeconds(50);
	//else if(Simulator::Now().GetMilliSeconds()<=10000)// within 10 sec
	//	nextTime = MilliSeconds(500);
	//else
	//	nextTime = MilliSeconds(500);

	Simulator::Schedule(nextTime, &DcVmHypervisorApplication::SchedulePeriodLog,
			this);
}

void DcVmHypervisorApplication::LogPeriodAllFlowCost() {

	uint64_t cost = 0;

	policyLog->logPeriodAllFlowCost << Simulator::Now().GetMilliSeconds()
			<< "\t";
	for (uint32_t i = 0; i < fc->GetN(); i++) {

		Ptr<DcFlowApplication> flow = fc->Get(i);
		cost = CalcCostofFlow(flow);//CalcPathCost_Node2Node(flow->m_dataRate, flow->m_srcNodeID, flow->m_dstNodeID);

		policyLog->logPeriodAllFlowCost << cost << "\t";
	}
	policyLog->logPeriodAllFlowCost << "\n";

	policyLog->logPeriodAllFlowCost.flush();
}

void DcVmHypervisorApplication::LogPeriodStats() {

	int64_t currentTime = Simulator::Now().GetMilliSeconds();

	std::cout << currentTime << ": Log Period Stat...\n";
	for (uint32_t i = 0; i < fc->GetN(); ++i) {
		Ptr<DcFlowApplication> flow = fc->Get(i);
		policyLog->logPeriodStat << currentTime << "\t" << flow->m_flowID
				<< "\t" << flow->m_stats.rxBytes << "\t"
				<< flow->m_stats.rxPackets << "\t"
				<< flow->m_stats.delaySum.GetMilliSeconds() << "\t"
				<< flow->m_stats.jitterSum.GetMilliSeconds() << "\t"
				<< flow->m_stats.timesForwarded << "\t"
				<< flow->m_stats.lostPackets << "\n";
	}
	policyLog->logPeriodStat.flush();
}

void DcVmHypervisorApplication::LogPeriodStatsOLD() {
	monitor->CheckForLostPackets();

	int64_t currentTime = Simulator::Now().GetMilliSeconds();
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

	std::cout << currentTime << ": Log Period Stat...\n";
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i =
			stats.begin(); i != stats.end(); ++i) {
		//FlowId fid = i->first;
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
		DcFlowContainer::Iterator itFlow = fc->GetFlow(t.sourceAddress.Get(),
				t.destinationAddress.Get(), t.protocol, t.sourcePort,
				t.destinationPort);

		uint32_t fid;
		bool isFlowOK = true;
		if (itFlow != fc->End())
			fid = (*itFlow)->m_flowID;
		else {
			fid = 0;
			isFlowOK = false;
			std::cout << "Warning: can not find a flow in fc..\n";
			NS_ASSERT_MSG(isFlowOK, "ERROR: can not find a flow in fc..");
		}

		FlowMonitor::FlowStats statsNew = i->second;
		policyLog->logPeriodStat << currentTime << "\t" << (isFlowOK ? fid : -1)
				<< "\t" << statsNew.rxBytes << "\t" << statsNew.rxPackets
				<< "\t" << statsNew.delaySum.GetMilliSeconds() << "\t"
				<< statsNew.jitterSum.GetMilliSeconds() << "\t"
				<< statsNew.timesForwarded << "\t" << statsNew.lostPackets
				<< "\n";
	}
	policyLog->logPeriodStat.flush();
}

/*
 void DcVmHypervisorApplication::LogPeriodStatsOLD()
 {
 monitor->CheckForLostPackets ();

 int64_t currentTime = Simulator::Now().GetMilliSeconds();

 std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

 if(!m_isFirstMonitorStatReady) //for the first time, get stats of all flows
 {
 statsLast = stats;
 lastStatTime = Simulator::Now().GetMilliSeconds();
 m_isFirstMonitorStatReady = true;
 }

 std::cout<<Simulator::Now().GetMilliSeconds()<<": Log Period Stat...\n";
 
 double totalThroughput = 0;
 uint64_t avgE2EDelay = 0;
 double numForwards = 0;
 for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = statsLast.begin (); i != statsLast.end (); ++i)
 {
 FlowMonitor::FlowStats &statsNew = monitor->GetStatsForFlow(i->first);

 if(m_isFirstMonitorStatReady)//get new stats compared to last
 {
 totalThroughput += (statsNew.rxBytes - i->second.rxBytes)*8.0/(currentTime - lastStatTime) / 1000;//Mbps, bits per milli sec = Kbps
 if(statsNew.rxPackets - i->second.rxPackets != 0)
 {
 avgE2EDelay += (statsNew.delaySum.GetMicroSeconds() - i->second.delaySum.GetMicroSeconds())
 /(statsNew.rxPackets - i->second.rxPackets);
 }
 else
 avgE2EDelay += 0;

 if(statsNew.rxPackets - i->second.rxPackets != 0)
 numForwards += (statsNew.timesForwarded - i->second.timesForwarded)*1.0 / (statsNew.rxPackets - i->second.rxPackets);
 else
 numForwards += 0;
 }
 else  //for the first log
 {
 totalThroughput += statsNew.rxBytes*8.0/ currentTime /1000;//Mbps, bits per milli sec = Kbps
 avgE2EDelay += statsNew.delaySum.GetMicroSeconds()/statsNew.rxPackets;
 numForwards += statsNew.timesForwarded*1.0 / statsNew.rxPackets;
 }
 }

 avgE2EDelay = avgE2EDelay/statsLast.size(); //averged of all flows
 numForwards = numForwards/statsLast.size(); //averged of all flows

 policyLog->logPeriodStat
 <<currentTime<<"\t"
 <<totalThroughput<<"\t"
 <<avgE2EDelay<<"\t"
 <<numForwards<<"\t"
 <<"\n";
 policyLog->logPeriodStat.flush();

 statsLast = stats;
 lastStatTime = currentTime;
 }
 */

void DcVmHypervisorApplication::LogVmMigration(uint32_t migratedVmID,
		uint32_t srcNodeID, uint32_t dstNodeID, int64_t utility) {
	policyLog->logVmMigration << Simulator::Now().GetMilliSeconds() << "\t"
			<< migratedVmID << "\t" << srcNodeID << "\t" << dstNodeID << "\t"
			<< utility << "\n";

	policyLog->logVmMigration.flush();

	//check and log policy violation
	//LogPolicyViolation();
}

void DcVmHypervisorApplication::LogPolicyMigration(uint32_t policyID,
		uint32_t flowID, uint32_t seqIndex, uint32_t srcMbID, uint32_t dstMbID,
		int64_t utility) {
	policyLog->logPolicyMigration << Simulator::Now().GetMilliSeconds() << "\t"
			<< policyID << "\t" << flowID << "\t" << seqIndex << "\t" << srcMbID
			<< "\t" << dstMbID << "\t" << utility << "\n";
	policyLog->logPolicyMigration.flush();

	//check and log policy violation
	//LogPolicyViolation();
}

void DcVmHypervisorApplication::LogPolicyViolation() {
	int64_t currentTime = Simulator::Now().GetMilliSeconds();

	uint32_t numViolationTypeCapacity = 0;
	uint32_t numViolationTypeReachability = 0;

	for (uint32_t i = 0; i < fc->GetN(); i++) {

		numViolationTypeCapacity = 0;
		numViolationTypeReachability = 0;

		Ptr<DcFlowApplication> flow = fc->Get(i);

		PolicyItemIterator itPolicy = policies->getPolicyByFlow(flow);

		//only consider policy flow
		if (policies->isOK(itPolicy)) {
			Ptr<DcVm> srcVM = *(vmc->GetVmByID(flow->m_srcVmID));
			Ptr<DcVm> dstVM = *(vmc->GetVmByID(flow->m_dstVmID));

			//======================================
			//check for reachabiltiy
			Ptr<DcMiddleBox> mb = itPolicy->mbList.Get(0); //first mb
			if (!(fattree->checkConnectivity(srcVM->m_hostServer, mb->m_nodeID))) {
				numViolationTypeReachability++; //violation
				//std::cout<<"LOG: Unreachable: flowID: "<<flow->m_flowID<<", srcVM->m_hostServer="<<srcVM->m_hostServer<<", mb->m_nodeID="<<mb->m_nodeID<<"\n";
			}

			for (uint32_t i = 0; i < (uint32_t) itPolicy->len - 1; i++) {
				mb = itPolicy->mbList.Get(i);
				Ptr<DcMiddleBox> mb2 = itPolicy->mbList.Get(i + 1);
				if (!(fattree->checkConnectivity(mb->m_nodeID, mb2->m_nodeID))) {
					numViolationTypeReachability++; //violation
					//std::cout<<"LOG: Unreachable: flowID: "<<flow->m_flowID<<", mb->m_nodeID="<<mb->m_nodeID<<", mb2->m_nodeID="<<mb2->m_nodeID<<"\n";
				}
			}

			mb = itPolicy->mbList.Get(itPolicy->len - 1); //last mb
			if (!(fattree->checkConnectivity(mb->m_nodeID, dstVM->m_hostServer))) {
				numViolationTypeReachability++; //violation
				//std::cout<<"LOG: Unreachable: flowID: "<<flow->m_flowID<<", mb->m_nodeID="<<mb->m_nodeID<<", dstVM->m_hostServer="<<dstVM->m_hostServer<<"\n";
			}

			//======================================
			//check for capacity
			for (uint32_t i = 0; i < (uint32_t) itPolicy->len; i++) {
				mb = itPolicy->mbList.Get(i);

				//if(!policies->checkMiddleboxCapacity(mb, fc))
				if (!mb->m_capacityRemain < 0) {
					numViolationTypeCapacity++;
				}
			}

			//if policy-aware, there should be no violation. only log violation to reduce file size
			//if(m_usePolicy && numViolationTypeReachability==0 && numViolationTypeCapacity==0)
			//	continue;

			//only log for valid policy
			policyLog->logPolicyViolation << currentTime << "\t"
					<< flow->m_flowID << "\t" << numViolationTypeReachability
					<< "\t" << numViolationTypeCapacity << "\t"
					<< (uint32_t) itPolicy->len << "\t" << "\n";
		}
	}

	policyLog->logPolicyViolation.flush();
}

void DcVmHypervisorApplication::LogAlgPerformance(struct timeval start,
		struct timeval phase1, struct timeval phase2) {
	int t1 = 1000000 * (phase1.tv_sec - start.tv_sec) + phase1.tv_usec
			- start.tv_usec;
	int t2 = 1000000 * (phase2.tv_sec - phase1.tv_sec) + phase2.tv_usec
			- phase1.tv_usec;
	int t_all = 1000000 * (phase2.tv_sec - start.tv_sec) + phase2.tv_usec
			- start.tv_usec;

	policyLog->logAlgPerformance
	//<<t0<<"\t"  //start time
	<< t1 << "\t"
			// time of phase I, microseconds
			<< t2 << "\t"
			// time of phase II, microseconds
			<< t_all << "\t"
			//overall time, microseconds
			<< vmc->GetN() << "\t" << fc->GetN() << "\t"
			<< fattree->GetFatTreeSize() << "\n";
	policyLog->logAlgPerformance.flush();

	//check and log policy violation
	//LogPolicyViolation();
}

void DcVmHypervisorApplication::globalChangePolicy(uint32_t num, bool status) {
	/*
	 if(num==0)
	 {
	 policy->setFirstNPolicies(num,status);
	 }
	 else if(num==1 && status==true)
	 {
	 policy->resetMBList(num,middleboxes);
	 }
	 std::cout<<Simulator::Now().GetMilliSeconds()<<" : policy changing, status="<<status<<std::endl;
	 //policy->printPolicies();
	 */
}

void DcVmHypervisorApplication::globalChangeFlow(uint32_t num, bool status) {
	/*
	 dcVMs->setFirstNFlow(num,status);
	 std::cout<<Simulator::Now().GetMilliSeconds()<<" : flow changing, status="<<status<<std::endl;
	 //policy->printPolicies();
	 */
}

void DcVmHypervisorApplication::ScheduleSnapshot() {
	if (!isStarted)
		return;

	LogSnapshotAllVmCost();

	//logSnapshotFlowLength();
	logSnapshotLinkUtilization();

	Time nextTime(MilliSeconds(1000));
	Simulator::Schedule(nextTime, &DcVmHypervisorApplication::ScheduleSnapshot,
			this);
}

void DcVmHypervisorApplication::ScheduleVmCount() {
	if (!isStarted)
		return;

	Time nextTime(MilliSeconds(500));
	m_vmCountEvent = Simulator::Schedule(nextTime,
			&DcVmHypervisorApplication::CountNVM, this);
}
void DcVmHypervisorApplication::CountNVM() {
	/*
	 uint32_t count = 0;
	 for(DcVmNewItemI itVM=dcVMs->m_allVMs.begin(); itVM!=dcVMs->m_allVMs.end(); itVM++)
	 {
	 if(itVM->hostServer==GetNode()->GetId()) //on for VMs located by the current server
	 count++;
	 }

	 if(count>0)
	 {
	 policyLog->logVmCountOnServer<<Simulator::Now().GetMicroSeconds()<<"\t"
	 <<GetNode()->GetId()<<"\t"<<count<<"\n";
	 policyLog->logVmCountOnServer.flush();
	 }
	 ScheduleVmCount();
	 */
}

void DcVmHypervisorApplication::ScheduleNextCalc(Time nextTime) {
	//Time nextTime (MilliSeconds(interval));
	m_calcTotalCostEvent = Simulator::Schedule(nextTime,
			&DcVmHypervisorApplication::CalcTotalCost, this);
}

void DcVmHypervisorApplication::RemedySchedule() {
	Time nextAll = Seconds(10000);

	ScheduleNextCalc(nextAll);
}

void DcVmHypervisorApplication::CalcTotalCost() {

	/*

	 Time nextAll = Seconds(10000);

	 for(DcVmNewItemI itVM=dcVMs->m_allVMs.begin(); itVM!=dcVMs->m_allVMs.end(); itVM++)
	 {
	 if(itVM->hostServer!=GetNode()->GetId()) //on for VMs located by the current server
	 continue;

	 if(nextAll>itVM->nextTime)
	 {
	 nextAll = itVM->nextTime;
	 }

	 if(itVM->nextTime>Simulator::Now())
	 {
	 //std::cout<<"time for VM "<<itVM->id<<" not reached"<<std::endl;
	 continue;
	 }

	 if(itVM->flows.size()==0)
	 continue;

	 if(m_usePolicy)
	 itVM->CommCost = CalcCost_VmOnNode_ForDecisionMaking(itVM->hostServer,itVM->id);
	 else
	 itVM->CommCost = CalcCost_VmOnNode_NoPolicyScheme(itVM->hostServer,itVM->id);

	 //std::cout<<"On Node "<<GetNode()->GetId()<<", Calc Cost for VM "<<itVM->id
	 //	<<" is "<<itVM->CommCost<<std::endl;

	 //check whether need to be migrated
	 uint32_t minNodeID = itVM->hostServer;
	 uint64_t minCost = itVM->CommCost;
	 uint64_t tempCost = itVM->CommCost;
	 for (NodeContainer::Iterator itNode = fattree->HostNodes().Begin (); itNode != fattree->HostNodes().End (); ++itNode) {
	 Ptr<Node> node = *itNode;

	 if(node->GetId()==itVM->hostServer) //ignore the current host node
	 continue;

	 Ptr<DcVmHypervisorApplication> hvapp = getHyperVisor(node->GetId());

	 if(hvapp->m_totalCPU - hvapp->m_useCPU < itVM->cpu ||
	 hvapp->m_totalMemory - hvapp->m_useMemory < itVM->memory)
	 continue; //ignore host nodes which doesn't have enough resource

	 //std::cout<<"Caculate cost if on Node "<<node->GetId()<<std::endl;
	 //tempCost = CalcCost_VmOnNode(node->GetId(),itVM->id);
	 if(m_usePolicy)
	 tempCost = CalcCost_VmOnNode_ForDecisionMaking(node->GetId(),itVM->id);
	 else
	 tempCost = CalcCost_VmOnNode_NoPolicyScheme(node->GetId(),itVM->id);

	 if(tempCost < minCost)
	 {
	 minCost = tempCost;
	 minNodeID = node->GetId();
	 }
	 }

	 if(minNodeID!=itVM->hostServer) // find a node with smaller cost
	 {
	 //calculate migration cost
	 uint64_t migrationCost = CalcMigrationCost(itVM->id,itVM->hostServer,minNodeID);
	 std::cout<<Simulator::Now().GetMilliSeconds()<<": To Migrate VM "<<itVM->id<<" to host "<<minNodeID
	 <<" will have utility: "<<itVM->CommCost-minCost<<", has migration cost "<<migrationCost<<std::endl;


	 //Only migrate the VM when the utility is larger than migration cost
	 if(itVM->CommCost-minCost>migrationCost)
	 {

	 std::cout<<Simulator::Now().GetMilliSeconds()<<" : Migrate VM "<<itVM->id<<" to host "<<minNodeID<<std::endl;
	 migrateVM(itVM->id,minNodeID);

	 LogAllVmCost(itVM->id, itVM->CommCost-minCost, minCost);
	 LogAllLinkUtilization(false);


	 }
	 }

	 Time nexTime = Time(MicroSeconds(m_uniformRand->GetInteger(1500000, 3000000)));
	 //Time nexTime = Time(MicroSeconds(RandomVariable(UniformVariable(5000000, 8000000)).GetInteger()));
	 itVM->nextTime = Simulator::Now() + nexTime;
	 //std::cout<<"Next time for VM "<<itVM->id<<" is "<<itVM->nextTime.GetMilliSeconds()<<std::endl;


	 if(nextAll>itVM->nextTime)
	 {
	 nextAll = itVM->nextTime;
	 }
	 }

	 //ScheduleNextCalc(250);
	 ScheduleNextCalc(nextAll);
	 */
}

void DcVmHypervisorApplication::LogSnapshotAllVmCost() {
	/*
	 uint64_t cost = 0;
	 int64_t currentTime = Simulator::Now().GetMicroSeconds();
	 for(DcVmNewItemI itVM=dcVMs->m_allVMs.begin(); itVM!=dcVMs->m_allVMs.end(); itVM++)
	 {
	 cost = CalcCost_VmOnNode(itVM->hostServer,itVM->id);

	 policyLog->logSnapshotVmCost
	 <<currentTime<<"\t"
	 <<itVM->id<<"\t"
	 <<itVM->hostServer<<"\t"
	 <<cost<<"\t"
	 <<"\n";
	 policyLog->logSnapshotVmCost.flush();
	 }
	 */
}

//TODO:
void DcVmHypervisorApplication::logSnapshotLinkUtilization() {
	/*
	 uint64_t linkFlowsUp[] = {0, 0, 0}; //total sumation of all flows' rate on each kind of link
	 uint64_t linkFlowsDown[] = {0, 0, 0}; //total sumation of all flows' rate on each kind of link
	 uint32_t numLinks[3];
	 numLinks[0]= fattree->HostNodes().GetN(); //num of host-edge links
	 numLinks[1]= fattree->EdgeNodes().GetN() * fattree->GetFatTreeSize(); //num of edge-aggr links
	 numLinks[2]= fattree->CoreNodes().GetN() * 2 * fattree->GetFatTreeSize(); //num of aggr-core links

	 //core = fattree_size^2
	 uint32_t numCore = fattree->CoreNodes().GetN();
	 uint64_t coreLinkUp[100];
	 uint64_t coreLinkDown[100];
	 uint64_t total_temp = 0;

	 uint8_t mbID;
	 MiddleBoxItemI itMB;
	 MiddleBoxItemI itMB2;
	 PolicyItemI itPolicy;
	 uint64_t lamda = 0;

	 for(DcFlowItemI itFlow=dcVMs->m_allFlows.begin(); itFlow!=dcVMs->m_allFlows.end(); itFlow++)
	 {
	 Ptr<DcFlowApplication> flow = (Ptr<DcFlowApplication>)*itFlow;

	 if(!flow->isOK)
	 continue;

	 //if(m_usePolicy)
	 {
	 uint8_t proto = flow->useTCP?6:17;
	 Ipv4Address ipv4addr = NodeList::GetNode(flow->m_srcNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	 uint32_t saddr = ipv4addr.Get();
	 ipv4addr = NodeList::GetNode(flow->m_dstNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	 uint32_t daddr = ipv4addr.Get();
	 uint16_t sport = flow->m_srcPort;
	 uint16_t dport = flow->m_dstPort;

	 itPolicy = policy->lookup(saddr,daddr,proto,sport,dport);
	 }

	 //lamda = flow->m_dataRate.GetBitRate()/1000;
	 lamda = flow->m_dataRate.GetBitRate()/8*BW_BASE;
	 //lamda = lamda*TIME_PERIOD_LAMDA;

	 if(//m_usePolicy &&
	 policy->isOK(itPolicy)) //for flow with policies
	 {
	 //for src VM to first MB
	 mbID = policy->getFirstMB(itPolicy);
	 itMB = middleboxes->getMiddleBox(mbID);
	 CalcLinkUtilization_Node2Node(flow->m_srcNodeID, itMB->nodeID, lamda, linkFlowsUp, linkFlowsDown);

	 total_temp = linkFlowsUp[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkUp[index] += lamda;
	 }
	 }
	 total_temp =linkFlowsDown[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkDown[index] += lamda;
	 }
	 }


	 //for first MB to last MB
	 if(policy->getFirstMB(itPolicy) != policy->getLastMB(itPolicy))
	 {
	 for(int indexP=0; indexP < itPolicy->len-1; indexP++)
	 {
	 mbID = itPolicy->m_MB_list[indexP];
	 itMB = middleboxes->getMiddleBox(mbID);
	 mbID = itPolicy->m_MB_list[indexP+1];
	 itMB2 = middleboxes->getMiddleBox(mbID);
	 CalcLinkUtilization_Node2Node(itMB->nodeID, itMB2->nodeID, lamda, linkFlowsUp, linkFlowsDown);

	 total_temp = linkFlowsUp[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkUp[index] += lamda;
	 }
	 }
	 total_temp =linkFlowsDown[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkDown[index] += lamda;
	 }
	 }
	 }
	 }

	 //for last MB to dst VM
	 mbID = policy->getLastMB(itPolicy);
	 itMB = middleboxes->getMiddleBox(mbID);
	 CalcLinkUtilization_Node2Node(itMB->nodeID, flow->m_dstNodeID, lamda, linkFlowsUp, linkFlowsDown);

	 total_temp = linkFlowsUp[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkUp[index] += lamda;
	 }
	 }
	 total_temp =linkFlowsDown[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkDown[index] += lamda;
	 }
	 }
	 }
	 else
	 {
	 CalcLinkUtilization_Node2Node(flow->m_srcNodeID, flow->m_dstNodeID, lamda, linkFlowsUp, linkFlowsDown);

	 total_temp = linkFlowsUp[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkUp[index] += lamda;
	 }
	 }
	 total_temp =linkFlowsDown[2];
	 if(total_temp>0)
	 {
	 uint32_t n=total_temp/lamda;

	 for(uint32_t ic=0;ic<n;ic++)
	 {
	 //random choose a core
	 uint32_t index = m_uniformRand->GetInteger(0, numCore-1);
	 coreLinkDown[index] += lamda;
	 }
	 }
	 }


	 }

	 uint64_t totalCoreLink = 2 * fattree->GetFatTreeSize();
	 for(uint32_t i=0;i<numCore;i++)
	 {
	 policyLog->logSnapshotLinkUtilization
	 <<Simulator::Now().GetMicroSeconds()<<"\t"
	 <<i<<"\t"
	 <<coreLinkUp[i]<<"\t"
	 <<coreLinkDown[i]<<"\t"
	 <<totalCoreLink<<"\t"
	 <<"\n";
	 policyLog->logSnapshotLinkUtilization.flush();
	 }
	 */
}

void DcVmHypervisorApplication::LogAllLinkUtilization(bool isPeriod) {
	/*	uint64_t linkFlowsUp[] = {0, 0, 0}; //total sumation of all flows' rate on each kind of link
	 uint64_t linkFlowsDown[] = {0, 0, 0}; //total sumation of all flows' rate on each kind of link
	 uint32_t numLinks[3];
	 uint32_t routeLength = 0;
	 numLinks[0]= fattree->HostNodes().GetN(); //num of host-edge links
	 numLinks[1]= fattree->EdgeNodes().GetN() * fattree->GetFatTreeSize(); //num of edge-aggr links
	 numLinks[2]= fattree->CoreNodes().GetN() * 2 * fattree->GetFatTreeSize(); //num of aggr-core links


	 uint8_t mbID;
	 MiddleBoxItemI itMB;
	 MiddleBoxItemI itMB2;
	 PolicyItemI itPolicy;
	 uint64_t lamda = 0;

	 for(DcFlowItemI itFlow=dcVMs->m_allFlows.begin(); itFlow!=dcVMs->m_allFlows.end(); itFlow++)
	 {
	 Ptr<DcFlowApplication> flow = (Ptr<DcFlowApplication>)*itFlow;

	 if(!flow->isOK)
	 continue;

	 //if(m_usePolicy)
	 {
	 uint8_t proto = flow->useTCP?6:17;
	 Ipv4Address ipv4addr = NodeList::GetNode(flow->m_srcNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	 uint32_t saddr = ipv4addr.Get();
	 ipv4addr = NodeList::GetNode(flow->m_dstNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	 uint32_t daddr = ipv4addr.Get();
	 uint16_t sport = flow->m_srcPort;
	 uint16_t dport = flow->m_dstPort;

	 itPolicy = policy->lookup(saddr,daddr,proto,sport,dport);
	 }

	 //lamda = flow->m_dataRate.GetBitRate()/1000;
	 lamda = flow->m_dataRate.GetBitRate()/8*BW_BASE;
	 lamda = lamda*TIME_PERIOD_LAMDA;

	 if(//m_usePolicy &&
	 policy->isOK(itPolicy)) //for flow with policies
	 {
	 //for src VM to first MB
	 mbID = policy->getFirstMB(itPolicy);
	 itMB = middleboxes->getMiddleBox(mbID);
	 routeLength += CalcLinkUtilization_Node2Node(flow->m_srcNodeID, itMB->nodeID, lamda, linkFlowsUp, linkFlowsDown);

	 //for first MB to last MB
	 if(policy->getFirstMB(itPolicy) != policy->getLastMB(itPolicy))
	 {
	 for(int indexP=0; indexP < itPolicy->len-1; indexP++)
	 {
	 mbID = itPolicy->m_MB_list[indexP];
	 itMB = middleboxes->getMiddleBox(mbID);
	 mbID = itPolicy->m_MB_list[indexP+1];
	 itMB2 = middleboxes->getMiddleBox(mbID);
	 routeLength += CalcLinkUtilization_Node2Node(itMB->nodeID, itMB2->nodeID, lamda, linkFlowsUp, linkFlowsDown);
	 }
	 }

	 //for last MB to dst VM
	 mbID = policy->getLastMB(itPolicy);
	 itMB = middleboxes->getMiddleBox(mbID);
	 routeLength += CalcLinkUtilization_Node2Node(itMB->nodeID, flow->m_dstNodeID, lamda, linkFlowsUp, linkFlowsDown);
	 }
	 else
	 {
	 routeLength += CalcLinkUtilization_Node2Node(flow->m_srcNodeID, flow->m_dstNodeID, lamda, linkFlowsUp, linkFlowsDown);
	 }
	 }

	 if(!isPeriod)
	 {
	 policyLog->logLinkUtilization
	 <<Simulator::Now().GetMicroSeconds()<<"\t"
	 <<linkFlowsUp[0]<<"\t"<<linkFlowsUp[1]<<"\t"<<linkFlowsUp[2]<<"\t"
	 <<linkFlowsDown[0]<<"\t"<<linkFlowsDown[1]<<"\t"<<linkFlowsDown[2]<<"\t"
	 <<routeLength<<"\t"
	 <<numLinks[0]<<"\t"<<numLinks[1]<<"\t"<<numLinks[2]<<"\t"
	 <<dcVMs->m_allFlows.size()<<"\t"
	 <<(m_usePolicy?1:0)<<"\t"
	 <<"\n";
	 policyLog->logLinkUtilization.flush();
	 }
	 else
	 {
	 policyLog->logPeriodLinkUtilization
	 <<Simulator::Now().GetMicroSeconds()<<"\t"
	 <<linkFlowsUp[0]<<"\t"<<linkFlowsUp[1]<<"\t"<<linkFlowsUp[2]<<"\t"
	 <<linkFlowsDown[0]<<"\t"<<linkFlowsDown[1]<<"\t"<<linkFlowsDown[2]<<"\t"
	 <<routeLength<<"\t"
	 <<numLinks[0]<<"\t"<<numLinks[1]<<"\t"<<numLinks[2]<<"\t"
	 <<dcVMs->m_allFlows.size()<<"\t"
	 <<(m_usePolicy?1:0)<<"\t"
	 <<"\n";
	 policyLog->logPeriodLinkUtilization.flush();
	 }
	 */
}

//nodeID1 ---> nodeID2
// return: route length between the two nodes
uint32_t DcVmHypervisorApplication::CalcLinkUtilization_Node2Node(
		uint32_t nodeID1, uint32_t nodeID2, uint64_t lamda,
		uint64_t* linkFlowsUp, uint64_t* linkFlowsDown) {
	return 0;
	/*
	 if(nodeID1==nodeID2)
	 return 0;

	 uint32_t routeLength = 0;

	 if(fattree->IsHost(nodeID1) && fattree->IsHost(nodeID2)) //host - host
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 if(fattree->GetEdgeIDinSubTree(nodeID1)==fattree->GetEdgeIDinSubTree(nodeID2))
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsDown[0] += lamda;

	 routeLength = 2;
	 }
	 else
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;
	 linkFlowsDown[0] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 4;
	 }
	 }
	 else
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[0] += lamda;
	 linkFlowsDown[1] += lamda;
	 linkFlowsDown[2] += lamda;

	 routeLength = 6;
	 }
	 }
	 else if(fattree->IsHost(nodeID1) && !fattree->IsHost(nodeID2)) //host - MB
	 {
	 if(fattree->IsEdge(nodeID2)) //MB on Edge
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 if(fattree->GetEdgeIDinSubTree(nodeID1)==fattree->GetEdgeIDinSubTree(nodeID2))
	 {
	 linkFlowsUp[0] += lamda;

	 routeLength = 1;
	 }
	 else
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 3;
	 }
	 }
	 else
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 5;
	 }
	 }
	 else if(fattree->IsAggr(nodeID2)) //MB on Aggr
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;

	 routeLength = 2;
	 }
	 else
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;

	 routeLength = 4;
	 }
	 }
	 else if(fattree->IsCore(nodeID2)) //MB on Core
	 {
	 linkFlowsUp[0] += lamda;
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;

	 routeLength = 3;
	 }
	 }
	 else if(!fattree->IsHost(nodeID1) && fattree->IsHost(nodeID2)) //MB-host
	 {
	 if(fattree->IsEdge(nodeID1)) //MB on Edge
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 if(fattree->GetEdgeIDinSubTree(nodeID1)==fattree->GetEdgeIDinSubTree(nodeID2))
	 {
	 linkFlowsDown[0] += lamda;

	 routeLength = 1;
	 }
	 else
	 {
	 linkFlowsUp[1] += lamda;
	 linkFlowsDown[1] += lamda;
	 linkFlowsDown[0] += lamda;

	 routeLength = 3;
	 }
	 }
	 else
	 {
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;
	 linkFlowsDown[1] += lamda;
	 linkFlowsDown[0] += lamda;

	 routeLength = 5;
	 }
	 }
	 else if(fattree->IsAggr(nodeID1)) //MB on Aggr
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 linkFlowsDown[1] += lamda;
	 linkFlowsDown[0] += lamda;

	 routeLength = 2;
	 }
	 else
	 {
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;
	 linkFlowsDown[1] += lamda;
	 linkFlowsDown[0] += lamda;

	 routeLength = 4;
	 }
	 }
	 else if(fattree->IsCore(nodeID1)) //MB on Core
	 {
	 linkFlowsDown[0] += lamda;
	 linkFlowsDown[1] += lamda;
	 linkFlowsDown[2] += lamda;

	 routeLength = 3;
	 }
	 }
	 else if(!fattree->IsHost(nodeID1) && !fattree->IsHost(nodeID2)) //MB-MB
	 {
	 //Edge-Edge
	 if(fattree->IsEdge(nodeID1) && fattree->IsEdge(nodeID2))
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 linkFlowsUp[1] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 2;
	 }
	 else
	 {
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 4;
	 }
	 }

	 //Edge-Aggr
	 else if(fattree->IsEdge(nodeID1) && fattree->IsAggr(nodeID2))
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 linkFlowsUp[1] += lamda;

	 routeLength = 1;
	 }
	 else
	 {
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;

	 routeLength = 3;
	 }
	 }

	 //Edge-Core
	 else if(fattree->IsEdge(nodeID1) && fattree->IsCore(nodeID2))
	 {
	 linkFlowsUp[1] += lamda;
	 linkFlowsUp[2] += lamda;

	 routeLength = 2;
	 }

	 //Aggr-Edge
	 else if(fattree->IsAggr(nodeID1) && fattree->IsEdge(nodeID2))
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 linkFlowsDown[1] += lamda;

	 routeLength = 1;
	 }
	 else
	 {
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 3;
	 }
	 }

	 //Aggr-Aggr
	 else if(fattree->IsAggr(nodeID1) && fattree->IsAggr(nodeID2))
	 {
	 if(fattree->GetSubTreeID(nodeID1)==fattree->GetSubTreeID(nodeID2))
	 {
	 linkFlowsDown[1] += lamda;
	 linkFlowsUp[1] += lamda;

	 routeLength = 2;
	 }
	 else
	 {
	 linkFlowsUp[2] += lamda;
	 linkFlowsDown[2] += lamda;

	 routeLength = 2;
	 }
	 }

	 //Aggr-Core
	 else if(fattree->IsAggr(nodeID1) && fattree->IsCore(nodeID2))
	 {
	 linkFlowsUp[2] += lamda;

	 routeLength = 1;
	 }

	 //Core-Edge
	 else if(fattree->IsCore(nodeID1) && fattree->IsEdge(nodeID2))
	 {
	 linkFlowsDown[2] += lamda;
	 linkFlowsDown[1] += lamda;

	 routeLength = 2;
	 }

	 //Core-Aggr
	 else if(fattree->IsCore(nodeID1) && fattree->IsAggr(nodeID2))
	 {
	 linkFlowsDown[2] += lamda;

	 routeLength = 1;
	 }

	 //Core-Core
	 else if(fattree->IsCore(nodeID1) && fattree->IsCore(nodeID2))
	 {
	 linkFlowsDown[2] += lamda;
	 linkFlowsUp[2] += lamda;

	 routeLength = 2;
	 }
	 else
	 NS_ABORT_MSG("Node ID is wrong 1");
	 }
	 else
	 NS_ABORT_MSG("Node ID is wrong 2");

	 NS_ASSERT_MSG(routeLength>0,"Routelength calc error, shoule >0");
	 return routeLength;
	 */
}

Ptr<DcVmHypervisorApplication> DcVmHypervisorApplication::getHyperVisor(
		uint32_t NodeID) {
	if (!fattree->IsHost(NodeID))
		return 0;

	Ptr<DcVmHypervisorApplication> hvapp;
	Ptr<Application> app;

	uint32_t num = NodeList::GetNode(NodeID)->GetNApplications();

	for (uint32_t i = 0; i < num; i++) {
		app = NodeList::GetNode(NodeID)->GetApplication(i);
		NS_ASSERT(app != 0);
		hvapp = app->GetObject<DcVmHypervisorApplication>();
		if (hvapp != 0)
			return hvapp;
		//NS_ASSERT (hvapp != 0);
	}

	NS_ASSERT_MSG(hvapp != 0, "Can't find the DcVmHypervisorApplication");

	return hvapp;
}

} /* namespace ns3 */
