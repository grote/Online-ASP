#include "conjunctionaggregate.h"
#include "conditionalliteral.h"
#include "predicateliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"

using namespace NS_GRINGO;

ConjunctionAggregate::ConjunctionAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

bool ConjunctionAggregate::match(Grounder *g)
{
	fact_ = true;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match())
				return false;
			if(p->isFact())
				p->remove();
			else
				fact_ = false;
		}
	}
	return true;
}

void ConjunctionAggregate::match(Grounder *g, int &lower, int &upper, int &fixed)
{
	assert(false);
}

void ConjunctionAggregate::print(std::ostream &out)
{
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		out << *it;
	}
}

ConjunctionAggregate::ConjunctionAggregate(const ConjunctionAggregate &a) : AggregateLiteral(a)
{
}

void ConjunctionAggregate::preprocess(Grounder *g, Expandable *e)
{
	assert(literals_);
	for(size_t i = 0; i < literals_->size(); i++)
		(*literals_)[i]->preprocessDisjunction(g, this, e);
	assert(!upper_);
	assert(!lower_);
}

NS_OUTPUT::Object *ConjunctionAggregate::convert()
{
	NS_OUTPUT::ObjectVector lits;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
			lits.push_back(p->convert());
	}
	return new NS_OUTPUT::Conjunction(lits);
}

Literal *ConjunctionAggregate::clone() const
{
	return new ConjunctionAggregate(*this);
}

Literal *ConjunctionAggregate::createBody(PredicateLiteral *pred, LiteralVector *list)
{
	if(list == 0 || list->size() == 0)
		return pred;
	else
	{
		ConditionalLiteralVector *clv = new ConditionalLiteralVector();
		clv->push_back(new ConditionalLiteral(pred, list));
		return new ConjunctionAggregate(clv);
	}
}

ConjunctionAggregate::~ConjunctionAggregate()
{
}

