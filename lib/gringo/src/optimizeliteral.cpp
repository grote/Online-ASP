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

#include "optimizeliteral.h"
#include "conditionalliteral.h"
#include "value.h"
#include "grounder.h"
#include "statementdependencygraph.h"
#include "statementchecker.h"
#include "literaldependencygraph.h"
#include "output.h"

using namespace NS_GRINGO;

OptimizeLiteral::OptimizeLiteral(Type type, ConditionalLiteralVector *literals, bool setSemantic) : Literal(), type_(type), setSemantic_(setSemantic), literals_(literals)
{
}

bool OptimizeLiteral::solved()
{
	assert(false);
}

bool OptimizeLiteral::isFact()
{
	assert(false);
}

bool OptimizeLiteral::checkO(LiteralVector &unsolved) 
{
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(!(*it)->checkO(unsolved))
			return false;
	}
	return true;
}

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

NS_OUTPUT::Object *OptimizeLiteral::convert()
{
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	if(setSemantic_)
	{
		UidValueSet set;
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		{
			ConditionalLiteral *p = *it;
			for(p->start(); p->hasNext(); p->next())
			{
				// caution there is no -0
				// match has to be called before isFact
				if(!set.insert(std::make_pair(p->getNeg() ? -1 - p->getUid() : p->getUid(), p->getValues())).second || !p->match() || p->isFact())
				{
					p->remove();
					continue;
				}
				lits.push_back(p->convert());
				weights.push_back(1);
			}
		}
	}
	else
	{
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		{
			ConditionalLiteral *p = *it;
			for(p->start(); p->hasNext(); p->next())
			{
				if(p->getWeight() == 0 || !p->match() || p->isFact())
				{
					p->remove();
					continue;
				}
				lits.push_back(p->convert());
				weights.push_back(p->getWeight());
			}
		}
	
	}

	return new NS_OUTPUT::Optimize(type_ == MINIMIZE ? NS_OUTPUT::Optimize::MINIMIZE : NS_OUTPUT::Optimize::MAXIMIZE, lits, weights);
}

void OptimizeLiteral::ground(Grounder *g, GroundStep step)
{
	for(ConditionalLiteralVector::const_iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->ground(g, step);
}


bool OptimizeLiteral::match(Grounder *g)
{
	ground(g, GROUND);
	return true;
}

void OptimizeLiteral::getVars(VarSet &vars) const
{
	for(ConditionalLiteralVector::const_iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->getVars(vars);
}

SDGNode *OptimizeLiteral::createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
{
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		// since there cant be any cycles through these statements 
		// its ok to add only positive dependencies
		// even though negative dependencies wont hurt
		SDGNode *p = (*it)->createNode(dg, prev, ADD_NOTHING);
		assert(p);
		prev->addDependency(p);
	}
	return 0;
}

void OptimizeLiteral::createNode(LDGBuilder *dg, bool head)
{
	VarSet needed, provided;
	dg->createNode(this, head, needed, provided, true);
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->createNode(dg, head);
}

void OptimizeLiteral::createNode(StatementChecker *dg, bool head, bool delayed)
{
	if(delayed)
	{
		// second pass
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
			dg->createSubNode(*it, head);
	}
	else
	{
		// first pass
		VarSet needed, provided;
		dg->createDelayedNode(this, head, needed, provided);
	}	
}

void OptimizeLiteral::reset()
{
	assert(false);
}

void OptimizeLiteral::finish()
{
	assert(false);
}

IndexedDomain *OptimizeLiteral::createIndexedDomain(VarSet &index)
{
	assert(false);
}

OptimizeLiteral::OptimizeLiteral(const OptimizeLiteral &a) : type_(a.type_), setSemantic_(a.setSemantic_)
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

Literal *OptimizeLiteral::clone() const
{
	return new OptimizeLiteral(*this);
}

void OptimizeLiteral::appendLiteral(Literal *l, ExpansionType type)
{
	if(!literals_)
		literals_ = new ConditionalLiteralVector();
	assert(dynamic_cast<ConditionalLiteral*>(l));
	literals_->push_back((ConditionalLiteral*)l);
}

void OptimizeLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(literals_)
		for(size_t i = 0; i < literals_->size(); i++)
			(*literals_)[i]->preprocess(g, this);
}

void OptimizeLiteral::print(std::ostream &out)
{
	out << (type_ == MINIMIZE ? "minimize" : "maximize") << " " << (setSemantic_ ? "{" : "[");
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		out << *it;
	}
	out << (setSemantic_ ? "}" : "]");
}

double OptimizeLiteral::heuristicValue()
{
	assert(false);
}

OptimizeLiteral::~OptimizeLiteral()
{
	if(literals_)
	{
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
			delete *it;
		delete literals_;
	}
}

