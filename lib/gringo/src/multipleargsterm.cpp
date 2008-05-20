#include "multipleargsterm.h"
#include "literal.h"
#include "expandable.h"
#include "value.h"

using namespace NS_GRINGO;

MultipleArgsTerm::MultipleArgsTerm(Term *a, Term *b) : a_(a), b_(b)
{
}

MultipleArgsTerm::MultipleArgsTerm(MultipleArgsTerm &r)
{
	a_ = a_->clone();
	b_ = b_->clone();
}

Term* MultipleArgsTerm::clone()
{
	return new MultipleArgsTerm(*this);
}

void MultipleArgsTerm::print(std::ostream &out)
{
	out << "(" << a_ << "; " << b_ << ")" << std::endl;
}

void MultipleArgsTerm::getVars(VarSet &vars)
{
	assert(false);
}

bool MultipleArgsTerm::isComplex()
{
	assert(false);
}

Value MultipleArgsTerm::getValue()
{
	assert(false);
}

namespace
{
	class CloneSentinel : public Term
	{
	public:
		CloneSentinel(Term *a) : a_(a) {}
		Term* clone()
		{
			Term *a = a_;
			delete this;
			return a;
		}
		~CloneSentinel() { }
		// the rest is unused
		void getVars(VarSet &vars) { assert(false); }
		bool isComplex() { assert(false); }
		Value getValue() { assert(false); }
		void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e) { assert(false); }
		void print(std::ostream &stream) { assert(false); }
	protected:
		Term *a_;
	};
}

void MultipleArgsTerm::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	p = new CloneSentinel(b_);
	e->appendLiteral(l->clone(), true);
	p = a_;
	a_ = 0;
	b_ = 0;
	p->preprocess(l, p, g, e);
	delete this;
}

MultipleArgsTerm::~MultipleArgsTerm()
{
	if(a_)
		delete a_;
	if(b_)
		delete b_;
}

