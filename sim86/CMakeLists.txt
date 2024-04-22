find_package(cxxopts "3.2.1" EXACT REQUIRED)
add_executable(sim86 sim86.cpp)
target_link_libraries(sim86 PRIVATE cxxopts::cxxopts)


set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/data")
function(add_decoder_test listing)
	add_test(NAME ${listing}_disassemble COMMAND sim86 ${TEST_DATA_DIR}/${listing} -o ${CMAKE_BINARY_DIR}/${listing}.asm)
	add_test(NAME ${listing}_assemble COMMAND nasm ${CMAKE_BINARY_DIR}/${listing}.asm -o ${CMAKE_BINARY_DIR}/${listing}_gen)
	add_test(NAME ${listing}_compare COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_DATA_DIR}/${listing} ${CMAKE_BINARY_DIR}/${listing}_gen)
	set_tests_properties(${listing}_assemble PROPERTIES FIXTURES_SETUP ${listing}_disassemble)
	set_tests_properties(${listing}_compare PROPERTIES FIXTURES_SETUP ${listing}_assemble)
endfunction()

add_decoder_test(listing_0037_single_register_mov)
add_decoder_test(listing_0038_many_register_mov)
