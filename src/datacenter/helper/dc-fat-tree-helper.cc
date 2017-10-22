//#define __STDC_LIMIT_MACROS 1
#include <sstream>
#include <stdint.h>
#include <stdlib.h>

#include "ns3/bridge-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-nix-vector-helper.h"

#include "dc-fat-tree-helper.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("DcFatTreeHelper");

NS_OBJECT_ENSURE_REGISTERED(DcFatTreeHelper);

//unsigned DcFatTreeHelper::m_size = 0;	// Defining static variable

TypeId DcFatTreeHelper::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::DcFatTreeHelper").SetParent<Object>().AddAttribute(
					"HeDataRate",
					"The default data rate for point to point links",
					DataRateValue(DataRate("1000Mbps")),
					MakeDataRateAccessor(&DcFatTreeHelper::m_heRate),
					MakeDataRateChecker()).AddAttribute("EaDataRate",
					"The default data rate for point to point links",
					DataRateValue(DataRate("1000Mbps")),
					MakeDataRateAccessor(&DcFatTreeHelper::m_eaRate),
					MakeDataRateChecker()).AddAttribute("AcDataRate",
					"The default data rate for point to point links",
					DataRateValue(DataRate("1000Mbps")),
					MakeDataRateAccessor(&DcFatTreeHelper::m_acRate),
					MakeDataRateChecker()).AddAttribute("HeDelay",
					"Transmission delay through the channel",
					TimeValue(NanoSeconds(500)),
					MakeTimeAccessor(&DcFatTreeHelper::m_heDelay),
					MakeTimeChecker()).AddAttribute("EaDelay",
					"Transmission delay through the channel",
					TimeValue(NanoSeconds(500)),
					MakeTimeAccessor(&DcFatTreeHelper::m_eaDelay),
					MakeTimeChecker()).AddAttribute("AcDelay",
					"Transmission delay through the channel",
					TimeValue(NanoSeconds(500)),
					MakeTimeAccessor(&DcFatTreeHelper::m_acDelay),
					MakeTimeChecker());

	return tid;
}

DcFatTreeHelper::DcFatTreeHelper(unsigned K) {
	m_uniformRandomVariable = CreateObject<UniformRandomVariable>();

	m_sizeK = K;

	std::cout << "Create a fat-tree with size " << K << std::endl;
	//m_channelFactory.SetTypeId ("ns3::PointToPointChannel");
}


DcFatTreeHelper::DcFatTreeHelper() {
}


DcFatTreeHelper::~DcFatTreeHelper() {
}

// Function to create address string from numbers
//
char * DcFatTreeHelper::Num2IPv4String(int a, int b, int c, int d) {

	int first = a;
	int second = b;
	int third = c;
	int fourth = d;

	char *address = new char[30];
	char firstOctet[30], secondOctet[30], thirdOctet[30], fourthOctet[30];
	//address = firstOctet.secondOctet.thirdOctet.fourthOctet;

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

/* Create the whole topology */
void DcFatTreeHelper::Create() {
	int k = m_sizeK;			// number of ports per switch
	int num_pod = k;		// number of pod
	int num_host = (k / 2);		// number of hosts under a switch
	int num_edge = (k / 2);		// number of edge switch in a pod
	int num_bridge = num_edge;	// number of bridge in a pod
	int num_agg = (k / 2);		// number of aggregation switch in a pod
	int num_group = k / 2;		// number of group of core switches
	int num_core = (k / 2);		// number of core switch in a group
	//int total_host = k*k*k/4;	// number of hosts in the entire network	

	int i = 0;
	int j = 0;
	int h = 0;

	Ptr<Node> nodeTemp;

	char dataRate[] = "1000Mbps";	// 1Gbps
	float delay = 0.001;		// 0.001 ms

// Initialize Internet Stack and Routing Protocols
//	
	//set ECMP
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting",
			BooleanValue(true));

	InternetStackHelper internet;
	//Ipv4NixVectorHelper nixRouting; 
	PolicyRoutingHelper policyRouting;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	//list.Add (nixRouting, 10);	
	list.Add(policyRouting, 10);
	internet.SetRoutingHelper(list);
	
//=========== Creation of Node Containers ===========//
//
	NodeContainer core[num_group];			// NodeContainer for core switches
	for (i = 0; i < num_group; i++) {
		core[i].Create(num_core);
		internet.Install(core[i]);

		m_Core.Add(core[i]);
	}
	nodeTemp = m_Core.Get(m_Core.GetN() - 1);				//*(it--);  //
	m_maxCoreID = nodeTemp->GetId(); //Max CoreID

	//std::cout<<"Test OK for core, m_maxCoreID = "<<m_maxCoreID<<std::endl;

	NodeContainer agg[num_pod]; 	// NodeContainer for aggregation switches
	for (i = 0; i < num_pod; i++) {
		agg[i].Create(num_agg);
		internet.Install(agg[i]);

		m_Aggr.Add(agg[i]);
	}
	nodeTemp = m_Aggr.Get(m_Aggr.GetN() - 1); 			//*(m_Aggr.End());
	m_maxAggrID = nodeTemp->GetId(); //Max AggrID

	//std::cout<<"Test OK for aggr, m_maxAggrID="<<m_maxAggrID<<std::endl;

	NodeContainer edge[num_pod];			// NodeContainer for edge switches
	for (i = 0; i < num_pod; i++) {
		edge[i].Create(num_bridge);
		internet.Install(edge[i]);

		m_Edge.Add(edge[i]);
	}
	nodeTemp = m_Edge.Get(m_Edge.GetN() - 1);				//*(m_Edge.End());
	m_maxEdgeID = nodeTemp->GetId(); //Max CoreID

	//std::cout<<"Test OK for edge, m_maxEdgeID="<<m_maxEdgeID<<std::endl;

	/*
	 NodeContainer bridge[num_pod];				// NodeContainer for edge bridges
	 for (i=0; i<num_pod;i++){
	 bridge[i].Create (num_bridge);
	 internet.Install (bridge[i]);

	 m_Bridge.Add(bridge[i]);
	 }
	 nodeTemp = m_Bridge.Get(m_Bridge.GetN()-1);//(m_Bridge.End());
	 m_maxBridgeID = nodeTemp->GetId();
	 */
	m_maxBridgeID = m_maxEdgeID;

	NodeContainer host[num_pod][num_edge];		// NodeContainer for hosts
	for (i = 0; i < k; i++) {
		for (j = 0; j < num_edge; j++) {
			host[i][j].Create(num_host);
			internet.Install(host[i][j]);

			m_Host.Add(host[i][j]);
		}
	}
	nodeTemp = m_Host.Get(m_Host.GetN() - 1);		//*(m_Host.End());
	m_maxHostID = nodeTemp->GetId();

	m_AllNode.Add(m_Core);
	m_AllNode.Add(m_Aggr);
	m_AllNode.Add(m_Edge);
	//m_AllNode.Add(m_Bridge);
	m_AllNode.Add(m_Host);

// Inintialize Address Helper
//	
	Ipv4AddressHelper address;

// Initialize PointtoPoint helper
//	
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue(dataRate));
	p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));

// Initialize Csma helper
//
	//CsmaHelper csma;
	//csma.SetChannelAttribute ("DataRate", StringValue (dataRate));
	//csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));

//=========== Connect edge switches to hosts ===========//
//	
	NetDeviceContainer he[num_pod][num_edge][num_host];
	Ipv4InterfaceContainer ipHeContainer[num_pod][num_edge][num_host];
	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_edge; j++) {
			for (h = 0; h < num_host; h++) {
				he[i][j][h] = p2p.Install(edge[i].Get(j), host[i][j].Get(h));

				int second_octet = i;
				int third_octet = j;
				int fourth_octet;
				if (h == 0)
					fourth_octet = 1;
				else
					fourth_octet = h * 2 + 1;

				//Assign subnet
				char *subnet;
				subnet = Num2IPv4String(10, second_octet, third_octet, 0);
				char *base;
				base = Num2IPv4String(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipHeContainer[i][j][h] = address.Assign(he[i][j][h]);
			}
		}
	}

	std::cout << "Finished connecting edge switches and hosts  " << "\n";

//=========== Connect aggregate switches to edge switches ===========//
//
	NetDeviceContainer ae[num_pod][num_agg][num_edge];
	Ipv4InterfaceContainer ipAeContainer[num_pod][num_agg][num_edge];
	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_agg; j++) {
			for (h = 0; h < num_edge; h++) {
				ae[i][j][h] = p2p.Install(agg[i].Get(j), edge[i].Get(h));

				int second_octet = i;
				int third_octet = j + (k / 2);
				int fourth_octet;
				if (h == 0)
					fourth_octet = 1;
				else
					fourth_octet = h * 2 + 1;
				//Assign subnet
				char *subnet;
				subnet = Num2IPv4String(10, second_octet, third_octet, 0);
				//Assign base
				char *base;
				base = Num2IPv4String(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipAeContainer[i][j][h] = address.Assign(ae[i][j][h]);
			}
		}
	}
	std::cout << "Finished connecting aggregation switches and edge switches  "
			<< "\n";

//=========== Connect core switches to aggregate switches ===========//
//
	NetDeviceContainer ca[num_group][num_core][num_pod];
	Ipv4InterfaceContainer ipCaContainer[num_group][num_core][num_pod];
	int fourth_octet = 1;

	for (i = 0; i < num_group; i++) {
		for (j = 0; j < num_core; j++) {
			fourth_octet = 1;
			for (h = 0; h < num_pod; h++) {
				ca[i][j][h] = p2p.Install(core[i].Get(j), agg[h].Get(i));

				int second_octet = k + i;
				int third_octet = j;
				//Assign subnet
				char *subnet;
				subnet = Num2IPv4String(10, second_octet, third_octet, 0);
				//Assign base
				char *base;
				base = Num2IPv4String(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipCaContainer[i][j][h] = address.Assign(ca[i][j][h]);
				fourth_octet += 2;
			}
		}
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	std::cout << "Complete creating fattree" << std::endl;
	//Print();

} // DcFatTreeHelper::Create()

/* Create the whole topology */
void DcFatTreeHelper::CreateOLD() {
	int k = m_sizeK;			// number of ports per switch
	int num_pod = k;		// number of pod
	int num_host = (k / 2);		// number of hosts under a switch
	int num_edge = (k / 2);		// number of edge switch in a pod
	int num_bridge = num_edge;	// number of bridge in a pod
	int num_agg = (k / 2);		// number of aggregation switch in a pod
	int num_group = k / 2;		// number of group of core switches
	int num_core = (k / 2);		// number of core switch in a group
	//int total_host = k*k*k/4;	// number of hosts in the entire network	

	int i = 0;
	int j = 0;
	int h = 0;

	Ptr<Node> nodeTemp;

	char dataRate[] = "1000Mbps";	// 1Gbps
	float delay = 0.001;		// 0.001 ms

// Initialize Internet Stack and Routing Protocols
//	
	//set ECMP
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting",
			BooleanValue(true));

	InternetStackHelper internet;
	//Ipv4NixVectorHelper nixRouting; 
	PolicyRoutingHelper policyRouting;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	//list.Add (nixRouting, 10);	
	list.Add(policyRouting, 10);
	internet.SetRoutingHelper(list);

//=========== Creation of Node Containers ===========//
//
	NodeContainer core[num_group];			// NodeContainer for core switches
	for (i = 0; i < num_group; i++) {
		core[i].Create(num_core);
		internet.Install(core[i]);

		m_Core.Add(core[i]);
	}
	nodeTemp = m_Core.Get(m_Core.GetN() - 1);				//*(it--);  //
	m_maxCoreID = nodeTemp->GetId(); //Max CoreID

	//std::cout<<"Test OK for core, m_maxCoreID = "<<m_maxCoreID<<std::endl;

	NodeContainer agg[num_pod]; 	// NodeContainer for aggregation switches
	for (i = 0; i < num_pod; i++) {
		agg[i].Create(num_agg);
		internet.Install(agg[i]);

		m_Aggr.Add(agg[i]);
	}
	nodeTemp = m_Aggr.Get(m_Aggr.GetN() - 1); 			//*(m_Aggr.End());
	m_maxAggrID = nodeTemp->GetId(); //Max AggrID

	//std::cout<<"Test OK for aggr, m_maxAggrID="<<m_maxAggrID<<std::endl;

	NodeContainer edge[num_pod];			// NodeContainer for edge switches
	for (i = 0; i < num_pod; i++) {
		edge[i].Create(num_bridge);
		internet.Install(edge[i]);

		m_Edge.Add(edge[i]);
	}
	nodeTemp = m_Edge.Get(m_Edge.GetN() - 1);				//*(m_Edge.End());
	m_maxEdgeID = nodeTemp->GetId(); //Max CoreID

	//std::cout<<"Test OK for edge, m_maxEdgeID="<<m_maxEdgeID<<std::endl;

	NodeContainer bridge[num_pod];			// NodeContainer for edge bridges
	for (i = 0; i < num_pod; i++) {
		bridge[i].Create(num_bridge);
		internet.Install(bridge[i]);

		m_Bridge.Add(bridge[i]);
	}
	nodeTemp = m_Bridge.Get(m_Bridge.GetN() - 1);			//*(m_Bridge.End());
	m_maxBridgeID = nodeTemp->GetId();

	NodeContainer host[num_pod][num_bridge];		// NodeContainer for hosts
	for (i = 0; i < k; i++) {
		for (j = 0; j < num_bridge; j++) {
			host[i][j].Create(num_host);
			internet.Install(host[i][j]);

			m_Host.Add(host[i][j]);
		}
	}
	nodeTemp = m_Host.Get(m_Host.GetN() - 1);		//*(m_Host.End());
	m_maxHostID = nodeTemp->GetId();

	m_AllNode.Add(m_Core);
	m_AllNode.Add(m_Aggr);
	m_AllNode.Add(m_Edge);
	m_AllNode.Add(m_Bridge);
	m_AllNode.Add(m_Host);

// Inintialize Address Helper
//	
	Ipv4AddressHelper address;

// Initialize PointtoPoint helper
//	
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue(dataRate));
	p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));

// Initialize Csma helper
//
	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue(dataRate));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));

//=========== Connect edge switches to hosts ===========//
//	
	NetDeviceContainer hostSw[num_pod][num_bridge];
	NetDeviceContainer bridgeDevices[num_pod][num_bridge];
	Ipv4InterfaceContainer ipContainer[num_pod][num_bridge];

	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_bridge; j++) {
			NetDeviceContainer link1 = csma.Install(
					NodeContainer(edge[i].Get(j), bridge[i].Get(j)));
			hostSw[i][j].Add(link1.Get(0));
			bridgeDevices[i][j].Add(link1.Get(1));

			for (h = 0; h < num_host; h++) {
				NetDeviceContainer link2 = csma.Install(
						NodeContainer(host[i][j].Get(h), bridge[i].Get(j)));
				hostSw[i][j].Add(link2.Get(0));
				bridgeDevices[i][j].Add(link2.Get(1));
			}

			BridgeHelper bHelper;
			bHelper.Install(bridge[i].Get(j), bridgeDevices[i][j]);
			//Assign address
			char *subnet;
			subnet = Num2IPv4String(10, i, j, 0);
			address.SetBase(subnet, "255.255.255.0");
			ipContainer[i][j] = address.Assign(hostSw[i][j]);
		}
	}
	std::cout << "Finished connecting edge switches and hosts  " << "\n";

//=========== Connect aggregate switches to edge switches ===========//
//
	NetDeviceContainer ae[num_pod][num_agg][num_edge];
	Ipv4InterfaceContainer ipAeContainer[num_pod][num_agg][num_edge];
	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_agg; j++) {
			for (h = 0; h < num_edge; h++) {
				ae[i][j][h] = p2p.Install(agg[i].Get(j), edge[i].Get(h));

				int second_octet = i;
				int third_octet = j + (k / 2);
				int fourth_octet;
				if (h == 0)
					fourth_octet = 1;
				else
					fourth_octet = h * 2 + 1;
				//Assign subnet
				char *subnet;
				subnet = Num2IPv4String(10, second_octet, third_octet, 0);
				//Assign base
				char *base;
				base = Num2IPv4String(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipAeContainer[i][j][h] = address.Assign(ae[i][j][h]);
			}
		}
	}
	std::cout << "Finished connecting aggregation switches and edge switches  "
			<< "\n";

//=========== Connect core switches to aggregate switches ===========//
//
	NetDeviceContainer ca[num_group][num_core][num_pod];
	Ipv4InterfaceContainer ipCaContainer[num_group][num_core][num_pod];
	int fourth_octet = 1;

	for (i = 0; i < num_group; i++) {
		for (j = 0; j < num_core; j++) {
			fourth_octet = 1;
			for (h = 0; h < num_pod; h++) {
				ca[i][j][h] = p2p.Install(core[i].Get(j), agg[h].Get(i));

				int second_octet = k + i;
				int third_octet = j;
				//Assign subnet
				char *subnet;
				subnet = Num2IPv4String(10, second_octet, third_octet, 0);
				//Assign base
				char *base;
				base = Num2IPv4String(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipCaContainer[i][j][h] = address.Assign(ca[i][j][h]);
				fourth_octet += 2;
			}
		}
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	std::cout << "Complete creating fattree" << std::endl;
	//Print();

} // DcFatTreeHelper::Create()

bool DcFatTreeHelper::IsCore(uint32_t i) {
	if (i <= m_maxCoreID)
		return true;
	else
		return false;
}
bool DcFatTreeHelper::IsAggr(uint32_t i) {
	if (i > m_maxCoreID && i <= m_maxAggrID)
		return true;
	else
		return false;
}
bool DcFatTreeHelper::IsEdge(uint32_t i) {
	if (i > m_maxAggrID && i <= m_maxEdgeID)
		return true;
	else
		return false;
}
bool DcFatTreeHelper::IsBridge(uint32_t i) {
	if (i > m_maxEdgeID && i <= m_maxBridgeID)
		return true;
	else
		return false;
}
bool DcFatTreeHelper::IsHost(uint32_t i) {
	if (i > m_maxBridgeID && i <= m_maxHostID)
		return true;
	else
		return false;
}

//Pod ID, from left to right: 0, 1, 2, ...
int DcFatTreeHelper::GetPodID(uint32_t nodeID) {
	// subtree is only for host, edge and aggr
	if (IsCore(nodeID)) {
		return -2;
	}

	int num_edge = (m_sizeK / 2);		// number of edge switch in a pod
	//int num_bridge = num_edge;	// number of bridge in a pod
	int num_agg = (m_sizeK / 2);		// number of aggregation switch in a pod
	int num_host = (m_sizeK / 2);		// number of hosts under a switch

	int st = -1;

	if (IsAggr(nodeID)) {
		st = (nodeID - m_maxCoreID - 1) / num_agg;
	} else if (IsEdge(nodeID)) {
		st = (nodeID - m_maxAggrID - 1) / num_edge;
	} else if (IsHost(nodeID)) {
		st = (nodeID - m_maxBridgeID - 1) / (num_edge * num_host);
	}

	return st;
}

//Id in pod
//for Aggr, from left to right: 0, 1, 2....
//for Edge, from left to right: 0, 1, 2....
//for host, determined by the edge
int DcFatTreeHelper::GetIDinPod(uint32_t nodeID) {
	if (IsCore(nodeID)) {
		return -2;
	}

	int num_edge = (m_sizeK / 2);		// number of edge switch in a pod
	//int num_bridge = num_edge;	// number of bridge in a pod
	int num_agg = (m_sizeK / 2);		// number of aggregation switch in a pod
	int num_host = (m_sizeK / 2);		// number of hosts under a switch

	int id = -1;
	if (IsAggr(nodeID)) {
		id = (nodeID - m_maxCoreID - 1) % num_agg;
	} else if (IsEdge(nodeID)) {
		id = (nodeID - m_maxAggrID - 1) % num_edge;
	} else if (IsHost(nodeID)) {
		id = (nodeID - m_maxBridgeID - 1) % (num_edge * num_host);
		id = id / num_host;
	}

	return id;
}

//GroupID of core, from left to right: {first k/2 is 0}{next k/2 is 1}...
//all Agg with id=x within each pod connect to all Cores in group x
int DcFatTreeHelper::GetCoreGroupID(uint32_t nodeID) {
	if (!IsCore(nodeID)) {
		return -2;
	}

	//int num_group = m_sizeK/2;		// number of group of core switches
	int num_core = (m_sizeK / 2);		// number of core switch in a group

	int groupID = -1;
	groupID = nodeID / num_core;

	return groupID;
}

uint32_t DcFatTreeHelper::GetFatTreeSize() {
	return m_sizeK;
}

//check wheter two nodes are reachable to each other
//used for policy violation
bool DcFatTreeHelper::checkConnectivity(uint32_t nodeID1, uint32_t nodeID2) {

	//either one is Aggr or Core, it's OK
	if ((IsAggr(nodeID1) || IsAggr(nodeID2))
			|| (IsCore(nodeID1) || IsCore(nodeID2)))
		return true;

	int pod1 = GetPodID(nodeID1);
	int pod2 = GetPodID(nodeID2);

	//nodes within the same pod can reach each other
	if (pod1 == pod2)
		return true;

	int numPod = GetFatTreeSize();

	//MB on Edge, and server, must within the same pod
	//if( (IsEdge(nodeID1) && IsHost(nodeID2))
	//	|| (IsHost(nodeID1) && IsEdge(nodeID2)))
	//{
	//	return false;
	//}

	if ((pod1 < numPod / 2 && pod2 < numPod / 2)
			|| (pod1 >= numPod / 2 && pod2 >= numPod / 2)) {
		return true;
	} else
		return false;

	return true;
}

void DcFatTreeHelper::Test() {
	uint32_t id = 0;
	//Check Core
	for (NodeContainer::Iterator it = m_Core.Begin(); it != m_Core.End();
			++it) {
		Ptr<Node> node = *it;
		id = node->GetId();
		if (IsCore(id) && !IsAggr(id) && !IsEdge(id) && !IsBridge(id)
				&& !IsHost(id))
			continue;
		else
			std::cout << "ERROR for Core node ID: " << id << std::endl;
	}

	//Check Aggr
	for (NodeContainer::Iterator it = m_Aggr.Begin(); it != m_Aggr.End();
			++it) {
		Ptr<Node> node = *it;
		id = node->GetId();
		if (!IsCore(id) && IsAggr(id) && !IsEdge(id) && !IsBridge(id)
				&& !IsHost(id))
			continue;
		else
			std::cout << "ERROR for Aggr node ID: " << id << std::endl;
	}

	//Check Edge
	for (NodeContainer::Iterator it = m_Edge.Begin(); it != m_Edge.End();
			++it) {
		Ptr<Node> node = *it;
		id = node->GetId();
		if (!IsCore(id) && !IsAggr(id) && IsEdge(id) && !IsBridge(id)
				&& !IsHost(id))
			continue;
		else
			std::cout << "ERROR for Edge node ID: " << id << std::endl;
	}

	//Check Bridge
	for (NodeContainer::Iterator it = m_Bridge.Begin(); it != m_Bridge.End();
			++it) {
		Ptr<Node> node = *it;
		id = node->GetId();
		if (!IsCore(id) && !IsAggr(id) && !IsEdge(id) && IsBridge(id)
				&& !IsHost(id))
			continue;
		else
			std::cout << "ERROR for bridge node ID: " << id << std::endl;
	}

	//Check Host
	for (NodeContainer::Iterator it = m_Host.Begin(); it != m_Host.End();
			++it) {
		Ptr<Node> node = *it;
		id = node->GetId();
		if (!IsCore(id) && !IsAggr(id) && !IsEdge(id) && !IsBridge(id)
				&& IsHost(id))
			continue;
		else
			std::cout << "ERROR for Host node ID: " << id << std::endl;
	}

	std::cout << "All Test Passed OK " << std::endl;
}

void DcFatTreeHelper::Print() {
	for (NodeContainer::Iterator it = m_Core.Begin(); it != m_Core.End();
			++it) {
		Ptr<Node> node = *it;
		Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		for (uint32_t j = 1; j < m_ipv4->GetNInterfaces(); j++) {
			for (uint32_t i = 0; i < m_ipv4->GetNAddresses(j); i++) {
				std::cout << "Core Node:" << node->GetId() << ", Local Address("
						<< j << "," << i << ") "
						<< m_ipv4->GetAddress(j, i).GetLocal() << std::endl;
			}
		}
	}

	//Check Aggr
	for (NodeContainer::Iterator it = m_Aggr.Begin(); it != m_Aggr.End();
			++it) {
		Ptr<Node> node = *it;
		Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		for (uint32_t j = 1; j < m_ipv4->GetNInterfaces(); j++) {
			for (uint32_t i = 0; i < m_ipv4->GetNAddresses(j); i++) {
				std::cout << "Aggr Node:" << node->GetId() << ", Local Address("
						<< j << "," << i << ") "
						<< m_ipv4->GetAddress(j, i).GetLocal() << ", Pod ID:"
						<< GetPodID(node->GetId()) << ", Id in Pod:"
						<< GetIDinPod(node->GetId()) << std::endl;
			}
		}
	}

	//Check Edge
	for (NodeContainer::Iterator it = m_Edge.Begin(); it != m_Edge.End();
			++it) {
		Ptr<Node> node = *it;
		Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		for (uint32_t j = 1; j < m_ipv4->GetNInterfaces(); j++) {
			for (uint32_t i = 0; i < m_ipv4->GetNAddresses(j); i++) {
				std::cout << "Edge Node:" << node->GetId() << ", Local Address("
						<< j << "," << i << ") "
						<< m_ipv4->GetAddress(j, i).GetLocal() << ", Pod ID:"
						<< GetPodID(node->GetId()) << ", Id in Pod:"
						<< GetIDinPod(node->GetId()) << std::endl;
			}
		}
	}

	//Check Bridge
	for (NodeContainer::Iterator it = m_Bridge.Begin(); it != m_Bridge.End();
			++it) {
		Ptr<Node> node = *it;
		Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		for (uint32_t j = 1; j < m_ipv4->GetNInterfaces(); j++) {
			for (uint32_t i = 0; i < m_ipv4->GetNAddresses(j); i++) {
				std::cout << "Bridge Node:" << node->GetId()
						<< ", Local Address(" << j << "," << i << ") "
						<< m_ipv4->GetAddress(j, i).GetLocal() << std::endl;
			}
		}
	}

	//Check Host
	for (NodeContainer::Iterator it = m_Host.Begin(); it != m_Host.End();
			++it) {
		Ptr<Node> node = *it;
		Ptr<Ipv4> m_ipv4 = node->GetObject<Ipv4>();
		for (uint32_t j = 1; j < m_ipv4->GetNInterfaces(); j++) {
			for (uint32_t i = 0; i < m_ipv4->GetNAddresses(j); i++) {
				std::cout << "Host Node:" << node->GetId() << ", Local Address("
						<< j << "," << i << ") "
						<< m_ipv4->GetAddress(j, i).GetLocal() << ", Pod ID:"
						<< GetPodID(node->GetId()) << ", Id in Pod:"
						<< GetIDinPod(node->GetId()) << std::endl;
			}
		}
	}
}

}	//namespace
