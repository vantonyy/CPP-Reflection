/*
* Copyright (C) 2016 Vladimir Antonyan <antonyan_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef MESSANGER_HPP
#define MESSANGER_HPP

#include <iostream>

// @class massenger
class massenger
{
public:
	static void error(const char* ms)
	{
		std::cerr << "Error: " << ms << std::endl;
	}

	static void worrning(const char* ms)
	{
		std::cout << "Warning: " << ms << std::endl;
	}

	static void print(const char* ms)
	{
		std::cout << "Information: " << ms << std::endl;
	}

	static void error(const std::string& ms)
	{
		error(ms.c_str());
	}

	static void worrning(const std::string& ms)
	{
		worrning(ms.c_str());
	}

	static void print(const std::string& ms)
	{
		print(ms.c_str());
	}

}; // class massenger

#endif // MESSANGER_HPP
