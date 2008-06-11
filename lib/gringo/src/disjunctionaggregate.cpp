#include "disjunctionaggregate.h"
#include "conditionalliteral.h"
#include "predicateliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"

using namespace NS_GRINGO;

DisjunctionAggregate::DisjunctionAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

bool DisjunctionAggregate::match(Grounder *g)
{
	fact_ = false;
	bool match = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			// every literal will match :)
			if(p->match())
			{
				match = true;
				if(p->isFact())
				{
					fact_ = true;
					return true;
				}
			}
		}
	}
	return match;
}

void DisjunctionAggregate::match(Grounder *g, int &lower, int &upper)
{
	assert(false);
}

void DisjunctionAggregate::print(std::ostream &out)
{
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << " | ";
		else
			comma = true;
		out << *it;
	}
}

DisjunctionAggregate::DisjunctionAggregate(const DisjunctionAggregate &a) : AggregateLiteral(a)
{
}

NS_OUTPUT::Object *DisjunctionAggregate::convert()
{
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
			lits.push_back(p->convert());
	}
	return new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::DISJUNCTION, lits, weights);
}

void DisjunctionAggregate::preprocess(Grounder *g, Expandable *e)
{
	assert(literals_);
	for(size_t i = 0; i < literals_->size(); i++)
		(*literals_)[i]->preprocessDisjunction(g, this, e);
	if(upper_)
		upper_->preprocess(this, upper_, g, e);
	if(lower_)
		lower_->preprocess(this, lower_, g, e);
}

Literal *DisjunctionAggregate::clone() const
{
	return new DisjunctionAggregate(*this);
}

Literal *DisjunctionAggregate::createHead(ConditionalLiteralVector *list)
{
	if(list->size() == 1 && !list->front()->hasConditionals())
	{
		Literal *l = list->front()->toPredicateLiteral();
		delete list;
		return l;
	}
	else
		return new DisjunctionAggregate(list);
}

DisjunctionAggregate::~DisjunctionAggregate()
{
}

