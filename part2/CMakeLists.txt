add_subdirectory(json)

add_executable(haversine_generator haversine_generator.cpp)
target_link_libraries(haversine_generator PRIVATE cxxopts::cxxopts)

add_executable(haversine haversine.cpp)
target_link_libraries(haversine PRIVATE json)

add_library(platform_metrics STATIC platform_metrics.cpp platform_metrics.h)
add_executable(listing_0075_timed_haversine_main listing_0075_timed_haversine_main.cpp)
target_link_libraries(listing_0075_timed_haversine_main PRIVATE platform_metrics json)

option(PROFILER "Enable profiling" OFF)
add_library(profiler STATIC profiler.cpp profiler.h)
target_link_libraries(profiler PRIVATE platform_metrics)
target_compile_definitions(profiler PUBLIC PROFILER=$<BOOL:${PROFILER}>)

add_executable(profiled_haversine_main profiled_haversine_main.cpp)
target_link_libraries(profiled_haversine_main PRIVATE profiler platform_metrics json)
