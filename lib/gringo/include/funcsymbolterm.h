#ifndef FUNCSYMBOLTERM_H
#define FUNCSYMBOLTERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class FuncSymbolTerm : public Term
	{
	public:
		FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl);
		FuncSymbolTerm(const FuncSymbolTerm &f);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual void print(std::ostream &out);
		virtual Value getValue();
		virtual Value getConstValue();
 		virtual Term* clone() const;
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual bool unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitution, ValueVector& freeSubstitutions) const;
		virtual ~FuncSymbolTerm();
	protected:
		std::string*	name_;
		TermVector*		termList_;
		Grounder*		grounder_;
	};
}

#endif

