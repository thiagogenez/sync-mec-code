/*
 * mec-graph-helper.cc
 *
 *  Created on: 7 Jul 2017
 *      Author: thiagogenez
 */

#include <cmath>

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"
#include "ns3/ipv4.h"
#include "ns3/double.h"

#include "ns3/mec-e2e-checker.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-constants-helper.h"

#include "mec-policy-migration-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecPolicyMigrationHelper");
NS_OBJECT_ENSURE_REGISTERED(MecPolicyMigrationHelper);

TypeId MecPolicyMigrationHelper::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecPolicyMigrationHelper").SetParent<Object>().SetGroupName(
					"sync-mec");
	return tid;
}

MecPolicyMigrationHelper::MecPolicyMigrationHelper() {
	NS_LOG_FUNCTION(this);
}

MecPolicyMigrationHelper::~MecPolicyMigrationHelper() {
	NS_LOG_FUNCTION(this);
}

bool MecPolicyMigrationHelper::IsShortestPathValid() {
	std::vector<uint32_t> shortestPath = m_dag->GetShortestPath(0);
	return (shortestPath.size()
			== ((uint32_t) m_policy->GetLen() + (uint32_t) 4));
}

MecMiddleBoxContainer MecPolicyMigrationHelper::GetNewMiddleBoxContainer() {

	//std::cout << "Pointer: " << m_dag << std::endl;
	MecMiddleBoxContainer newContainer;
	std::vector<uint32_t> shortestPath = m_dag->GetShortestPath(0);

//	std::cout << "Shortest path: "
//			<< MecUtilsHelper::VectorToString(shortestPath.begin(),
//					shortestPath.end()) << std::endl;
//
//	m_dag->Print(std::cout);

	NS_ASSERT_MSG(
			shortestPath.size()
					== (uint32_t ) m_policy->GetLen() + (uint32_t )4,
			"ERROR: incorrect shortest path length: shortestPath.size()="<<shortestPath.size()<<", m_policy->GetLen() +  4 ="<<m_policy->GetLen() + 4);

	// since 0 is the first node (ue or vm) and 1 is the host server (or the "base station")
	uint32_t offset = 2;
	for (uint32_t i = 0; i < m_policy->GetLen(); i++) {

		uint32_t virtualId = shortestPath[i + offset];
		Ptr<MecMiddleBox> newMiddlebox = m_middleTierTrackIds[virtualId];

		NS_ASSERT_MSG(
				newMiddlebox->GetMiddleBoxType()
						== m_policy->GetMiddleBoxType(i),
				"ERROR: get a wrong MB; newMiddlebox->GetMiddleBoxType()="<<newMiddlebox->GetMiddleBoxType() << "; m_policy->GetMiddleBoxType(i)="<<m_policy->GetMiddleBoxType(i));

		newContainer.Add(newMiddlebox);
	}

	return newContainer;
}

Ptr<Node> MecPolicyMigrationHelper::GetPreferableHostServerForVm() {
	std::vector<uint32_t> shortestPath = m_dag->GetShortestPath(0);

//	std::cout << "Shortest path: "
//			<< MecUtilsHelper::VectorToString(shortestPath.begin(),
//					shortestPath.end()) << std::endl;

	if (m_isDownloadFlow) {
		return m_secondTierTrackIds[shortestPath[1]];
	}

	return m_penultimateTierTrackIds[shortestPath[shortestPath.size() - 2]];
}

Ptr<Node> MecPolicyMigrationHelper::GetRealNs3Node(uint32_t indexDagNode) {

	// speed up the find process used bellow
	std::map<uint32_t, Ptr<Node>>::iterator secondTierIt;
	std::map<uint32_t, Ptr<Node>>::iterator penultimateTierIt;
	std::map<uint32_t, Ptr<MecMiddleBox>>::iterator middleTierIt;

	secondTierIt = m_secondTierTrackIds.find(indexDagNode);
	penultimateTierIt = m_penultimateTierTrackIds.find(indexDagNode);
	middleTierIt = m_middleTierTrackIds.find(indexDagNode);

	// if the indexNode represents the first or the last DAG node
	if (indexDagNode == 0 || indexDagNode == m_dag->GetSize() - 1) {

		//first node
		if (indexDagNode == 0) {
			return MecUtilsHelper::GetNode(m_firstNode->GetHostServerId());
		}

		//last node
		else if (m_dag->GetSize() - 1) {
			return MecUtilsHelper::GetNode(m_lastNode->GetHostServerId());
		}

		else {
			std::cout
					<< "ERROR: -> MecGraphHelper::CalculateEdgeWeights --> not first and lat DAG node="
					<< indexDagNode << "not found!!" << std::endl;
			return 0;
		}
	}

	// if the indexNode represents a DAG node on the first or penultimate tiers of the DAG
	else if (secondTierIt != m_secondTierTrackIds.end()
			|| penultimateTierIt != m_penultimateTierTrackIds.end()) {

		// second tier
		if (secondTierIt != m_secondTierTrackIds.end()) {
			return m_secondTierTrackIds[indexDagNode];
		}

		// penultimate tier
		else if (penultimateTierIt != m_penultimateTierTrackIds.end()) {
			return m_penultimateTierTrackIds[indexDagNode];
		}

		else {
			std::cout
					<< "ERROR: -> MecGraphHelper::CalculateEdgeWeights --> MB node="
					<< indexDagNode << "not found!!" << std::endl;
			return 0;
		}
	}

// if the indexNode represents a DAG node on the MiddleBoxTier
	else if (middleTierIt != m_middleTierTrackIds.end()) {

		Ptr<MecMiddleBox> middlebox = m_middleTierTrackIds[indexDagNode];
		return MecUtilsHelper::GetNode(middlebox->GetAttachedNodeId());
	}

	else {

		std::cout
				<< "ERROR: -> MecGraphHelper::CalculateEdgeWeights --> DAG node="
				<< indexDagNode << "not found!!" << std::endl;
		NS_ASSERT_MSG(false,
				"MecGraphHelper::CalculateEdgeWeights --> DAG node not found!!");

	}

	return 0;
}

void MecPolicyMigrationHelper::DagConstruction(uint32_t numberOfNodes,
		Ptr<MecVm> firstNode, NodeContainer *secondNodeTier,
		std::vector<MecMiddleBoxContainer>& middleNodeTiers,
		NodeContainer *penultimateNodeTier, Ptr<MecVm> lastNode) {

	NS_LOG_FUNCTION(this);

	// create the dag
	CreateDag(numberOfNodes);

	// set the first and last nodes
	m_firstNode = firstNode;
	m_lastNode = lastNode;

	// index for DAG node
	// the node ID=0 is the node that represents the first Node
	uint32_t currentDagNodeIndex = 1;

	// first node -> second tier of nodes
	DagConstructionHeadPart(secondNodeTier, &currentDagNodeIndex);

	// if it has tiers of MBs on the path
	if (middleNodeTiers.size() > 0) {

		DagConstructionMiddlePartA(secondNodeTier, middleNodeTiers,
				penultimateNodeTier, &currentDagNodeIndex);
	}

	// no middlebox tiers, then second tier of nodes -> penultimate tier of nodes
	else {

		DagConstructionMiddlePartB(secondNodeTier, penultimateNodeTier,
				&currentDagNodeIndex);
	}

	// penultimate tier of nodes -> the last node
	DagConstructionTailPart(penultimateNodeTier, &currentDagNodeIndex);

	DagConstructionGetEdgeWeight();

//		m_dag->Print(std::cout);
//
//	std::vector<uint32_t> shortestPath = m_dag->GetShortestPath(0);
//	std::cout << "Size=" << shortestPath.size() << std::endl;
//	std::cout << "Shortest path: "
//			<< MecUtilsHelper::VectorToString(shortestPath.begin(),
//					shortestPath.end()) << std::endl;
//
//	std::map<uint32_t, Ptr<Node>>::iterator it;
//	std::map<uint32_t, Ptr<MecMiddleBox>>::iterator itMb;
//	std::map<uint32_t, uint32_t>::iterator itReverse;
//
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//	std::cout << "Groups of MBs=" << middleNodeTiers.size() << std::endl;
//	for (uint32_t i = 1; i <= middleNodeTiers.size(); i++) {
//
//		std::cout << "group=" << i << " :" << std::endl;
//		MecMiddleBoxContainer container = middleNodeTiers[i - 1];
//
//		for (MecMiddleBoxContainer::const_iterator it = container.CBegin();
//				it != container.CEnd(); it++) {
//			((*it))->Print(std::cout);
//		}
//	}
//
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//	std::cout << "First TIER: " << std::endl;
//	firstNode->Print(std::cout);
//
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//
//	std::cout << "Second TIER: " << std::endl;
//	for (it = m_secondTierTrackIds.begin(); it != m_secondTierTrackIds.end();
//			it++) {
//
//		Ptr<Node> node = it->second;
//
//		std::cout << "virtual_id=" << it->first << " => node_id="
//				<< node->GetId() << " IP="
//				<< node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << '\n';
//	}
//
//	std::cout << "reverseSecondTierTrackIds: " << std::endl;
//	for (itReverse = m_reverseSecondTierTrackIds.begin();
//			itReverse != m_reverseSecondTierTrackIds.end(); itReverse++) {
//		std::cout << "node_id=" << itReverse->first << " => virtual_id="
//				<< itReverse->second << std::endl;
//	}
//
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//
//	std::cout << "Middlebox TIER: " << std::endl;
//	for (itMb = m_middleTierTrackIds.begin();
//			itMb != m_middleTierTrackIds.end(); itMb++) {
//
//		Ptr<MecMiddleBox> mb = itMb->second;
//
//		std::cout << "virtual_id=" << itMb->first << " => mb_id=" << mb->GetId()
//				<< '\n';
//		mb->Print(std::cout);
//	}
//
//	std::cout << "reverseMiddleTierTrackIds: " << std::endl;
//	for (itReverse = m_reverseMiddleTierTrackIds.begin();
//			itReverse != m_reverseMiddleTierTrackIds.end(); itReverse++) {
//		std::cout << "mb_id=" << itReverse->first << " => virtual_id="
//				<< itReverse->second << std::endl;
//	}
//
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//
//	std::cout << "Penultimate TIER: " << std::endl;
//	for (it = m_penultimateTierTrackIds.begin();
//			it != m_penultimateTierTrackIds.end(); it++) {
//
//		Ptr<Node> node = it->second;
//
//		std::cout << "virtual_id=" << it->first << " => node_id="
//				<< node->GetId() << " IP="
//				<< node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << '\n';
//	}
//
//	std::cout << "reversePenultimateTierTrackIds: " << std::endl;
//	for (itReverse = m_reversePenultimateTierTrackIds.begin();
//			itReverse != m_reversePenultimateTierTrackIds.end(); itReverse++) {
//		std::cout << "node_id=" << itReverse->first << " => virtual_id="
//				<< itReverse->second << std::endl;
//	}
//
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//
//	std::cout << "Last TIER: " << std::endl;
//	lastNode->Print(std::cout);
//	std::cout << "--------------------------------------------------"
//			<< std::endl;
//	std::cout << std::endl;

}

void MecPolicyMigrationHelper::DagConstructionHeadPart(
		NodeContainer *secondNodeTier, uint32_t *currentDagNodeIndex) {

	NS_LOG_FUNCTION(this);
	double edgeWeight;

	for (uint32_t i = 0; i < secondNodeTier->GetN(); i++) {

		//get node
		Ptr<Node> node = secondNodeTier->Get(i);

		// get MB's virtual Id if exist; otherwise a new one will be returned
		uint32_t dagNodeId = DagConstructionGetDagNodeId(currentDagNodeIndex,
				m_reverseSecondTierTrackIds, node->GetId());

		//if it is a download flow
		if (IsDownloadFlow() == true) {

			// if the current node is the current server, then the weight is zero
			if (m_firstNode->GetHostServerId() == node->GetId()) {
				edgeWeight = std::numeric_limits<double>::quiet_NaN();
			}

			// otherwise, the weight is the traffic data to transfer the vm
			else {
				edgeWeight = m_firstNode->CalculateMigrationTraffic();
			}
		}

		// it is an upload flow, i.e. the first node is an UE not a VM
		else {
			edgeWeight = std::numeric_limits<double>::infinity();
		}

		// add an edge from VM to HostServer
		m_dag->AddEdge(0, dagNodeId, edgeWeight);

		// track ids
		m_secondTierTrackIds.insert(
				std::pair<uint32_t, Ptr<Node>>(dagNodeId, node));

		//reverse track ids
		m_reverseSecondTierTrackIds.insert(
				std::pair<uint32_t, uint32_t>(node->GetId(), dagNodeId));

	}
}

void MecPolicyMigrationHelper::DagConstructionMiddlePartA(
		NodeContainer *secondNodeTier,
		std::vector<MecMiddleBoxContainer>& middleNodeTiers,
		NodeContainer *penultimateNodeTier, uint32_t *currentDagNodeIndex) {

	NS_LOG_FUNCTION(this);

	// second tier of nodes -> the first tier of MBs
	for (uint32_t i = 0; i < secondNodeTier->GetN(); i++) {

		//get the node
		Ptr<Node> node = secondNodeTier->Get(i);

		// get its virtual DAG node ID
		uint32_t virtualId = m_reverseSecondTierTrackIds[node->GetId()];

		// first containers of MBs of same type
		MecMiddleBoxContainer middleboxContainer = middleNodeTiers.front();

		// for each MB of the first tier of MBs
		for (MecMiddleBoxContainer::const_iterator itMiddlebox =
				middleboxContainer.CBegin();
				itMiddlebox != middleboxContainer.CEnd(); itMiddlebox++) {

			// get the MB
			Ptr<MecMiddleBox> middlebox = (*itMiddlebox);

			// get MB's virtual Id if exist; otherwise a new one will be returned
			uint32_t dagNodeId = DagConstructionGetDagNodeId(
					currentDagNodeIndex, m_reverseMiddleTierTrackIds,
					middlebox->GetId());

			// add an edge from a DAG node (host server) to a DAG node (middlebox)
			m_dag->AddEdge(virtualId, dagNodeId,
					std::numeric_limits<double>::infinity());

			// track ids
			m_middleTierTrackIds.insert(
					std::pair<uint32_t, Ptr<MecMiddleBox>>(dagNodeId,
							middlebox));
			//reverse track Ids
			m_reverseMiddleTierTrackIds.insert(
					std::pair<uint32_t, uint32_t>(middlebox->GetId(),
							dagNodeId));

		}
	}

	// second tiers of MBs -> last tiers of MBs
	for (uint32_t i = 1; i < middleNodeTiers.size(); i++) {

		// the previous MB tier
		MecMiddleBoxContainer previousMiddleboxContainer =
				middleNodeTiers[i - 1];

		// the currebt MB tier
		MecMiddleBoxContainer currentMiddleboxContainer = middleNodeTiers[i];

		// for each previous MB of the previous tier
		for (MecMiddleBoxContainer::const_iterator itPrevious =
				previousMiddleboxContainer.CBegin();
				itPrevious != previousMiddleboxContainer.CEnd(); itPrevious++) {

			// get previous MB
			Ptr<MecMiddleBox> previousMiddleBox = (*itPrevious);

			// get its virtual DAG node ID
			uint32_t previousMiddleBoxVirtualId =
					m_reverseMiddleTierTrackIds[previousMiddleBox->GetId()];

			// for the current MB of the current tier
			for (MecMiddleBoxContainer::const_iterator itCurrent =
					currentMiddleboxContainer.CBegin();
					itCurrent != currentMiddleboxContainer.CEnd();
					itCurrent++) {

				// get the current MB
				Ptr<MecMiddleBox> currentMiddleBox = (*itCurrent);

				// get MB's virtual Id if exist; otherwise a new one will be returned
				uint32_t dagNodeId = DagConstructionGetDagNodeId(
						currentDagNodeIndex, m_reverseMiddleTierTrackIds,
						currentMiddleBox->GetId());

				// add an edge from a DAG node (previous MB) to a DAG node (current MB)
				m_dag->AddEdge(previousMiddleBoxVirtualId, dagNodeId,
						std::numeric_limits<double>::infinity());

				// track ids
				m_middleTierTrackIds.insert(
						std::pair<uint32_t, Ptr<MecMiddleBox>>(dagNodeId,
								currentMiddleBox));

				//reverse track Ids
				m_reverseMiddleTierTrackIds.insert(
						std::pair<uint32_t, uint32_t>(currentMiddleBox->GetId(),
								dagNodeId));

			}

		}
	}

	// last tier of MB  -> the penultimate tier of nodes
	MecMiddleBoxContainer lastMiddleBoxContainer = middleNodeTiers.back();

	for (MecMiddleBoxContainer::const_iterator itMiddlebox =
			lastMiddleBoxContainer.CBegin();
			itMiddlebox != lastMiddleBoxContainer.CEnd(); itMiddlebox++) {

		// get middlebox
		Ptr<MecMiddleBox> middlebox = (*itMiddlebox);

		// get its virtual DAG node ID
		uint32_t middleBoxVirtualId =
				m_reverseMiddleTierTrackIds[middlebox->GetId()];

		for (uint32_t i = 0; i < penultimateNodeTier->GetN(); i++) {

			// get the node
			Ptr<Node> nodeUltimateTier = penultimateNodeTier->Get(i);

			// get MB's virtual Id if exist; otherwise a new one will be returned
			uint32_t dagNodeId = DagConstructionGetDagNodeId(
					currentDagNodeIndex, m_reversePenultimateTierTrackIds,
					nodeUltimateTier->GetId());

			// add an edge from a DAG node (previous MB) to a DAG node (host)
			m_dag->AddEdge(middleBoxVirtualId, dagNodeId,
					std::numeric_limits<double>::infinity());

			// track ids
			m_penultimateTierTrackIds.insert(
					std::pair<uint32_t, Ptr<Node>>(dagNodeId,
							nodeUltimateTier));

			//reverse track id
			m_reversePenultimateTierTrackIds.insert(
					std::pair<uint32_t, uint32_t>(nodeUltimateTier->GetId(),
							dagNodeId));

		}

	}
}

void MecPolicyMigrationHelper::DagConstructionMiddlePartB(
		NodeContainer *secondNodeTier, NodeContainer *penultimateNodeTier,
		uint32_t *currentDagNodeIndex) {

	NS_LOG_FUNCTION(this);

	// second tier of nodes -> the first tier of MBs
	for (uint32_t i = 0; i < secondNodeTier->GetN(); i++) {

		//get the node
		Ptr<Node> nodeSecondTier = secondNodeTier->Get(i);

		// get its virtual DAG node ID
		uint32_t nodeVirtualId =
				m_reverseSecondTierTrackIds[nodeSecondTier->GetId()];

		for (uint32_t i = 0; i < penultimateNodeTier->GetN(); i++) {

			// get the node
			Ptr<Node> nodeUltimateTier = penultimateNodeTier->Get(i);

			// get MB's virtual Id if exist; otherwise a new one will be returned
			uint32_t dagNodeId = DagConstructionGetDagNodeId(
					currentDagNodeIndex, m_reversePenultimateTierTrackIds,
					nodeUltimateTier->GetId());

			// add an edge from a DAG node of the second tier to a DAG node of the penultimate tier
			m_dag->AddEdge(nodeVirtualId, dagNodeId, 1.0);

			// track ids
			m_penultimateTierTrackIds.insert(
					std::pair<uint32_t, Ptr<Node>>(dagNodeId,
							nodeUltimateTier));

			//reverse track id
			m_reversePenultimateTierTrackIds.insert(
					std::pair<uint32_t, uint32_t>(nodeUltimateTier->GetId(),
							dagNodeId));

		}
	}
}

void MecPolicyMigrationHelper::DagConstructionTailPart(
		NodeContainer *penultimateNodeTier, uint32_t *currentDagNodeIndex) {

	NS_LOG_FUNCTION(this);

	double edgeWeight;

	for (uint32_t i = 0; i < penultimateNodeTier->GetN(); i++) {

		// get the node
		Ptr<Node> nodeUltimateTier = penultimateNodeTier->Get(i);

		// get its virtual DAG node ID
		uint32_t nodeVirtualId =
				m_reversePenultimateTierTrackIds[nodeUltimateTier->GetId()];

		//if it is an upload flow
		if (IsDownloadFlow() == false) {

			// if the current node is the current server, then the weight is zero
			if (m_lastNode->GetHostServerId() == nodeUltimateTier->GetId()) {
				edgeWeight = std::numeric_limits<double>::quiet_NaN();
			}

			// otherwise, the weight is the traffic data to transfer the vm
			else {
				edgeWeight = m_lastNode->CalculateMigrationTraffic();
			}
		}

		// it is an download flow, i.e. the last node is an UE not a VM
		else {
			edgeWeight = std::numeric_limits<double>::infinity();
		}

		// add an edge from VM to HostServer
		m_dag->AddEdge(nodeVirtualId, (*currentDagNodeIndex), edgeWeight);

	}
}

uint32_t MecPolicyMigrationHelper::DagConstructionGetDagNodeId(
		uint32_t *currentDagNodeIndex,
		std::map<uint32_t, uint32_t> & reverserMap, uint32_t key) {

	uint32_t virtualMbId = (*currentDagNodeIndex);

	// check if the current middlebox has a virtual id already
	std::map<uint32_t, uint32_t>::iterator it = reverserMap.find(key);

	if (it != reverserMap.end()) {
		virtualMbId = reverserMap[key];
	} else {
		//update index DAG node
		(*currentDagNodeIndex)++;
	}

	return virtualMbId;
}

void MecPolicyMigrationHelper::DagConstructionGetEdgeWeight() {
	NS_LOG_FUNCTION(this);

	// get all dag nodes
	for (uint32_t fromVirtualDagNodeId = 0;
			fromVirtualDagNodeId < m_dag->GetSize(); fromVirtualDagNodeId++) {

		// get the adjacent list
		std::vector<const AdjListNode *> adjList = m_dag->GetAdjListNode(
				fromVirtualDagNodeId);

		// get the real ns3 node represented by the virtual dag node id
		Ptr<Node> from = GetRealNs3Node(fromVirtualDagNodeId);

		// for each node in the adjacent list
		for (uint32_t i = 0; i < adjList.size(); i++) {

			// get the node from the adjacent list
			const AdjListNode *adjDagNode = adjList[i];

			// get the NS3 node represented by this virtual dag node id
			Ptr<Node> to = GetRealNs3Node(adjDagNode->GetNode());

			// get the rtt from the checkers
			double rtt = MecE2EChecker::GetRtt(from, to);

			// get the current weight node
			double currentWeight = m_dag->GetEdgeWeight(fromVirtualDagNodeId,
					adjDagNode->GetNode());

			// if it is not an infinity value, then it is an edge from a VM to a Server
			// and the current weight is the expected traffic to transfer the vm
			// the currentWeight / rtt is the expected time to trasfer
			if (std::isinf(currentWeight) == false) {
				rtt = currentWeight / rtt;
			}

			/// if it is nan, then the an edge from VM to the server that alreday host the vm is zero
			if (std::isnan(currentWeight) == true) {
				rtt = 0.1;
			}

			// otherwise it is a normal edge and the weight is the rtt value
			m_dag->SetEdgeWeight(fromVirtualDagNodeId, adjDagNode->GetNode(),
					rtt, true);
		}
	}
}


void GainServer(){

}

void MecPolicyMigrationHelper::SetPolicy(Ptr<MecPolicy> policy) {
	m_policy = policy;
}

void MecPolicyMigrationHelper::SetFlow(Ptr<MecFlow> flow) {
	m_flow = flow;
}

void MecPolicyMigrationHelper::CreateDag(uint32_t nodes) {
	m_dag = CreateObject<MecGraph>();
	m_dag->SetNodes(nodes);
}

bool MecPolicyMigrationHelper::IsDagReady() {
	//std::cout << "Pointer: " << m_dag << std::endl;
	return m_dag->IsAllEdgeWeightSet();
}

Ptr<MecPolicy> MecPolicyMigrationHelper::GetPolicy() {
	return m_policy;
}

Ptr<MecFlow> MecPolicyMigrationHelper::GetFlow() {
	return m_flow;
}

bool MecPolicyMigrationHelper::IsDownloadFlow() const {
	return m_isDownloadFlow;
}

void MecPolicyMigrationHelper::SetIsDownloadFlow(bool isDownloadflow) {
	m_isDownloadFlow = isDownloadflow;
}

uint32_t MecPolicyMigrationHelper::GetId() const {
	return m_policy->GetId();
}

void MecPolicyMigrationHelper::SetIsPolicyMigrationDone(bool isPolicyMigrated) {
	m_policyMigrationDone = isPolicyMigrated;
}

bool MecPolicyMigrationHelper::IsPolicyMigrationDone() const {
	return m_policyMigrationDone;
}

} /* namespace ns3 */
