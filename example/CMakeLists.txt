cmake_minimum_required(VERSION 3.11)

project("TestAppForSF2ML"
	VERSION 0.1.0
	DESCRIPTION "External Library Example"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../install")
find_package(SF2ML 0.1.0 CONFIG REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE sf2ml::SF2ML)