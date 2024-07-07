cmake_minimum_required(VERSION 3.11)

project("Catch2Test"
	VERSION 0.1.0
	DESCRIPTION "Catch2 testing"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find SF2ML package
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../install")
find_package(SF2ML 0.1.0 CONFIG REQUIRED)

# find Catch2 package (Catch2 must be installed beforehand!)
find_package(Catch2 3 REQUIRED)

add_executable(${PROJECT_NAME} test.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE sf2ml::SF2ML Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})