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

#include "statementdependencygraph.h"
#include "literaldependencygraph.h"
#include "statementchecker.h"
#include "predicateliteral.h"
#include "assignmentliteral.h"
#include "relationliteral.h"
#include "grounder.h"
#include "constant.h"
#include "term.h"
#include "domain.h"
#include "rangeterm.h"
#include "rangeliteral.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "value.h"
#include "multipleargsterm.h"
#include "output.h"
#include "evaluator.h"

using namespace NS_GRINGO;
		
PredicateLiteral::PredicateLiteral(Grounder *g, std::string *id, TermVector *variables) : Literal(), uid_(g->createPred(id, variables->size())), predNode_(g->getDomain(uid_)), id_(id), variables_(variables), values_(variables ? variables->size() : 0)
{
}

PredicateLiteral::PredicateLiteral(const PredicateLiteral &p) : uid_(p.uid_), predNode_(p.predNode_), id_(p.id_), values_(p.values_.size())
{
        if(p.variables_)
        {
                variables_ = new TermVector();
                for(TermVector::iterator it = p.variables_->begin(); it != p.variables_->end(); it++)
                                variables_->push_back((*it)->clone());
        }
        else
                variables_ = 0;
}

bool PredicateLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

SDGNode *PredicateLiteral::createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
{
	SDGNode *n = dg->createPredicateNode(this);
	switch(todo)
	{
		case ADD_BODY_DEP:
			prev->addDependency(n, getNeg());
			break;
		case ADD_HEAD_DEP:
			n->addDependency(prev);
			break;
		case ADD_NOTHING:
			break;
	}
	return n;
}

void PredicateLiteral::createNode(LDGBuilder *dg, bool head)
{
	// TODO: take the parameter check into consideration
	VarSet needed, provided;
	if(head || getNeg() || !predNode_->complete())
	{
		if(variables_)
			for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
				(*it)->getVars(needed);
	}
	else
	{
		if(variables_)
			for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
				if((*it)->isComplex())
					(*it)->getVars(needed);
				else
					(*it)->getVars(provided);
	}
	dg->createNode(this, head, needed, provided);
}

void PredicateLiteral::createNode(StatementChecker *dg, bool head, bool delayed)
{
	VarSet vars, empty;
	getVars(vars);
	if(head || getNeg())
		dg->createNode(vars, empty);
	else
		dg->createNode(empty, vars);
}

void PredicateLiteral::print(std::ostream &out)
{
	if(getNeg())
		out << "not ";
	out << *id_;
	if(getArity() > 0)
	{
		out << "(";
		bool comma = false;
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		{
			if(comma)
				out << ", ";
			else
				comma = true;
			out << *it;
		}
		out << ")";
	}
}

const ValueVector &PredicateLiteral::getValues()
{
	return values_;
}

void PredicateLiteral::reset()
{
	if(!getNeg())
		predNode_->reset();
}

void PredicateLiteral::finish()
{
	if(!getNeg())
		predNode_->finish();
}

void PredicateLiteral::evaluate()
{
	if(!getNeg())
		predNode_->evaluate();
}

bool PredicateLiteral::isFact(const ValueVector &values)
{
	// a precondition for this method is that the predicate is not false!
	if(predNode_->solved())
		return true;
	if(getNeg())
	{
		assert(!predNode_->isFact(values));
		if(predNode_->complete())
			return !predNode_->inDomain(values);
		else
			return false;
	}
	else
		return predNode_->isFact(values);
}

bool PredicateLiteral::isFact()
{
	// a precondition for this method is that the predicate is not false!
	if(predNode_->solved())
		return true;
	for(int i = 0; i < (int)variables_->size(); i++)
		values_[i] = (*variables_)[i]->getValue();
	if(getNeg())
	{
		assert(!predNode_->isFact(values_));
		if(predNode_->complete())
			return !predNode_->inDomain(values_);
		else
			return false;
	}
	else
		return predNode_->isFact(values_);
}

bool PredicateLiteral::solved()
{
	return predNode_->solved();
}

bool PredicateLiteral::match(const ValueVector &values)
{
	if(!predNode_->complete())
	{
		if(getNeg())
			return !predNode_->isFact(values);
		else
			return true;
	}
	bool match = predNode_->inDomain(values);
	if(getNeg())
	{
		return !(match && (predNode_->solved() || predNode_->isFact(values)));
	}
	else
	{
		return match;
	}
}

bool PredicateLiteral::match(Grounder *g)
{
	assert(!predNode_->solved() || predNode_->complete());
	// incomplete prediactes match everything
	// except if they contain some values that are facts!!!
	if(!predNode_->complete())
	{
		if(getNeg() && predNode_->hasFacts())
		{
			for(int i = 0; i < (int)variables_->size(); i++)
				values_[i] = (*variables_)[i]->getValue();
			return !predNode_->isFact(values_);
		}
		else
			return true;
	}
	// check if current assignment is satisfied wrt the domain
	bool match = true;
	if(variables_)
	{
		for(int i = 0; i < (int)variables_->size(); i++)
			values_[i] = (*variables_)[i]->getValue();
	}
	match = predNode_->inDomain(values_);
	if(getNeg())
	{
		return !(match && (predNode_->solved() || predNode_->isFact(values_)));
	}
	else
	{
		return match;
	}
}

NS_OUTPUT::Object * PredicateLiteral::convert(const ValueVector &values)
{
	return new NS_OUTPUT::Atom(getNeg(), predNode_, uid_, values);
}

NS_OUTPUT::Object *PredicateLiteral::convert()
{
	// the method isFact has to be called before this method
	return convert(values_);
}

void PredicateLiteral::addDomain(ValueVector &values)
{
	predNode_->addDomain(values);
}

IndexedDomain *PredicateLiteral::createIndexedDomain(VarSet &index)
{
	if(!predNode_->complete() || getNeg())
		return new IndexedDomainMatchOnly(this);
	if(variables_)
	{
		VarSet vars, free;
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			(*it)->getVars(vars);
		for(VarSet::iterator it = vars.begin(); it != vars.end(); it++)
			if(index.find(*it) == index.end())
				free.insert(*it);
		if(free.size() > 0)
		{
			return new IndexedDomainNewDefault(predNode_->getDomain(), index, *variables_);
		}
		else
			return new IndexedDomainMatchOnly(this);
	}
	else
		return new IndexedDomainMatchOnly(this);
}

Literal* PredicateLiteral::clone() const
{
	return new PredicateLiteral(*this);
}

PredicateLiteral::~PredicateLiteral()
{
	if(variables_)
	{
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
		{
			delete *it;
		}
		delete variables_;
	}
}

Domain *PredicateLiteral::getDomain() const
{
	return predNode_;
}

void PredicateLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(variables_)
	{
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			(*it)->preprocess(this, *it, g, e);
		for(TermVector::iterator it = variables_->begin(); it != variables_->end(); it++)
			if((*it)->isComplex())
			{
				std::string *var = g->createUniqueVar();
				e->appendLiteral(new AssignmentLiteral(new Constant(Constant::VAR, g, var), *it), Expandable::COMPLEXTERM);
				*it = new Constant(Constant::VAR, g, var);
			}
	}
	if((*id_)[0] == '-')
		g->addTrueNegation(id_, variables_ ? variables_->size() : 0);
}

TermVector *PredicateLiteral::getArgs()
{
	return variables_;
}


int PredicateLiteral::getUid()
{
	return uid_;
}

std::string *PredicateLiteral::getId()
{
	return id_;
}

int PredicateLiteral::getArity()
{
	return variables_->size();
}

double PredicateLiteral::heuristicValue()
{
	// TODO: thats too simple !!! i want something better
	if(!getNeg())
	{
		if(predNode_->complete())
		{
			if(variables_ && variables_->size() > 0)
				return pow(predNode_->getDomain().size(), 1.0 / variables_->size());
			else
				return 0;
		}
		else
			// the literal doenst help at all
			return DBL_MAX;
	}
	else if(predNode_->solved())
	{
		// the literal can be used to check if the assignment is valid
		return 0;
	}
	else
		return DBL_MAX;
}


void PredicateLiteral::getVars(VarSet &vars) const
{
	if(variables_)
		for(TermVector::const_iterator it = variables_->begin(); it != variables_->end(); it++)
			(*it)->getVars(vars);
}

