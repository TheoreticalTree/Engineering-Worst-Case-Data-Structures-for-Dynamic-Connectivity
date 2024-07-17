//
// Created by michaelk on 23.04.23.
//

#include "DTree.hpp"

#include <iostream>
#include <cassert>

DTree::DTree(const Graph &G) {
    //Initialize all relevant data structures
    Init(G.getN() - 1, G.getN() - 1);
    assert(num_nodes_ == G.getN());

    //Use the predefined functions to add every edge
    for (node u = 0; u < num_nodes_; u++)
        for (node v : G.getNeighbors(u))
            if (u < v) addEdge(u, v);
    SanityCheck();
}

DTree::DTree(count n) {
    Init(n-1, n-1);
}

bool DTree::query(node u, node v) {
    SanityCheck();
    if (std::max(u, v) >= num_nodes_) {
        return false;
    }
    return Conn(u, v);
}

count DTree::numberOfComponents() {
    return compNum;
}

count DTree::componentOfNode(node v) {
    //Move to the root, which is the only vertex guaranteed to have the correct ID
    node w = Find(v);
    return compID[w];
}

std::vector<count> DTree::getComponentSizes() {
    std::vector<count> ret(compNum);
    for (count i = 0; i < compNum; i++) {
        ret[i] = size_[compRep[i]];
    }

    return ret;
}

std::vector<node> DTree::getComponentOf(node v) {
    std::vector<node> result;

    node root = Find(v);

    std::deque<node> queue(1, root);

    //Just breadth first search over the spanning tree
    node temp;
    while (!queue.empty()) {
        temp = queue.front();
        queue.pop_front();
        result.push_back(temp);

        for (node w: children_[temp]) {
            queue.push_back(w);
        }
    }

    return result;
}

std::vector<std::vector<node>> DTree::getComponents() {
    std::vector<std::vector<node>> ret(compNum);
    for (count i = 0; i < compNum; i++) {
        ret[i] = getComponentOf(compRep[i]);
    }

    return ret;
}

void DTree::cleanComponent(count comp) {
    if (comp < compNum && dirtyComp[comp]) {
        for (node v: getComponentOf(compRep[comp])) {
            compID[v] = comp;
        }
        dirtyComp[comp] = false;
    }
}

std::vector<node> DTree::getPath(node u, node v) {
    //Check for valid path request
    assert(query(u, v));

    if (u == v) return {u};

    node ancestor = none, uMove = u, vMove = v;

    //We move up the tree from both vertices one step at a time to find the earliest ancestor
    bool moveU = true;

    while (marked[uMove] == false && marked[vMove] == false) {
        if (moveU) {
            if (uMove != none) {
                //Mark the vertex in the path of u as visited and move up one
                marked[uMove] = true;
                uMove = parent_[uMove];
            }
        } else {
            if (vMove != none) {
                //Mark the vertex in the path of u as visited
                marked[vMove] = true;
                vMove = parent_[vMove];
            }
        }

        moveU = !moveU;

        //Check if both reached the root and it's not the same root
        if (uMove == none && vMove == none) throw std::runtime_error("Path doesn't exist");
    }

    //We now know that one of the vertices from moving up is positioned on the earliest ancestor
    if (uMove != none && marked[uMove] == true) ancestor = uMove;
    else ancestor = vMove;

    //Now we restore the marked property
    uMove = u, vMove = v;
    while (uMove != none && marked[uMove] == true) {
        marked[uMove] = false;
        uMove = parent_[uMove];
    }
    while (vMove != none && marked[vMove] == true) {
        marked[vMove] = false;
        vMove = parent_[vMove];
    }

    //Get the path of u and v to the ancestor
    std::vector<node> pathUA, pathVA;

    uMove = u, vMove = v;
    while (uMove != ancestor) {
        pathUA.push_back(uMove);
        uMove = parent_[uMove];
    }
    while (vMove != ancestor) {
        pathVA.push_back(vMove);
        vMove = parent_[vMove];
    }

    //Now combine the two paths by attaching pathVA inverted to pahUA
    pathUA.push_back(ancestor);
    for (count i = pathVA.size(); i > 0; i--) {
        pathUA.push_back(pathVA[i - 1]);
    }

    return pathUA;
}

void DTree::addEdge(node v, node w) {
    assert(compID[v] != none && compID[w] != none);

    Init(v, w);

    SanityCheck();
    if (Conn(v, w)) {
        InsertNTEdge(v, w);
    } else {
        InsertTreeEdge(v, Find(v), w, Find(w));
    }
    SanityCheck();
}

void DTree::deleteEdge(node v, node w) {
    assert(compID[v] != none && compID[w] != none);

    //    SanityCheck();
    if (IsTreeEdge(v, w)) {
        DeleteTreeEdge(v, w);
    } else {
        DeleteNTEdge(v, w);
    }
    SanityCheck();
}

void DTree::restoreNode(node v) {
    assert(compID[v] == none);

    compID[v] = compNum;
    compNum++;
    compRep.push_back(v);
    dirtyComp.push_back(false);
}

void DTree::deleteNode(node v) {
    assert(compID[v] != none);

    //Delete all nontree-edges first so they can't be used as replacements
    for (std::pair<node, size_t> e: non_tree_neighbors_[v]) {
        DeleteNTEdge(v, e.first);
    }
    //Now delete all tree neighbours
    while (not children_[v].empty()) {
        DeleteTreeEdge(v, children_[v].back());
    }
    if (parent_[v] != none) {
        DeleteTreeEdge(v, parent_[v]);
    }

    //Now declare the component of v nonexistent
    count id = compID[v];
    //Swap the compID of the currently highest component down to use this ID now
    compRep[id] = compRep[compNum - 1];
    compID[compRep[id]] = id;

    checkDirtyListOvergrow();
    if (!dirtyListBlocked && !dirtyComp[id]) dirtyList.push_back(id);
    dirtyPartition = true;

    compNum--;
    compRep.resize(compNum);
    dirtyComp.resize(compNum);

    compID[v] = none;
}

void DTree::checkDirtyListOvergrow() {
    if (!dirtyListBlocked && dirtyList.size() > num_nodes_ / 2) {
        dirtyListBlocked = true;
        dirtyList.resize(0);
    }
}

void DTree::Init(node u, node v) {
    u = std::max(u, v);
    if (u >= num_nodes_) {
        count new_nn = u + 1;
        parent_.resize(new_nn, none);
        child_idx_.resize(new_nn, -1);
        size_.resize(new_nn, 1);
        children_.resize(new_nn);
        non_tree_neighbors_.resize(new_nn);

        //Note down new number of connected components
        //For now every new node is a new connected component and as such gets its own ID and represents itself
        compID.resize(new_nn);
        for (count i = num_nodes_; i < new_nn; i++)
            compID[i] = (i - num_nodes_ + compNum);
        compRep.resize(compNum + (new_nn - num_nodes_));
        dirtyComp.resize(compNum + (new_nn - num_nodes_), false);
        for (count i = 0; i < new_nn - num_nodes_; i++)
            compRep[compNum + i] = num_nodes_ + i;
        compNum += new_nn - num_nodes_;

        marked.resize(new_nn, false);

        num_nodes_ = new_nn;
    }
    SanityCheck();
}

node DTree::Find(node u, bool allow_reroot) {
    SanityCheck();
    node last_child = none;
    while (parent_[u] != none) {
        last_child = u;
        u = parent_[u];
    }
    if (allow_reroot && last_child != none and size_[last_child] > size_[u] / 2) {
        Reroot(last_child);
        return last_child;
    }
    return u;
}

bool DTree::Conn(node v, node w) {
    SanityCheck();
    //Check id nodes don't exist
    if (v >= num_nodes_ || w >= num_nodes_) return false;
    if (compID[v] == none || compID[w] == none) return false;
    if (w == v) {
        return true;
    }
    v = Find(v);
    w = Find(w);
    v = Find(v); // again to guard against wrong root when Find(w) did a reroot
    SanityCheck();
    return v == w;
}

void DTree::AddNTEdge(node u, node v) {
    SanityCheck();
    assert(not IsTreeEdge(u, v));
    assert(std::none_of(non_tree_neighbors_[u].begin(), non_tree_neighbors_[u].end(),
                        [v](const auto &a) { return a.first == v; }));
    assert(std::none_of(non_tree_neighbors_[v].begin(), non_tree_neighbors_[v].end(),
                        [u](const auto &a) { return a.first == u; }));
    assert(u != v);
    non_tree_neighbors_[u].push_back({v, non_tree_neighbors_[v].size()});
    non_tree_neighbors_[v].push_back({u, non_tree_neighbors_[u].size() - 1});
    SanityCheck();
}

bool DTree::IsTreeEdge(node v, node w) {
    assert(v != w);
    return parent_[v] == w or parent_[w] == v;
}

void DTree::Reroot(node v, bool trueRoot) {
    SanityCheck();
    node ch = v, cur = parent_[v];
    CutChild(ch, cur);
    while (cur != none) {
        node g = parent_[cur];
        CutChild(cur, g);
        AddChild(cur, ch);
        ch = cur, cur = g;
    }

    if (trueRoot) {
        // Adjust the component representative accordingly
        count compIDroot = compID[ch];
        compID[v] = compIDroot;
        compRep[compIDroot] = v;
    }

    while (parent_[ch] != none) {
        size_[ch] -= size_[parent_[ch]];
        size_[parent_[ch]] += size_[ch];
        ch = parent_[ch];
    }
    SanityCheck();
}

void DTree::AddChild(node c, node p) {
    SanityCheck();
    parent_[c] = p;
    child_idx_[c] = children_[p].size();
    children_[p].push_back(c);
    SanityCheck();
}

void DTree::CutChild(node c, node p) {
    if (p == none)
        return;
    parent_[c] = none;
    auto c_idx = child_idx_[c];
    if (children_[p].size() > 1 && c_idx < children_[p].size() - 1) {
        children_[p][c_idx] = children_[p].back();
        child_idx_[children_[p][c_idx]] = c_idx;
    }
    child_idx_[c] = -1;
    children_[p].pop_back();
    SanityCheck();
}

void DTree::CutNTEdge(std::pair<node, size_t> e) {
    node u = e.first;
    node u_idx = e.second;
    auto backedge = non_tree_neighbors_[u][u_idx];
    for (int i = 0; i < 2; i++) {
        if (non_tree_neighbors_[u].size() > 1 && u_idx < non_tree_neighbors_[u].size() - 1) {
            non_tree_neighbors_[u][u_idx] = non_tree_neighbors_[u].back();
            auto next_edge = non_tree_neighbors_[u][u_idx];
            non_tree_neighbors_[next_edge.first][next_edge.second].second = u_idx;
        }
        non_tree_neighbors_[u].pop_back();
        u = backedge.first;
        u_idx = backedge.second;
    }
    SanityCheck();
}

void DTree::DeleteNTEdge(node u, node v) {
    for (const auto x: non_tree_neighbors_[u])
        if (x.first == v) {
            CutNTEdge(x);
            break;
        }
    SanityCheck();
}

void DTree::InsertTreeEdge(node v, node rv, node w, node rw) {
    SanityCheck();
    if (size_[rv] < size_[rw]) { // append the smaller to the bigger tree
        std::swap(rv, rw);
        std::swap(v, w);
    }

    //Get the compID of node rw
    count compIDrootW = compID[rw];
    count compIDrootV = compID[rv];

    Reroot(w);
    Link(v, rv, w);

    //Swap the compID of the currently highest component down to use this ID now
    compRep[compIDrootW] = compRep[compNum - 1];
    compID[compRep[compIDrootW]] = compIDrootW;
    //Denote that the partition is now dirty and that the involved connected components need cleaning
    dirtyPartition = true;
    if (!dirtyListBlocked && !dirtyComp[compIDrootV]) dirtyList.push_back(compIDrootV);
    if (!dirtyListBlocked && !dirtyComp[compIDrootW]) dirtyList.push_back(compIDrootW);
    dirtyComp[compIDrootV] = true;
    dirtyComp[compIDrootW] = true;

    compNum--;
    compRep.resize(compNum);
    dirtyComp.resize(compNum);

    SanityCheck();
}

void DTree::Link(node v, node rv, node w) {
    SanityCheck();
    AddChild(w, v);
    auto m = none;
    for (node i = v; i != none; i = parent_[i]) {
        size_[i] += size_[w];
        if (size_[i] > (size_[rv] + size_[w]) / 2 and m == none)
            m = i;
    }
    if (m != none and m != rv)
        Reroot(m);
    SanityCheck();
}

node DTree::Unlink(node v) {
    assert(parent_[v] != none);
    node i, ci;
    for (i = parent_[v], ci = v; i != none; ci = i, i = parent_[i])
        size_[i] -= size_[v];
    CutChild(v, parent_[v]);
    SanityCheck();
    return ci;
}

void DTree::DeleteTreeEdge(node u, node v) {
    assert(IsTreeEdge(u, v));
    if (parent_[v] == u) // make sure u is child of v
        std::swap(u, v);

    node rs = Unlink(u), rl = u;
    //Store if the components were suspicious before
    bool wasDirtyTotal = dirtyPartition, wasDirtyLocal = dirtyComp[compID[rs]];
    count oldID = compID[rs];

    if (size_[rs] > size_[rl]) // make sure rs is the root of the smaller tree
        std::swap(rs, rl);

    //Declare the tree of rl its own component (if a replacement edge is found later this will be reverted)
    compNum++;
    compRep.resize(compNum);
    dirtyComp.resize(compNum);
    compRep[compNum - 1] = rs;
    compID[rs] = compNum - 1;
    compRep[oldID] = rl;
    compID[rl] = oldID;

    dirtyPartition = true;
    dirtyComp[compID[rs]] = true;
    dirtyComp[compID[rl]] = true;

    if (!dirtyListBlocked) dirtyList.push_back(compID[rs]);
    if (!dirtyListBlocked && !wasDirtyLocal) dirtyList.push_back(compID[rl]);

    std::deque<node> queue(1, rs);
    node m = none;
    while (not queue.empty()) {
        node a = queue.front();
        queue.pop_front();
        for (auto [b, i]: non_tree_neighbors_[a]) {
            if (Find(b, false) == rl) {
                DeleteNTEdge(a, b);
                InsertTreeEdge(b, rl, a, rs);
                //The tree has been reconnected, so the dirty states are returned to their previous versions
                //Pop that the (nonexistant) component of lr isn't dirty
                if (!dirtyListBlocked) dirtyList.pop_back();
                //If ls was newly added as dirty remove that notion
                if (!dirtyListBlocked && !wasDirtyLocal) dirtyList.pop_back();
                dirtyPartition = wasDirtyTotal;
                compID[rs] = oldID;
                compID[a] = oldID;
                dirtyComp[oldID] = wasDirtyLocal;

                return;
            }
        }
        if (size_[a] > size_[rs] / 2)
            m = a;
        for (auto c: children_[a])
            queue.push_back(c);
    }


    if (m != none)
        Reroot(m);
    SanityCheck();
}

void DTree::InsertNTEdge(node u, node v) {
    SanityCheck();
    count du, dv;
    node r;
    for (du = 0, r = u; parent_[r] != none; du++)
        r = parent_[r];
    for (dv = 0, r = v; parent_[r] != none; dv++)
        r = parent_[r];
    if (du < dv) { // make sure du > dv
        std::swap(u, v);
        std::swap(du, dv);
    }
    count delta = du - dv;
    if (delta < 2) {
        AddNTEdge(u, v);
    } else {
        node i = u;
        for (int c = 0; c < delta - 2; c++) {
            i = parent_[i];
        }
        node t = parent_[i];
        Unlink(i);
        AddNTEdge(i, t);
        Reroot(u, false);
        Link(v, r, u);
    }
    SanityCheck();
}

void DTree::SanityCheck() {
#ifndef NDEBUG // the following code is only executed in debug mode
    /*
    assert(num_nodes_ >= 0);
    assert(num_nodes_ == G->numberOfNodes());
    assert(parent_.size() == num_nodes_);
    assert(children_.size() == num_nodes_);
    assert(child_idx_.size() == num_nodes_);
    assert(non_tree_neighbors_.size() == num_nodes_);
    for (node i = 0; i < num_nodes_; i++) {
        if (parent_[i] != none) {
            //            assert(G->hasEdge(i, parent_[i]));
            assert(std::count(children_[parent_[i]].begin(), children_[parent_[i]].end(), i) == 1);
            assert(children_[parent_[i]][child_idx_[i]] == i);
        } else
            assert(child_idx_[i] == -1);
        for (node c : children_[i]) {
            //            assert(G->hasEdge(c, i));
            assert(parent_[c] == i);
        }
        for (auto [v, bi] : non_tree_neighbors_[i]) {
            assert(G->hasEdge(v, i));
            assert(non_tree_neighbors_[v][bi].first == i);
            assert(std::count_if(non_tree_neighbors_[v].begin(), non_tree_neighbors_[v].end(),
                                 [&](auto a) { return a.first == i; })
                   == 1);
        }
        auto p = i;
        for (int cc = 0; cc < num_nodes_; cc++) {
            if (p == none)
                break;
            p = parent_[p];
        }
        assert(p == none);
    }
     */
#endif // NDEBUG
}
