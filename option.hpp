#ifndef OPTION_HPP
#define OPTION_HPP

#include "messenger.hpp"

#include <clang/Basic/FileManager.h>

#include <string>

namespace reflector {

// @class option
class option
{
public:
	option(int c, char const** v)
		: m_arg_count(c)
		, m_arg_value(v)
		, m_in_name("")
		, m_out_name("")
	{
		parse();
	}

	const std::string& get_input_file_name() const
	{
		return m_in_name;
	}

	const std::string& get_output_file_name() const
	{
		return m_out_name;
	}

	const char* const* arg_begin() const
	{
		return m_arg_value + 1;
	}

	const char* const* arg_end() const
	{
		return m_arg_value + m_arg_count;
	}

	bool is_valid() const
	{
		return !m_in_name.empty() && !m_out_name.empty();
	}

private:
	void parse()
	{
		if (m_arg_count != 2 ) {
			massenger::print("Usage: " + usage());
			return;
		}
		if (!exist_file(m_arg_value[1])) {
			massenger::error("The file with name '" + std::string(m_arg_value[1]) + "' does not exist");
			return;
		}
		m_in_name = m_arg_value[1];
		m_out_name = m_in_name.substr(0, m_in_name.find('.')) + "_reflected.hpp";
		if (exist_file(m_out_name.c_str())) {
			massenger::worrning("The file with name '" + m_out_name + "'" + "rewrited");
		}
	}

	bool exist_file(const char* file_name)
	{
		static clang::FileSystemOptions fso;
		clang::FileManager fm(fso);
		return 0 != fm.getFile(file_name);
	}

	std::string usage() const
	{
		return std::string(m_arg_value[0]) + "  <input file>";
	}

private:
	unsigned m_arg_count;
	char const** m_arg_value;
	std::string m_in_name;
	std::string m_out_name;
}; // class option

} // namespace reflector

#endif // OPTION_HPP
