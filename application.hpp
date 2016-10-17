#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "reflect.hpp"
#include "exception.hpp"
#include "visitor.hpp"
#include "option.hpp"

#include <clang/Tooling/CommonOptionsParser.h>
#include "clang/Frontend/CompilerInstance.h"

///@{
//Should be move to application.cpp
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/ADT/Twine.h>

#include <cstdio>
#include <sstream>
///}@

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <cassert> 

#define ASSERT assert

namespace reflector {

// @class application
class application
{
public:
        application(int c, char const **v)
		: m_option(c, v)
        {
		if (m_option.is_valid()) {
			init_compiler_instance();
		}
        }
public:
       	int run();
private:
	void init_compiler_instance();
private:
        clang::CompilerInstance m_compiler;
	reflector::option m_option;
}; // class application

///@{
//Should be move to application.cpp
void application::init_compiler_instance()
{
	m_compiler.createDiagnostics();
	typedef std::shared_ptr<clang::TargetOptions> target_opts_ptr;
	target_opts_ptr target_option(new clang::TargetOptions);
	target_option->Triple = llvm::sys::getDefaultTargetTriple();
	clang::TargetInfo *target_info = clang::TargetInfo::CreateTargetInfo(
		m_compiler.getDiagnostics(),
		target_opts_ptr(target_option));
	m_compiler.setTarget(target_info);
	m_compiler.createFileManager();
	clang::FileManager& file_mgr = m_compiler.getFileManager();
	m_compiler.createSourceManager(file_mgr);
	m_compiler.createPreprocessor(clang::TU_Complete);
	m_compiler.createASTContext();
	clang::SourceManager& source_mgr = m_compiler.getSourceManager();
	clang::Rewriter rewriter;
	rewriter.setSourceMgr(source_mgr, m_compiler.getLangOpts());
	m_compiler.setASTConsumer(std::unique_ptr<clang::ASTConsumer>(new reflector::consumer(rewriter)));
	const clang::FileEntry* input_file = m_compiler.getFileManager().getFile(m_option.get_input_file_name());
	if (0 == input_file) {
		throw reflector::exception("Error: Input file with name '" + m_option.get_input_file_name() + "' not found");
	}
	m_compiler.getSourceManager().setMainFileID(m_compiler.getSourceManager()
		.createFileID(input_file, clang::SourceLocation(), clang::SrcMgr::C_User));
}

int application::run()
{
	if (!m_option.is_valid()) {
		throw reflector::exception(m_option.help());
	}
	ASSERT(m_compiler.hasASTConsumer());
	ASSERT(m_compiler.hasASTContext());
	ASSERT(m_compiler.hasFileManager());
	ASSERT(m_compiler.hasPreprocessor());
	ASSERT(m_compiler.hasTarget());
	reflector::consumer& cons = static_cast<reflector::consumer&>(m_compiler.getASTConsumer());
	m_compiler.getDiagnosticClient().BeginSourceFile(m_compiler.getLangOpts(), 
							&m_compiler.getPreprocessor());
	clang::ParseAST(m_compiler.getPreprocessor(), &cons, m_compiler.getASTContext(), clang::TU_Complete);
	
	m_compiler.getDiagnosticClient().EndSourceFile();
	const clang::RewriteBuffer* buffer = cons.get_rewriter().getRewriteBufferFor(m_compiler
							                     .getSourceManager()
						                             .getMainFileID());
	if (0 == buffer) {
		//llvm::errs() << "Error:...";
		return 1;
	}
	llvm::outs() << std::string(buffer->begin(), buffer->end());
	return 0;
}
///@}

} // namespace reflector

#endif // APPLICATION_HPP
