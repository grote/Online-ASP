// Copyright (c) 2008, Roland Kaminski
//
// This file is part of GrinGo.
//
// GrinGo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GrinGo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GrinGo.  If not, see <http://www.gnu.org/licenses/>.

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

void CountAggregate::match(Grounder *g, int &lower, int &upper, int &fixed)
{
	UidValueSet set;
	fact_ = true;
	lower = 0;
	upper = 0;
	fixed = 0;
	maxUpperBound_ = 0;
	minLowerBound_ = 0;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g, GROUND);
		for(p->start(); p->hasNext(); p->next())
		{
			// caution there is no -0
			if(!set.insert(std::make_pair(p->getNeg() ? -1 - p->getUid() : p->getUid(), p->getValues())).second)
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
			maxUpperBound_++;
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
	bool hasUpper = upperBound_ < maxUpperBound_;
	bool hasLower = lowerBound_ > minLowerBound_;

	if(hasLower && hasUpper)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lowerBound_, lits, weights, upperBound_);
	else if(hasLower)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lowerBound_, lits, weights);
	else if(hasUpper)
		a = new NS_OUTPUT::Aggregate(getNeg(), NS_OUTPUT::Aggregate::COUNT, lits, weights, upperBound_);
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

