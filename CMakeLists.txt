cmake_minimum_required(VERSION 3.8)
project(Client)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES client.cpp)

add_executable(Client src/client.cpp src/connectionHandler.cpp src/EncoderDecoder.cpp)

find_package(Boost REQUIRED COMPONENTS thread)
target_link_libraries(Client Boost::thread)