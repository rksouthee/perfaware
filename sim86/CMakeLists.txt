find_package(cxxopts "3.2.1" EXACT REQUIRED)
add_executable(sim86 sim86.cpp)
target_link_libraries(sim86 PRIVATE cxxopts::cxxopts)
