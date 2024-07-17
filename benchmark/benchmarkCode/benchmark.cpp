#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include "json.hpp"


#include "base.hpp"


#include "io.hpp"
#include "graph_generator.hpp"

#include "cxxopts.hpp"

std::string runExperiment(const std::string& algo_name, unsigned long n, std::string density, double pstart, double qf, std::string inst_fname, std::string mode, unsigned seed){
    double edgeChance;
    if(density == "4s") edgeChance = (double) 4/n;
    else if(density == "20s") edgeChance = (double) 20/n;
    else if(density == "4logn") edgeChance = (double) 4*log2(n)/n;
    else if(density == "6logn") edgeChance = (double) 6*log2(n)/n;
    else if(density == "8logn") edgeChance = (double) 8*log2(n)/n;
    else if(density == "10logn") edgeChance = (double) 10*log2(n)/n;
    else if(density == "12logn") edgeChance = (double) 12*log2(n)/n;
    else if(density == "14logn") edgeChance = (double) 14*log2(n)/n;
    else if(density == "16logn") edgeChance = (double) 16*log2(n)/n;
    else if(density == "18logn") edgeChance = (double) 18*log2(n)/n;
    else if(density == "20logn") edgeChance = (double) 20*log2(n)/n;
    else if(density == "4sqrtn") edgeChance = (double) 4*sqrt(n)/n;
    else if(density == "6sqrtn") edgeChance = (double) 6*sqrt(n)/n;
    else if(density == "8sqrtn") edgeChance = (double) 8*sqrt(n)/n;
    else if(density == "10sqrtn") edgeChance = (double) 10*sqrt(n)/n;
    else if(density == "12sqrtn") edgeChance = (double) 12*sqrt(n)/n;
    else if(density == "14sqrtn") edgeChance = (double) 14*sqrt(n)/n;
    else if(density == "16sqrtn") edgeChance = (double) 16*sqrt(n)/n;
    else if(density == "18sqrtn") edgeChance = (double) 18*sqrt(n)/n;
    else if(density == "20sqrtn") edgeChance = (double) 20*sqrt(n)/n;
    else edgeChance = 0;

    std::vector<IO::Action> actionList;

    if(0 == inst_fname.compare(inst_fname.length() - 8, 8, "test.txt")){
        std::vector<IO::Edge> graph = GraphGen::createRandomGraph(n, edgeChance, 42);

        actionList = GraphGen::MixedTestFromGraph(graph, pstart, 0.5, 100*n, seed, qf);
    }
    else {
        actionList = IO::fileToActionList(inst_fname);

        //Get the correct query frequency
        std::ifstream openFile(inst_fname);
        std::string line;
        getline(openFile, line);
        auto firstLine = IO::splitString(line);
        qf = std::stof(firstLine[3]);
        openFile.close();

        //Figure out n from the action list
        n = 0;
        for(IO::Action act : actionList){
            n = std::max(n, std::max(act.u, act.v));
        }
        n++;
    }

    std::string runtime = IO::runBenchmark(actionList, algo_name, n, mode, seed);

    return mode + "," + algo_name + "," + std::to_string(n) + "," + density + "," + std::to_string(qf) + "," + inst_fname + "," + runtime + "\n";
}

/**
 * The main requires the algorithm type and test size in that order
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    cxxopts::Options options(
        "benchmark_dyn_connectivity", "Runs an experiment for our dynamic connectivity code");
    options.add_options()("a,algo", "Choose algorithm to be run", cxxopts::value<std::string>())
        ("n,number", "Choose the size of the test (if none is given from the instance)", cxxopts::value<int>()->default_value("1"))
        ("d,density","Choose the density of the test (iX where i in {4,20} and X in {s,logn,sqrtn} for linear, logarithmic or root many adjacent edges on average)", cxxopts::value<std::string>()->default_value("0"))
        ("i,instance", "Choose instance file to be run", cxxopts::value<std::string>())
        ("o,output", "Specify output file", cxxopts::value<std::string>())
        ("ps, p_start" , "Specify amount of edges before start", cxxopts::value<double>()->default_value("0"))
        ("s,seed", "", cxxopts::value<unsigned>()->default_value("1234"))
        ("qf,query_frequency", "Expected number of queries per update", cxxopts::value<double>()->default_value("0"))
        ("m,mode", "Choose mode between maxTimeUpdate, correctnessCheck, queryTime or regular", cxxopts::value<std::string>()->default_value("regular"));

    options.parse_positional({"instance", "output"});
    auto arguments = options.parse(argc, argv);
    const std::string algo_name = arguments["algo"].as<std::string>();
    const std::string density_arg = arguments["density"].as<std::string>();
    const unsigned long n = arguments["number"].as<int>();
    const std::string inst_fname = arguments["instance"].as<std::string>();
    const std::string output_fname = arguments["output"].as<std::string>();
    const std::string mode = arguments["mode"].as<std::string>();
    const double p_start = arguments["p_start"].as<double>();
    const unsigned seed = arguments["seed"].as<unsigned>();
    const double qf = arguments["qf"].as<double>();

    std::string ret = runExperiment(algo_name, n, density_arg, p_start, qf, inst_fname, mode, seed);

    std::fstream output(output_fname);
    output << ret;
    output.close();
    //std::cout << ret;
    //std::cout << output_fname << "\n";

    return 0;
}