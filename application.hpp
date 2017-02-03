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
#include "option_parser.hpp"
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

private:
        clang::CompilerInstance m_compiler;
	option_parser m_option_parser;
}; // class application

application::application(int c, char const **v)
	: m_option_parser(c, v)
{
	if (m_option_parser.is_valid()) {
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
	clang::CompilerInvocation::CreateFromArgs(
				*invocation, m_option_parser.begin() + 1, m_option_parser.end(), m_compiler.getDiagnostics());
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
	const clang::FileEntry* file = m_compiler.getFileManager().getFile(m_option_parser.get_input_file());
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
		writer(m_option_parser.get_output_file()).write_reflected(visitor.get_reflected_classes());
	}
}

int application::run()
{
	if (!m_option_parser.is_valid()) {
		return 1;
	}
	parse_ast();
	return 0;
}

} // namespace reflector

#endif // APPLICATION_HPP
