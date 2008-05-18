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
		RangeTerm(RangeTerm &r);
		virtual Term* clone();
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars);
		virtual bool isComplex();
		virtual Value getValue();
		virtual void preprocess(Term *&p, Grounder *g, Expandable *e);
		Term *getLower();
		Term *getUpper();
		virtual ~RangeTerm();
	protected:
		Term *lower_;
		Term *upper_;
		
	};
}

#endif

