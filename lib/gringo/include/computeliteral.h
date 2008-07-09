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

#ifndef COMPUTELITERAL_H
#define COMPUTELITERAL_H

#include <gringo.h>
#include <literal.h>
#include <expandable.h>

namespace NS_GRINGO
{
	class ComputeLiteral : public Literal, public Expandable
	{
	public:
		ComputeLiteral(ConditionalLiteralVector *literals, int number);
		ComputeLiteral(const ComputeLiteral &a);
		virtual SDGNode *createNode(SDG *dg, SDGNode *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		void ground(Grounder *g, GroundStep step);
		virtual bool match(Grounder *g);
		virtual void reset();
		virtual void finish();
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual void appendLiteral(Literal *l, ExpansionType type);
		virtual double heuristicValue();
		virtual bool isFact();
		virtual bool solved();
		virtual Literal *clone() const;
		virtual NS_OUTPUT::Object *convert();
		virtual void print(std::ostream &out);
		virtual ~ComputeLiteral();
	protected:
		int number_;
		ConditionalLiteralVector *literals_;
	};
}

#endif

