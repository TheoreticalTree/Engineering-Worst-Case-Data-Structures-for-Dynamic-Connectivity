#include "HDT.hpp"
#include <iostream>

HDT::HDT(count n) {
    init(n-1, n-1);
}

HDT::HDT(const Graph &G) {
    init(G.getN()-1, G.getN()-1);

    for(node u = 0; u < n; u++){
        for(node v : G.getNeighbors(u)){
            if(v > u) addEdge(u, v);
        }
    }
}

bool HDT::query(node u, node v) {
    if (u >= n || v >= n) return false;
    if (u == v) return true;
    if (activeEdge[u].empty() || activeEdge[v].empty()) return false;
    if (activeEdge[u][0] == nullptr || activeEdge[v][0] == nullptr) return false;
    return etForest.getRoot(activeEdge[u][0]) == etForest.getRoot(activeEdge[v][0]);
}

count HDT::numberOfComponents() {
    return numComponents;
}

void HDT::addEdge(node u, node v) {
    assert(u != v);

    init(u, v);

    if (query(u, v))
        addNonTreeEdge(u, v, 0);
    else {
        addTreeEdge(u, v, 0, true);
        numComponents--;
    }
}

void HDT::deleteEdge(node u, node v) {
    assert(u != v);

    if (nonTreeEdgesLevelIndex[u].contains(v))
        deleteNonTreeEdge(u, v, nonTreeEdgesLevelIndex[u].findVal(v));
    else {
        deleteTreeEdge(u, v);
        numComponents++;
    }
}

void HDT::deleteTreeEdge(node u, node v) {
    int i;
    for (i = 0; i < treeEdges[u].size(); i++) {
        if (treeEdges[u][i].contains(v)) {
            addressHDT uEdge = treeEdges[u][i].remove(v);
            addressHDT vEdge = treeEdges[v][i].remove(u);

            etForest.deleteETEdge(uEdge, vEdge);

            if (activeEdge[u][i] == uEdge)
                refreshActiveEdge(u, i);
            if (activeEdge[v][i] == vEdge)
                refreshActiveEdge(v, i);
        }
        else break;
    }

    //To get back to the last level containing {u, v}
    i--;

    edge replacement = noEdge;

    for (; i >= 0; i--) {
        addressHDT uTree = etForest.getRoot(activeEdge [u][i]);
        addressHDT vTree = etForest.getRoot(activeEdge [v][i]);

        //Special case of size 1 trees
        if (uTree == nullptr || vTree == nullptr) {
            node small = (uTree == nullptr) ? u : v;
            if (nonTreeEdgesOnLevels[small].size() > i && not nonTreeEdgesOnLevels[small][i].empty()) {
                //Any edge leaving a single vertex is a replacement edge
                std::pair<node, bool> repEdge = nonTreeEdgesOnLevels[small][i].getAnyEntry();
                deleteNonTreeEdge(small, repEdge.first, i);
                replacement = {small, repEdge.first};
                break;
            }
            else
                continue;
        }

        addressHDT smallTree = (uTree->getSize() < vTree->getSize()) ? uTree : vTree;
        //Step 1: promote every on-level-edge in the entire tree
        edge toPromote = etForest.getOnLevelTreeEdge(smallTree);
        while (toPromote != noEdge) {
            etForest.setEdgeOnLevel(treeEdges[toPromote.v][i].findVal(toPromote.w), false);
            etForest.setEdgeOnLevel(treeEdges[toPromote.w][i].findVal(toPromote.v), false);

            addTreeEdge(toPromote.v, toPromote.w, i + 1, true);

            toPromote = etForest.getOnLevelTreeEdge(smallTree);
        }

        node hasNonTreeEdges = etForest.getNodeWithOnLevelNontreeEdge(smallTree);
        while (hasNonTreeEdges != none) {
            while (not nonTreeEdgesOnLevels[hasNonTreeEdges][i].empty()) {
                std::pair<node, bool> repEdge = nonTreeEdgesOnLevels[hasNonTreeEdges][i].getAnyEntry();
                deleteNonTreeEdge(hasNonTreeEdges, repEdge.first, i);

                assert(activeEdge[repEdge.first][i] != nullptr);
                if (etForest.getRoot(activeEdge[repEdge.first][i]) != smallTree) {
                    replacement = {hasNonTreeEdges, repEdge.first};
                    break;
                }
                else {
                    addNonTreeEdge(hasNonTreeEdges, repEdge.first, i + 1);
                }
            }

            if (replacement != noEdge)
                break;

            hasNonTreeEdges = etForest.getNodeWithOnLevelNontreeEdge(smallTree);
        }

        if (replacement != noEdge)
            break;
    }

    if (replacement != noEdge) {
        for (int j = 0; j <= i; j++)
            addTreeEdge(replacement.v, replacement.w, j, i == j);
        numComponents--;
    }
}

void HDT::deleteNonTreeEdge(node u, node v, count level) {
    assert(nonTreeEdgesLevelIndex[u].findVal(v) == level);

    nonTreeEdgesLevelIndex[u].remove(v);
    nonTreeEdgesLevelIndex[v].remove(u);

    nonTreeEdgesOnLevels[u][level].remove(v);
    if (nonTreeEdgesOnLevels[u][level].empty()) {
        if (activeEdge[u][level] != nullptr) {
            etForest.setEdgeNontreeEdges(activeEdge[u][level], 0);
        }
    }
    nonTreeEdgesOnLevels[v][level].remove(u);
    if (nonTreeEdgesOnLevels[v][level].empty()) {
        if (activeEdge[v][level] != nullptr) {
            etForest.setEdgeNontreeEdges(activeEdge[v][level], 0);
        }
    }

    assert(not nonTreeEdgesLevelIndex[u].contains(v));
    assert(not nonTreeEdgesLevelIndex[v].contains(u));
}

void HDT::refreshActiveEdge(node v, count level) {
    if (not treeEdges[v][level].empty()) {
        activeEdge[v][level] = treeEdges[v][level].getAnyEntry().second;
        if (nonTreeEdgesOnLevels[v].size() > level && not nonTreeEdgesOnLevels[v][level].empty())
            etForest.setEdgeNontreeEdges(activeEdge[v][level], 1);
    }
    else activeEdge[v][level] = nullptr;
}

void HDT::addTreeEdge(node u, node v, count level, bool onLevel) {
    assert(u < n && v < n);
    assert(u != v);
    assert(treeEdges[u].size() <= level || not treeEdges[u][level].contains(v));
    assert(treeEdges[v].size() <= level || not treeEdges[v][level].contains(u));

    if (activeEdge[u].size() <= level) {
        activeEdge[u].resize(level + 1, nullptr);
        treeEdges[u].resize(level + 1);
    }
    if (activeEdge[v].size() <= level) {
        activeEdge[v].resize(level + 1, nullptr);
        treeEdges[v].resize(level + 1);
    }

    bool uHasNonTreeEdges = (activeEdge[u][level] == nullptr) && (nonTreeEdgesOnLevels[u].size() > level) && (not nonTreeEdgesOnLevels[u][level].empty());
    bool vHasNonTreeEdges = (activeEdge[v][level] == nullptr) && (nonTreeEdgesOnLevels[v].size() > level) && (not nonTreeEdgesOnLevels[v][level].empty());

    std::pair<addressHDT, addressHDT> newEdges = etForest.insertETEdge(u, v, activeEdge[u][level], activeEdge[v][level],
                                                                 onLevel, onLevel,
                                                                 uHasNonTreeEdges, vHasNonTreeEdges);

    treeEdges[u][level].insert(v, newEdges.first);
    if (activeEdge[u][level] == nullptr) activeEdge[u][level] = newEdges.first;
    treeEdges[v][level].insert(u, newEdges.second);
    if (activeEdge[v][level] == nullptr) activeEdge[v][level] = newEdges.second;
}

void HDT::addNonTreeEdge(node u, node v, count level) {
    assert(u < n && v < n);
    assert(u != v);

    if(nonTreeEdgesOnLevels[u].size() <= level)
        nonTreeEdgesOnLevels[u].resize(level + 1);
    if(nonTreeEdgesOnLevels[v].size() <= level)
        nonTreeEdgesOnLevels[v].resize(level + 1);

    assert(not nonTreeEdgesLevelIndex[u].contains(v));
    assert(not nonTreeEdgesOnLevels[u][level].contains(v));
    assert(not nonTreeEdgesLevelIndex[v].contains(u));
    assert(not nonTreeEdgesOnLevels[v][level].contains(u));

    //Check if the active edges need to be informed that they now have nonTreeEdges
    if (nonTreeEdgesOnLevels[u][level].empty()) {
        etForest.setEdgeNontreeEdges(activeEdge[u][level], 1);
    }
    if (nonTreeEdgesOnLevels[v][level].empty()) {
        etForest.setEdgeNontreeEdges(activeEdge[v][level], 1);
    }

    nonTreeEdgesLevelIndex[u].insert(v, level);
    nonTreeEdgesLevelIndex[v].insert(u, level);
    nonTreeEdgesOnLevels[u][level].insert(v, false);
    nonTreeEdgesOnLevels[v][level].insert(u, false);
}

void HDT::init(node u, node v) {
    u = std::max(u, v);
    if (u >= n) {
        numComponents += (u + 1 - n);
        nonTreeEdgesOnLevels.resize(u + 1);
        nonTreeEdgesLevelIndex.resize(u + 1);
        treeEdges.resize(u + 1);
        activeEdge.resize(u + 1, {nullptr});
        for(count i = n; i < u + 1; i++) {
            nonTreeEdgesOnLevels[i].resize(1);
            treeEdges[i].resize(1);
        }
        n = u + 1;
    }
}