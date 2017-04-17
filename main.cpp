/*
* Copyright (C) 2016 Vladimir Antonyan <antonyan_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

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
