/*
* Copyright (C) 2016 Vladimir Antonyan <antonyan_v@outlook.com>
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
		reflect_output out(out_file);
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
		[&h](const definition& d)
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
