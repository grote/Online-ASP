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
#include "statementdependencygraph.h"
#include "literaldependencygraph.h"
#include "statementchecker.h"
#include "conditionalliteral.h"
#include "output.h"
#include "grounder.h"
#include "evaluator.h"
#include "gringoexception.h"

using namespace NS_GRINGO;
		
LiteralStatement::LiteralStatement(Literal *lit, bool preserveOrder) : lit_(lit), preserveOrder_(preserveOrder)
{
}

void LiteralStatement::buildDepGraph(SDG *dg)
{
	// the order of optimize statements is important
	SDGNode *prev = dg->createStatementNode(this, preserveOrder_);
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

bool LiteralStatement::check(Grounder *g, VarVector &free)
{
	StatementChecker s;
	lit_->createNode(&s, false, false);

	if(s.check())
		return true;
	else
	{
		s.getFreeVars(free);
		return false;
	}
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

bool LiteralStatement::ground(Grounder *g, GroundStep step)
{
	if(step == GROUND)
	{
		lit_->match(g);
		grounded(g);
	}
	else
		lit_->ground(g, step);
	return true;
}

void LiteralStatement::addDomain(PredicateLiteral *pl)
{
	assert(false);
}

void LiteralStatement::print(const GlobalStorage *g, std::ostream &out) const
{
	out << pp(g, lit_) << "." << std::endl;
}

LiteralStatement::~LiteralStatement()
{
	delete lit_;
}

void LiteralStatement::setIncPart(Grounder *g, IncPart part, const Value &v)
{
	std::cerr << "The following statement cant be used with the incremental interface: " << std::endl << "\t" << this << std::endl;
	throw GrinGoException("Error: unsupported statement.");
}

