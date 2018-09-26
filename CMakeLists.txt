cmake_minimum_required(VERSION 3.12)
project(Client)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES client.cpp)

set(BOOST_ROOT "/usr/local/Cellar/boost/1.67.0_1")


message(STATUS "start running cmake...")

find_package(Boost 1.67.0 COMPONENTS thread system REQUIRED)

if(Boost_FOUND)

    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})

endif()

add_executable(Client src/client.cpp src/connectionHandler.cpp src/EncoderDecoder.cpp)

if(Boost_FOUND)

    target_link_libraries(Client ${Boost_LIBRARIES})

endif()