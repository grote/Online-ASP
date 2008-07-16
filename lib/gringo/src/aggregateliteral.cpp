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

#include "aggregateliteral.h"
#include "predicateliteral.h"
#include "conditionalliteral.h"
#include "term.h"
#include "value.h"
#include "grounder.h"
#include "statementdependencygraph.h"
#include "literaldependencygraph.h"
#include "programdependencygraph.h"
#include "indexeddomain.h"
#include "output.h"
#include "evaluator.h"
#include "constant.h"
#include "dlvgrounder.h"

using namespace NS_GRINGO;

AggregateLiteral::AggregateLiteral(ConditionalLiteralVector *literals) : Literal(), literals_(literals), lower_(0), upper_(0), equal_(0)
{
}

bool AggregateLiteral::solved()
{
	return false;
}

bool AggregateLiteral::isFact()
{
	return fact_;
}

void AggregateLiteral::setBounds(Term *lower, Term *upper)
{
	lower_ = lower;
	upper_ = upper;
}

void AggregateLiteral::setEqual(Constant *equal)
{
	equal_ = equal;
	lower_ = equal_->clone();
	upper_ = equal_->clone();
}

bool AggregateLiteral::checkO(LiteralVector &unsolved) 
{
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(!(*it)->checkO(unsolved))
			return false;
	}
	return true;
}

void AggregateLiteral::setEqual(int bound)
{
	lowerBound_ = bound;
	upperBound_ = bound;
}

bool AggregateLiteral::checkBounds(int lower, int upper)
{
	lowerBound_ = lower_ ? std::max((int)lower_->getValue(), lower) : lower;
	upperBound_ = upper_ ? std::min((int)upper_->getValue(), upper) : upper;
	// stupid bounds
	if(lowerBound_ > upperBound_)
		return getNeg();
	// the aggregate lies completely in the intervall
	// ---|         |--- <- Bounds
	// ------|   |------ <- Aggregate
	if(lower >= lowerBound_ && upper <= upperBound_)
	{
		return !getNeg();
	}
	// the intervals dont intersect
	// ----------|   |--- <- Bounds
	// ---|   |---------- <- Aggregate
	if(upper < lowerBound_ || lower > upperBound_)
		return getNeg();
	// the intervalls intersect
	return true;
}

bool AggregateLiteral::match(Grounder *g)
{
	int upper, lower, fixed;
	match(g, lower, upper, fixed);
	return checkBounds(lower, upper);
}

void AggregateLiteral::getVars(VarSet &vars) const
{
	if(equal_)
		equal_->getVars(vars);
	if(lower_)
		lower_->getVars(vars);
	if(upper_)
		upper_->getVars(vars);
	for(ConditionalLiteralVector::const_iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->getVars(vars);
}

SDGNode *AggregateLiteral::createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
{
	// TODO: this is only needed as long as the truth value of aggregates is not determined
	if(todo == ADD_BODY_DEP)
		prev->addDependency(prev, true);
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		// aggregate literals always depend negativly on its literals 
		SDGNode *p = (*it)->createNode(dg, prev, ADD_NOTHING);
		assert(p);
		prev->addDependency(p, true);
		// if used in the head it also depends cyclically on its literals
		// but not if the literal is negative
		if(todo == ADD_HEAD_DEP && !(*it)->getNeg())
			p->addDependency(prev);
	}
	return 0;
}

void AggregateLiteral::createNode(LDGBuilder *dg, bool head)
{
	VarSet needed, provided;
	if(equal_)
		equal_->getVars(provided);
	else
	{
		if(lower_)
			lower_->getVars(needed);
		if(upper_)
			upper_->getVars(needed);
	}
	dg->createNode(this, head, needed, provided, true);
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->createNode(dg, head);
}

void AggregateLiteral::createNode(PDGBuilder *dg, bool head, bool defining, bool delayed)
{
	if(delayed)
	{
		// second pass
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
			(*it)->createNode(dg, head, defining, false);
	}
	else
	{
		// first pass
		VarSet needed, provided;
		if(equal_)
		{
			equal_->getVars(provided);
		}
		else
		{
			if(lower_)
				lower_->getVars(needed);
			if(upper_)
				upper_->getVars(needed);
		}
		dg->createDelayedNode(this, head, defining, needed, provided);
	}
}

void AggregateLiteral::reset()
{
	// aggregate literals should never occur negativly in heads
	assert(!getNeg());
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->reset();
}

void AggregateLiteral::finish()
{
	// aggregate literals should never occur negativly in heads
	assert(!getNeg());
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->finish();
}

namespace
{
	class IndexedDomainAggregate : public IndexedDomain
	{
	public:
		IndexedDomainAggregate(AggregateLiteral *l, int var);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainAggregate();
	protected:
		AggregateLiteral *l_;
		int var_;
		int current_;
		int lower_;
		int upper_;
	};

	IndexedDomainAggregate::IndexedDomainAggregate(AggregateLiteral *l, int var) : l_(l), var_(var)
	{
	}

	void IndexedDomainAggregate::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		int fixed;
		l_->match(g->g_, lower_, upper_, fixed);
		current_ = lower_;
		if(current_ <= upper_)
		{
			g->g_->setValue(var_, Value(current_), binder);
			l_->setEqual(current_); 
			status = SuccessfulMatch;
		}
		else
			status = FailureOnFirstMatch;
	}

	void IndexedDomainAggregate::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		if(current_ < upper_)
		{
			current_++;
			g->g_->setValue(var_, Value(current_), binder);
			l_->setEqual(current_); 
			status = SuccessfulMatch;
		}
		else
			status = FailureOnNextMatch;
	}

	IndexedDomainAggregate::~IndexedDomainAggregate()
	{
	}
	
}

IndexedDomain *AggregateLiteral::createIndexedDomain(VarSet &index)
{
	if(equal_)
	{
		if(index.find(equal_->getUID()) != index.end())
		{
			return new IndexedDomainMatchOnly(this);
		}
		else
		{
			return new IndexedDomainAggregate(this, equal_->getUID());
		}
	}
	else
		return new IndexedDomainMatchOnly(this);
}

AggregateLiteral::AggregateLiteral(const AggregateLiteral &a) : lower_(a.lower_ ? a.lower_->clone() : 0), upper_(a.upper_ ? a.upper_->clone() : 0), equal_(a.equal_ ? static_cast<Constant*>(equal_->clone()) : 0)
{
	if(a.literals_)
	{
		literals_ = new ConditionalLiteralVector();
		for(ConditionalLiteralVector::iterator it = a.literals_->begin(); it != a.literals_->end(); it++)
			literals_->push_back((ConditionalLiteral*)(*it)->clone());
	}
	else
		literals_ = 0;
}

void AggregateLiteral::appendLiteral(Literal *l, ExpansionType type)
{
	if(!literals_)
		literals_ = new ConditionalLiteralVector();
	assert(dynamic_cast<ConditionalLiteral*>(l));
	literals_->push_back((ConditionalLiteral*)l);
}

void AggregateLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(literals_)
		for(size_t i = 0; i < literals_->size(); i++)
			(*literals_)[i]->preprocess(g, this);
	if(upper_)
		upper_->preprocess(this, upper_, g, e);
	if(lower_)
		lower_->preprocess(this, lower_, g, e);
}

ConditionalLiteralVector *AggregateLiteral::getLiterals() const
{
	return literals_;
}

Term *AggregateLiteral::getLower() const
{
	return lower_;
}

Term *AggregateLiteral::getUpper() const
{
	return upper_;
}

void AggregateLiteral::ground(Grounder *g, GroundStep step)
{
	//std::cerr << "grounding " << this << " (" << step << ")" << std::endl;
	if(literals_)
		for(ConditionalLiteralVector::const_iterator it = literals_->begin(); it != literals_->end(); it++)
			(*it)->ground(g, step);
}

double AggregateLiteral::heuristicValue()
{
	return DBL_MAX;
}

AggregateLiteral::~AggregateLiteral()
{
	if(equal_)
		delete equal_;
	if(lower_)
		delete lower_;
	if(upper_)
		delete upper_;
	if(literals_)
	{
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
			delete *it;
		delete literals_;
	}
}

