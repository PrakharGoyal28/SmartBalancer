cmake_minimum_required(VERSION 3.10)
project(LB)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(include)

add_executable(lb
    src/main.cpp
    src/listener.cpp
    src/session.cpp
)

target_link_libraries(lb Boost::system pthread)