cmake_minimum_required(VERSION 3.11)

set(namespace "sf2ml")
project(SF2ML
	VERSION 0.1.0
	DESCRIPTION "Soundfont2 Manipulation Library"
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	PRIVATE
		src/instrument_manager.cpp
		src/preset_manager.cpp
		src/sample_manager.cpp
		src/sfinfo.cpp
		src/sfinstrument.cpp
		src/sfinstrumentzone.cpp
		src/sfpreset.cpp
		src/sfpresetzone.cpp
		src/sfsample.cpp
		src/sflib.cpp
		src/sfmap.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

### target_compile_features(SFML PUBLIC cxx_std_20)

set(public_headers
	include/sflib.hpp
	include/sfml/sfhandle.hpp
	include/sfml/sfhandleinterface.hpp
	include/sfml/sfinfo.hpp
	include/sfml/sfinstrument.hpp
	include/sfml/sfinstrumentzone.hpp
	include/sfml/sfpreset.hpp
	include/sfml/sfpresetzone.hpp
	include/sfml/sfsample.hpp
	include/sfml/sfspec.hpp
	include/sfml/wavspec.hpp
)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Installing)