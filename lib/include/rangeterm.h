#ifndef RANGETERM_H
#define RANGETERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class RangeTerm : public Term
	{
	public:
		RangeTerm(int lower, int upper);
		RangeTerm(RangeTerm &r);
		virtual Term* clone();
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars);
		virtual bool isComplex();
		virtual Value getValue();
		int getLower();
		int getUpper();
		virtual ~RangeTerm();
	protected:
		int lower_;
		int upper_;
		
	};
}

#endif

