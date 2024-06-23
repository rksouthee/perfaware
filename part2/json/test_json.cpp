#include <catch2/catch_test_macros.hpp>

#include "json.h"

using namespace json;

TEST_CASE("test empty json")
{
	const auto result = parse("");
	REQUIRE(std::holds_alternative<std::monostate>(result));
}

TEST_CASE("test empty object")
{
	const auto result = parse("{}");
	REQUIRE(std::holds_alternative<json::Object>(result));
}

TEST_CASE("test empty array")
{
	const auto result = parse("[]");
	REQUIRE(std::holds_alternative<json::Array>(result));
}

TEST_CASE("test object with one key-value pair")
{
	const auto result = parse(R"({"pairs": []})");
	REQUIRE(std::holds_alternative<json::Object>(result));
	const auto& object = std::get<json::Object>(result);
	REQUIRE(object.size() == 1);
	//REQUIRE(object.front().first == "pairs");
	REQUIRE(std::get<json::Array>(object.front().second) == json::Array{});
}
