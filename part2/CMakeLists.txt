add_subdirectory(json)

add_executable(haversine_generator haversine_generator.cpp)
target_link_libraries(haversine_generator PRIVATE cxxopts::cxxopts)

add_executable(haversine haversine.cpp)
target_link_libraries(haversine PRIVATE json)
