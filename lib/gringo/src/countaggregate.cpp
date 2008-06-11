#include "countaggregate.h"
#include "conditionalliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"

using namespace NS_GRINGO;

namespace
{
	struct Hash
	{
		Value::VectorHash hash;
		size_t operator()(const std::pair<int, ValueVector> &k) const
		{
			return (size_t)k.first + hash(k.second);
		}
	};
	struct Equal
	{
		Value::VectorEqual equal;
		size_t operator()(const std::pair<int, ValueVector> &a, const std::pair<int, ValueVector> &b) const
		{
			return a.first == b.first && equal(a.second, b.second);
		}
	};
	typedef __gnu_cxx::hash_set<std::pair<int, ValueVector>, Hash, Equal> UidValueSet;
}

CountAggregate::CountAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

void CountAggregate::match(Grounder *g, int &lower, int &upper)
{
	UidValueSet set;
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
			if(!set.insert(std::make_pair(p->getUid(), p->getValues())).second)
			{
				p->remove();
				continue;
			}
			if(!p->match())
			{
				p->remove();
				continue;
			}
			if(p->isFact())
				fixed++;
			else
			{
				fact_ = false;
				upper++;
			}
		}
	}
	lower+= fixed;
	upper+= fixed;
}

void CountAggregate::print(std::ostream &out)
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

CountAggregate::CountAggregate(const CountAggregate &a) : AggregateLiteral(a)
{
}

NS_OUTPUT::Object *CountAggregate::convert()
{
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
			lits.push_back(p->convert());
	}
	NS_OUTPUT::Aggregate *a;
	if(lower_ && upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lower_->getValue(), lits, weights, upper_->getValue());
	else if(lower_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lower_->getValue(), lits, weights);
	else if(upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lits, weights, upper_->getValue());
	else
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lits, weights);

	return a;
}

Literal *CountAggregate::clone() const
{
	return new CountAggregate(*this);
}

CountAggregate::~CountAggregate()
{
}

