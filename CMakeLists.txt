cmake_minimum_required(VERSION 3.16)
project(GKKT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# project
add_subdirectory(src)

# executable
add_executable(test_main
        src/main.cpp
        src/BiasedBinaryForest.cpp
        src/BiasedBinaryHeap.ipp
        src/LinkCutTrees.cpp
        src/ETForestCutSet.cpp
        src/ETForestHDT.cpp
        src/AVLTree.ipp
        src/TabularHash.cpp
        src/CutSet.cpp
        src/QueryForestAVL.cpp
        src/DTree.cpp
        src/Graph.cpp
        src/GKKT.cpp
        src/Wang.cpp
        src/Kaibel.cpp
        src/HDT.cpp
        )

target_include_directories(test_main
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include)

# benchmarking executable
add_executable(benchmark
        benchmark/benchmarkCode/benchmark.cpp
        benchmark/benchmarkCode/graph_generator.cpp
        benchmark/benchmarkCode/io.cpp
        src/BiasedBinaryForest.cpp
        src/BiasedBinaryHeap.ipp
        src/LinkCutTrees.cpp
        src/ETForestCutSet.cpp
        src/ETForestHDT.cpp
        src/AVLTree.ipp
        src/TabularHash.cpp
        src/CutSet.cpp
        src/QueryForestAVL.cpp
        src/DTree.cpp
        src/Graph.cpp
        src/GKKT.cpp
        src/Wang.cpp
        src/Kaibel.cpp
        src/HDT.cpp
        )

target_include_directories(benchmark
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include)

# overwrite install() command with a dummy macro that is a nop
macro (install)
endmacro ()


# tests
option(TESTS "Build the tests" ON)
if (TESTS)
    enable_testing()
    add_subdirectory(extlibs/googletest)
    include(GoogleTest)
    add_subdirectory(test)
endif ()

# benchmark
option(BENCHMARK "Additional Code" OFF)
if (BENCHMARK)
    add_subdirectory(extlibs/googlebenchmark)
    add_executable(test_benchmark)
    target_include_directories(test_benchmark
            PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            )
    target_link_libraries(test_benchmark
            benchmark::benchmark)
endif ()