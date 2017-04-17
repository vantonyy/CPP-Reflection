/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef UTILS_HPP
#define UTILS_HPP

#include <clang/Basic/FileManager.h>

#include <string>
#include <vector>

namespace utils {

bool exist_file(const std::string& file_name)
{
	static clang::FileSystemOptions fs_opts;
	static clang::FileManager fm(fs_opts);
	return 0 != fm.getFile(file_name.c_str());
}

std::string generate_out_file_name(const std::string& in_file)
{
	static const std::string prefix = "_reflected.hpp";
	return in_file.substr(0, in_file.find('.')) + prefix;
}

void replace(std::string& str, const std::string& from, const std::string& to)
{
	std::string::size_type start_pos = 0;
	while ( std::string::npos != (start_pos = str.find(from, start_pos)) ) {
		str.replace(start_pos, from.size(), to);
		start_pos += to.size();
	}
}

void split(const std::string& str, std::vector<std::string>& strings, std::string any_of)
{
	std::string::size_type b_pos = 0;
	std::string::size_type e_pos = 0;
	std::string::size_type step = any_of.size();
	while (std::string::npos != (e_pos = str.find(any_of, e_pos))) {
		strings.push_back(str.substr(b_pos, e_pos - b_pos));
		e_pos += step;
		b_pos = e_pos;
	}
	strings.push_back(str.substr(b_pos, e_pos));
}

} // namespace utils

#endif // UTILS_HPP
