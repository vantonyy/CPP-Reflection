#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/Decl.h"

#include <iostream>

namespace reflector {

class visitor : public clang::RecursiveASTVisitor<visitor>
{
public:
        explicit visitor(clang::Rewriter& r)
                : m_rewriter(r)
        {
        }

        virtual bool VisitVarDecl(clang::VarDecl *var) 
        {
                return true;
        }

        virtual bool VisitFunctionDecl(clang::FunctionDecl *func)
        {
                return true;
        }
	
        virtual bool VisitStmt(clang::Stmt *st) 
        {
		return true;
        }

	clang::Rewriter& get_rewriter()
	{
		return m_rewriter;
	}

private:
        clang::Rewriter m_rewriter;
};

class consumer : public clang::ASTConsumer
{
public:
	explicit consumer(clang::Rewriter& r)
		: m_visitor(r)
	{
	}

        virtual void HandleTranslationUnit(clang::ASTContext& context)
        {
                m_visitor.TraverseDecl(context.getTranslationUnitDecl());
        }

	virtual bool HandleTopLevelDecl(clang::DeclGroupRef d)
	{
		for (clang::DeclGroupRef::iterator b = d.begin(), e = d.end(); b != e; ++b) {
			m_visitor.TraverseDecl(*b);
			//(*b)->dump();
		}
		return true;
	}

	clang::Rewriter& get_rewriter()
	{
		return m_visitor.get_rewriter();
	}
private:
        visitor m_visitor;
};

}