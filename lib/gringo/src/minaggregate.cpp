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

#include "minaggregate.h"
#include "conditionalliteral.h"
#include "term.h"
#include "value.h"
#include "output.h"
#include "dlvgrounder.h"
#include "grounder.h"
#include "indexeddomain.h"
#include "constant.h"

using namespace NS_GRINGO;

MinAggregate::MinAggregate(ConditionalLiteralVector *literals) : AggregateLiteral(literals)
{
}

void MinAggregate::match(Grounder *g, int &lower, int &upper, int &fixed)
{
	fact_ = true;
	lower = INT_MAX;
	upper = INT_MIN;
	fixed = INT_MAX;
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

namespace
{
	class IndexedDomainMinAggregate : public IndexedDomain
	{
	protected:
		typedef std::set<int> IntSet;
	public:
		IndexedDomainMinAggregate(MinAggregate *l, int var);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainMinAggregate();
	protected:
		MinAggregate *l_;
		int var_;
		IntSet set_;
		IntSet::iterator current_;
	};

	IndexedDomainMinAggregate::IndexedDomainMinAggregate(MinAggregate *l, int var) : l_(l), var_(var)
	{
		
	}

	void IndexedDomainMinAggregate::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		int lower, upper, fixed;
		l_->match(g->g_, lower, upper, fixed);
		set_.clear();
		ConditionalLiteralVector *l = l_->getLiterals();
		for(ConditionalLiteralVector::iterator it = l->begin(); it != l->end(); it++)
		{
			ConditionalLiteral *p = *it;
			for(p->start(); p->hasNext(); p->next())
			{
				// insert all possible bindings for the bound
				int weight = p->getWeight();
				if(weight < fixed)
					set_.insert(weight);
			}
		}
		set_.insert(fixed);

		current_ = set_.begin();
		g->g_->setValue(var_, Value(*current_), binder);
		status = SuccessfulMatch;
	}

	void IndexedDomainMinAggregate::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		
		current_++;
		if(current_ != set_.end())
		{
			g->g_->setValue(var_, Value(*current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnNextMatch;
	}

	IndexedDomainMinAggregate::~IndexedDomainMinAggregate()
	{
	}
}

IndexedDomain *MinAggregate::createIndexedDomain(VarSet &index)
{
	if(equal_)
	{
		if(index.find(equal_->getUID()) != index.end())
		{
			return new IndexedDomainMatchOnly(this);
		}
		else
		{
			return new IndexedDomainMinAggregate(this, equal_->getUID());
		}
	}
	else
		return new IndexedDomainMatchOnly(this);
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

