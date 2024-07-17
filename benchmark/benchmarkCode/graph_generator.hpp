#ifndef NETWORKIT_GRAPH_GENERATOR_HPP
#define NETWORKIT_GRAPH_GENERATOR_HPP

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include "base.hpp"

#include "io.hpp"

namespace GraphGen {
/**
     * This function just creates a completely random graph
     * @param n the number of vertexes
     * @param p the probability for every edge to exist
     * @param seed a seed that determines the random elements. Same parameters <=> same testfile
 */
std::vector<IO::Edge> createRandomGraph(unsigned int n, double p, unsigned int seed);


std::vector<IO::Action> StaticTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed, double queryFrequency);

/**
     * Creates an Actionlist that creates the given Graph and adds queries
     * @param edges assumes no duplicates
     * @param seed
     * @param queryFrequency expected number of queries between two updates
     * @return
 */
std::vector<IO::Action>
IncrementalTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed, double queryFrequency);

std::vector<IO::Action>
DecrementalTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed, double queryFrequency);

/**
     * This function creates a random dynamic test. First a graph is build. Then random edges are inserted and removed and queries pop up
     * @param edges graph to base test on
     * @param pStart chance for each edge to be in the graph as it is first build up
     * @param pAddEdgeInStep chance that any step after the first version of the graph is build adds an edge (removes edges otherwise)
     * @param stepsToFinal steps taken after the first version of the graph
     * @param queryFrequency chance a query is added after an edge is added or deleted
     * @param seed random seed
 */
std::vector<IO::Action>
MixedTestFromGraph(std::vector<IO::Edge> &edges, double pStart, double pAddEdgeInStep, unsigned int stepsToFinal,
                   unsigned int seed, double queryFrequency);

std::vector<IO::Edge> generateDiamGraph(unsigned int n, unsigned int k);

/**
     * to calculate avg_sp of diam graphs
     * starts bfs from every node and adds up the length of every path.
     * @param edges
     * @param
     * @return length of all paths divided by number of paths
 */
double calcAvgSp(std::vector<std::vector<node>> &adjacency_list);

std::vector<IO::Edge> generateLineGraph(unsigned int n);

std::vector<IO::Action> edgeListToActions(const std::vector<IO::Edge> &edges);

/**
     * Makes a graph specifically to test BFS/DFS
 */
void makeBFSTestFiles(const std::string &filename, const std::string &fileSolution, unsigned int n, double p,
                      unsigned int seed);

std::vector<IO::Action> ChordalTestFromGraph(std::vector<IO::Edge> &edges, unsigned int seed);

std::vector<IO::Edge> starLineTest(count n);

std::vector<IO::Action> ActionBlockTestFromActionList(std::vector<IO::Action> &actions, count Blocksize, bool timeAddActions);
} // namespace GraphGen

#endif // NETWORKIT_GRAPH_GENERATOR_HPP
