#ifndef GKKT_GRAPH_HPP
#define GKKT_GRAPH_HPP

#include <vector>

#include "base.hpp"

class Graph {
public:
    Graph(count n);

    Graph(std::vector<edge> edges);

    std::vector<node> const &getNeighbors(node v) const;

    bool hasEdge(node u, node v) const;

    void addEdge(node u, node v);

    void deleteEdge(node u, node v);

    count getN() const;

    count getM() const;

protected:
    count n = 0, m = 0;

    std::vector<std::vector<node>> adjacencyArrays;
};

#endif //GKKT_GRAPH_HPP
