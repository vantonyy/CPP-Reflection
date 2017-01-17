#ifndef CMD_PARSER_HPP
#define CMD_PARSER_HPP

#include "messenger.hpp"

#include <clang/Basic/FileManager.h>

#include <string>

// @class cmd_parser
class cmd_parser
{
public:
	cmd_parser(int c, char const** v)
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

	static bool exist_file(const char* file_name)
	{
		static clang::FileSystemOptions fso;
		clang::FileManager fm(fso);
		return 0 != fm.getFile(file_name);
	}

private:
	void parse()
	{
		if (2 != m_arg_count) {
			massenger::print("Usage: " + usage());
			return;
		}
		if (!exist_file(m_arg_value[1])) {
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
}; // class cmd_parser

#endif // CMD_PARSER_HPP
