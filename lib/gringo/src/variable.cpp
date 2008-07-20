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

#include "variable.h"
#include "grounder.h"

using namespace NS_GRINGO;
		
Variable::Variable(Grounder *g, std::string *id) : id_(id), uid_(g->registerVar(id))
{
}

void Variable::getVars(VarSet &vars) const
{
	vars.insert(uid_);
}

bool Variable::isComplex()
{
	return false;
}

void Variable::print(std::ostream &out)
{
	out << *id_;
}

Value Variable::getConstValue(Grounder *g)
{
	assert(false);
}

Value Variable::getValue(Grounder *g)
{
	return g->getValue(uid_);
}

int Variable::getUID()
{
	return uid_;
}

Variable::~Variable()
{
}

void Variable::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
}

Variable::Variable(const Variable &c) : id_(c.id_), uid_(c.uid_)
{
}

Term* Variable::clone() const
{
	return new Variable(*this);
}

bool Variable::unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitutions, ValueVector& freeSubstitutions) const
{
	for (unsigned int i = 0; i < boundVariables.size(); ++i)
	{
		//if variable is bound
		if (boundVariables[i] == uid_)
		{

			// variable is not set, then set, else compare
			if (boundSubstitutions[i].type_ == Value::UNDEF)
			{
				boundSubstitutions[i] = t;
				return true;
			}
			else
				return boundSubstitutions[i] == t;
		}
	}
	for (unsigned int i = 0; i < freeVariables.size(); ++i)
	{
		if (freeVariables[i] == uid_)
		{
			if (freeSubstitutions[i].type_ == Value::UNDEF)
			{
				freeSubstitutions[i] = t;
				return true;
			}
			else
				return freeSubstitutions[i] == t;
		}
	}
	assert(false);
}
