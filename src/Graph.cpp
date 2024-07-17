#include <cassert>

#include "Graph.hpp"

Graph::Graph(count n) : n(n) {
    adjacencyArrays.resize(n);
    m = 0;
}

Graph::Graph(std::vector<edge> edges) {
    for(edge e : edges){
        addEdge(e.v, e.w);
    }
}

void Graph::addEdge(node u, node v) {
    assert(not hasEdge(u, v));

    if(n <= std::max(u, v)){
        n = std::max(u, v) + 1;
        adjacencyArrays.resize(n);
    }

    adjacencyArrays[u].emplace_back(v);
    adjacencyArrays[v].emplace_back(u);

    m++;
}

void Graph::deleteEdge(node u, node v) {
    assert(hasEdge(u, v));

    for(count i = 0; i < adjacencyArrays[u].size(); i++){
        if(adjacencyArrays[u][i] == v){
            adjacencyArrays[u][i] = adjacencyArrays[u].back();
            adjacencyArrays[u].pop_back();
            break;
        }
    }

    for(count i = 0; i < adjacencyArrays[v].size(); i++){
        if(adjacencyArrays[v][i] == u){
            adjacencyArrays[v][i] = adjacencyArrays[v].back();
            adjacencyArrays[v].pop_back();
            break;
        }
    }

    m--;
}

bool Graph::hasEdge(node u, node v) const {
    if(u >= n || v >= n){
        return false;
    }

    for(count i = 0; i < adjacencyArrays[u].size(); i++){
        if(adjacencyArrays[u][i] == v) return true;
    }

    return false;
}

const std::vector<node> &Graph::getNeighbors(node v) const {
    assert(v < n);

    return adjacencyArrays[v];
}

count Graph::getN() const {
    return n;
}

count Graph::getM() const {
    return m;
}