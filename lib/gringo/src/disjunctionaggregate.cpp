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
		p->ground(g, GROUND);
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

void DisjunctionAggregate::match(Grounder *g, int &lower, int &upper, int &fixed)
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
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
			lits.push_back(p->convert());
	}
	return new NS_OUTPUT::Disjunction(lits);
}

void DisjunctionAggregate::preprocess(Grounder *g, Expandable *e)
{
	assert(literals_);
	for(size_t i = 0; i < literals_->size(); i++)
		(*literals_)[i]->preprocessDisjunction(g, this, e);
	assert(!upper_);
	assert(!lower_);
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

