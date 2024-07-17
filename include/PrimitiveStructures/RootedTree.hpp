//
// Created by michaelk on 21.06.23.
//

#include <vector>

#include "base.hpp"


#ifndef BIASEDBINARYTREES_ROOTEDTREE_HPP
#define BIASEDBINARYTREES_ROOTEDTREE_HPP

class RootedForest {
public:
    RootedForest(count n) {
        parent.resize(n, none);
        pCost.resize(n, 0);
    }

    node getRoot(node v){
        while (parent[v] != none) v = parent[v];
        return v;
    }

    void reroot(node v) {
        if(parent[v] == none) return;
        node prev = v, here = parent[v], next;
        cost prevCost = pCost[v], hereCost;
        while (here != none){
            next = parent[here];
            hereCost = pCost[here];
            parent[here] = prev;
            pCost[here] = prevCost;
            prev = here;
            prevCost = hereCost;
            here = next;
        }
        parent[v] = none;
        pCost[v] = 0;
    }

    void link(node v, node w, cost x) {
        reroot(v);
        parent[v] = w;
        pCost[v] = x;
    }

    cost cut(node v){
        parent[v] = none;
        cost ret = pCost[v];
        pCost[v] = 0;
        return ret;
    }

    cost cutEdge(node v, node w) {
        if(parent[v] == w) std::swap(v,w);
        cost ret = pCost[w];
        parent[w] = none;
        pCost[w] = 0;
        return ret;
    }

    node getParent(node v){
        return parent[v];
    }

    cost getCost(node v){
        return pCost[v];
    }

    std::pair<std::pair<node, node>, cost> getMinEdge(node v){
        if(parent[v] == none) return {{none, none}, 0};

        std::pair<std::pair<node, node>, cost> minSoFar = {{v, parent[v]}, pCost[v]};

        while (parent[v] != none){
            if(pCost[v] <= minSoFar.second){
                minSoFar = {{v, parent[v]}, pCost[v]};
            }
            v = parent[v];
        }

        return minSoFar;
    }

    std::pair<std::pair<node, node>, cost> getMaxEdge(node v){
        if(parent[v] == none) return {{none, none}, 0};

        std::pair<std::pair<node, node>, cost> maxSoFar = {{v, parent[v]}, pCost[v]};

        while (parent[v] != none){
            if(pCost[v] >= maxSoFar.second){
                maxSoFar = {{v, parent[v]}, pCost[v]};
            }
            v = parent[v];
        }

        return maxSoFar;
    }

    void update(node v, diff x){
        while (parent[v] != none){
            pCost[v] += x;
            v = parent[v];
        }
    }

protected:
    std::vector<node> parent;
    std::vector<cost> pCost;
};

#endif //BIASEDBINARYTREES_ROOTEDTREE_HPP
