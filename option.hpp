///Temporary solution should be use clang::optionsParser
#pragma once

#include <vector>
#include <string>
#include <iostream>

namespace reflector {

namespace info {
const std::string name    = "Reflection Generator";
const std::string disc    = "Reflection Generator for C++ programming language ";
const std::string version = "Version: v:0001";
const std::string help    = "Generic Options:\n"
			    "   -help                 - Display available options (-help-hidden for more)\n"
			    "   -version              - Display the version of this program\n"
  			    "Reflect options:\n"
			    "   -i     		      - Input file name\n"
			    "   -o                    - Output file name\n";
}

class option
{
public:
	class data
	{
	public:
		std::string path;
		std::string input_file_name;
		std::string output_file_name;
	};

	option(int c, char const **v)
		: m_is_valid(pars(c, v))
	{
	}

	const std::string& get_input_file_name() const
	{
		return m_data.input_file_name;
	}

	const std::string& get_output_file_name() const
	{
		return m_data.output_file_name;
	}

	bool is_valid() const
	{
		return m_is_valid;
	}

	const std::string& help() const
	{
		return info::help;
	}
private:
	bool pars(int c, char const **v)
	{
		if (c == 2) {
			if (0 == strcmp(v[1], "-version")) {
				throw reflector::exception(info::version);
			}
			else if (0 == strcmp(v[1], "-help")) {
				throw reflector::exception(info::help);
			}
		} else if (c == 5) {
			for (int i = 1; i != c - 1; ++i) {
				if (0 == strcmp(v[i], "-i")) {
					m_data.input_file_name = v[i + 1];
				}
				else if (0 == strcmp(v[i], "-o")) {
					m_data.output_file_name = v[i + 1];
				}
			}
		}
		m_data.path = v[0];
		return !m_data.input_file_name.empty();
	}
private:
	data m_data;
	bool m_is_valid;
};

}
