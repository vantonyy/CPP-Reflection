#ifndef VISITOR_HPP
#define VISITOR_HPP

#include "debug.hpp"
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
        explicit visitor(clang::Rewriter& r)
                : m_rewriter(r)
        {
        }

	virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl* cl)
	{
		if (!cl->isClass()) {
			return true;
		}
		reflected_class::ptr p = reflected_class::ptr(new reflected_class(cl));
		m_classes.insert(std::make_pair(p->get_name(), p));
		return true;
	}
	
    	clang::Rewriter& get_rewriter()
	{
		return m_rewriter;
	}

	void get_reflected_classes(reflected_class::reflected_collection& out)
	{
		for (auto i : m_classes) {
			out.push_back(i.second);
		}
	}

	/*
	virtual bool VisitCXXRecordDecl(clang::CXXMethodDecl* cl);
	virtual bool VisitVarDecl(clang::VarDecl* var);
	virtual bool VisitFunctionDecl(clang::FunctionDecl* func);
	*/
private:
        clang::Rewriter m_rewriter;
	std::map<std::string, reflected_class::ptr> m_classes;
}; // class visitor

// @class consumer
class consumer : public clang::ASTConsumer
{
public:
	explicit consumer(clang::Rewriter& r)
		: m_visitor(r)
	{
	}

        virtual void HandleTranslationUnit(clang::ASTContext& c)
        {
                m_visitor.TraverseDecl(c.getTranslationUnitDecl());
        }

	virtual bool HandleTopLevelDecl(clang::DeclGroupRef d)
	{
		for (clang::DeclGroupRef::iterator b = d.begin(), e = d.end(); b != e; ++b) {
			m_visitor.TraverseDecl(*b);
		}
		return true;
	}

	clang::Rewriter& get_rewriter()
	{
		return m_visitor.get_rewriter();
	}

	void get_reflected_classes(reflected_class::reflected_collection& out)
	{
		m_visitor.get_reflected_classes(out);
	}

private:
        visitor m_visitor;
}; // class consumer

} // namespace reflector

#endif // VISITOR_HPP
