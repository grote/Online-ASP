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
		FunctionTerm(const FunctionTerm &f);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual void print(std::ostream &out);
		virtual Value getValue();
		virtual Value getConstValue();
		virtual Term* clone() const;
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual bool unify(const Value&, const VarVector&, const VarVector&,
			       	ValueVector&, ValueVector&) const
		{
			assert(false);
		}
		virtual ~FunctionTerm();
	protected:
		FunctionType type_;
		Term *a_;
		Term *b_;
	};
}

#endif

