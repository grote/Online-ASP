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

#ifndef AGGREGATELITERAL_H
#define AGGREGATELITERAL_H

#include <gringo.h>
#include <literal.h>
#include <expandable.h>

namespace NS_GRINGO
{
	class AggregateLiteral : public Literal, public Expandable
	{
	public:
		AggregateLiteral(ConditionalLiteralVector *literals);
		AggregateLiteral(const AggregateLiteral &a);
		virtual SDGNode *createNode(SDG *dg, SDGNode *prev, DependencyAdd todo);
		virtual void createNode(LDGBuilder *dg, bool head);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		void ground(Grounder *g, GroundStep step);
		virtual bool match(Grounder *g);
		virtual void match(Grounder *g, int &lower, int &upper, int &fixed) = 0;
		virtual void reset();
		virtual void finish();
		virtual void preprocess(Grounder *g, Expandable *e);
		virtual IndexedDomain *createIndexedDomain(VarSet &index);
		virtual void appendLiteral(Literal *l, ExpansionType type);
		virtual double heuristicValue();
		virtual void setBounds(Term *lower, Term *upper);
		virtual void setEqual(Constant *equal);
		virtual void setEqual(int bound);
		virtual bool checkBounds(int lower, int upper);
		/**
		 * \brief This function returns if the aggregate is a fact
		 * This function will/may only return true if all the literals of
		 * the aggregate are facts (or if they dont match all). If there is
		 * at least one unsolved literal the function has to return false.
		 */
		virtual bool isFact();
		virtual bool solved();
		ConditionalLiteralVector *getLiterals() const;
		Term *getLower() const;
		Term *getUpper() const;
		virtual ~AggregateLiteral();
	protected:
		ConditionalLiteralVector *literals_;
		Term *lower_;
		Term *upper_;
		Constant *equal_;
		bool fact_;
	public:
		int lowerBound_;
		int upperBound_;
		int minLowerBound_;
		int maxUpperBound_;
	};
}

#endif
