add_library(printer printer.h printer.cpp simulator.h simulator.cpp)

add_executable(sim86 main.cpp)
target_link_libraries(sim86 PRIVATE cxxopts::cxxopts printer)

if(BUILD_TESTING)
	set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/data")
	find_program(NASM nasm REQUIRED)
	function(add_decoder_test listing)
		add_test(NAME ${listing}_disassemble COMMAND sim86 ${TEST_DATA_DIR}/${listing} -o ${CMAKE_BINARY_DIR}/${listing}.asm)
		add_test(NAME ${listing}_assemble COMMAND ${NASM} ${CMAKE_BINARY_DIR}/${listing}.asm -o ${CMAKE_BINARY_DIR}/${listing}_gen)
		add_test(NAME ${listing}_compare COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_DATA_DIR}/${listing} ${CMAKE_BINARY_DIR}/${listing}_gen)
		set_tests_properties(${listing}_assemble PROPERTIES FIXTURES_SETUP ${listing}_disassemble)
		set_tests_properties(${listing}_compare PROPERTIES FIXTURES_SETUP ${listing}_assemble)
	endfunction()

	add_decoder_test(listing_0037_single_register_mov)
	add_decoder_test(listing_0038_many_register_mov)
	add_decoder_test(listing_0039_more_movs)
	add_decoder_test(listing_0040_challenge_movs)
	add_decoder_test(listing_0041_add_sub_cmp_jnz)
	# TODO: add_decoder_test(listing_0042_completionist_decode)

	add_executable(test_sim86 test_printer.cpp)
	target_link_libraries(test_sim86 PRIVATE Catch2::Catch2WithMain printer)

	catch_discover_tests(test_sim86)
endif()
