#ifndef REFLECTED_CLASS_HPP
#define REFLECTED_CLASS_HPP

#include "debug.hpp"

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Specifiers.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <sstream>
#include <set>

namespace utils {

template <typename DataType>
std::string to_string(const DataType& x)
{
	std::stringstream ss;
	ss << x;
	return ss.str();
}

void replace(std::string& str, const std::string& from, const std::string& to)
{
	std::string::size_type start_pos = 0;
	while ( std::string::npos != (start_pos = str.find(from, start_pos)) ) {
		str.replace(start_pos, from.size(), to);
		start_pos += to.size();
	}
}

} // namespace utils

// @class method_info
class method_info
{
public:
	class signature_comparator
	{
	public:
		bool operator ()(const method_info& i1, const method_info& i2) const
		{
			if (i1.get_params_count() == i2.get_params_count()) {
				const std::string& s1 = i1.get_signture();
				const std::string& s2 = i2.get_signture();
				return s1.size() < s2.size() || s1 < s2;
			}
			return i1.get_params_count() < i2.get_params_count();
		}
	};

	typedef clang::CXXMethodDecl method;
	typedef std::set<std::string> method_names;
public:
	explicit method_info(method* m)
		: m_method(m)
		, m_return_type(extract_return_type())
		, m_param_types(exctrat_param_type_list())
		, m_arguments(extract_argument_list())
		, m_signature(extract_signature())
		
	{
	}

	static const std::string get_def()
	{
		static std::string d = "InvFuncType";
		return d;
	}

	const std::string& get_signture() const
	{
		return m_signature;
	}

	const std::string& get_param_type_list() const
	{
		return m_param_types;
	}

	const std::string& get_argument_list() const
	{
		return m_arguments;
	}

	const std::string& get_return_type() const
	{
		return m_return_type;
	}

	unsigned get_params_count() const
	{
		ASSERT(0 != m_method);
		return m_method->getNumParams();
	}

	bool is_const() const
	{
		ASSERT(0 != m_method);
		return m_method->isConst();
	}

	bool has_param() const
	{
		return 0 < get_params_count();
	}

	bool non_void_return_type() const
	{
		return m_return_type != "void";
	}
private:
	std::string extract_signature() const
	{
		ASSERT(0 != m_method);
		std::string res = get_return_type() + " (Type::*" + get_def() + ")(";
		method::param_const_iterator b = m_method->param_begin();
		method::param_const_iterator e = m_method->param_end();
		while ( b != e ) {
			ASSERT(0 != *b);
			res += (*b++)->getType().getAsString();
			if (b != e) {
				res += ", ";
			}
		}
		res += is_const() ? ") const" : ")";
		utils::replace(res, "_Bool", "bool");
		return res;
	}

	std::string extract_return_type() const
	{
		ASSERT(0 != m_method);
		std::string res = m_method->getReturnType().getAsString();
		utils::replace(res, "_Bool", "bool");
		return res;
	}

	std::string exctrat_param_type_list() const
	{
		ASSERT(0 != m_method);
		std::string res;
		method::param_const_iterator b = m_method->param_begin();
		method::param_const_iterator e = m_method->param_end();
		unsigned idx = 0;
		while (b != e) {
			res += (*b++)->getType().getAsString() + " p" + utils::to_string(++idx);
			if (b != e) {
				res += ", ";
			}
		}
		utils::replace(res, "_Bool", "bool");
		return res;
	}

	std::string extract_argument_list() const
	{
		ASSERT(0 != m_method);
		std::string res;
		const unsigned count = get_params_count() + 1;
		for (unsigned i = 1; i != count; ++i) {
			res += "p" + utils::to_string(i);
			if (i + 1 != count) {
				res += ", ";
			}
		}
		return res;
	}
private:
	method* m_method;
	std::string m_return_type;
	std::string m_param_types;
	std::string m_arguments;
	std::string m_signature;
	
}; // class method_info

//@class invok_output
class invok_output
{
private:
	typedef method_info::method method;
	typedef method_info::method_names method_names;
	typedef std::map<method_info, method_names, method_info::signature_comparator> methods_map;
public:
	invok_output(clang::CXXRecordDecl::method_range r)
	{
		init(r);
	}
public:

	bool has_methods() const
	{
		return !m_methods_map.empty();
	}

	void get_methods(method_names& ns) const
	{
		for (auto i : m_methods_map) {
			ns.insert(i.second.begin(), i.second.end());
		}
	}

	void dump(clang::raw_ostream& out) const
	{
		for (auto i : m_methods_map) {
			 dump(out, i.first, i.second);
		}
	}

private:
	void init(clang::CXXRecordDecl::method_range r)
	{
		for (clang::CXXRecordDecl::method_iterator i = r.begin(); i != r.end(); ++i) {
			method* m = *i;
			ASSERT(0 != m);
			if (supported(m)) {
				m_methods_map[method_info(m)].insert(m->getNameAsString());
			}
		}
	}

	bool supported(method* m) const
	{
		return !m->isStatic() && m->isUserProvided() && clang::Decl::Kind::CXXMethod == m->getKind() &&
	                m->getAccess() == clang::AccessSpecifier::AS_public && !m->isDefaulted() &&
	               !m->isCopyAssignmentOperator() && !m->isMoveAssignmentOperator();
	}

	void dump(clang::raw_ostream& out, const method_info& info, const method_names& names) const
	{
		ASSERT(!names.empty());
		std::string const_qual = info.is_const() ? "const " : "";
		out << "\t" << info.get_return_type() << " invok(" << const_qual << "Type & o, const char * n";
		if (info.has_param()) {
			out << ", " + info.get_param_type_list();
		}
		out << ") " << const_qual << "\n\t{\n";
		out << "\t\ttypedef " << info.get_signture() << ";\n";
		out << "\t\ttypedef std::map<std::string, " << method_info::get_def() << "> funcMap;\n";
		out << "\t\tstatic funcMap f_map;\n";
		out << "\t\tif (f_map.empty()) {\n";
		for (auto i : names) {
			out << "\t\t\tf_map.insert(std::make_pair(\"" << i << "\", &Type::" << i << "));\n";
		}
		out << "\t\t}\n\t\tfuncMap::const_iterator found = f_map.find(n);\n";
		out << "\t\tif(found == f_map.end()) {\n";
		out << "\t\t\tthrow std::runtime_error(\"Incorect func name\");\n\t\t}\n";
		out << "\t\t";
		if (info.non_void_return_type()) {
			out << "return ";
		}
		out << "(o.*found->second)(" << info.get_argument_list() <<  ");\n\t}\n\n";
	}

private:
	methods_map m_methods_map;
}; // class invok_output

//@class reflected_class
class reflected_class
{
private:
	typedef clang::CXXRecordDecl source_class;
public:
	typedef std::shared_ptr<reflected_class> ptr;
	typedef std::list<ptr> reflected_collection;
public:
	reflected_class(source_class* d)
		: m_source_class(d)
		, m_methods(d->methods())
	{
		ASSERT(d->isClass());
		ASSERT(d->hasDefinition()); 
	}

	std::string get_qualified_name() const
	{
		return m_source_class->getQualifiedNameAsString();
	}

	std::string get_name() const
	{
		return m_source_class->getNameAsString();
	}

	int get_num_bases() const
	{
		return m_source_class->getNumBases();
	}

	bool has_any_dependent_bases() const
	{
		return m_source_class->hasAnyDependentBases();
	}

	bool has_friends() const
	{
		return m_source_class->hasFriends();
	}

	bool has_user_declared_constructor() const
	{
		return m_source_class->hasUserDeclaredConstructor();
	}

	bool has_user_declared_copy_assignment() const
	{
		return m_source_class->hasUserDeclaredCopyAssignment();
	}

	bool has_user_declared_destructor() const
	{
		return m_source_class->hasUserDeclaredDestructor();
	}

	bool has_user_provided_default_constructor() const
	{
		return m_source_class->hasUserProvidedDefaultConstructor();
	}

	bool is_aggregate() const
	{
		return m_source_class->isAggregate();
	}

	bool is_derived_from(const std::string& base_name) const
	{
		static method_info::method_names names;
		if (names.empty()) {
			get_base_names(names);
		}
		return names.find("class " + base_name) != names.end();
	}

	bool is_template_decl() const
	{
		return m_source_class->isTemplateDecl();
	}

	bool is_polymorphic() const
	{
		return m_source_class->isPolymorphic();
	}

	int get_num_virtual_bases() const
	{
		return m_source_class->getNumVBases();
	}

	bool is_abstract() const
	{
		return m_source_class->isAbstract();
	}

	bool has_default_constructor() const
	{
		return m_source_class->hasDefaultConstructor();
	}

	void get_base_names(method_info::method_names& names) const
	{
		source_class::base_class_iterator b = m_source_class->bases_begin();
		source_class::base_class_iterator e = m_source_class->bases_end();
		for (; b != e; ++b) {
			names.insert(b->getType().getAsString());
		}
	}
 
	void dump(clang::raw_ostream& out) const
	{
		dump_begin_specalization(out);
		dump_create(out);
		dump_get_name(out);
		dump_get_qualified_name(out);
		dump_get_base_names(out);
		dump_get_num_bases(out);
		dump_get_num_virtual_bases(out);
		dump_get_methods(out);
		dump_has_default_constructor(out);
		dump_has_any_dependent_bases(out);
		dump_has_friends(out);
		dump_has_user_declared_constructor(out);
		dump_has_user_declared_copy_assignment(out);
		dump_has_user_declared_destructor(out);
		dump_has_user_provided_default_constructor(out);
		dump_is_aggregate(out);
		dump_is_derived_from(out);
		dump_is_template_decl(out);
		dump_is_abstract(out);
		dump_is_polymorphic(out);
		dump_invokes(out);
		dump_end_specalization(out);
	}

private:
	void dump_begin_specalization(clang::raw_ostream& out) const 
	{
		std::string name = get_qualified_name();
		out << "// @class reflect<" << name << ">\n";
		out << "template <>\nclass reflect<" << name << ">\n{\n";
		out << "public:\n\ttypedef " << name << " Type;\n";
		out << "\ttypedef std::set<std::string> names;\n\n";
		out << "public:\n";
	}

	void dump_end_specalization(clang::raw_ostream& out) const 
	{
		out << "\n}; // class reflect<" << get_name() << ">\n\n\n";
	}

	void dump_get_name(clang::raw_ostream& out) const 
	{
		out << "\tstd::string get_name() const\n\t{\n";
		out << "\t\treturn \"" << get_name() << "\";\t\n\t}\n\n";
	}

	void dump_get_qualified_name(clang::raw_ostream& out) const
	{
		out << "\tstd::string get_qualified_name() const\n\t{\n";
		out << "\t\treturn \"" << get_qualified_name() << "\";\t\n\t}\n\n";
	}

	void dump_get_num_bases(clang::raw_ostream& out) const 
	{
		out << "\tint get_num_bases() const\n\t{\n";
		out << "\t\treturn " << get_num_bases() << ";\n\t}\n\n";
	}

	void dump_get_num_virtual_bases(clang::raw_ostream& out) const 
	{
		out << "\tint get_num_virtual_bases() const\n\t{\n";
		out << "\t\treturn " << get_num_virtual_bases() << ";\n\t}\n\n";
	}

	void dump_get_methods(clang::raw_ostream& out) const
	{
		out << "\tvoid get_methods(names& ns) const\n\t{\n";
		std::set<std::string> names;
		m_methods.get_methods(names);
		for (auto i : names ) {
			out << "\t\tns.insert(\"" << i << "\");\n";
		}
		out << "\t}\n\n";
	}

	void dump_is_abstract(clang::raw_ostream& out) const
	{
		out << "\tbool is_abstract() const\n\t{\n";
		out << "\t\treturn " << (is_abstract() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_is_polymorphic(clang::raw_ostream& out) const
	{
		out << "\tbool is_polymorphic() const\n\t{\n";
		out << "\t\treturn " << (is_polymorphic() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_has_default_constructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_default_constructor() const\n\t{\n";
		out << "\t\treturn "<< (has_default_constructor() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_create(clang::raw_ostream& out) const 
	{
		out << "\ttemplate<typename ...Args>\n";
		out << "\tType create(const Args&...args)\n\t{\n";
		out << "\t\treturn Type(std::forward<const Args&>(args)...);\n\t}\n\n";
	}

	void dump_get_base_names(clang::raw_ostream& out) const 
	{
		out << "\tvoid get_base_names(names& ns) const\n\t{\n";
		source_class::base_class_iterator b = m_source_class->bases_begin();
		source_class::base_class_iterator e = m_source_class->bases_end();
		if (b == e) {
			out << "\t\t///Note: Has not any base class\n";
		}
		for (; b != e; ++b) {
			out << "\t\tns.insert(\"" << b->getType().getAsString() << "\");\n";
		}
		out << "\t}\n\n";
	}

	void dump_has_any_dependent_bases(clang::raw_ostream& out) const
	{
		out << "\tbool dump_has_any_dependent_bases() const\n\t{\n";
		out << "\t\treturn " << (has_any_dependent_bases() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_has_friends(clang::raw_ostream& out) const
	{
		out << "\tbool has_friends() const\n\t{\n";
		out << "\t\treturn " << (has_friends() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_has_user_declared_constructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_declared_constructor() const\n\t{\n";
		out << "\t\treturn " << (has_user_declared_constructor() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_has_user_declared_copy_assignment(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_declared_copy_assignment() const\n\t{\n";
		out << "\t\treturn " << (has_user_declared_copy_assignment() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_has_user_declared_destructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_declared_destructor() const\n\t{\n";
		out << "\t\treturn " << (has_user_declared_destructor() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_has_user_provided_default_constructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_provided_default_constructor() const\n\t{\n";
		out << "\t\treturn " << (has_user_provided_default_constructor() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_is_aggregate(clang::raw_ostream& out) const
	{
		out << "\tbool is_aggregate() const\n\t{\n";
		out << "\t\treturn " << (is_aggregate() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_is_derived_from(clang::raw_ostream& out) const
	{
		out << "\tbool is_derived_from(const std::string& base_name) const\n\t{\n";
		out << "\t\tnames ns;\n";
		out << "\t\tget_base_names(ns);\n";
		out << "\t\treturn ns.find(\"class \" + base_name) != ns.end();\n\t}\n\n";
	}

	void dump_is_template_decl(clang::raw_ostream& out) const
	{
		out << "\tbool is_template_decl() const\n\t{\n";
		out << "\t\treturn " << (is_template_decl() ? "true" : "false") << ";\n\t}\n\n";
	}

	void dump_invokes(clang::raw_ostream& out) const
	{
		if (!is_abstract()) {
			m_methods.dump(out);
		}
	}
	
private:
	source_class* m_source_class;
	invok_output m_methods;
}; // class reflected_class

#endif // REFLECTED_CLASS_HPP