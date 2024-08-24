# Part 3: Moving Data

## Code Alignment
add_executable(profile_jump_alignment jump_alignment_main.cpp jump_alignment.asm )
target_link_libraries(profile_jump_alignment PRIVATE profiler)

## Execution Ports and the Scheduler
add_executable(profile_read_ports read_ports_main.cpp read_ports.asm)
target_link_libraries(profile_read_ports PRIVATE profiler)
### Homework
add_executable(profile_write_ports write_ports_main.cpp write_ports.asm)
target_link_libraries(profile_write_ports PRIVATE profiler)

## Increasing Read Bandwidth with SIMD Instructions
add_executable(profile_read_wide read_wide_main.cpp read_wide.asm)
target_link_libraries(profile_read_wide PRIVATE profiler)

## Cache Size and Bandwidth Testing
add_executable(profile_cache_size cache_size_main.cpp cache_size.asm)
target_link_libraries(profile_cache_size PRIVATE profiler)