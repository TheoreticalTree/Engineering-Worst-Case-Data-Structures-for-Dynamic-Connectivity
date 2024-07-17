#ifndef GKKT_DYNCONNECTIVITYALGORITHM_HPP
#define GKKT_DYNCONNECTIVITYALGORITHM_HPP

#include <vector>
#include <system_error>

#include "base.hpp"
#include "Graph.hpp"

/**
 * Interface that dynamic connectivity algorithms must implement
 */
class DynConnectivity {
public:
    DynConnectivity() = default;

    /**
     * Query if nodes u and v are connected
     * @param u node u
     * @param v node v
     * @return true iff there is a path between u and v
     */
    virtual bool query(node u, node v){
        throw std::runtime_error("Not implemented yet.");
    };

    /**
     * Add edge @e into the graph (the edge must not have been inserted before)
     * @param e the edge
     */
    virtual void addEdge(node u, node v) = 0;

    /**
     * Delete edge @e from the graph
     * @param e the edge
     */
    virtual void deleteEdge(node u, node v) = 0;

    /**
     * Get the number of components
     * @return The number of connected components
     */
    virtual count numberOfComponents(){
        throw std::runtime_error("Not implemented yet.");
    };

    /**
     * Get an ID in [0, numberOfComponents -1] for the component of vertex v.
     * @param v the node v
     * @return The ID of the connected component of @v
     */
    virtual count componentOfNode(node v){
        throw std::runtime_error("Not implemented yet.");
    };

    /**
     * Get the sizes of every connected component
     * @return a vector storing the sizes of the connected components
     */
    virtual std::vector<count> getComponentSizes(){
        throw std::runtime_error("Not implemented yet.");
    }

    /**
     * Get a vector of every vertex connected to v
     * @param v the node v
     * @return a vector of every node connected to v
     */
    virtual std::vector<node> getComponentOf(node v){
        throw std::runtime_error("Not implemented yet.");
    }

    /**
     * Get a vector of components, each stored as an unordered vector of nodes
     * @return a vector of components, each stored as a vector of the components nodes
     */
    virtual std::vector<std::vector<node>> getComponents(){
        throw std::runtime_error("Not implemented yet.");
    };
};

#endif //GKKT_DYNCONNECTIVITYALGORITHM_HPP
