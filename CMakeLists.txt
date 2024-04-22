cmake_minimum_required(VERSION 3.27)

project(perfaware
	VERSION 0.0.1
	LANGUAGES CXX
	DESCRIPTION "Homework for the Performance-Aware Programming Series"
)

enable_testing()

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(sim86)
