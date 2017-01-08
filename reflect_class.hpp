#ifndef REFLECTED_CLASS_HPP
#define REFLECTED_CLASS_HPP

#include "debug.hpp"

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/Specifiers.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <set>

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
	{
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

	bool has_method() const
	{
		source_class::method_iterator i = m_source_class->method_begin();
		for (; i != m_source_class->method_end(); ++i) {
			if (need_skip(i)) {
				continue;
			}
			return true;
		}
		return false;
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
		std::set<std::string> names;
		get_base_names(names);
		return names.find("class " + base_name) != names.end();
	}

	bool is_template_decl() const
	{
		return m_source_class->isTemplateDecl();
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

	void get_base_names(std::set<std::string>& names) const 
	{
		source_class::base_class_iterator b = m_source_class->bases_begin();
		source_class::base_class_iterator e = m_source_class->bases_end();
		for (; b != e; ++b) {
			names.insert(b->getType().getAsString());
		}
	}
 
	void get_methods_name(std::set<std::string> name, std::string as)
	{
		clang::AccessSpecifier access = get_accees_as_enum(as);
		source_class::method_iterator i = m_source_class->method_begin();
		for (; i != m_source_class->method_end(); ++i) {
			if (access != i->getAccess() || need_skip(i)) {
				continue;
			}
			name.insert(i->getNameAsString());
		}
	}

	void dump(clang::raw_ostream& out) const
	{
		dump_begin_specalization(out);
		dump_create(out);
		dump_get_name(out);
		dump_get_base_names(out);
		dump_get_num_bases(out);
		dump_get_num_virtual_bases(out);
		dump_get_methods_name(out);
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
		dump_invoke(out);
		dump_end_specalization(out);
	}

private:
	void dump_begin_specalization(clang::raw_ostream& out) const 
	{
		std::string name = get_name();
		out << "class " << name << "; // @forward declatation\n\n";
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
		out << "\tconst std::string& get_name() const\n\t{\n";
		out << "\t\treturn \"" << get_name() << "\";\t\n\t}\n\n";
	}

	void dump_invoke(clang::raw_ostream& out) const
	{
		out << "\ttemplate<typename ...Args>\n";
		out << "\tvoid invok(Type& object, const char* name, const Args& ...args)\n";
		out << "\t{\n";
		if (!has_method()) {
			out << "\t}\n\n";
			return;
		}
		out << "\t\tstatic auto t = " << get_tuple() << "\n";
		out << "\t\ttypedef std::map<std::string, int> IdxMap;\n";
		out << "\t\tstatic IdxMap idx_map;\n";
		out << "\t\tif (idx_map.empty()) {\n";
		out << "\t\t\tint idx = -1;\n";
		source_class::method_iterator i = m_source_class->method_begin();
		for (; i != m_source_class->method_end(); ++i) {
			if (need_skip(i)) {
				continue;
			}
			out << "\t\t\tidx_map.insert(std::make_pair(" 
				<< "\"" + i->getNameAsString() + "\"" << ", ++idx));\n";
		}
		out << "\t\t}\n";
		out << "\t\tIdxMap::const_iterator found = idx_map.find(name);\n";
		out << "\t\tif (found != idx_map.end()) {\n";
		out << "\t\t\tauto func = detail::lookup_function(t, found->second);\n";
		out << "\t\t\t(object.*func)(args...);\n";
		out << "\t\t}\n";
		out << "\t}\n";
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

	void dump_get_methods_name(clang::raw_ostream& out) const
	{
		out << "\tvoid get_methods_name(const std::string& access, names& ns)\n\t{\n";
		out << "\t\ttypedef std::map<std::string, names> AccessMethodMap;\n";
		out << "\t\tstatic AccessMethodMap access_method_map;\n";
		out << "\t\tif ( access_method_map.empty() ) {\n";
		source_class::method_iterator i = m_source_class->method_begin();
		for (; i != m_source_class->method_end(); ++i) {
			if (need_skip(i)) {
				continue;
			}
			out << "\t\t\taccess_method_map[\"" << get_accees_as_string(i->getAccess())
						<< "\"].insert(\"" << i->getNameAsString() << "\");\n";
		}
		out << "\t\t}\n\t\tAccessMethodMap::const_iterator i = access_method_map.find(access);\n";
		out << "\t\tif ( i != taccess_method_map.end() ) {\n";
		out << "\t\t\tns = i->second;\n\t\t}\n\t}\n\n";
	}

	void dump_is_abstract(clang::raw_ostream& out) const
	{
		out << "\tbool is_abstract() const\n\t{\n";
		out << "\t\treturn " << is_abstract() << ";\n\t}\n\n";
	}

	void dump_has_default_constructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_default_constructor() const\n\t{\n";
		out << "\t\treturn "<< has_default_constructor() << ";\n\t}\n\n";
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
		out << "\t\treturn " << has_any_dependent_bases() << ";\n\t}\n\n";
	}

	void dump_has_friends(clang::raw_ostream& out) const
	{
		out << "\tbool has_friends() const\n\t{\n";
		out << "\t\treturn " << has_friends() << ";\n\t}\n\n";
	}

	void dump_has_user_declared_constructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_declared_constructor() const\n\t{\n";
		out << "\t\treturn " << has_user_declared_constructor() << ";\n\t}\n\n";
	}

	void dump_has_user_declared_copy_assignment(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_declared_copy_assignment() const\n\t{\n";
		out << "\t\treturn " << has_user_declared_copy_assignment() << ";\n\t}\n\n";
	}

	void dump_has_user_declared_destructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_declared_destructor() const\n\t{\n";
		out << "\t\treturn " << has_user_declared_destructor() << ";\n\t}\n\n";
	}

	void dump_has_user_provided_default_constructor(clang::raw_ostream& out) const
	{
		out << "\tbool has_user_provided_default_constructor() const\n\t{\n";
		out << "\t\treturn " << has_user_provided_default_constructor() << ";\n\t}\n\n";
	}

	void dump_is_aggregate(clang::raw_ostream& out) const
	{
		out << "\tbool is_aggregate() const\n\t{\n";
		out << "\t\treturn " << is_aggregate() << ";\n\t}\n\n";
	}

	void dump_is_derived_from(clang::raw_ostream& out) const
	{
		out << "\tbool is_derived_from(const std::strin& base_name) const\n\t{\n";
		out << "\t\tnames ns;\n";
		out << "\t\tget_base_names(ns);\n";
		out << "\t\treturn ns.find(\"class \" + base_name) != ns.end();\n\t}\n\n";
	}

	void dump_is_template_decl(clang::raw_ostream& out) const
	{
		out << "\tbool is_template_decl() const\n\t{\n";
		out << "\t\treturn " << is_template_decl() << ";\n\t}\n\n";
	}

private:
	bool need_skip(source_class::method_iterator i) const
	{
		return i->isCopyAssignmentOperator() || i->isDefaulted() ||
		       i->getKind() == i->CXXDestructor || i->getKind() == i->CXXConstructor;
	}

	std::string get_tuple() const 
	{
		std::string res = "std::make_tuple(";
		std::string class_name = get_name();
		static const std::string comma = ",\n\t\t\t\t\t";
		source_class::method_iterator i = m_source_class->method_begin();
		for (; i != m_source_class->method_end(); ++i) {
			if (need_skip(i)) {
				continue;
			}
			res += "&" + class_name + "::" + i->getNameAsString() + comma;
		}
		return res.substr(0, res.size() - comma.size()) + ");";
	}

	clang::AccessSpecifier get_accees_as_enum(std::string as) const
	{
		typedef std::map<std::string, clang::AccessSpecifier> AccessMap;
		static AccessMap access_map;
		if (access_map.empty()) {
			access_map.insert(std::make_pair("private", clang::AccessSpecifier::AS_private));
			access_map.insert(std::make_pair("protected", clang::AccessSpecifier::AS_protected));
			access_map.insert(std::make_pair("public", clang::AccessSpecifier::AS_public));

		}
		AccessMap::const_iterator i = access_map.find(as);
		return access_map.end() != i ? i->second : clang::AccessSpecifier::AS_none;
	}

	std::string get_accees_as_string(clang::AccessSpecifier as) const
	{
		typedef std::map<clang::AccessSpecifier, std::string> AccessMap;
		static AccessMap access_map;
		if (access_map.empty()) {
			access_map.insert(std::make_pair(clang::AccessSpecifier::AS_private, "private"));
			access_map.insert(std::make_pair(clang::AccessSpecifier::AS_protected, "protected"));
			access_map.insert(std::make_pair(clang::AccessSpecifier::AS_public, "public"));

		}
		AccessMap::const_iterator i = access_map.find(as);
		return access_map.end() != i ? i->second : "none";
	}

private:
	source_class* m_source_class;
}; // class reflected_class

#endif // REFLECTED_CLASS_HPP
