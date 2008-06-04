#ifndef RANGETERM_H
#define RANGETERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class RangeTerm : public Term
	{
	public:
		RangeTerm(Term *lower, Term *upper);
		RangeTerm(const RangeTerm &r);
		virtual Term* clone() const;
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual Value getValue();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual bool unify(const Value&, const VarVector&, const VarVector&,
			       	ValueVector&, ValueVector&) const
		{
			assert(false);
		}
		Term *getLower();
		Term *getUpper();
		virtual ~RangeTerm();
	protected:
		Term *lower_;
		Term *upper_;
		
	};
}

#endif

