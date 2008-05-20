#ifndef MULTIPLEARGSTERM_H
#define MULTIPLEARGSTERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class MultipleArgsTerm : public Term
	{
	public:
		MultipleArgsTerm(Term *a, Term *b);
		MultipleArgsTerm(MultipleArgsTerm &r);
		virtual Term* clone();
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars);
		virtual bool isComplex();
		virtual Value getValue();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual ~MultipleArgsTerm();
	protected:
		Term *a_;
		Term *b_;
	};
}

#endif

