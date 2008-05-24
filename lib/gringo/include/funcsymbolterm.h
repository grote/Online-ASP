#ifndef FUNCSYMBOLTERM_H
#define FUNCSYMBOLTERM_H

#include <gringo.h>
//#include <term.h>
#include <constant.h>

namespace NS_GRINGO
{
	class FuncSymbolTerm : public Constant
	{
	public:
		FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl);
		FuncSymbolTerm(FuncSymbolTerm &f);
		virtual void getVars(VarSet &vars);
		virtual bool isComplex();
		virtual void print(std::ostream &out);
		virtual Value getValue();
		virtual Term* clone();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual ~FuncSymbolTerm();
	protected:
		TermVector*		termList_;
		std::string*	funcSymbol_;
	};
}

#endif

