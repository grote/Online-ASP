#include "sumaggregate.h"
#include "conditionalliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"

using namespace NS_GRINGO;

SumAggregate::SumAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

void SumAggregate::match(Grounder *g, int &lower, int &upper)
{
	fact_ = true;
	lower = 0;
	upper = 0;
	int fixed = 0;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			int weight = p->getWeight();
			if(!p->match() || weight == 0)
			{
				p->remove();
				continue;
			}
			if(p->isFact())
				fixed+= weight;
			else
			{
				fact_ = false;
				if(weight > 0)
					upper+= weight;
				else
					lower+= weight;
			}
		}
	}
	lower+= fixed;
	upper+= fixed;
}

void SumAggregate::print(std::ostream &out)
{
	if(lower_)
		out << lower_ << " ";
	out << "sum [";
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

SumAggregate::SumAggregate(const SumAggregate &a) : AggregateLiteral(a)
{
}

NS_OUTPUT::Object *SumAggregate::convert()
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
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::SUM, lower_->getValue(), lits, weights, upper_->getValue());
	else if(lower_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::SUM, lower_->getValue(), lits, weights);
	else if(upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::SUM, lits, weights, upper_->getValue());
	else
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::SUM, lits, weights);

	return a;
}

Literal *SumAggregate::clone() const
{
	return new SumAggregate(*this);
}

SumAggregate::~SumAggregate()
{
}

