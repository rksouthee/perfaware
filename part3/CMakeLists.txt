# Code Alignment
add_executable(profile_jump_alignment
	jump_alignment_main.cpp
	jump_alignment.asm
	)
target_link_libraries(profile_jump_alignment PRIVATE profiler)
