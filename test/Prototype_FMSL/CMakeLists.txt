cmake_minimum_required(VERSION 3.10)
project(PacmanNetwork)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

add_executable(server server/main.cc)
target_link_libraries(server sfml-network sfml-system)

add_executable(client client/main.cc)
target_link_libraries(client sfml-graphics sfml-window sfml-system sfml-network)
