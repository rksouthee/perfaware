add_subdirectory(json)

add_executable(haversine_generator haversine_generator.cpp)
target_link_libraries(haversine_generator PRIVATE cxxopts::cxxopts)

add_executable(haversine haversine.cpp)
target_link_libraries(haversine PRIVATE json)

add_library(platform_metrics STATIC platform_metrics.cpp platform_metrics.h)
add_executable(listing_0075_timed_haversine_main listing_0075_timed_haversine_main.cpp)
target_link_libraries(listing_0075_timed_haversine_main PRIVATE platform_metrics json)

option(PROFILER "Enable profiling" OFF)
add_library(profiler STATIC profiler.cpp profiler.h repetition_tester.cpp repetition_tester.h)
target_link_libraries(profiler PUBLIC platform_metrics)
target_compile_definitions(profiler PUBLIC PROFILER=$<BOOL:${PROFILER}>)

add_executable(profiled_haversine_main profiled_haversine_main.cpp)
target_link_libraries(profiled_haversine_main PRIVATE profiler json)

add_executable(time_file_read time_file_read.cpp)
target_link_libraries(time_file_read PRIVATE profiler)

add_executable(count_page_faults count_page_faults.cpp)
target_link_libraries(count_page_faults PRIVATE platform_metrics)

add_executable(profile_move_bytes profile_move_bytes.cpp move_bytes.asm)
target_link_libraries(profile_move_bytes PRIVATE profiler)

# Part 3: 9. CPU Front End Basics
add_executable(time_front_end time_front_end.cpp noops.asm)
target_link_libraries(time_front_end PRIVATE profiler)
