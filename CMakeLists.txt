cmake_minimum_required(VERSION 3.28)
project(OK_grasp)

set(CMAKE_CXX_STANDARD 17)

add_executable(OK_grasp
        main.cpp
        DataLoader.h
        DataPoint.h
        DataPoint.cpp
        DataLoader.cpp
        Problem.h
        Problem.h
        Problem.cpp
        Graph.h
        Graph.cpp
        Solution.cpp
        ResultSaver.h
        utils.h
        utils.cpp
)