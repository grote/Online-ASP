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

#ifndef FUNCSYMBOLTERM_H
#define FUNCSYMBOLTERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class FuncSymbolTerm : public Term
	{
	public:
		FuncSymbolTerm(Grounder* g, std::string* s, TermVector* tl);
		FuncSymbolTerm(const FuncSymbolTerm &f);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual void print(std::ostream &out);
		virtual Value getValue(Grounder *g);
		virtual Value getConstValue(Grounder *g);
 		virtual Term* clone() const;
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual bool unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitution, ValueVector& freeSubstitutions) const;
		virtual ~FuncSymbolTerm();
	protected:
		std::string* name_;
		TermVector*  termList_;
	};
}

#endif

