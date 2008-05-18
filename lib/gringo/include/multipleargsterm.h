#ifndef MULTIPLEARGSTERM_H
#define MULTIPLEARGSTERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class MultipleArgsTerm : public Term
	{
	public:
		MultipleArgsTerm();
		MultipleArgsTerm(MultipleArgsTerm &r);
		virtual Term* clone();
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars);
		virtual bool isComplex();
		virtual Value getValue();
		virtual void preprocess(Term *&p, Grounder *g, Expandable *e);
		int argc();
		Term *pop();
		void push(Term *t);
		virtual ~MultipleArgsTerm();
		static MultipleArgsTerm *create(Term *list, Term *term);
	protected:
		TermVector args_;
		
	};
}

#endif

