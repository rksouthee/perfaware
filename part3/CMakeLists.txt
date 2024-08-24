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
