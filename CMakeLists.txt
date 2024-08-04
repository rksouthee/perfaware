cmake_minimum_required(VERSION 3.27)

project(perfaware
	VERSION 0.0.1
	LANGUAGES CXX ASM_NASM
	DESCRIPTION "Homework for the Performance-Aware Programming Series"
	)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
add_compile_options($<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall>)

find_package(cxxopts "3.2.1" EXACT REQUIRED)

if(BUILD_TESTING)
	find_package(Catch2 "3.5.4" EXACT REQUIRED)
	include(Catch)
endif()


add_subdirectory(sim86)
add_subdirectory(part2)
