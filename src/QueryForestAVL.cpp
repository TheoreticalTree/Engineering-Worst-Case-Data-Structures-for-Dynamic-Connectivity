#include "QueryForestAVL.hpp"

QueryForestAVL::QueryForestAVL(count n) : n(n), forest(ETForestCutSet(0, n, true)) {
    treeEdges.resize(n);
    numConnectedComponents = n;
}

bool QueryForestAVL::query(node u, node v) const {
    if(u == v) return true;
    if(u >= n || v >= n) return false;
    if(treeEdges[u].empty() || treeEdges[v].empty()) return false;
    return forest.getRoot(treeEdges[u].getAnyEntry().second) == forest.getRoot(treeEdges[v].getAnyEntry().second);
}

count QueryForestAVL::compSize(node v) const {
    if(v >= n) return 0;
    if(treeEdges[v].empty()) return 1;
    return forest.getRoot(treeEdges[v].getAnyEntry().second)->getSize();
}

count QueryForestAVL::compRepresentative(node v) const {
    if(v >= n) return none;
    if(treeEdges[v].empty()) return 1;
    return forest.getRoot(treeEdges[v].getAnyEntry().second)->getSize();
}

bool QueryForestAVL::isTreeEdge(node u, node v) const {
    return treeEdges[u].contains(v);
}

count QueryForestAVL::numberOfComponents() const {
    return numConnectedComponents;
}

void QueryForestAVL::addEdge(node u, node v) {
    assert(u < n && v < n);
    assert(not query(u, v));

    //Get two tree edges if any exist
    address uEdge = nullptr, vEdge = nullptr;
    if(not treeEdges[u].empty()) uEdge = treeEdges[u].getAnyEntry().second;
    if(not treeEdges[v].empty()) vEdge = treeEdges[v].getAnyEntry().second;

    //Insert the edge into the ETForest
    std::pair<address, address> newEdges = forest.insertETEdge(u, v, uEdge, vEdge);

    //Store the edges for later use
    treeEdges[u].insert(v, newEdges.first);
    treeEdges[v].insert(u, newEdges.second);

    numConnectedComponents--;
}

void QueryForestAVL::deleteEdge(node u, node v) {
    assert(u < n && v < n);

    //Get the edges and delete them
    address uEdge = treeEdges[u].remove(v), vEdge = treeEdges[v].remove(u);
    forest.deleteETEdge(uEdge, vEdge);

    numConnectedComponents++;
}