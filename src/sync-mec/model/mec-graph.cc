/*
 * mec-graph.cc
 *
 *  Created on: 4 Jul 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/double.h"

#include "mec-graph.h"

namespace ns3 {

//---------------------------------------//
//---------- CLASS: AdjListNode ---------//
//---------------------------------------//

AdjListNode::AdjListNode() :
		m_node(0), m_weight(0.0), m_pong(false) {
}

AdjListNode::~AdjListNode() {
}

uint32_t AdjListNode::GetNode() const {
	return m_node;
}

void AdjListNode::SetNode(uint32_t value) {
	m_node = value;
}
void AdjListNode::SetWeight(double value) {
	m_weight = value;
}

double AdjListNode::GetWeight() const {
	return m_weight;
}

void AdjListNode::SetPong(bool pong) {
	m_pong = pong;
}

bool AdjListNode::GetPong() const {
	return m_pong;
}

void AdjListNode::Print(std::ostream& os) {
	os << "node=" << m_node << "\tweight=" << m_weight << "\tpong=" << m_pong
			<< std::endl;
}

//---------------------------------------//
//---------- CLASS: MecGraph ------------//
//---------------------------------------//

NS_LOG_COMPONENT_DEFINE("MecGraph");
NS_OBJECT_ENSURE_REGISTERED(MecGraph);

TypeId MecGraph::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecGraph").SetParent<Object>().SetGroupName("Network");
	return tid;
}

MecGraph::MecGraph() {
}

MecGraph::~MecGraph() {
	delete[] m_adj;
}

void MecGraph::SetNodes(uint32_t nodes) {
	m_adj = new std::vector<AdjListNode>[nodes];
	m_nodes = nodes;
}

void MecGraph::AddEdge(uint32_t u, uint32_t v, double weight) {
	AddEdge(u, v, weight, true);
}

void MecGraph::AddEdge(uint32_t u, uint32_t v, double weight,
		bool leftToRight) {

	AdjListNode node;
	node.SetWeight(weight);

	if (leftToRight) {

		node.SetNode(v);

		// Add v to u's list
		m_adj[u].push_back(node);
	}

	else {
		// create a node
		node.SetNode(u);

		// Add v to u's list
		m_adj[v].push_back(node);
	}
}

// A recursive function used by shortestPath. See below link for details
// http://www.geeksforgeeks.org/topological-sorting/
void MecGraph::TopologicalSortUtil(uint32_t v, bool visited[],
		std::stack<uint32_t> &stack) {

	// Mark the current node as visited
	visited[v] = true;

	// Recur for all the vertices adjacent to this vertex
	std::vector<AdjListNode>::iterator it;
	for (it = m_adj[v].begin(); it != m_adj[v].end(); ++it) {
		AdjListNode node = (*it);

		if (!visited[node.GetNode()]) {
			TopologicalSortUtil(node.GetNode(), visited, stack);
		}
	}

	// Push current vertex to stack which stores topological sort
	stack.push(v);
}

// The function to find shortest paths from given vertex. It uses recursive
// topologicalSortUtil() to get topological sorting of given graph.
void MecGraph::ShortestPath(int32_t dist[], uint32_t parent[]) {

	std::stack<uint32_t> stack;

	// Mark all the vertices as not visited
	bool *visited = new bool[m_nodes];
	for (uint32_t i = 0; i < m_nodes; i++) {
		visited[i] = false;
	}

	// Call the recursive helper function to store Topological Sort
	// starting from all vertices one by one
	for (uint32_t i = 0; i < m_nodes; i++) {
		if (visited[i] == false) {
			TopologicalSortUtil(i, visited, stack);
		}
	}

	// Process vertices in topological order
	while (stack.empty() == false) {

		// Get the next vertex from topological order
		uint32_t u = stack.top();
		stack.pop();

		// Update distances of all adjacent vertices
		std::vector<AdjListNode>::iterator it;
		if (dist[u] != INF) {
			for (it = m_adj[u].begin(); it != m_adj[u].end(); ++it) {
				if (dist[it->GetNode()] > dist[u] + it->GetWeight()) {
					dist[it->GetNode()] = dist[u] + it->GetWeight();
					parent[it->GetNode()] = u;
				}
			}
		}
	}
}

std::vector<uint32_t> MecGraph::GetShortestPath(uint32_t src) {
	return GetShortestPath(src, m_nodes - 1);
}

std::vector<uint32_t> MecGraph::GetShortestPath(uint32_t src, uint32_t dst) {

	// used by the ShortestPath function
	int32_t dist[m_nodes];
	uint32_t parent[m_nodes];

	// Initialise distances to all vertices as infinite and distance
	// to source as 0
	for (uint32_t i = 0; i < m_nodes; i++) {
		dist[i] = INF;
		parent[i] = m_nodes;
	}
	dist[src] = 0;

	// find the shortest path
	ShortestPath(dist, parent);

	// vector that return the solution
	std::vector<uint32_t> shortestPath;
	shortestPath.push_back(src);

	// get the path
	CreateSolution(parent, dst, shortestPath);

	//return the solution
	return shortestPath;
}

void MecGraph::CreateSolution(uint32_t parent[], uint32_t j,
		std::vector<uint32_t>& shortestPath) {

	// Base Case : If j is source
	if (parent[j] == m_nodes)
		return;

	CreateSolution(parent, parent[j], shortestPath);

	shortestPath.push_back((uint32_t) j);
}

void MecGraph::Print(std::ostream& os) {
	os << "number of vertices=" << m_nodes << std::endl;
	for (uint32_t i = 0; i < m_nodes; i++) {
		os << "node=" << i << " list-size=" << m_adj[i].size() << std::endl;
		for (std::vector<AdjListNode>::iterator it = m_adj[i].begin();
				it != m_adj[i].end(); ++it) {
			AdjListNode node = (*it);

			os << "\t-";
			node.Print(os);
		}
	}
}

bool MecGraph::IsAllEdgeWeightSet() {
	for (uint32_t i = 0; i < m_nodes; i++) {
		for (uint32_t j = 0; j < m_adj[i].size(); j++) {
			if (!m_adj[i][j].GetPong()) {
				return false;
			}
		}
	}
	return true;
}

void MecGraph::SetEdgeWeight(uint32_t u, uint32_t v, double weight, bool pong) {

	for (uint32_t i = 0; i < m_adj[u].size(); i++) {

		if (v == m_adj[u][i].GetNode()) {
			m_adj[u][i].SetWeight(weight);
			m_adj[u][i].SetPong(pong);
			return;
		}
	}

	NS_ASSERT_MSG(false,
			"MecGraph::SetEdgeWeight --> node v="<<v<< " not found in m_adj[" <<u <<"]");

}

double MecGraph::GetEdgeWeight(uint32_t u, uint32_t v) {

	for (uint32_t i = 0; i < m_adj[u].size(); i++) {

		if (v == m_adj[u][i].GetNode()) {
			return m_adj[u][i].GetWeight();
		}
	}

	NS_ASSERT_MSG(false,
			"MecGraph::SetEdgeWeight --> node v="<<v<< " not found in m_adj[" <<u <<"]");


	return 0.0;
}

std::vector<const AdjListNode *> MecGraph::GetAdjListNode(uint32_t nodeIndex) {
	std::vector<const AdjListNode *> nodes;
	for (uint32_t i = 0; i < m_adj[nodeIndex].size(); i++) {
		const AdjListNode *adjDagNode = &(m_adj[nodeIndex][i]);
		nodes.push_back(adjDagNode);

	}
	return nodes;
}

uint32_t MecGraph::GetSize() const {
	return m_nodes;
}

} /* namespace ns3 */
