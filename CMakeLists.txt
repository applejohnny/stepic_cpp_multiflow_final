cmake_minimum_required (VERSION 2.8)

project(stepic_cpp_multiflow_final)

find_package(Threads)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(final main.cpp server.cpp)

target_link_libraries(final ${CMAKE_THREAD_LIBS_INIT})