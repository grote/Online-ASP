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

#include "relationliteral.h"
#include "term.h"
#include "value.h"
#include "indexeddomain.h"
#include "literaldependencygraph.h"
#include "programdependencygraph.h"

using namespace NS_GRINGO;

RelationLiteral::RelationLiteral(RelationType type, Term *a, Term *b) : Literal(), type_(type), a_(a), b_(b)
{

}

SDGNode *RelationLiteral::createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
{
	return 0;
}

void RelationLiteral::createNode(LDGBuilder *dg, bool head)
{
	assert(!head);
	VarSet needed, provided;
	a_->getVars(needed);
	if(b_)
		b_->getVars(needed);
	dg->createNode(this, head, needed, provided);
}

void RelationLiteral::createNode(PDGBuilder *dg, bool head, bool defining, bool delayed)
{
	assert(!head && !defining && !delayed);
	VarSet needed, provided;
	a_->getVars(needed);
	b_->getVars(needed);
	dg->createNode(needed, provided);
}

void RelationLiteral::print(std::ostream &out)
{
	out << a_;
	switch(type_)
	{ 
		case EQ:
			out << " == ";
			break;
		case NE:
			out << " != ";
			break;
		case GT:
			out << " > ";
			break;
		case GE:
			out << " >= ";
			break;
		case LE:
			out << " <= ";
			break;
		case LT:
			out << " < ";
			break;
	}
	out << b_;
}

bool RelationLiteral::isFact()
{
	return true;
}

bool RelationLiteral::solved()
{
	return true;
}

void RelationLiteral::getVars(VarSet &vars) const
{
	a_->getVars(vars);
	b_->getVars(vars);
}

bool RelationLiteral::checkO(LiteralVector &unsolved)
{
	return true;
}

void RelationLiteral::reset()
{
	assert(false);
}

void RelationLiteral::finish()
{
	assert(false);
}

bool RelationLiteral::match(Grounder *g)
{
	switch(type_)
	{ 
		case EQ:
			return a_->getValue() == b_->getValue();
		case NE:
			return a_->getValue() != b_->getValue();
		case GT:
			return a_->getValue() > b_->getValue();
		case GE:
			return a_->getValue() >= b_->getValue();
		case LE:
			return a_->getValue() <= b_->getValue();
		case LT:
			return a_->getValue() < b_->getValue();
	}
	assert(false);
}

void RelationLiteral::preprocess(Grounder *g, Expandable *e)
{
	a_->preprocess(this, a_, g, e);
	b_->preprocess(this, b_, g, e);
}

double RelationLiteral::heuristicValue()
{
	// match it as soon as possible
	return 0;
}

IndexedDomain *RelationLiteral::createIndexedDomain(VarSet &index)
{
	return new IndexedDomainMatchOnly(this);
}

RelationLiteral::RelationLiteral(const RelationLiteral &r) : type_(r.type_), a_(r.a_->clone()), b_(r.b_->clone())
{
}

Literal* RelationLiteral::clone() const
{
	return new RelationLiteral(*this);
}

NS_OUTPUT::Object *RelationLiteral::convert()
{
	assert(false);
}

RelationLiteral::~RelationLiteral()
{
	if(a_)
		delete a_;
	if(b_)
		delete b_;
}

