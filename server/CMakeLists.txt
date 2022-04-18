cmake_minimum_required(VERSION 3.0.0)
project(server VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(server main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread") #why ? read this https://stackoverflow.com/a/1665110