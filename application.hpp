#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "debug.hpp"
#include "exception.hpp"
#include "messenger.hpp"
#include "option.hpp"
#include "reflect_output.hpp"
#include "visitor.hpp"

#include <clang/Frontend/CompilerInstance.h>

///@{
//Should be move to application.cpp
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Parse/ParseAST.h>
#include <llvm/Support/raw_ostream.h>
///}@

#include <string>

namespace reflector {

///@{
//Should be move to application.cpp
// @class writer
class writer
{
public:
	explicit writer(const std::string& out_name)
		: m_error_info()
		, m_out_file(llvm::StringRef(out_name), m_error_info, llvm::sys::fs::F_Text)
	{
	}

	~writer()
	{
		m_out_file.close();
	}

	void write_reflected(reflected_class::reflected_collection& reflected)
	{
		reflect_output out(m_out_file);
		out.dump(reflected);
	}

	bool ok() const
	{
		static std::error_code non_error;
		return non_error == m_error_info;
	}

	std::string error_message() const
	{
		return m_error_info.message();
	}

private:
	std::error_code m_error_info;
	llvm::raw_fd_ostream m_out_file;
}; // class writer
///@}

// @class application
class application
{
public:
	application(int c, char const **v);
public:
	int run();

	static const std::string& get_name();

	static const std::string& get_description();
private:	
	void initialize_compiler();

	void set_header_search_options(clang::CompilerInstance& c);

	void set_invocation(clang::CompilerInstance& c);

	void set_default_target_triple(clang::CompilerInstance& c);
	
	void set_main_file_id(clang::CompilerInstance& c);

	void parse_the_AST();

private:
        clang::CompilerInstance m_compiler;
	reflector::option m_option;
}; // class application

///@{
//Should be move to application.cpp
application::application(int c, char const **v)
	: m_option(c, v)
{
	if (m_option.is_valid()) {
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
	set_header_search_options(m_compiler);
	set_invocation(m_compiler);
	set_default_target_triple(m_compiler);
	m_compiler.createFileManager();
	m_compiler.createSourceManager(m_compiler.getFileManager());
	m_compiler.createPreprocessor(clang::TU_Complete);
	m_compiler.getPreprocessorOpts().UsePredefines = false;
	m_compiler.createASTContext();
	set_main_file_id(m_compiler);
}

void application::set_header_search_options(clang::CompilerInstance& c)
{
	clang::HeaderSearchOptions &h = c.getHeaderSearchOpts();
	h.AddPath("/usr/include/c++/4.6", clang::frontend::Angled, false, false);
	h.AddPath("/usr/include/c++/4.6/i686-linux-gnu", clang::frontend::Angled, false, false);
	h.AddPath("/usr/include/c++/4.6/backward", clang::frontend::Angled, false, false);
	h.AddPath("/usr/local/include", clang::frontend::Angled, false, false);
	h.AddPath("/usr/local/lib/clang/3.3/include", clang::frontend::Angled, false, false);
	h.AddPath("/usr/include/i386-linux-gnu", clang::frontend::Angled, false, false);
	h.AddPath("/usr/include", clang::frontend::Angled, false, false);
	h.AddPath("/c/msys64/mingw64/include", clang::frontend::Angled, false, false);
}

void application::set_invocation(clang::CompilerInstance& c)
{
	ASSERT(c.hasDiagnostics());
	clang::CompilerInvocation* invocation = new clang::CompilerInvocation;
	clang::CompilerInvocation::CreateFromArgs(
				*invocation, m_option.arg_begin(), m_option.arg_end(), c.getDiagnostics());
	clang::LangOptions lang_opts;
	lang_opts.GNUMode = 1;
	lang_opts.CXXExceptions = 1;
	lang_opts.RTTI = 1;
	lang_opts.Bool = 1;
	lang_opts.CPlusPlus = 1;
	invocation->setLangDefaults(lang_opts, clang::IK_CXX, clang::LangStandard::lang_cxx0x);
	c.setInvocation(invocation);
}

void application::set_default_target_triple(clang::CompilerInstance& c)
{
	std::shared_ptr<clang::TargetOptions> to = std::make_shared<clang::TargetOptions>();
	to->Triple = llvm::sys::getDefaultTargetTriple();
	ASSERT(c.hasDiagnostics());
	c.setTarget(clang::TargetInfo::CreateTargetInfo(c.getDiagnostics(), to));
}

void application::set_main_file_id(clang::CompilerInstance& c)
{
	ASSERT(m_compiler.hasSourceManager());
	const clang::FileEntry* file = c.getFileManager().getFile(m_option.get_input_file_name());
	ASSERT(0 != file);
	clang::SourceManager& sc_mgr = c.getSourceManager();
	sc_mgr.setMainFileID(sc_mgr.createFileID(file, clang::SourceLocation(), clang::SrcMgr::C_User));
}

void application::parse_the_AST()
{
	writer w(m_option.get_output_file_name());
	if (!w.ok()) {
		throw reflector::exception(w.error_message());
	}
	ASSERT(m_compiler.hasSourceManager());
	clang::SourceManager& sc_mgr = m_compiler.getSourceManager();
	clang::Rewriter rewrite;
	clang::LangOptions& lopts = m_compiler.getLangOpts();
	rewrite.setSourceMgr(sc_mgr, lopts);
	ASSERT(m_compiler.hasPreprocessor());
	clang::Preprocessor& preproc = m_compiler.getPreprocessor();
	m_compiler.getDiagnosticClient().BeginSourceFile(lopts, &preproc);
	ASSERT(m_compiler.hasASTContext());
	reflector::consumer consumer(rewrite);
	ParseAST(preproc, &consumer, m_compiler.getASTContext());
	m_compiler.getDiagnosticClient().EndSourceFile();
	reflected_class::reflected_collection reflected;
	consumer.get_reflected_classes(reflected);
	w.write_reflected(reflected);
}

int application::run()
{
	if (!m_option.is_valid()) {
		return 1;
	}
	parse_the_AST();
	massenger::print("Generated as reflected output: " + m_option.get_output_file_name());
	return 0;
}
///@}

} // namespace reflector

#endif // APPLICATION_HPP