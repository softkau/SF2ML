cmake_minimum_required(VERSION 3.11)

set(namespace "sf2ml")
project(SF2ML
	VERSION 0.1.0
	DESCRIPTION "Soundfont2 Manipulation Library"
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
	PRIVATE
		src/sf2ml.cpp
		src/sfinfo.cpp
		src/sfinstrument.cpp
		src/sfinstrumentzone.cpp
		src/sfloader.cpp
		src/sfmap.cpp
		src/sfpreset.cpp
		src/sfpresetzone.cpp
		src/sfsample.cpp
		src/sfserializer.cpp
		src/sftypes.cpp
		src/wav_utility.cpp
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
	include/sf2ml.hpp
	include/sfgenerator.hpp
	include/sfhandle.hpp
	include/sfinfo.hpp
	include/sfinstrument.hpp
	include/sfinstrumentzone.hpp
	include/sfpreset.hpp
	include/sfpresetzone.hpp
	include/sfsample.hpp
	include/sfspec.hpp
	include/sftypes.hpp
	include/wavspec.hpp
)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Installing)