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

#ifndef BINDERSPLITTER_H
#define BINDERSPLITTER_H

#include <gringo.h>
#include <literal.h>
#include <groundable.h>
#include <expandable.h>
#include <value.h>

namespace NS_GRINGO
{
	class BinderSplitter : public Literal
	{
	public:
		BinderSplitter(Domain *domain, TermVector *param, const VarVector &relevant);
		virtual Literal* clone() const;
		virtual SDGNode *createNode(SDG *dg, SDGNode *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void createNode(StatementChecker *dg, bool head, bool delayed);
		virtual void print(std::ostream &out);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual void reset();
		virtual bool solved();
		virtual bool isFact(Grounder *g);
		virtual void finish();
		virtual void evaluate();
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual bool match(Grounder *g);
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual NS_OUTPUT::Object *convert();
		virtual double heuristicValue();
		virtual ~BinderSplitter();
	protected:
		Domain *domain_;
		TermVector *param_;
		VarVector relevant_;
	};
}

#endif
