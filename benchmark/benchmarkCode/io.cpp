#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <algorithm>
#include <ctime>
#include <random>

#include "io.hpp"

#include "base.hpp"
#include "DynConnectivityAlgorithm.hpp"
#include "GKKT.hpp"
#include "Wang.hpp"
#include "Kaibel.hpp"
#include "DTree.hpp"
#include "HDT.hpp"

constexpr count maxQueryBlockNum = 50000000;

namespace IO {
std::vector<std::string> splitString(const std::string &s) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);

    std::string buf;
    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

std::vector<Action> fileToActionList(const std::string &f) {
    std::string currentLine;

    std::ifstream openFile(f);

    std::vector<Action> actions;

    while (getline(openFile, currentLine)) {
        auto entries = splitString(currentLine);
        if (entries.empty())
            continue;
        node u, v;
        switch (entries[0][0]) {
        case 'a':
            u = std::stoi(entries[1]);
            v = std::stoi(entries[2]);
            actions.push_back({ActionType::addAction, u, v});
            break;
        case 'd':
            u = std::stoi(entries[1]);
            v = std::stoi(entries[2]);
            actions.push_back(Action{ActionType::deleteAction, u, v});
            break;
        case 'q':
            u = std::stoi(entries[1]);
            v = std::stoi(entries[2]);
            actions.push_back(Action{ActionType::queryAction, u, v});
            break;
        case 'b':
            actions.push_back(Action{ActionType::queryBlock, 0, 0});
            break;
        case 't':
            actions.push_back(Action{ActionType::timerAction, 0, 0});
            break;
        case 'c':
            break;
        default:
            throw std::runtime_error("File format error. First char was " + entries[0]);
        }
    }
    openFile.close();
    return actions;
}

void actionListToFile(const std::vector<Action> &actions, const std::string &f, const std::string &info) {
    std::ofstream fileWriter(f);
    // Note all the details of the test in a comment
    fileWriter << "c file: " << f << "\n";
    fileWriter << "c " << info << "\n";
    for (auto a: actions) {
        switch (a.type) {
        case ActionType::addAction:
            fileWriter << "a " << a.u << " " << a.v << "\n";
            break;
        case ActionType::deleteAction:
            fileWriter << "d " << a.u << " " << a.v << "\n";
            break;
        case ActionType::queryAction:
            fileWriter << "q " << a.u << " " << a.v << "\n";
        }
    }
}

std::vector<Edge> readEdgeList(const std::string &fname) {
    std::ifstream input(fname);
    std::string s;
    std::set<Edge> edges;
    while (std::getline(input, s)) {
        if (s[0] == '%' or s[0] == '#')
            continue;
        auto entries = splitString(s);
        auto u = std::stoi(entries[0]), v = std::stoi(entries[1]);
        if (u > v)
            std::swap(u, v);
        if (u == v)
            continue;
        edges.emplace(u, v);
    }
    return {edges.begin(), edges.end()};
}

std::string runBenchmark(const std::vector<Action> &actions, std::string algo, count n, std::string type, count seed) {
    std::mt19937 rng;
    rng.seed(42);
    std::uniform_int_distribution<uint32_t> randomNumber(0, n);

    Graph G(n);

    std::shared_ptr<DynConnectivity> connectivity;

    if (algo == "DTree"){
        connectivity = std::make_shared<DTree>(G);
    }
    else if (algo == "GKKT[base]"){
        connectivity = std::make_shared<GKKT>(G, 1, 1);
    }
    else if (algo == "GKKT[pHeu]") {
        connectivity = std::make_shared<GKKT>(G, 1, 1, 1, GKKT::Mode::pHeu);
    }
    else if (algo == "GKKT[lvlHeu]") {
        connectivity = std::make_shared<GKKT>(G, 1, 1, 1, GKKT::Mode::lvlHeu);
    }
    else if (algo == "Wang[base]"){
        connectivity = std::make_shared<Wang>(G, 1, 1);
    }
    else if (algo == "Wang[pHeu]") {
        connectivity = std::make_shared<Wang>(G, 1, 1, 1, Wang::Mode::pHeu);
    }
    else if (algo == "Wang[lvlHeu]") {
        connectivity = std::make_shared<Wang>(G, 1, 1, 1, Wang::Mode::lvlHeu);
    }
    else if (algo == "Kaibel[base]"){
        connectivity = std::make_shared<Kaibel>(G, 1, 1);
    }
    else if (algo == "Kaibel[pHeu]") {
        connectivity = std::make_shared<Kaibel>(G, 1, 1, 1, Kaibel::Mode::pHeu);
    }
    else if (algo == "Kaibel[lvlHeu]") {
        connectivity = std::make_shared<Kaibel>(G, 1, 1, 1, Kaibel::Mode::lvlHeu);
    }
    else if (algo == "HDT") {
        connectivity = std::make_shared<HDT>(G);
    }
    else if (algo == "DynConnectivityOneForestConnect"){
        //TDOD port over OneForestConnect and HDT
        // connectivity = std::make_shared<DynConnectivityOneForestConnect>(G);
    }
    else throw std::runtime_error("Not a valid algorithm");

    if (type == "regular") {
        clock_t runtime = runAllUpdatesTime(actions, connectivity, G);
        return std::to_string(runtime);
    }
    else if (type == "maxTimeUpdate") {
        std::pair<clock_t, clock_t> runtime = runSingleUpdateTime(actions, connectivity, G);
        return std::to_string(runtime.first) + "," + std::to_string(runtime.second);
    }
    else if (type == "correctnessCheck") {
        std::pair<std::pair<count, count>, std::pair<count, count>> errors = runAccuracyCheck(actions, connectivity, G);
        return std::to_string(errors.first.first) + "," + std::to_string(errors.first.second) + "," + std::to_string(errors.second.first) + "," + std::to_string(errors.second.second);
    }
    else if (type == "queryTime") {
        clock_t tSum = runQueryTime(actions, connectivity, G);
        return std::to_string(tSum);
    }
    else
        throw std::runtime_error("no valid mode selected");
}

clock_t runAllUpdatesTime(const std::vector<Action> &actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G) {
    auto t0 = clock();

    for(Action act : actions){
        if(act.type == ActionType::addAction){
            connectivity->addEdge(act.u, act.v);
        }
        else if(act.type == ActionType::deleteAction){
            connectivity->deleteEdge(act.u, act.v);
        }
        else if(act.type == ActionType::queryAction){
            connectivity->query(act.u, act.v);
        }
        else if(act.type == ActionType::queryBlock){
            throw std::runtime_error("there should not be query blocks in a single operation time test!");
        }
        else if(act.type == ActionType::timerAction) {
            t0 = clock();
        }
    }

    return clock() - t0;
}

std::pair<clock_t, clock_t> runSingleUpdateTime(const std::vector<Action> &actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G) {
    auto t0 = clock();
    auto t1 = clock();
    clock_t tMaxAdd = 0, tMaxDel = 0;
    bool currentlyTiming = true;

    for(Action act : actions){
        if(act.type == ActionType::addAction){
            t0 = clock();
            connectivity->addEdge(act.u, act.v);
            tMaxAdd = std::max(tMaxAdd, (clock() - t0));
        }
        else if(act.type == ActionType::deleteAction){
            t0 = clock();
            connectivity->deleteEdge(act.u, act.v);
            tMaxDel = std::max(tMaxDel, (clock() - t0));
        }
        else if(act.type == ActionType::queryAction){
            throw std::runtime_error("there should not be query actions in a single operation time test!");
        }
        else if(act.type == ActionType::queryBlock){
            throw std::runtime_error("there should not be query blocks in a single operation time test!");
        }
        else if(act.type == ActionType::timerAction) {
            tMaxAdd = 0;
            tMaxDel = 0;
        }
    }

    return {tMaxAdd, tMaxDel};
}

clock_t runQueryTime(const std::vector<Action> &actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G) {
    auto t0 = clock();
    auto t1 = clock();
    clock_t tSum = 0;
    bool currentlyTiming = true;

    std::mt19937 rng;
    rng.seed(42);
    std::uniform_int_distribution<uint32_t> randomNumber(0, 1000000000);

    for(Action act : actions){
        if(act.type == ActionType::addAction){
            connectivity->addEdge(act.u, act.v);
        }
        else if(act.type == ActionType::deleteAction){
            connectivity->deleteEdge(act.u, act.v);
        }
        else if(act.type == ActionType::queryAction){
            throw std::runtime_error("there should not be query actions in a single operation time test!");
        }
        else if(act.type == ActionType::queryBlock){
            std::vector<std::pair<node, node>> queries;

            for (uint i = 0; i < 10000; i++) {
                node u = randomNumber(rng) % G.getN();
                node v = randomNumber(rng) % G.getN();
                queries.emplace_back(u, v);
            }

            clock_t tStart = clock();
            for (std::pair<node, node> query : queries) {
                connectivity->query(query.first, query.second);
            }

            tSum += (clock() - tStart);
        }
        else if(act.type == ActionType::timerAction) {
            tSum = 0;
        }
    }

    return tSum;
}

std::pair<std::pair<count, count>, std::pair<count, count>> runAccuracyCheck(const std::vector<Action> & actions, std::shared_ptr<DynConnectivity> connectivity, Graph &G) {
    DTree correct(G);

    std::mt19937 rng;
    rng.seed(42);
    std::uniform_int_distribution<uint32_t> randomNumber(0, 1000000000);

    count generalChecks = 0, errorStates = 0, queryChecks = 0, queryErrors = 0;

    for(Action act : actions){
        if(act.type == ActionType::addAction){
            connectivity->addEdge(act.u, act.v);
            correct.addEdge(act.u, act.v);

            generalChecks++;
            if (connectivity->numberOfComponents() != correct.numberOfComponents()) {
                errorStates++;

                for (uint i = 0; i < 1000; i++) {
                    queryChecks++;
                    node u = randomNumber(rng) % G.getN();
                    node v = randomNumber(rng) % G.getN();
                    if (connectivity->query(u,v) != correct.query(u, v))
                        queryErrors++;
                }
            }
        }
        else if(act.type == ActionType::deleteAction){
            connectivity->deleteEdge(act.u, act.v);
            correct.deleteEdge(act.u, act.v);

            generalChecks++;
            if (connectivity->numberOfComponents() != correct.numberOfComponents()) {
                errorStates++;

                for (uint i = 0; i < 1000; i++) {
                    queryChecks++;
                    node u = randomNumber(rng) % G.getN();
                    node v = randomNumber(rng) % G.getN();
                    if (connectivity->query(u,v) != correct.query(u, v))
                        queryErrors++;
                }
            }
        }
        else if(act.type == ActionType::queryAction){
            connectivity->query(act.u, act.v);
        }
        else if(act.type == ActionType::queryBlock){
            throw std::runtime_error("there should not be query blocks in a single operation time test!");
        }
        else if(act.type == ActionType::timerAction) {
            generalChecks = 0;
            errorStates = 0;
            queryChecks = 0;
            queryErrors = 0;
        }
    }

    return {{generalChecks, errorStates}, {queryChecks, queryErrors}};
}

} // IO