/*
 * mec-graph.h
 *
 *  Created on: 4 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_GRAPH_H_
#define MEC_GRAPH_H_

#include "ns3/object.h"

#include <iostream>
#include <stack>
#include <vector>
#include <stdint.h>

#define INF INT32_MAX

namespace ns3 {

// Graph is represented using adjacency list. Every node of adjacency list
// contains vertex number of the vertex to which edge connects. It also
// contains weight of the edge
class MecGraph;

class AdjListNode {
public:
	AdjListNode();
	virtual ~AdjListNode();

	uint32_t GetNode() const;
	double GetWeight() const;
	bool GetPong() const;

	void SetNode(uint32_t value);
	void SetWeight(double value);
	void SetPong(bool pong);

	void Print(std::ostream& os);

private:
	uint32_t m_node;
	double m_weight;
	bool m_pong;

};

class MecGraph: public Object {
public:

	static TypeId GetTypeId();

	MecGraph();
	virtual ~MecGraph();

	// function to add an edge to graph
	void AddEdge(uint32_t u, uint32_t v, double weight, bool leftToRight);
	void AddEdge(uint32_t u, uint32_t v, double weight);

	void SetEdgeWeight(uint32_t u, uint32_t v, double weight, bool pong);
	double GetEdgeWeight(uint32_t u, uint32_t v);
	bool IsAllEdgeWeightSet();

	// Get the shortest paths from given source vertex
	std::vector<uint32_t> GetShortestPath(uint32_t src, uint32_t dst);
	std::vector<uint32_t> GetShortestPath(uint32_t src);

	void SetNodes(uint32_t nodes);
	void Print(std::ostream& os);

	std::vector<const AdjListNode *> GetAdjListNode(uint32_t nodeIndex);

	uint32_t GetSize() const;

private:

	// No. of vertices'
	uint32_t m_nodes = 0;

	// Pointer to an array containing adjacency lists
	std::vector<AdjListNode> *m_adj = NULL;

	// A function used by shortestPath
	void TopologicalSortUtil(uint32_t v, bool visited[],
			std::stack<uint32_t> &Stack);

	// find the shortest paths from given source vertex
	void ShortestPath(int32_t dist[], uint32_t parent[]);

	void CreateSolution(uint32_t parent[], uint32_t j,
			std::vector<uint32_t>& shortestPath);

};

}
/* namespace ns3 */

#endif /* MEC_GRAPH_H_ */
