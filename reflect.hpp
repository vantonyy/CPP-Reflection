#ifndef REFLECT_HPP
#define REFLECT_HPP

#include <string>
#include <utility>

#include <tuple>

namespace reflector {

//@class reflect
template<typename T>
class reflect
{
public:
	//typedefs
	typedef T Type;
public:
	reflect()
	{
	}
	
	~reflect()
	{
	}
public:
	template<typename ...Args>
	Type create(const Args&...args)
	{
		return Type(args...);
	}

	template<typename ...Args>
	void invoke(Type& ob, const char*, const Args&...)
	{
	}

	const std::string& get_name()
	{
		return "";
	}

private:
}; // class reflect

} // namespace reflector

#endif // REFLECT_HPP