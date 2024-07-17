#ifndef GKKT_QUERYFOREST_HPP
#define GKKT_QUERYFOREST_HPP

#include "base.hpp"

/**
 * Template for classes that provide connectivity queries on dynamic forests
 */
class QueryForest {
public:
    virtual bool query(node u, node v) const = 0;

    virtual count compSize(node v) const = 0;

    virtual count compRepresentative(node v) const = 0;

    virtual count numberOfComponents() const = 0;

    virtual bool isTreeEdge(node u, node v) const = 0;

    virtual void addEdge(node u, node v) = 0;

    virtual void deleteEdge(node u, node v) = 0;
};

#endif //GKKT_QUERYFOREST_HPP
