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

#ifndef RANGELITERAL_H
#define RANGELITERAL_H

#include <gringo.h>
#include <literal.h>

namespace NS_GRINGO
{
	class RangeLiteral : public Literal
	{
	public:
		RangeLiteral(Constant *var, Term *lower, Term *upper);
		RangeLiteral(const RangeLiteral &r);
		virtual SDGNode *createNode(SDG *dg, SDGNode *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void createNode(PDGBuilder *dg, bool head, bool defining, bool delayed);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual void finish();
		virtual bool solved();
		virtual bool isFact();
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual NS_OUTPUT::Object *convert();
		virtual Literal* clone() const;
		virtual void print(std::ostream &out);
		virtual double heuristicValue();
		virtual ~RangeLiteral();
	protected:
		Constant *var_;
		Term *lower_;
		Term *upper_;
	};
}

#endif

