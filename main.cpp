#include "application.hpp"
#include "exception.hpp"
#include "messenger.hpp"

int main(int argc, char const **argv)
{
	try {
                return reflector::application(argc, argv).run();
	} catch(const std::exception& e) {
		massenger::error(e.what()); 
	} catch(const reflector::exception& e) {
		massenger::error(e.get_msg());
	} catch(...) {
		massenger::error("Unhandled exception\n");
	}
	return 1;
}
