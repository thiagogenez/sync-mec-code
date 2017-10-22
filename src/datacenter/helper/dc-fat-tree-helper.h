#ifndef DC_FAT_TREE_HELPER_H
#define DC_FAT_TREE_HELPER_H

//#include "ns3/type-id.h"

#include "ns3/net-device.h"
#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/core-module.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"

#include "ns3/policy-routing-helper.h"

namespace ns3 {

enum FatTreeNodeType {
	CORE, AGGREAATE, EDGE, BRIDGE, HOST, OTHER
};

class DcFatTreeHelper: public Object {
public:
	static TypeId GetTypeId(void);
	DcFatTreeHelper();
	DcFatTreeHelper(unsigned n);
	~DcFatTreeHelper();

	char * Num2IPv4String(int a, int b, int c, int d);
	virtual void Create(void);
	void CreateOLD(void);

	FatTreeNodeType getFatTreeNodeType(Ptr<Node>);

	// Functions to retrieve nodes and interfaces
	NodeContainer& AllNodes(void) {
		return m_AllNode;
	}
	;
	NodeContainer& CoreNodes(void) {
		return m_Core;
	}
	;
	NodeContainer& AggrNodes(void) {
		return m_Aggr;
	}
	;
	NodeContainer& EdgeNodes(void) {
		return m_Edge;
	}
	;
	NodeContainer& HostNodes(void) {
		return m_Host;
	}
	;

	//add by cuilin
	bool IsHost(uint32_t i);
	bool IsBridge(uint32_t i);
	bool IsEdge(uint32_t i);
	bool IsAggr(uint32_t i);
	bool IsCore(uint32_t i);

	int GetPodID(uint32_t i);
	int GetIDinPod(uint32_t nodeID);
	int GetCoreGroupID(uint32_t nodeID);

	uint32_t GetFatTreeSize();

	bool checkConnectivity(uint32_t nodeID1, uint32_t nodeID2);

	void Print();
	void Test();

protected:
	Ptr<UniformRandomVariable> m_uniformRandomVariable;

	// Parameters
	unsigned m_sizeK;
	DataRate m_heRate;
	DataRate m_eaRate;
	DataRate m_acRate;
	Time m_heDelay;
	Time m_eaDelay;
	Time m_acDelay;

	NodeContainer m_AllNode;
	NodeContainer m_Core;
	NodeContainer m_Aggr;
	NodeContainer m_Edge;
	NodeContainer m_Bridge;
	NodeContainer m_Host;

	uint32_t m_maxCoreID;
	uint32_t m_maxAggrID;
	uint32_t m_maxEdgeID;
	uint32_t m_maxBridgeID;
	uint32_t m_maxHostID;

	//Ipv4InterfaceContainer	m_hostIface;
	//Ipv4InterfaceContainer	m_edgeIface;
	//Ipv4InterfaceContainer	m_aggrIface;
	//Ipv4InterfaceContainer	m_coreIface;
	//ObjectFactory	m_channelFactory;

};
}
;

#endif
