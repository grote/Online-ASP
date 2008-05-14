#ifndef FUNCTIONTERM_H
#define FUNCTIONTERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class FunctionTerm : public Term
	{
	public:
		enum FunctionType { PLUS, MINUS, TIMES, DIVIDE, MOD, ABS };
	public:
		FunctionTerm(FunctionType type, Term *a, Term *b = 0);
		FunctionTerm(FunctionTerm &f);
		virtual void getVars(VarSet &vars);
		virtual bool isComplex();
		virtual void print(std::ostream &out);
		virtual Value getValue();
		virtual Term* clone();
		virtual ~FunctionTerm();
	protected:
		FunctionType type_;
		Term *a_;
		Term *b_;
	};
}

#endif

