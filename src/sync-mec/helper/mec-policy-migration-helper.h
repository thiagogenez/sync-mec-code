/*
 * mec-graph-helper.h
 *
 *  Created on: 7 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_GRAPH_HELPER_H_
#define MEC_GRAPH_HELPER_H_

#include <map>
#include <vector>

#include "ns3/object.h"
#include "ns3/mec-vm.h"
#include "ns3/nstime.h"

#include "ns3/mec-middle-box-container.h"
#include "ns3/mec-v4-ping.h"
#include "ns3/mec-v4-ping-helper.h"

#include "ns3/mec-graph.h"


namespace ns3 {

class MecPolicyMigrationHelper: public Object {
public:
	MecPolicyMigrationHelper();
	virtual ~MecPolicyMigrationHelper();
	static TypeId GetTypeId();

	MecMiddleBoxContainer GetNewMiddleBoxContainer();

	//setters
	void SetPolicy(Ptr<MecPolicy> policy);
	void SetFlow(Ptr<MecFlow> flow);

	void DagConstruction(uint32_t numberOfNodes, Ptr<MecVm> firstNode,
			NodeContainer *secondNodeTier,
			std::vector<MecMiddleBoxContainer>& middleNodeTiers,
			NodeContainer *penultimateNodeTier, Ptr<MecVm> lastNode);

	Ptr<Node> GetPreferableHostServerForVm();

	bool IsDagReady();
	bool IsDownloadFlow() const;
	Ptr<MecPolicy> GetPolicy();
	Ptr<MecFlow> GetFlow();
	uint32_t GetId() const;

	void SetIsDownloadFlow(bool isDownloadflow);
	void SetIsPolicyMigrationDone(bool isPolicyMigrated);
	bool IsPolicyMigrationDone() const;

	bool IsShortestPathValid();

private:

	void DagConstructionHeadPart(NodeContainer *secondNodeTier,
			uint32_t *currentDagNodeIndex);

	void DagConstructionMiddlePartA(NodeContainer *secondNodeTier,
			std::vector<MecMiddleBoxContainer>& middleNodeTiers,
			NodeContainer *penultimateNodeTier, uint32_t *currentDagNodeIndex);

	void DagConstructionMiddlePartB(NodeContainer *secondNodeTier,
			NodeContainer *penultimateNodeTier, uint32_t *currentDagNodeIndex);

	void DagConstructionTailPart(NodeContainer *penultimateNodeTier,
			uint32_t *currentDagNodeIndex);

	uint32_t DagConstructionGetDagNodeId(uint32_t *currentDagNodeIndex,
			std::map<uint32_t, uint32_t> & reverserMap, uint32_t key);

	void DagConstructionGetEdgeWeight();

	void CreateDag(uint32_t nodes);


	Ptr<MecVm> m_firstNode;
	Ptr<MecVm> m_lastNode;
	Ptr<MecGraph> m_dag;
	Ptr<MecPolicy> m_policy;
	Ptr<MecFlow> m_flow;

	std::map<uint32_t, Ptr<Node>> m_secondTierTrackIds;
	std::map<uint32_t, Ptr<Node>> m_penultimateTierTrackIds;
	std::map<uint32_t, Ptr<MecMiddleBox>> m_middleTierTrackIds;

	std::map<uint32_t, uint32_t> m_reverseSecondTierTrackIds;
	std::map<uint32_t, uint32_t> m_reversePenultimateTierTrackIds;
	std::map<uint32_t, uint32_t> m_reverseMiddleTierTrackIds;

	bool FindLowLatencyPath();

	Ptr<Node> GetRealNs3Node(uint32_t indexDagNode);

	bool m_isDownloadFlow = false;
	bool m_policyMigrationDone = false;



};

} /* namespace ns3 */

#endif /* MEC_GRAPH_HELPER_H_ */
