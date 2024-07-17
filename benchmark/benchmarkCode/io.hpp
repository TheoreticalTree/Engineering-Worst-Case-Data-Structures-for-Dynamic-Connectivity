//
// Created by kaibelm0 on 5/8/23.
//

#ifndef NETWORKIT_IO_HPP
#define NETWORKIT_IO_HPP

#include <vector>
#include <sstream>
#include <fstream>
#include <optional>
#include <ctime>
#include <memory>

#include "base.hpp"

#include "DynConnectivityAlgorithm.hpp"

namespace IO {
using Edge = std::pair<node, node>;

enum ActionType {
    addAction, deleteAction, queryAction, queryBlock, timerAction
};

struct Action {
    ActionType type;
    node u, v;

    Action() = default;

    Action(ActionType t, node u, node v) {
        type = t;
        this->u = u;
        this->v = v;
    };
};

std::vector<std::string> splitString(std::string const &s);

std::vector<Action> fileToActionList(const std::string &f);
void actionListToFile(const std::vector<Action> &actions, const std::string &f, const std::string &info = "");

/**
     * Runs all the operations from one file at high speed for benchmark
     * @param actions List of actions applied to the graph
     * @param graph The graph in the state it should have at the start of the file run
     * @param algo The algorithm to be run
     *
     * @return the time or times as a string
     *
     * AddEdge and DeleteEdge are called for every algorithm attached to graph, but only algo is run on queries. All of these times are measured
 */
std::string runBenchmark(const std::vector<Action> &actions, std::string algo, count n, std::string type, count seed);

clock_t runAllUpdatesTime(const std::vector<Action> & actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G);

std::pair<clock_t, clock_t> runSingleUpdateTime(const std::vector<Action> & actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G);

clock_t runQueryTime(const std::vector<Action> & actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G);

std::pair<std::pair<count, count>, std::pair<count, count>> runAccuracyCheck(const std::vector<Action> & actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G);

/**
     * Reads a graph from a file, lines starting with % or # are ignored otherwise it expects two integers per line denoting the endpoints of an edge
     * @param fname
     * @return returns a sorted vector of unique edges
 */
std::vector<Edge> readEdgeList(const std::string &fname);

} //namespace IO

#endif // NETWORKIT_IO_HPP
