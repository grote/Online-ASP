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
		MultipleArgsTerm(const MultipleArgsTerm &r);
		virtual Term* clone() const;
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual Value getValue();
		virtual Value getConstValue();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual bool unify(const Value&, const VarVector&, const VarVector&,
			       	ValueVector&, ValueVector&) const
		{
			assert(false);
		}
		virtual ~MultipleArgsTerm();
	protected:
		Term *a_;
		Term *b_;
	};
}

#endif

