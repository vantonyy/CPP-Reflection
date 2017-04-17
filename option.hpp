/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef OPTION_HPP
#define OPTION_HPP

#include "messenger.hpp"
#include "utils.hpp"

#include <set>
#include <string>

enum flag {
	requared = 0,
	optional = 1,
	hidden = 2
};

class definition
{
public:
	definition()
	{
	}

	definition(const std::string& n,
		   const std::string& d,
		   flag f, const std::string v = "")
		: m_name(n)
		, m_description(d)
		, m_flag(f)
		, m_value(v)
	{
	}

	bool is_optional() const
	{
		return m_flag == optional;
	}

	bool is_hidden() const
	{
		return m_flag == hidden;
	}

	bool is_requared() const
	{
		return m_flag == requared;
	}
	
	const std::string& get_name() const
	{
		return m_name;
	}

	const std::string& get_description() const
	{
		return m_description;
	}

	const std::string& get_value() const
	{
		return m_value;
	}

	void set_value(const std::string& v)
	{
		ASSERT(!is_hidden());
		m_value = v;
	}
public:
	std::string m_name;
	std::string m_description;
	flag m_flag;
	std::string m_value;
};

class options
{
private:
	typedef std::map<std::string, definition> name_to_definition;
public:
	void add_option(const definition& d)
	{
		m_name_to_definition.insert(std::make_pair(d.get_name(), d));
	}

	bool get_option(const std::string& name, definition& d) const
	{
		name_to_definition::const_iterator i = m_name_to_definition.find(name);
		if (i == m_name_to_definition.end()) {
			return false;
		}
		d = i->second;
		return true;
	}
	
	bool set_value(const std::string& name, const std::string& value)
	{
		name_to_definition::iterator i = m_name_to_definition.find(name);
		if (i == m_name_to_definition.end()) {
			return false;
		}
		i->second.set_value(value);
		return true;
	}

	const std::string& get_value(const std::string& name) const
	{
		name_to_definition::const_iterator i = m_name_to_definition.find(name);
		if (i == m_name_to_definition.end()) {
			static std::string empty;
			return empty;
		}
		return i->second.get_value();
	}
	
	template <typename Functor>
	void for_each_option(Functor f) const
	{
		for (auto i : m_name_to_definition) {
			f(i.second);
		}
	}

private:
	name_to_definition m_name_to_definition;
};

#endif // OPTION_HPP
