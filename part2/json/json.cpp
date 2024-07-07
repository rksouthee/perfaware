#include "json.h"
#include "..\profiler.h"

#include <cassert>

#include <algorithm>
#include <charconv>

namespace json
{
	Token_iterator::Token_iterator(std::string_view json) :
		m_data(json)
	{
		if (!m_data.empty())
		{
			m_value.value = json.substr(0, 0);
			++(*this);
		}
		else
		{
			m_value.type = Token::Type::none;
		}
	}

	Token_iterator& Token_iterator::operator++()
	{
		const auto end = m_data.data() + m_data.size();
		const char* begin = m_value.value.data() + m_value.value.size();
		const char* iter = begin;
begin:
		switch (*iter++)
		{
		case ' ': case '\t': case '\n':
			iter = std::find_if_not(iter, end, [](char c) { return c == ' ' || c == '\t' || c == '\n'; });
			begin = iter;
			goto begin;
		case '{':
			m_value.type = Token::Type::object_begin;
			break;
		case '}':
			m_value.type = Token::Type::object_end;
			break;
		case '[':
			m_value.type = Token::Type::array_begin;
			break;
		case ']':
			m_value.type = Token::Type::array_end;
			break;
		case '"':
			// NOTE(rksouthee): This doesn't handle escaped quotes
			iter = std::find(iter, end, '"');
			if (iter == end)
			{
				m_value.type = Token::Type::string_unterminated;
			}
			else
			{
				m_value.type = Token::Type::string;
				++iter;
			}
			break;
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '-':
			iter = std::find_if_not(iter, end, [](char c) { return c >= '0' && c <= '9'; });
			if (iter != end && *iter == '.')
			{
				iter = std::find_if_not(iter + 1, end, [](char c) { return c >= '0' && c <= '9'; });
			}
			if (iter != end && (*iter == 'e' || *iter == 'E'))
			{
				++iter;
				if (iter != end && (*iter == '+' || *iter == '-'))
				{
					++iter;
				}
				iter = std::find_if_not(iter, end, [](char c) { return c >= '0' && c <= '9'; });
			}
			m_value.type = Token::Type::number;
			break;
		case ',':
			m_value.type = Token::Type::comma;
			break;
		case ':':
			m_value.type = Token::Type::colon;
			break;
		}
		m_value.value = std::string_view(begin, iter);
		return *this;
	}

	Token_iterator Token_iterator::operator++(int)
	{
		Token_iterator temp{ *this };
		++(*this);
		return temp;
	}

	Token_iterator::reference Token_iterator::operator*() const
	{
		return m_value;
	}

	Token_iterator::pointer Token_iterator::operator->() const
	{
		return &**this;
	}

	bool operator==(const Token_iterator& x, const Token_iterator& y)
	{
		return x->value.data() == y->value.data();
	}

	bool Json::is_valid() const
	{
		return !std::holds_alternative<std::monostate>(*this);
	}

	bool Json::is_string() const
	{
		return std::holds_alternative<std::string>(*this);
	}

	bool Json::is_number() const
	{
		return std::holds_alternative<double>(*this);
	}

	bool Json::is_boolean() const
	{
		return std::holds_alternative<bool>(*this);
	}

	bool Json::is_null() const
	{
		return std::holds_alternative<std::nullptr_t>(*this);
	}

	bool Json::is_array() const
	{
		return std::holds_alternative<Array>(*this);
	}

	bool Json::is_object() const
	{
		return std::holds_alternative<Object>(*this);
	}

	const std::string& Json::get_string() const
	{
		return std::get<std::string>(*this);
	}

	double Json::get_number() const
	{
		return std::get<double>(*this);
	}

	bool Json::get_boolean() const
	{
		return std::get<bool>(*this);
	}

	const Array& Json::get_array() const
	{
		return std::get<Array>(*this);
	}

	const Object& Json::get_object() const
	{
		return std::get<Object>(*this);
	}

	Json parse(Token_iterator& f, Token_iterator l);

	Object parse_object(Token_iterator& f, Token_iterator l)
	{
		Object object;
		if (f->type != Token::Type::object_begin)
		{
			// TODO(rksouthee): Error handling
			return object;
		}
		++f;
		while (f != l && f->type != Token::Type::object_end)
		{
			if (f->type != Token::Type::string)
			{
				// Error handling
				return object;
			}
			assert(f->value.size() >= 2);
			const std::string_view key = f->value.substr(1, f->value.size() - 2);
			++f;
			if (f->type != Token::Type::colon)
			{
				// Error handling
				return object;
			}
			++f;
			// NOTE(rksouthee): This is assumes the keys are unique!
			object.emplace_back(key, parse(f, l));
			if (f->type == Token::Type::comma)
			{
				++f;
			}
		}
		if (f != l && f->type == Token::Type::object_end)
		{
			++f;
		}
		return object;
	}

	Array parse_array(Token_iterator& f, Token_iterator l)
	{
		Array array;
		if (f->type != Token::Type::array_begin)
		{
			// Error handling
			return array;
		}
		++f;
		while (f != l && f->type != Token::Type::array_end)
		{
			array.push_back(parse(f, l));
			if (f->type == Token::Type::comma)
			{
				++f;
			}
		}
		if (f != l && f->type == Token::Type::array_end)
		{
			++f;
		}
		return array;
	}

	Json parse_string(Token_iterator& f, Token_iterator l)
	{
		if (f->type != Token::Type::string)
		{
			// Error handling
			return Json{};
		}
		std::string value{f->value};
		++f;
		return Json{ std::move(value) };
	}

	Json parse_number(Token_iterator& f, Token_iterator l)
	{
		if (f->type != Token::Type::number)
		{
			// Error handling
			return Json{};
		}
		double value;
		std::from_chars(f->value.data(), f->value.data() + f->value.size(), value);
		++f;
		return Json{value};
	}

	Json parse(Token_iterator& f, Token_iterator l)
	{
		TIME_FUNCTION;
		if (f == l) return Json{};
		switch (f->type)
		{
		case Token::Type::none:
			return Json{};
		case Token::Type::object_begin:
			return Json{parse_object(f, l)};
		case Token::Type::array_begin:
			return Json{parse_array(f, l)};
		case Token::Type::string:
			return parse_string(f, l);
		case Token::Type::number:
			return parse_number(f, l);
		}
		// TODO(rksouthee): Error handling
		return Json{};
	}

	Json parse(const std::string_view& json)
	{
		Token_iterator begin{ json };
		Token_iterator end{json.substr(json.size(), 0)};
		return parse(begin, end);
	}
}
