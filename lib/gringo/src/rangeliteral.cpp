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

#include "rangeliteral.h"
#include "constant.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "grounder.h"
#include "literaldependencygraph.h"
#include "statementchecker.h"

using namespace NS_GRINGO;

RangeLiteral::RangeLiteral(Constant *var, Term *lower, Term *upper) : Literal(), var_(var), lower_(lower), upper_(upper)
{
}

SDGNode *RangeLiteral::createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
{
	return 0;
}

void RangeLiteral::createNode(LDGBuilder *dg, bool head)
{
	assert(!head);
	VarSet needed, provided;
	upper_->getVars(needed);
	lower_->getVars(needed);
	var_->getVars(provided);
	dg->createNode(this, head, needed, provided);
}

void RangeLiteral::createNode(StatementChecker *dg, bool head, bool delayed)
{
	assert(!head && !delayed);
	VarSet needed, provided;
	var_->getVars(provided);
	lower_->getVars(needed);
	upper_->getVars(needed);
	dg->createNode(needed, provided);
}

void RangeLiteral::print(std::ostream &out)
{
	out << var_ << " = [" << lower_ << ", " << upper_ << "]";
}

bool RangeLiteral::isFact(Grounder *g)
{
	return true;
}

bool RangeLiteral::solved()
{
	return true;
}

void RangeLiteral::getVars(VarSet &vars) const
{
	lower_->getVars(vars);
	upper_->getVars(vars);
	var_->getVars(vars);
}

bool RangeLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

void RangeLiteral::reset()
{
	assert(false);
}

void RangeLiteral::finish()
{
	assert(false);
}

bool RangeLiteral::match(Grounder *g)
{
	return ((int)lower_->getValue(g) <= (int)var_->getValue(g) && (int)var_->getValue(g) <= (int)upper_->getValue(g));
}

namespace
{
	class IndexedDomainRange : public IndexedDomain
	{
	public:
		IndexedDomainRange(int var, Term *lower, Term *upper);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainRange();
	protected:
		int var_;
		int current_;
		int end_;
		Term *lower_;
		Term *upper_;
	};

	IndexedDomainRange::IndexedDomainRange(int var, Term *lower, Term *upper) : IndexedDomain(), var_(var), current_(0), end_(0), lower_(lower), upper_(upper)
	{
	}

	void IndexedDomainRange::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		end_     = upper_->getValue(g->g_);
		current_ = lower_->getValue(g->g_);
		if(current_ <= end_)
		{
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnFirstMatch;
	}

	void IndexedDomainRange::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		if(current_ < end_)
		{
			current_++;
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnNextMatch;
	}

	IndexedDomainRange::~IndexedDomainRange()
	{
	}

}

IndexedDomain *RangeLiteral::createIndexedDomain(VarSet &index)
{
	if(index.find(var_->getUID()) == index.end())
		return new IndexedDomainRange(var_->getUID(), lower_, upper_);
	else
		return new IndexedDomainMatchOnly(this);
}

void RangeLiteral::preprocess(Grounder *g, Expandable *e)
{
	lower_->preprocess(this, lower_, g, e);
	upper_->preprocess(this, upper_, g, e);
}

NS_OUTPUT::Object *RangeLiteral::convert()
{
	assert(false);
}

RangeLiteral::RangeLiteral(const RangeLiteral &r) : var_((Constant*)r.var_->clone()), lower_(r.lower_->clone()), upper_(r.upper_->clone())
{
}

Literal* RangeLiteral::clone() const
{
	return new RangeLiteral(*this);
}

double RangeLiteral::heuristicValue()
{
	// hard to estimate cause in general the domainsize is hard to predict
	return DBL_MAX;
}

RangeLiteral::~RangeLiteral()
{
	if(var_)
		delete var_;
	if(lower_)
		delete lower_;
	if(upper_)
		delete upper_;
}

