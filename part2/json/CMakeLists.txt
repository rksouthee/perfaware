add_library(json json.h json.cpp)

if(BUILD_TESTING)
	add_executable(test_json test_json.cpp)
	target_link_libraries(test_json PRIVATE json Catch2::Catch2WithMain)
	catch_discover_tests(test_json)
endif()
