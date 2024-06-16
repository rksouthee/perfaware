#pragma once

#include <cstddef>

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace json
{
	struct Token
	{
		enum class Type
		{
			none,
			object_begin,
			object_end,
			array_begin,
			array_end,
			string,
			string_unterminated,
			number,
			boolean,
			null,
			comma,
			colon,
		};

		Type type;
		std::string_view value;
	};

	class Token_iterator
	{
	public:
		using value_type = Token;
		using reference = const value_type&;
		using pointer = const value_type*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;

	private:
		Token m_value;
		std::string_view m_data;

	public:
		Token_iterator() = default;
		Token_iterator(std::string_view json);

		Token_iterator& operator++();
		Token_iterator operator++(int);

		reference operator*() const;
		pointer operator->() const;
	};

	bool operator==(const Token_iterator& x, const Token_iterator& y);

	struct Json;
	using Array = std::vector<Json>;
	// NOTE(rksouthee): std::unordered_map is not noexcept move constructible, so we use std::vector instead.
	using Object = std::vector<std::pair<std::string_view, Json>>;

	struct Json : std::variant<std::monostate, std::string, double, bool, std::nullptr_t, Array, Object>
	{
		using variant::variant;

		bool is_valid() const;
		bool is_string() const;
		bool is_number() const;
		bool is_boolean() const;
		bool is_null() const;
		bool is_array() const;
		bool is_object() const;

		const std::string& get_string() const;
		double get_number() const;
		bool get_boolean() const;
		const Array& get_array() const;
		const Object& get_object() const;
	};

	Json parse(const std::string_view& json);
}
