#include "application.hpp"
#include "messenger.hpp"

#include <exception>

int main(int argc, char const **argv)
{
	try {
                return reflector::application(argc, argv).run();
	} catch(const std::exception& e) {
		massenger::error(e.what()); 
	} catch(...) {
		massenger::error("Unhandled exception");
	}
	return 1;
}
