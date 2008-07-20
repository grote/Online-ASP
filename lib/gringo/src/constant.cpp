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
		
Constant::Constant(int value) : Term(), type_(NUM), value_(value), uid_(0)
{
}

Constant::Constant(ConstantType type, Grounder *g, std::string *value) : Term(), type_(type), uid_(type == VAR ? g->registerVar(value) : 0)
{
	switch(type_)
	{
		case ID:
		case VAR:
		{
			value_.type_        = Value::STRING;
			value_.stringValue_ = value;
			break;
		}
		case NUM: 
		{
			value_.type_     = Value::INT;
			value_.intValue_ = atol(value->c_str());
			delete value;
			break;
		}
	}
}

void Constant::getVars(VarSet &vars) const
{
	if(type_ == VAR)
		vars.insert(uid_);
}

bool Constant::isComplex()
{
	return false;
}

void Constant::print(std::ostream &out)
{
	switch(type_)
	{
		case VAR:
		case ID:
		{
			out << *(value_.stringValue_);
			break;
		}
		case NUM: 
		{
			out << value_.intValue_;
			break;
		}
	}
}

Value Constant::getConstValue(Grounder *g)
{
	switch(type_)
	{
		case VAR:
			assert(false);
		case ID:
			return g->getConstValue(value_.stringValue_);
		case NUM:
			return value_;
	}
	assert(false);
}

Value Constant::getValue(Grounder *g)
{
	if(type_ == VAR)
		return g->getValue(uid_);
	else
		return value_;
}

int Constant::getUID()
{
	return uid_;
}

Constant::~Constant()
{
}

void Constant::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	if(type_ == ID)
	{
		value_ = g->getConstValue(value_.stringValue_);
		if(value_.type_ == Value::INT)
			type_ = NUM;
	}
}

Constant::Constant(const Constant &c) : type_(c.type_), value_(c.value_), uid_(c.uid_)
{
}

Term* Constant::clone() const
{
	return new Constant(*this);
}

bool Constant::unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitutions, ValueVector& freeSubstitutions) const
{
	switch(type_)
	{
		case VAR:
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
				break;
			}
		case ID:
		case NUM:
			{
				return t == value_;
			}	
		default: 
		{
			assert(false);
		}
	}
}
