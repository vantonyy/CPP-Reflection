/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "debug.hpp"
#include "messenger.hpp"
#include "option.hpp"
#include "reflect_output.hpp"
#include "utils.hpp"
#include "visitor.hpp"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Parse/ParseAST.h>
#include <llvm/Support/raw_ostream.h>

#include <string>

namespace reflector {

// @class writer
class writer
{
public:
	explicit writer(const std::string& file_name)
		: m_file_name(file_name)
		, m_do(utils::exist_file(file_name.c_str()) ? "Rewrite" : "Generate")
	{
	}

	void write_reflected(const reflected_class::reflected_collection& reflected)
	{
		std::error_code error_info;
		llvm::raw_fd_ostream out_file(llvm::StringRef(m_file_name), error_info, llvm::sys::fs::F_Text);
		const std::error_code ok;
		if (ok != error_info) {
			out_file.close();
			throw std::runtime_error(error_info.message());
		}
		file_reflector out(out_file);
		out.dump(reflected);
		out_file.close();
		massenger::print(m_do + " reflection to " + m_file_name);
	}
private:
	const std::string& m_file_name;
	std::string m_do;
	
}; // class writer

// @class application
class application
{
public:
	//@brief constructor
	application(int c, char const **v);
public:
	//@brief Runs application.
	//@return Exit code.
	int run();

	//@brief Gets application's name.
	//@return name of application
	static const std::string& get_name();

	//@brief Gets application's description.
	//@return description of application
	static const std::string& get_description();
private:
	void initialize_compiler();

	void set_header_search_options();

	void set_invocation();

	void set_default_target_triple();
	
	void set_main_file_id();

	void parse_ast();

	void get_options(options&) const;

	bool parse_parameters(unsigned, char const **);

	std::string get_hidden_option_value(const std::string&) const;

	std::string usage(const char*) const;

	std::string version() const;

	std::string help() const;
private:
        clang::CompilerInstance m_compiler;
	std::string m_input_file_name;
	std::string m_output_file_name;
}; // class application

application::application(int c, char const **v)
{
	if ( parse_parameters(c, v) ) {
		initialize_compiler();
	}
}

const std::string& application::get_name()
{
	static const std::string name = "greflect";
	return name;
}

const std::string& application::get_description()
{
	static const std::string desc = "Generate reflection for C++ programming language";
	return desc;
}

std::string application::usage(const char* path) const
{
	return std::string(path ) + " -i <input file>";
}

std::string application::version() const
{
	return "Version: a2017.06";
}

std::string application::help() const
{
	std::string h = get_name() + ":  " + get_description() + "\n";
	h += "Options:\n";
	options o;
	get_options(o);
	o.for_each_option(
		[&h](definition d)
		{
			h += "  " + d.get_name() + ": " + d.get_description() +
			(d.is_optional() ? ": optional" : d.is_hidden() ? "" : ": requared") + "\n"; 
		}
	);
	return h;
}

void application::initialize_compiler()
{
	m_compiler.createDiagnostics();
	set_header_search_options();
	set_invocation();
	set_default_target_triple();
	m_compiler.createFileManager();
	m_compiler.createSourceManager(m_compiler.getFileManager());
	m_compiler.createPreprocessor(clang::TU_Complete);
	m_compiler.getPreprocessorOpts().UsePredefines = false;
	m_compiler.createASTContext();
	set_main_file_id();
}

void application::set_header_search_options()
{
	clang::HeaderSearchOptions &h = m_compiler.getHeaderSearchOpts();
	h.AddPath("/usr/local/include", clang::frontend::Angled, false, false);
	h.AddPath("/usr/include", clang::frontend::Angled, false, false);
	//TODO: should to add an option to allowing user set the path.
}

void application::set_invocation()
{
	ASSERT(m_compiler.hasDiagnostics());
	clang::CompilerInvocation* invocation = new clang::CompilerInvocation;
	ASSERT(!m_input_file_name.empty());
	const char* dummy_arg[1];
	dummy_arg[0] = m_input_file_name.c_str();
	clang::CompilerInvocation::CreateFromArgs(*invocation,
						   dummy_arg,
						   dummy_arg + 1,
						   m_compiler.getDiagnostics());
	clang::LangOptions lang_opts;
	lang_opts.GNUMode = 1;
	lang_opts.CXXExceptions = 1;
	lang_opts.RTTI = 1;
	lang_opts.Bool = 1;
	lang_opts.CPlusPlus = 1;
	invocation->setLangDefaults(lang_opts, clang::IK_CXX, clang::LangStandard::lang_cxx0x);
	m_compiler.setInvocation(invocation);
}

void application::set_default_target_triple()
{
	std::shared_ptr<clang::TargetOptions> to = std::make_shared<clang::TargetOptions>();
	to->Triple = llvm::sys::getDefaultTargetTriple();
	ASSERT(m_compiler.hasDiagnostics());
	m_compiler.setTarget(clang::TargetInfo::CreateTargetInfo(m_compiler.getDiagnostics(), to));
}

void application::set_main_file_id()
{
	ASSERT(m_compiler.hasSourceManager());
	const clang::FileEntry* file = m_compiler.getFileManager().getFile(m_input_file_name);
	ASSERT(0 != file);
	clang::SourceManager& sc_mgr = m_compiler.getSourceManager();
	sc_mgr.setMainFileID(sc_mgr.createFileID(file, clang::SourceLocation(), clang::SrcMgr::C_User));	
}

void application::parse_ast()
{
	ASSERT(m_compiler.hasSourceManager());
	clang::SourceManager& sc_mgr = m_compiler.getSourceManager();
	ASSERT(m_compiler.hasPreprocessor());
	clang::Preprocessor& preproc = m_compiler.getPreprocessor();
	preproc.getDiagnostics().setSuppressAllDiagnostics(true);
	m_compiler.getDiagnosticClient().BeginSourceFile(m_compiler.getLangOpts(), &preproc);
	ASSERT(m_compiler.hasASTContext());
	reflector::visitor visitor(sc_mgr);
	reflector::consumer consumer(visitor);
	ParseAST(preproc, &consumer, m_compiler.getASTContext(), false, clang::TU_Complete, 0, true);
	m_compiler.getDiagnosticClient().EndSourceFile();
	if (visitor.has_reflected_class()) {
		writer(m_output_file_name).write_reflected(visitor.get_reflected_classes());
	}
}

int application::run()
{
	if (m_input_file_name.empty()) {
		return 1;
	}
	parse_ast();
	return 0;
}

void application::get_options(options& o) const
{
	definition d1("-i", "input file", requared);
	o.add_option(d1);
	definition d2("-o", "output file", optional);
	o.add_option(d2);
	definition d3("-h", "help", hidden);
	o.add_option(d3);
	definition d4("-v", "version", hidden);
	o.add_option(d4);
}

bool application::parse_parameters(unsigned c, char const **v)
{
	ASSERT(0 != v);
	if (1 == c) {
		massenger::print("Usage: " + usage(v[0]));
		return false;
	}
	options o;
	get_options(o);
	for (unsigned i = 1; i < c; i += 2) {
		definition d;
		if (!o.get_option(v[i], d)) {
			massenger::print("Unknown option: " + std::string(v[i]));
			return false;
		}
		if (d.is_hidden()) {
			massenger::print(get_hidden_option_value(d.get_name()));
			return false;
		}
		if (i + 1 == c) {
			massenger::print("Incorect argument for option: " + std::string(v[i]));
			return false;
		}
		o.set_value(v[i], v[i + 1]);
	}
	m_input_file_name = o.get_value("-i");
	if (m_input_file_name.empty()) {
		massenger::error("The input file must to provide");
		return false;
	}
	if (!utils::exist_file(m_input_file_name)) {
		massenger::error("The input file with name '" + m_input_file_name + "' does not exist");
		return false;
	}
	m_output_file_name = o.get_value("-o");
	if (m_output_file_name.empty()) {
		m_output_file_name = utils::generate_out_file_name(m_input_file_name);
	}
	return true;
}

std::string application::get_hidden_option_value(const std::string& n) const
{
	if (n == "-h") {
		return help();
	}
	ASSERT(n == "-v");
	return version();
}

} // namespace reflector

#endif // APPLICATION_HPP


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

/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef UTILS_HPP
#define UTILS_HPP

#include <clang/Basic/FileManager.h>

#include <string>
#include <vector>

namespace utils {

bool exist_file(const std::string& file_name)
{
	static clang::FileSystemOptions fs_opts;
	static clang::FileManager fm(fs_opts);
	return 0 != fm.getFile(file_name.c_str());
}

std::string generate_out_file_name(const std::string& in_file)
{
	static const std::string prefix = "_reflected.hpp";
	return in_file.substr(0, in_file.find('.')) + prefix;
}

void replace(std::string& str, const std::string& from, const std::string& to)
{
	std::string::size_type start_pos = 0;
	while ( std::string::npos != (start_pos = str.find(from, start_pos)) ) {
		str.replace(start_pos, from.size(), to);
		start_pos += to.size();
	}
}

void split(const std::string& str, std::vector<std::string>& strings, std::string any_of)
{
	std::string::size_type b_pos = 0;
	std::string::size_type e_pos = 0;
	std::string::size_type step = any_of.size();
	while (std::string::npos != (e_pos = str.find(any_of, e_pos))) {
		strings.push_back(str.substr(b_pos, e_pos - b_pos));
		e_pos += step;
		b_pos = e_pos;
	}
	strings.push_back(str.substr(b_pos, e_pos));
}

} // namespace utils

#endif // UTILS_HPP

/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <cassert>

#if ENABLE_ASSERTION == 1
#define ASSERT( x ) assert(x)
#else
#define ASSERT( x ) (void)sizeof(x);
#endif // DEBUG_BUILD

#endif // DEBUG_HPP


/*
* Copyright (C) 2016 Vladimir Antonyan <antony_v@outlook.com>
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*/

#ifndef VISITOR_HPP
#define VISITOR_HPP

#include "debug.hpp"
#include "messenger.hpp"
#include "reflect_class.hpp"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Tooling.h>

#include <map>

namespace reflector {

// @class visitor
class visitor : public clang::RecursiveASTVisitor<visitor>
{
public:
        explicit visitor(const clang::SourceManager& sm)
		: m_source_mgr(sm)
        {
        }

	virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl* d)
	{
		ASSERT(0 != d);
		if (supported(d)) {
			m_collection.push_back(reflected_class::ptr(new reflected_class(d)));
		}
		return true;
	}
	
	bool has_reflected_class() const
	{
		return !m_collection.empty();
	}

	const reflected_class::reflected_collection& get_reflected_classes() const
	{
		return m_collection;
	}

private:
	bool supported(clang::CXXRecordDecl* d) const
	{
		ASSERT(0 != d);
		if (!d->isClass() || !m_source_mgr.isInMainFile(d->getLocStart())) {
			return false;
		}
		if (!d->hasDefinition()) {
			massenger::print("Skip reflection of class '" 
				+ d->getNameAsString() + "', becouse has not definition in given file.");
			return false;
		}
		if (0 != d->getDescribedClassTemplate()) {
			massenger::print("Skip reflection of class '" + d->getNameAsString()
								+ "', becouse it described template.");
			return false;
		}
		return true;
	}

private:
	const clang::SourceManager& m_source_mgr;
	reflected_class::reflected_collection m_collection;
}; // class visitor

// @class consumer
class consumer : public clang::ASTConsumer
{
public:
	explicit consumer(visitor& v)
		: m_visitor(v)
	{
	}

        virtual void HandleTranslationUnit(clang::ASTContext& c)
        {
                m_visitor.TraverseDecl(c.getTranslationUnitDecl());
        }

private:
        visitor& m_visitor;
}; // class consumer

} // namespace reflector

#endif // VISITOR_HPP
