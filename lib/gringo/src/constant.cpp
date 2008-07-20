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

#include "constant.h"
#include "grounder.h"

using namespace NS_GRINGO;
		
Constant::Constant(int value) : value_(value)
{
}

Constant::Constant(std::string *value) : value_(value)
{
}

void Constant::getVars(VarSet &vars) const
{
}

bool Constant::isComplex()
{
	return false;
}

void Constant::print(std::ostream &out)
{
	out << value_;
}

Value Constant::getConstValue(Grounder *g)
{
	if(value_.type_ == Value::STRING)
		return g->getConstValue(value_.stringValue_);
	else
		return value_;
}

Value Constant::getValue(Grounder *g)
{
	return value_;
}

Constant::~Constant()
{
}

void Constant::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	if(value_.type_ == Value::STRING)
		value_ = g->getConstValue(value_.stringValue_);
}

Constant::Constant(const Constant &c) :  value_(c.value_)
{
}

Term* Constant::clone() const
{
	return new Constant(*this);
}

bool Constant::unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitutions, ValueVector& freeSubstitutions) const
{
	return t == value_;
}
