/*
 * mec-backhaul-core-helper.h
 *
 *  Created on: 6 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_BACKHAUL_CORE_HELPER_H_
#define MEC_BACKHAUL_CORE_HELPER_H_

#include <ns3/ipv4-address-helper.h>
#include "ns3/node-container.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"

namespace ns3 {

class NodeContainer;
class InternetStackHelper;
class Ipv4AddressHelper;
class DcFatTreeHelper;
class PointToPointHelper;

class MecBackhaulCoreHelper: public Object {

private:

	uint32_t CreateNodeContainers(NodeContainer *node_container_temp,
			NodeContainer *node_container, int total_number_of_containers,
			int number_of_nodes_per_container, InternetStackHelper *internet);

	void SetupNodeCointainers(NodeContainer, NodeContainer, NodeContainer,
			NodeContainer);

	void AssignSubset(int second_octet, int third_octet, int fourth_octet,
			Ipv4AddressHelper *address, Ipv4InterfaceContainer *ipContainer,
			NetDeviceContainer *netContainer);

	void ConnectingCoreToAggregate(int k, int num_group, int num_core,
			int num_pod, NodeContainer *core, NodeContainer *agg,
			PointToPointHelper *p2p, Ipv4AddressHelper *address);

	void ConnectingAggregateToEdge(int k, int num_pod, int num_agg,
			int num_edge, NodeContainer *agg, NodeContainer *edge,
			PointToPointHelper *p2p, Ipv4AddressHelper *address);

	void ConnectingEdgeToHost(int num_pod, int num_edge, int num_host,
			NodeContainer *edge, NodeContainer *host, PointToPointHelper *p2p,
			Ipv4AddressHelper *address);

	void Initialise(uint32_t size);

	char *Num2IPv4String(uint32_t first, uint32_t second, uint32_t third,
			uint32_t fourth) const;

	uint32_t GetPodID(uint32_t nodeID) const;
	uint32_t GetIDinPod(uint32_t nodeID) const;

	void Print(const NodeContainer *nodeContainer, std::string nodeName,
			std::ostream& os) const;

	NodeContainer m_AllNode;	// all nodes
	NodeContainer m_Core;		// core nodes
	NodeContainer m_Aggr;		// aggregation nodes
	NodeContainer m_Edge;		// edge nodes
	NodeContainer m_Host;		// hosts nodes

	uint32_t m_sizeK = 4; 	// number of ports per switch
	uint32_t m_num_edge = 0;	// number of edge switch in a pod
	uint32_t m_num_agg = 0;	// number of aggregation switch in a pod
	uint32_t m_num_host = 0;	// number of hosts under a switch

	uint32_t m_maxCoreID = 0;	// last ID of a core node
	uint32_t m_maxAggrID = 0;	// last ID of an aggregation node
	uint32_t m_maxEdgeID = 0;	// last ID of an edge node
	uint32_t m_maxHostID = 0;	// last ID of a host node

public:
	static TypeId GetTypeId();

	MecBackhaulCoreHelper();
	virtual ~MecBackhaulCoreHelper();

	bool CheckConnectivity(uint32_t nodeID1, uint32_t nodeID2);

	NodeContainer& AllNodes();
	NodeContainer& CoreNodes();
	NodeContainer& AggrNodes();
	NodeContainer& EdgeNodes();
	NodeContainer& HostNodes();

	bool IsCore(uint32_t nodeID) const;
	bool IsAggr(uint32_t nodeID) const;
	bool IsEdge(uint32_t nodeID) const;
	bool IsHost(uint32_t nodeID) const;

	void Create(uint32_t size);

	void Print(std::ostream& os) const;

};

}	// ns3 namespace

#endif /* MEC_BACKHAUL_CORE_HELPER_H_ */
