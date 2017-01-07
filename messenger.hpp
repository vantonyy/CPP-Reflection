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
		std::cout <<"Worrning: "<< ms << std::endl;
	}

	static void print(const char* ms)
	{
		std::cout << ms << std::endl;
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