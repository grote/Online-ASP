#include "multipleargsterm.h"
#include "value.h"

using namespace NS_GRINGO;

MultipleArgsTerm::MultipleArgsTerm()
{
}

MultipleArgsTerm::MultipleArgsTerm(MultipleArgsTerm &r)
{
	for(TermVector::iterator it = r.args_.begin(); it != r.args_.end(); it++)
		args_.push_back((*it)->clone());
}

Term* MultipleArgsTerm::clone()
{
	return new MultipleArgsTerm(*this);
}

void MultipleArgsTerm::print(std::ostream &out)
{
	bool comma = false;
	for(TermVector::iterator it = args_.begin(); it != args_.end(); it++)
	{
		if(comma)
			out << "; ";
		else
			comma = true;
		out << *it;
	}
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

int MultipleArgsTerm::argc()
{
	return args_.size();
}

Term *MultipleArgsTerm::pop()
{
	Term *t = args_.back();
	args_.pop_back();
	return t;
}

void MultipleArgsTerm::push(Term *t)
{
	args_.push_back(t);
}

void MultipleArgsTerm::preprocess(Term *&p, Grounder *g, Expandable *e)
{
	assert(false);
}

MultipleArgsTerm *MultipleArgsTerm::create(Term *list, Term *term)
{
	MultipleArgsTerm *guess = dynamic_cast<MultipleArgsTerm*>(list);
	// if we dont get a multiple args term we simply make one :)
	if(!guess)
	{
		guess = new MultipleArgsTerm();
		guess->push(list);
	}
	guess->push(term);
	return guess;
}

MultipleArgsTerm::~MultipleArgsTerm()
{
	for(TermVector::iterator it = args_.begin(); it != args_.end(); it++)
		delete *it;
}

