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

#ifndef VARIABLE_H
#define VARIABLE_H

#include <gringo.h>
#include <term.h>
#include <value.h>

namespace NS_GRINGO
{
	class Variable : public Term
	{
	public:
		Variable(Grounder *g, std::string *id);
		Variable(const Variable &c);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual Value getConstValue(Grounder *g);
		virtual Value getValue(Grounder *g);
		int getUID();
		virtual Term* clone() const;
		bool unify(const Value& t, const VarVector& vars, ValueVector& vals) const;
 		virtual ~Variable();
	protected:
		std::string *id_;
		int uid_;
	};
}

#endif

