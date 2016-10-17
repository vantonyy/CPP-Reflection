#include "application.hpp"

#include <exception>
#include <iostream>

int main(int ac, char const **av)
{
	try {
                return reflector::application(ac, av).run();
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl; 
	} catch(const reflector::exception& e) {
		std::cerr << e.get_msg() << std::endl;
	} catch(...) {
		std::cerr << "Error: " << "Unhandled exception." << std::endl;
	}
	return 1;
}
