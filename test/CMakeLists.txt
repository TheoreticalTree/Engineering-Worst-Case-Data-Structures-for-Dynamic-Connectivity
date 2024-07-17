macro(package_add_test TESTNAME)
    # create an executable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_include_directories(${TESTNAME} PRIVATE ../include)
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ..
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
            )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test(basic basic.cpp)

package_add_test(BBTNodes BBTNodes.cpp
        ../src/BiasedBinaryForest.cpp)

package_add_test(BBTForest BBTForest.cpp
        ../src/BiasedBinaryForest.cpp)

package_add_test(BBTTracking BBTTracking.cpp
        ../src/BiasedBinaryForest.cpp)

package_add_test(BBTTree BBTTree.cpp
        ../src/BiasedBinaryTree.ipp)

package_add_test(LinkCutTrees
        ../src/BiasedBinaryForest.cpp
        ../src/BiasedBinaryHeap.ipp
        ../src/LinkCutTrees.cpp

        LinkCutTrees.cpp)

package_add_test(AVLTree AVLTree.cpp
        ../src/AVLTree.ipp)

package_add_test(ETForestCutSet ETForestCutSet.cpp
        ../src/ETForestCutSet.cpp)

package_add_test(TabularHash TabularHash.cpp
        ../src/TabularHash.cpp)

package_add_test(CutSet CutSet.cpp
        ../src/CutSet.cpp
        ../src/TabularHash.cpp
        ../src/ETForestCutSet.cpp
        ../src/AVLTree.ipp
        ../src/DTree.cpp
        ../src/Graph.cpp)

package_add_test(GKKT GKKT.cpp
        ../src/CutSet.cpp
        ../src/TabularHash.cpp
        ../src/ETForestCutSet.cpp
        ../src/AVLTree.ipp
        ../src/DTree.cpp
        ../src/Graph.cpp
        ../src/GKKT.cpp
        ../src/BiasedBinaryForest.cpp
        ../src/BiasedBinaryHeap.ipp
        ../src/LinkCutTrees.cpp
        ../src/QueryForestAVL.cpp)

package_add_test(Wang Wang.cpp
        ../src/CutSet.cpp
        ../src/TabularHash.cpp
        ../src/ETForestCutSet.cpp
        ../src/AVLTree.ipp
        ../src/DTree.cpp
        ../src/Graph.cpp
        ../src/Wang.cpp
        ../src/BiasedBinaryForest.cpp
        ../src/BiasedBinaryHeap.ipp
        ../src/LinkCutTrees.cpp
        ../src/QueryForestAVL.cpp)

package_add_test(Kaibel Kaibel.cpp
        ../src/CutSet.cpp
        ../src/TabularHash.cpp
        ../src/ETForestCutSet.cpp
        ../src/AVLTree.ipp
        ../src/DTree.cpp
        ../src/Graph.cpp
        ../src/Kaibel.cpp
        ../src/BiasedBinaryForest.cpp
        ../src/BiasedBinaryHeap.ipp
        ../src/LinkCutTrees.cpp
        ../src/QueryForestAVL.cpp)

package_add_test(HDT HDT.cpp
        ../src/DTree.cpp
        ../src/Graph.cpp
        ../src/AVLTree.ipp
        ../src/ETForestHDT.cpp
        ../src/HDT.cpp
)