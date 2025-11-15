cmake_minimum_required(VERSION 3.16)
project(PacMan)

set(CMAKE_CXX_STANDARD 17)

set(GF_CONFIG_FILE "/usr/local/lib/cmake/gf/gfConfig.cmake")

if(NOT EXISTS ${GF_CONFIG_FILE})
    set(GF_CONFIG_FILE "/usr/lib/cmake/gf/gfConfig.cmake")
endif()

include(${GF_CONFIG_FILE})

include_directories(common)

add_executable(server server/main.cc)
target_link_libraries(server PRIVATE gf::core gf::graphics gf::net)

add_executable(client client/main.cc)
target_link_libraries(client PRIVATE gf::core gf::graphics gf::net)