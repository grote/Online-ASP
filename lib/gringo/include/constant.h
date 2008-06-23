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

#ifndef CONSTANT_H
#define CONSTANT_H

#include <gringo.h>
#include <term.h>
#include <value.h>

namespace NS_GRINGO
{
	class Constant : public Term
	{
	public:
		enum ConstantType { ID, VAR, NUM };
	public:
		Constant(int value);
		Constant(ConstantType type, Grounder *g, std::string *value);
		Constant(const Constant &c);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual Value getConstValue();
		virtual Value getValue();
		int getUID();
		virtual Term* clone() const;
		virtual bool unify(const Value& t, const VarVector& boundVariables, const VarVector& freeVariables,
			       	ValueVector& boundSubstitution, ValueVector& freeSubstitutions) const;
 		virtual ~Constant();
	protected:
		ConstantType type_;
		Grounder *g_;
		Value value_;
		int uid_;
	};
}

#endif

