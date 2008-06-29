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

#include "literalstatement.h"
#include "dependencygraph.h"
#include "literaldependencygraph.h"
#include "conditionalliteral.h"
#include "node.h"
#include "output.h"
#include "grounder.h"
#include "evaluator.h"

using namespace NS_GRINGO;
		
LiteralStatement::LiteralStatement(Literal *lit, bool preserveOrder) : lit_(lit), preserveOrder_(preserveOrder)
{
}

void LiteralStatement::buildDepGraph(DependencyGraph *dg)
{
	// the order of optimize statements is important
	Node *prev = dg->createStatementNode(this, preserveOrder_);
	lit_->createNode(dg, prev, Literal::ADD_NOTHING);
}

void LiteralStatement::getVars(VarSet &vars) const
{
	// noone needs to know the vars in this statement
	//for(ConditionalLiteralVector::iterator it = literals_.begin(); it != literals_.end(); it++)
	//	(*it)->getVars(vars, VARS_ALL);
}

bool LiteralStatement::checkO(LiteralVector &unsolved)
{
	return lit_->checkO(unsolved);
}

bool LiteralStatement::check(VarVector &free)
{
	LDG dg;
	free.clear();
	LDGBuilder dgb(&dg);
	dgb.addToBody(lit_);
	dgb.create();
	dg.check(free);

	return free.size() == 0;
}

void LiteralStatement::preprocess(Grounder *g)
{
	lit_->preprocess(g, 0);
}

void LiteralStatement::reset()
{
	// nothing todo
}

void LiteralStatement::finish()
{
	// nothing todo
}

void LiteralStatement::evaluate()
{
	// nothing todo
}

void LiteralStatement::grounded(Grounder *g)
{
	g->getEvaluator()->add(lit_->convert());
}

bool LiteralStatement::ground(Grounder *g)
{
	lit_->match(g);
	grounded(g);
	return true;
}

void LiteralStatement::addDomain(PredicateLiteral *pl)
{
	assert(false);
}

void LiteralStatement::print(std::ostream &out)
{
	out << lit_ << "." << std::endl;
}

LiteralStatement::~LiteralStatement()
{
	delete lit_;
}

