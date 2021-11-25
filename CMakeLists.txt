cmake_minimum_required(VERSION 3.19)
project(IPS\ |\ parallel\ grep)
set(CMAKE_SOURCE_DIR src/)
set(PROJECT_BINARY_DIR ../build/)
set(CMAKE_BINARY_DIR ../build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build/)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(pgrep src/pgrep.cc)

target_link_libraries(pgrep pthread)
