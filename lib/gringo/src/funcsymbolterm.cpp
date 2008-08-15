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

#include "funcsymbolterm.h"
#include "value.h"
#include "grounder.h"
#include "funcsymbol.h"
#include "assignmentliteral.h"
#include "variable.h"
#include "expandable.h"

using namespace NS_GRINGO;

FuncSymbolTerm::FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl) : Term(), name_(s), termList_(tl)
{
}

void FuncSymbolTerm::print(std::ostream &out)
{
	out << *name_ << "(";
	for (unsigned int i = 0; i != termList_->size()-1; ++i)
	{
		out << *(*termList_)[i] << ",";
	}
	out << *(*termList_)[termList_->size()-1] << ")";
}

void FuncSymbolTerm::getVars(VarSet &vars) const
{
	for (TermVector::const_iterator i = termList_->begin(); i != termList_->end(); ++i)
		(*i)->getVars(vars);
}

void FuncSymbolTerm::preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e)
{
	for (TermVector::iterator i = termList_->begin(); i != termList_->end(); ++i)
		(*i)->preprocess(l, (*i), g, e);
	for(TermVector::iterator it = termList_->begin(); it != termList_->end(); it++)
		if((*it)->isComplex())
		{
			std::string *var = g->createUniqueVar();
			e->appendLiteral(new AssignmentLiteral(new Variable(g, var), *it), Expandable::COMPLEXTERM);
			*it = new Variable(g, var);
		}
}

bool FuncSymbolTerm::isComplex()
{
	// TODO: change this to false in the new implementation of functionsymbols
	//       i want to keep a working version in the trunk so its better to 
	//       return true for now
	//return true;
	return false;
}

Value FuncSymbolTerm::getConstValue(Grounder *g)
{
	ValueVector args;
	for (unsigned int i = 0; i != termList_->size(); ++i)
	{
		args.push_back((*termList_)[i]->getConstValue(g));
	}

	FuncSymbol* funcSymbol = new FuncSymbol(name_, args);

	funcSymbol = g->createFuncSymbol(funcSymbol);
	return Value(funcSymbol);
}

Value FuncSymbolTerm::getValue(Grounder *g)
{
	ValueVector args;
	for (unsigned int i = 0; i != termList_->size(); ++i)
	{
		args.push_back((*termList_)[i]->getValue(g));
	}

	FuncSymbol* funcSymbol = new FuncSymbol(name_, args);

	funcSymbol = g->createFuncSymbol(funcSymbol);
	return Value(funcSymbol);
}

FuncSymbolTerm::FuncSymbolTerm(const FuncSymbolTerm &f) : name_(f.name_)
{
	termList_ = new TermVector();
	for (TermVector::const_iterator i = f.termList_->begin(); i != f.termList_->end(); ++i)
	{
		termList_->push_back((*i)->clone());
	}
}

Term* FuncSymbolTerm::clone() const
{
	return new FuncSymbolTerm(*this);
}

bool FuncSymbolTerm::unify(const Value& t, const VarVector& vars, ValueVector& vals) const
{
	if (t.type_ == Value::FUNCSYMBOL)
	{
		const std::string* name = t.funcSymbol_->getName();
		if (name != name_)
			return false;
		const ValueVector& values = t.funcSymbol_->getValues();
		if (values.size() != termList_->size())
			return false;
		TermVector::const_iterator term = termList_->begin();
		for (ValueVector::const_iterator i = values.begin(); i != values.end(); ++i, ++term)
		{
			if (!(*term)->unify(*i, vars, vals))
				return false;
		}
		return true;
	}
	else
		return false;

}

FuncSymbolTerm::~FuncSymbolTerm()
{
	for (TermVector::iterator i = termList_->begin(); i != termList_->end(); ++i)
		delete (*i);
	delete termList_;
}

