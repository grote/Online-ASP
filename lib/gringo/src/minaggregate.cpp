#include "minaggregate.h"
#include "conditionalliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"

using namespace NS_GRINGO;

MinAggregate::MinAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

void MinAggregate::match(Grounder *g, int &lower, int &upper)
{
	fact_ = true;
	lower = INT_MAX;
	upper = INT_MIN;
	int fixed = INT_MAX;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match())
			{
				p->remove();
				continue;
			}
			int weight = p->getWeight();
			if(p->isFact())
				fixed = std::min(fixed, weight);
			else
			{
				fact_ = false;
				lower = std::min(lower, weight);
				upper = std::max(upper, weight);
			}
		}
	}

	lower = std::min(lower, fixed);
	if(upper == INT_MIN || fixed > upper)
		upper = fixed;
}

void MinAggregate::print(std::ostream &out)
{
	if(lower_)
		out << lower_ << " ";
	out << "min [";
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		out << *it;
	}
	out << "]";
	if(upper_)
		out << " " << upper_;
}

MinAggregate::MinAggregate(const MinAggregate &a) : AggregateLiteral(a)
{
}

NS_OUTPUT::Object *MinAggregate::convert()
{
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
		{
			lits.push_back(p->convert());
			weights.push_back(p->getWeight());
		}
	}
	NS_OUTPUT::Aggregate *a;
	if(lower_ && upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lower_->getValue(), lits, weights, upper_->getValue());
	else if(lower_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lower_->getValue(), lits, weights);
	else if(upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lits, weights, upper_->getValue());
	else
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::MIN, lits, weights);

	return a;
}

Literal *MinAggregate::clone() const
{
	return new MinAggregate(*this);
}

MinAggregate::~MinAggregate()
{
}

