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

#include "functionterm.h"
#include "value.h"

using namespace NS_GRINGO;

FunctionTerm::FunctionTerm(FunctionType type, Term *a, Term *b) : Term(), type_(type), a_(a), b_(b)
{
}

void FunctionTerm::print(std::ostream &out)
{
	switch(type_)
	{
		case ABS:
			out << "abs" << "(" << a_ << ")";
			break;
		case PLUS:
			out << "(" << a_ << " + " << b_ << ")";
			break;
		case MINUS:
			out << "(" << a_ << " - " << b_ << ")";
			break;
		case TIMES:
			out << "(" << a_ << " * " << b_ << ")";
			break;
		case DIVIDE:
			out << "(" << a_ << " / " << b_ << ")";
			break;
		case MOD:
			out << "(" << a_ << " mod " << b_ << ")";
			break;
		case BITXOR:
			out << "(" << a_ << " ^ " << b_ << ")";
			break;
		case BITOR:
			out << "(" << a_ << " ? " << b_ << ")";
			break;
		case BITAND:
			out << "(" << a_ << " & " << b_ << ")";
			break;
		case COMPLEMENT:
			out << "(~" << a_ << ")";
			break;
	}
}

void FunctionTerm::getVars(VarSet &vars) const
{
	a_->getVars(vars);
	if(b_)
		b_->getVars(vars);
}

void FunctionTerm::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	a_->preprocess(l, a_, g, e);
	if(b_)
		b_->preprocess(l, b_, g, e);
}

bool FunctionTerm::isComplex()
{
	return true;
}

Value FunctionTerm::getConstValue()
{
	switch(type_)
	{
		case ABS:
			return Value(abs(a_->getConstValue()));
		case PLUS:
			return Value(a_->getConstValue() + b_->getConstValue());
		case MINUS:
			return Value(a_->getConstValue() - b_->getConstValue());
		case TIMES:
			return Value(a_->getConstValue() * b_->getConstValue());
		case DIVIDE:
			return Value(a_->getConstValue() / b_->getConstValue());
		case MOD:
			return Value(a_->getConstValue() % b_->getConstValue());
		case BITXOR:
			return Value(a_->getConstValue() ^ b_->getConstValue());
		case BITOR:
			return Value(a_->getConstValue() | b_->getConstValue());
		case BITAND:
			return Value(a_->getConstValue() & b_->getConstValue());
		case COMPLEMENT:
			return Value(~a_->getConstValue());
	}
	assert(false);
}

Value FunctionTerm::getValue()
{
	switch(type_)
	{
		case ABS:
			return Value(abs(a_->getValue()));
		case PLUS:
			return Value(a_->getValue() + b_->getValue());
		case MINUS:
			return Value(a_->getValue() - b_->getValue());
		case TIMES:
			return Value(a_->getValue() * b_->getValue());
		case DIVIDE:
			return Value(a_->getValue() / b_->getValue());
		case MOD:
			return Value(a_->getValue() % b_->getValue());
		case BITXOR:
			return Value(a_->getValue() ^ b_->getValue());
		case BITOR:
			return Value(a_->getValue() | b_->getValue());
		case BITAND:
			return Value(a_->getValue() & b_->getValue());
		case COMPLEMENT:
			return Value(~a_->getValue());
	}
	assert(false);
}

FunctionTerm::FunctionTerm(const FunctionTerm &f) : type_(f.type_), a_(f.a_->clone()), b_(f.b_ ? f.b_->clone() : 0)
{
}

Term* FunctionTerm::clone() const
{
	return new FunctionTerm(*this);
}

FunctionTerm::~FunctionTerm()
{
	if(a_)
		delete a_;
	if(b_)
		delete b_;

}

