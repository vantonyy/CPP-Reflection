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
	
	bool has_reflected_classes() const
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
		return d->isClass() && d->hasDefinition() && m_source_mgr.isInMainFile(d->getLocStart());
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
