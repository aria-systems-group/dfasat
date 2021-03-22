cmake_minimum_required(VERSION 3.15)
project(dfasat)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

include_directories(./lib)

add_executable(dfasat
        evaluation/aike.cpp
        evaluation/aike.h
        evaluation/alergia.cpp
        evaluation/alergia.h
        evaluation/alergia94.cpp
        evaluation/alergia94.h
        evaluation/conflict-driven-edsm.cpp
        evaluation/conflict-driven-edsm.h
        evaluation/conflict-driven-overlap.cpp
        evaluation/conflict-driven-overlap.h
        evaluation/conflict-driven.cpp
        evaluation/conflict-driven.h
        evaluation/depth-driven.cpp
        evaluation/depth-driven.h
        evaluation/evidence-driven.cpp
        evaluation/evidence-driven.h
        evaluation/fixed_depth_regression.cpp
        evaluation/fixed_depth_regression.h
        evaluation/full-overlap-driven.cpp
        evaluation/full-overlap-driven.h
        evaluation/kldistance.cpp
        evaluation/kldistance.h
        evaluation/likelihood.cpp
        evaluation/likelihood.h
        evaluation/mealy.cpp
        evaluation/mealy.h
        evaluation/mse-error.cpp
        evaluation/mse-error.h
        evaluation/num_count.cpp
        evaluation/num_count.h
        evaluation/overlap-driven.cpp
        evaluation/overlap-driven.h
        evaluation/overlap4logs.cpp
        evaluation/overlap4logs.h
        evaluation/process-mining.cpp
        evaluation/process-mining.h
        lib/apta.h
        lib/common.h
        lib/conflict_graph.h
        lib/dfasat.h
        lib/evaluate.h
        lib/evaluation_factory.h
        lib/evaluators.h
        lib/inputdata.h
        lib/interactive.h
        lib/json.hpp
        lib/parameters.h
        lib/random_greedy.h
        lib/refinement.h
        lib/searcher.h
        lib/state_merger.h
        lib/stream.h
        lib/formal_language.h
        lib/safety.h
        apta.cpp
        conflict_graph.cpp
        dfasat.cpp
        evaluate.cpp
        evaluation_factory.cpp
        gitversion.cpp
        inputdata.cpp
        interactive.cpp
        main.cpp
        parameters.cpp
        random_greedy.cpp
        refinement.cpp
        searcher.cpp
        state_merger.cpp
        stream.cpp
        safety.cpp)

target_link_libraries(dfasat LINK_PUBLIC popt)
target_link_libraries(dfasat LINK_PUBLIC gsl)
target_link_libraries(dfasat LINK_PUBLIC gslcblas)
target_link_libraries(dfasat LINK_PUBLIC yaml-cpp)


