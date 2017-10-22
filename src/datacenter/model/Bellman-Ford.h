//#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <limits.h>
#include <iostream>
#include <vector>

#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H  
//add by cuilin
//typedef std::vector<int> Path;

namespace ns3 {


// a structure to represent a weighted edge in graph
struct Edge
{
        int src, dest, weight;
};
 
// a structure to represent a connected, directed and weighted graph
struct Graph
{
        // V-> Number of vertices, E-> Number of edges
        int V, E;
 
        // graph is represented as an array of edges.
        struct Edge* edge;
};
 
// Creates a graph with V vertices and E edges
struct Graph* createGraph(int V, int E)
{
    struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph));
    graph->V = V;
    graph->E = E;
 
    graph->edge = (struct Edge*) malloc(graph->E * sizeof(struct Edge));
 
    return graph;
}

void printGraph(struct Graph* graph)
{
	int V = graph->V;
    int E = graph->E;

	std::cout<<"Graph: "<<V<<" vertex and "<<E<<" edges:\n";

	for (int j = 0; j < E; j++)
    {
        int u = graph->edge[j].src;
        int v = graph->edge[j].dest;
        int weight = graph->edge[j].weight;
        std::cout<<"Edge "<<j<<": "<<u<<"-->"<<v<<" (weight="<<weight<<")\n";
    }
}

 
// A utility function used to print the solution
void printArr(int dist[], int n)
{
    std::cout<<"Vertex   Distance from Source\n";
    for (int i = 0; i < n; ++i)
        std::cout<<i<<"\t\t"<<dist[i]<<std::endl;
}

//print shortest path
void printPath(std::vector<int> &path)//(int src, int dst, std::vector<int> &path)
{
	std::cout<<"Shortest path from "<<path[0]<<" to "<<path[path.size()-1]<<": ";
	std::cout<<path[0];
    for (uint32_t i = 1; i < path.size(); ++i)
        std::cout<<"-->"<<path[i];
	std::cout<<std::endl;
}

// The main function that finds shortest distances from src to all other
// vertices using Bellman-Ford algorithm.  The function also detects negative
// weight cycle

//Modified by cuilin: return true if a shortest path exist, otherwise false
bool BellmanFord(struct Graph* graph, int src, int dst, std::vector<int> &path)
{
    int V = graph->V;
    int E = graph->E;
    int dist[V];
    int predecessor[V];
 
    // Step 1: Initialize distances from src to all other vertices as INFINITE
    for (int i = 0; i < V; i++)
    {
        dist[i] = INT_MAX;
        predecessor[i] = -1;
    }
    dist[src] = 0;
 
    // Step 2: Relax all edges |V| - 1 times. A simple shortest path from src
    // to any other vertex can have at-most |V| - 1 edges
    for (int i = 1; i <= V - 1; i++)
    {
        for (int j = 0; j < E; j++)
        {
            int u = graph->edge[j].src;
            int v = graph->edge[j].dest;
            int weight = graph->edge[j].weight;
            if (dist[u] != INT_MAX && dist[u] + weight < dist[v])
            {			
                dist[v] = dist[u] + weight;
                predecessor[v] = u;
            }
        }
    }
 
    // Step 3: check for negative-weight cycles.  The above step guarantees
    // shortest distances if graph doesn't contain negative weight cycle.
    // If we get a shorter path, then there is a cycle.
    for (int i = 0; i < E; i++)
    {
        int u = graph->edge[i].src;
        int v = graph->edge[i].dest;
        int weight = graph->edge[i].weight;
        if (dist[u] != INT_MAX && dist[u] + weight < dist[v])
        {
         	  std::cout<<"Graph contains negative weight cycle\n";
         	  return false;
         }
    }
 
 	std::vector<int>::iterator it;
 	int nextVertex = dst;
 	
 	path.push_back(dst);
 	
 	while(predecessor[nextVertex]!=src)
 	{
 		it = path.begin();
 		path.insert(it, predecessor[nextVertex]);
 		nextVertex = predecessor[nextVertex];
	 }
	it = path.begin();
	path.insert(it,src);
 	
    //printArr(dist, V);
    
    //printPath(path);
 
    return true;
}

// Driver program to test above functions
int main_test()
{
    /* Let us create the graph given in above example */
//    int V = 5; // Number of vertices in graph
//    int E = 8; // Number of edges in graph
//    struct Graph* graph = createGraph(V, E);
// 
//    // add edge 0-1 (or A-B in above figure)
//    graph->edge[0].src = 0;
//    graph->edge[0].dest = 1;
//    graph->edge[0].weight = -1;
// 
//    // add edge 0-2 (or A-C in above figure)
//    graph->edge[1].src = 0;
//    graph->edge[1].dest = 2;
//    graph->edge[1].weight = 4;
// 
//    // add edge 1-2 (or B-C in above figure)
//    graph->edge[2].src = 1;
//    graph->edge[2].dest = 2;
//    graph->edge[2].weight = 3;
// 
//    // add edge 1-3 (or B-D in above figure)
//    graph->edge[3].src = 1;
//    graph->edge[3].dest = 3;
//    graph->edge[3].weight = 2;
// 
//    // add edge 1-4 (or A-E in above figure)
//    graph->edge[4].src = 1;
//    graph->edge[4].dest = 4;
//    graph->edge[4].weight = 2;
// 
//    // add edge 3-2 (or D-C in above figure)
//    graph->edge[5].src = 3;
//    graph->edge[5].dest = 2;
//    graph->edge[5].weight = 5;
// 
//    // add edge 3-1 (or D-B in above figure)
//    graph->edge[6].src = 3;
//    graph->edge[6].dest = 1;
//    graph->edge[6].weight = 1;
// 
//    // add edgD in above figure)
//    graph->edge[7].src = 4;
//    graph->edge[7].dest = 3;
//    graph->edge[7].weight = -3;
 
  	int V = 6; // Number of vertices in graph
    int E = 9; // Number of edges in graph
    struct Graph* graph = createGraph(V, E);
 
    // add edge 0-1
    graph->edge[0].src = 0;
    graph->edge[0].dest = 1;
    graph->edge[0].weight = 2;
    
    // add edge 0-2
    graph->edge[1].src = 0;
    graph->edge[1].dest = 2;
    graph->edge[1].weight = 1;
    
    // add edge 0-3
    graph->edge[2].src = 0;
    graph->edge[2].dest = 3;
    graph->edge[2].weight = 4;
    
    // add edge 1-2
    graph->edge[3].src = 1;
    graph->edge[3].dest = 2;
    graph->edge[3].weight = 1;
    
    // add edge 1-4
    graph->edge[4].src = 1;
    graph->edge[4].dest = 4;
    graph->edge[4].weight = 5;
    
    // add edge 2-3
    graph->edge[5].src = 2;
    graph->edge[5].dest = 3;
    graph->edge[5].weight = 3;
    
    // add edge 2-4
    graph->edge[6].src = 2;
    graph->edge[6].dest = 4;
    graph->edge[6].weight = 2;
    
    // add edge 3-5
    graph->edge[7].src = 3;
    graph->edge[7].dest = 5;
    graph->edge[7].weight = 2;
    
    // add edge 4-5
    graph->edge[8].src = 4;
    graph->edge[8].dest = 5;
    graph->edge[8].weight = 1;
    
 	std::vector<int> path;
 	
 	int src = 1;
 	int dst = 5;
    BellmanFord(graph, src, dst, path);
 
 	printPath(path);
 	
    return 0;
}

}
#endif

