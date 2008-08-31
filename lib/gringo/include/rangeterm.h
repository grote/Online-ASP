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

#ifndef RANGETERM_H
#define RANGETERM_H

#include <gringo.h>
#include <term.h>

namespace NS_GRINGO
{
	class RangeTerm : public Term
	{
	public:
		RangeTerm(Term *lower, Term *upper);
		RangeTerm(const RangeTerm &r);
		virtual Term* clone() const;
		virtual void print(const GlobalStorage *g, std::ostream &out) const;
		virtual void getVars(VarSet &vars) const;
		virtual bool isComplex();
		virtual Value getValue(Grounder *g);
		virtual Value getConstValue(Grounder *g);
		virtual void preprocess(Literal *l, Term *&p, Grounder *g, Expandable *e);
		virtual bool unify(const GlobalStorage *g, const Value& t, const VarVector& vars, ValueVector& subst) const
		{
			assert(false);
		}
		Term *getLower();
		Term *getUpper();
		virtual void addIncParam(Grounder *g, Term *&p, const Value &v);
		virtual ~RangeTerm();
	protected:
		Term *lower_;
		Term *upper_;
		
	};
}

#endif

