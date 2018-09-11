cmake_minimum_required(VERSION 3.8.2)
project(Client)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES client.cpp)

add_executable(Client src/client.cpp src/connectionHandler.cpp src/EncoderDecoder.cpp)

find_package(Boost COMPONENTS thread system REQUIRED)




target_link_libraries(Client ${Boost_LIBRARIES})

