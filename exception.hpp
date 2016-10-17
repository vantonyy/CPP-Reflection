#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>

namespace reflector {

class exception
{
public:
	exception(const std::string& desc)
		: m_desc(desc)
	{		
	}

	const std::string& get_msg() const
	{
		return m_desc;
	}
private:
	const std::string& m_desc;
};

}

#endif