/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef OPTION_PARSER_HPP
#define OPTION_PARSER_HPP

#include "messenger.hpp"

#include <string>

// @class option_parser
class option_parser
{
public:
	option_parser(int c, char const** v)
		: m_arg_count(c)
		, m_arg_value(v)
		, m_input_file("")
		, m_output_file("")
	{
		parse();
	}

	const std::string& get_input_file() const
	{
		return m_input_file;
	}

	const std::string& get_output_file() const
	{
		return m_output_file;
	}

	const char* const* begin() const
	{
		return m_arg_value;
	}

	const char* const* end() const
	{
		return m_arg_value + m_arg_count;
	}

	bool is_valid() const
	{
		return !m_input_file.empty();
	}
private:
	void parse()
	{
		if (2 != m_arg_count) {
			massenger::print("Usage: " + usage());
			return;
		}
		if (!utils::exist_file(m_arg_value[1])) {
			massenger::error("The file with name '" + std::string(m_arg_value[1]) + "' does not exist");
			return;
		}
		m_input_file = m_arg_value[1];
		m_output_file = m_input_file.substr(0, m_input_file.find('.')) + "_reflected.hpp";
	}

	std::string usage() const
	{
		return std::string(m_arg_value[0]) + "  <input file>";
	}

private:
	unsigned m_arg_count;
	char const** m_arg_value;
	std::string m_input_file;
	std::string m_output_file;
}; // class option_parser

#endif // OPTION_PARSER_HPP
