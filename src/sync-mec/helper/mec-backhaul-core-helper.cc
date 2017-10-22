/*
 * mec-backhaul-core-helper.cc
 *
 *  Created on: 6 Jun 2017
 *      Author: thiagogenez
 */

#include <sstream>
#include <stdint.h>
#include <stdlib.h>

#include "ns3/log.h"
#include "ns3/bridge-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/string.h"
#include "ns3/config.h"

#include "ns3/mec-policy-routing-helper.h"
#include "ns3/mec-constants-helper.h"

#include "mec-backhaul-core-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecBackhaulCoreHelper");
NS_OBJECT_ENSURE_REGISTERED(MecBackhaulCoreHelper);

TypeId MecBackhaulCoreHelper::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecBackhaulCoreHelper").SetParent<Object>().SetGroupName(
					"sync-mec").AddConstructor<MecBackhaulCoreHelper>();

	return tid;
}

MecBackhaulCoreHelper::MecBackhaulCoreHelper() {
	NS_LOG_FUNCTION(this);

}

MecBackhaulCoreHelper::~MecBackhaulCoreHelper() {
	NS_LOG_FUNCTION(this);
}

uint32_t MecBackhaulCoreHelper::CreateNodeContainers(
		NodeContainer *nodeContainerAux, NodeContainer *nodeContainerOriginal,
		int totalNumberContainers, int number_of_nodes_per_container,
		InternetStackHelper *internet) {

	int i = 0;

	for (i = 0; i < totalNumberContainers; i++) {
		nodeContainerAux[i].Create(number_of_nodes_per_container);
		internet->Install(nodeContainerAux[i]);

		nodeContainerOriginal->Add(nodeContainerAux[i]);
	}

	Ptr<Node> nodeTemp = nodeContainerOriginal->Get(
			nodeContainerOriginal->GetN() - 1); //*(it--);
	return nodeTemp->GetId(); //Max CoreID

}

void MecBackhaulCoreHelper::AssignSubset(int second_octet, int third_octet,
		int fourth_octet, Ipv4AddressHelper *address,
		Ipv4InterfaceContainer *ipContainer, NetDeviceContainer *netContainer) {

	//Assign subnet
	char *subnet;
	char *base;

	subnet = Num2IPv4String(10, second_octet, third_octet, 0);
	base = Num2IPv4String(0, 0, 0, fourth_octet);

	address->SetBase(subnet, "255.255.255.0", base);
	*ipContainer = address->Assign(*netContainer);

}

void MecBackhaulCoreHelper::ConnectingCoreToAggregate(int k, int num_group,
		int num_core, int num_pod, NodeContainer *core, NodeContainer *agg,
		PointToPointHelper *p2p, Ipv4AddressHelper *address) {

	NetDeviceContainer ca[num_group][num_core][num_pod];
	Ipv4InterfaceContainer ipCaContainer[num_group][num_core][num_pod];

	int fourth_octet = 1;
	int i = 0, j = 0, h = 0;

	for (i = 0; i < num_group; i++) {
		for (j = 0; j < num_core; j++) {
			fourth_octet = 1;
			for (h = 0; h < num_pod; h++) {
				ca[i][j][h] = p2p->Install(core[i].Get(j), agg[h].Get(i));

				int second_octet = k + i;
				int third_octet = j;

				AssignSubset(second_octet, third_octet, fourth_octet, address,
						&ipCaContainer[i][j][h], &ca[i][j][h]);

				fourth_octet += 2;
			}
		}
	}
}

void MecBackhaulCoreHelper::ConnectingAggregateToEdge(int k, int num_pod,
		int num_agg, int num_edge, NodeContainer *agg, NodeContainer *edge,
		PointToPointHelper *p2p, Ipv4AddressHelper *address) {
	NetDeviceContainer ae[num_pod][num_agg][num_edge];
	Ipv4InterfaceContainer ipAeContainer[num_pod][num_agg][num_edge];
	int i = 0, j = 0, h = 0;

	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_agg; j++) {
			for (h = 0; h < num_edge; h++) {
				ae[i][j][h] = p2p->Install(agg[i].Get(j), edge[i].Get(h));

				int second_octet = i;
				int third_octet = j + (k / 2);
				int fourth_octet;
				if (h == 0)
					fourth_octet = 1;
				else
					fourth_octet = h * 2 + 1;

				AssignSubset(second_octet, third_octet, fourth_octet, address,
						&ipAeContainer[i][j][h], &ae[i][j][h]);

			}
		}
	}
}

void MecBackhaulCoreHelper::ConnectingEdgeToHost(int num_pod, int num_edge,
		int num_host, NodeContainer *edge, NodeContainer *host,
		PointToPointHelper *p2p, Ipv4AddressHelper *address) {

	NetDeviceContainer he[num_pod][num_edge][num_host];
	Ipv4InterfaceContainer ipHeContainer[num_pod][num_edge][num_host];

	int i = 0, j = 0, h = 0;
	NodeContainer *ptr; // use this as a marker to go to next block

	for (i = 0; i < num_pod; i++) {
		ptr = host + i * num_edge; // this is the starting for host[i] ...

		for (j = 0; j < num_edge; j++) {
			for (h = 0; h < num_host; h++) {
				//ptr[j] is the same as host[i][j]
				he[i][j][h] = p2p->Install(edge[i].Get(j), ptr[j].Get(h));

				int second_octet = i;
				int third_octet = j;
				int fourth_octet;

				if (h == 0)
					fourth_octet = 1;
				else
					fourth_octet = h * 2 + 1;

				AssignSubset(second_octet, third_octet, fourth_octet, address,
						&ipHeContainer[i][j][h], &he[i][j][h]);

				// Just a test to drop Throughput
//
//				Ptr<RateErrorModel> error = CreateObject<RateErrorModel>();
//				error->SetUnit(RateErrorModel::ERROR_UNIT_PACKET);
//				error->SetRate(0.95);
//				error->Enable();
//
//				for (uint32_t teste = 0; teste < he[i][j][h].GetN(); ++teste) {
//					Ptr<NetDevice> dev = he[i][j][h].Get(teste);
//					dev->SetAttribute("ReceiveErrorModel", PointerValue(error));
//				}

			}
		}
	}

	std::cout << "Finished connecting edge switches and hosts  " << "\n";
}

void MecBackhaulCoreHelper::Initialise(uint32_t size) {
	m_sizeK = size;					// number of ports per switch
	m_num_edge = (size / 2);		// number of edge switch in a pod
	m_num_agg = (size / 2);			// number of aggregation switch in a pod
	m_num_host = (size / 2);		// number of hosts under a switch
}

void MecBackhaulCoreHelper::Create(uint32_t size) {

	std::cout << "Initialising a mobile core network with a size of " << size
			<< std::endl;

	Initialise(size);

	int32_t k = m_sizeK;			// number of ports per switch
	uint32_t num_agg = (k / 2);		// number of aggregation switch in a pod
	uint32_t num_edge = (k / 2);	// number of edge switch in a pod
	uint32_t num_host = (k / 2);	// number of hosts under a switch

	uint32_t num_pod = k;			// number of pod
	uint32_t num_group = k / 2;		// number of group of core switches
	uint32_t num_core = (k / 2);	// number of core switch in a group

	//=========== BEGIN: Initialise Internet Stack and Routing Protocols ===========//

	//Set ECMP
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting",
			BooleanValue(true));

	// Create the Internet
	InternetStackHelper internet;
	MecPolicyRoutingHelper policyRouting;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(policyRouting, 10);
	internet.SetRoutingHelper(list);

	//=========== END: Initialise Internet Stack and Routing Protocols ===========//

	//=========== BEGIN: Creation of Node Containers ===========//

	// NodeContainer for core switches
	NodeContainer core[num_group];
	m_maxCoreID = CreateNodeContainers(core, &m_Core, num_group, num_core,
			&internet);
	std::cout << "Test OK for core, m_maxCoreID = " << m_maxCoreID << std::endl;

	// NodeContainer for aggregation switches
	NodeContainer agg[num_pod];
	m_maxAggrID = CreateNodeContainers(agg, &m_Aggr, num_pod, num_agg,
			&internet);
	std::cout << "Test OK for aggr, m_maxAggrID = " << m_maxAggrID << std::endl;

	// NodeContainer for edge switches
	NodeContainer edge[num_pod];
	m_maxEdgeID = CreateNodeContainers(edge, &m_Edge, num_pod, num_edge,
			&internet);
	std::cout << "Test OK for edge, m_maxEdgeID = " << m_maxEdgeID << std::endl;

	// NodeContainer for hosts
	NodeContainer host[num_pod][num_edge];
	m_maxHostID = 0;
	int i = 0;
	for (i = 0; i < k; i++) {
		m_maxHostID = CreateNodeContainers(host[i], &m_Host, num_edge, num_host,
				&internet);
	}
	std::cout << "Test OK for host, m_maxHostID = " << m_maxHostID << std::endl;

	// put all together
	m_AllNode.Add(m_Core);
	m_AllNode.Add(m_Aggr);
	m_AllNode.Add(m_Edge);
	m_AllNode.Add(m_Host);

	std::cout << "Complete node creations" << std::endl;

	//=========== END: Creation of Node Containers ===========//

	//=========== BEGIN: Connecting Nodes ====================//

	// Inintialise Address Helper
	Ipv4AddressHelper address;

	// Initialise PointtoPoint helper
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", MecConstantsHelper::BACKHAUL_DATA_RATE);
	p2p.SetChannelAttribute("Delay", MecConstantsHelper::DELAY);
	//p2p.SetDeviceAttribute("Mtu", UintegerValue(1500));
	//p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(100));

	//Connecting edge switches to hosts
	ConnectingEdgeToHost(num_pod, num_edge, num_host, edge,
			(NodeContainer *) host, &p2p, &address);

	//Connecting aggregate switches to edge switches
	ConnectingAggregateToEdge(k, num_pod, num_agg, num_edge, agg, edge, &p2p,
			&address);

	//Connecting core switches to aggregate switches
	ConnectingCoreToAggregate(k, num_group, num_core, num_pod, core, agg, &p2p,
			&address);

	std::cout << "Finished connecting aggregation switches and edge switches  "
			<< "\n";

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	std::cout << "Complete node connections" << std::endl;
	//=========== END: Connecting nodes ====================//

}

//Pod ID, from left to right: 0, 1, 2, ...
uint32_t MecBackhaulCoreHelper::GetPodID(uint32_t nodeID) const {

	// subtree is only for host, edge and aggr
	if (IsCore(nodeID)) {
		return -2;
	}

	if (IsAggr(nodeID)) {
		return (nodeID - m_maxCoreID - 1) / m_num_agg;
	} else if (IsEdge(nodeID)) {
		return (nodeID - m_maxAggrID - 1) / m_num_edge;
	} else if (IsHost(nodeID)) {
		return (nodeID - m_maxEdgeID - 1) / (m_num_edge * m_num_host);
	}

	return -1;
}

//Id in pod
//for Aggr, from left to right: 0, 1, 2....
//for Edge, from left to right: 0, 1, 2....
//for host, determined by the edge
uint32_t MecBackhaulCoreHelper::GetIDinPod(uint32_t nodeID) const {
	if (IsCore(nodeID)) {
		return -2;
	}

	if (IsAggr(nodeID)) {
		return (nodeID - m_maxCoreID - 1) % m_num_agg;
	} else if (IsEdge(nodeID)) {
		return (nodeID - m_maxAggrID - 1) % m_num_edge;
	} else if (IsHost(nodeID)) {
		return (((nodeID - m_maxEdgeID - 1) % (m_num_edge * m_num_host))
				/ m_num_host);
	}

	return -1;
}

void MecBackhaulCoreHelper::Print(const NodeContainer *nodeContainer,
		std::string nodeName, std::ostream& os) const {
	//Check Aggr
	for (NodeContainer::Iterator it = nodeContainer->Begin();
			it != nodeContainer->End(); ++it) {
		Ptr<Node> node = *it;
		Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		for (uint32_t j = 1; j < m_ipv4->GetNInterfaces(); j++) {
			for (uint32_t i = 0; i < m_ipv4->GetNAddresses(j); i++) {

				os << nodeName << node->GetId() << ", Local Address(" << j
						<< "," << i << ") "
						<< m_ipv4->GetAddress(j, i).GetLocal();

				if (nodeContainer != &m_Core) {

					os << ", Pod ID:" << GetPodID(node->GetId())
							<< ", Id in Pod:" << GetIDinPod(node->GetId());

				}

				os << std::endl;
			}
		}
	}
	std::cout << std::endl;
}

void MecBackhaulCoreHelper::Print(std::ostream& os) const {
	Print(&m_Core, "Code Node:", os);	//Check Node
	Print(&m_Aggr, "Aggr Node:", os);	//Check Aggr
	Print(&m_Edge, "Edge Node:", os);	//Check Edge
	Print(&m_Host, "Host Node:", os);	//Check Host

}

char *MecBackhaulCoreHelper::Num2IPv4String(uint32_t first, uint32_t second,
		uint32_t third, uint32_t fourth) const {

	char *address = new char[30];
	char firstOctet[30], secondOctet[30], thirdOctet[30], fourthOctet[30];

	bzero(address, 30);

	snprintf(firstOctet, 10, "%d", first);
	strcat(firstOctet, ".");
	snprintf(secondOctet, 10, "%d", second);
	strcat(secondOctet, ".");
	snprintf(thirdOctet, 10, "%d", third);
	strcat(thirdOctet, ".");
	snprintf(fourthOctet, 10, "%d", fourth);

	strcat(thirdOctet, fourthOctet);
	strcat(secondOctet, thirdOctet);
	strcat(firstOctet, secondOctet);
	strcat(address, firstOctet);

	return address;
}

bool MecBackhaulCoreHelper::IsCore(uint32_t nodeID) const {
	if (nodeID <= m_maxCoreID)
		return true;
	return false;
}

bool MecBackhaulCoreHelper::IsAggr(uint32_t nodeID) const {
	if (nodeID > m_maxCoreID && nodeID <= m_maxAggrID)
		return true;
	return false;
}

bool MecBackhaulCoreHelper::IsEdge(uint32_t nodeID) const {
	if (nodeID > m_maxAggrID && nodeID <= m_maxEdgeID)
		return true;
	return false;
}
bool MecBackhaulCoreHelper::IsHost(uint32_t nodeID) const {
	if (nodeID > m_maxEdgeID && nodeID <= m_maxHostID)
		return true;
	return false;
}

NodeContainer& MecBackhaulCoreHelper::AllNodes() {
	return m_AllNode;
}
NodeContainer& MecBackhaulCoreHelper::CoreNodes() {
	return m_Core;
}
NodeContainer& MecBackhaulCoreHelper::AggrNodes() {
	return m_Aggr;
}
NodeContainer& MecBackhaulCoreHelper::EdgeNodes() {
	return m_Edge;
}
NodeContainer& MecBackhaulCoreHelper::HostNodes() {
	return m_Host;
}

bool MecBackhaulCoreHelper::CheckConnectivity(uint32_t nodeID1,
		uint32_t nodeID2) {

	//either one is Aggr or Core, it's OK
	if ((IsAggr(nodeID1) || IsAggr(nodeID2))
			|| (IsCore(nodeID1) || IsCore(nodeID2)))
		return true;

	int pod1 = GetPodID(nodeID1);
	int pod2 = GetPodID(nodeID2);

	//nodes within the same pod can reach each other
	if (pod1 == pod2)
		return true;

	int numPod = m_sizeK;

	if ((pod1 < numPod / 2 && pod2 < numPod / 2)
			|| (pod1 >= numPod / 2 && pod2 >= numPod / 2)) {
		return true;
	} else {
		return false;
	}

	return true;
}

} //namespace
