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

#ifndef WEIGHTEDSTATEMENT_H
#define WEIGHTEDSTATEMENT_H

#include <gringo.h>
#include <statement.h>
#include <expandable.h>

namespace NS_GRINGO
{
	/**
	 * \brief A generic class for statements with weights
	 *
	 * Includes compute, minimize and maximize statements. Weights in compute statements are ignored.
	 */
	class WeightedStatement : public Statement, public Expandable
	{
	public:
		enum Type { COMPUTE, MINIMIZE, MAXIMIZE };
	public:
		/// Constructor
		WeightedStatement(Type type, ConditionalLiteralVector *literals, bool setSemantics, int number = 0);
		virtual void buildDepGraph(DependencyGraph *dg);
		virtual void getVars(VarSet &vars) const;
		virtual bool checkO(LiteralVector &unsolved);
		virtual bool check(VarVector &free);
		virtual void preprocess(Grounder *g);
		virtual void reset();
		virtual void finish();
		virtual void evaluate();
		virtual bool ground(Grounder *g);
		virtual void addDomain(PredicateLiteral *pl);
		virtual void print(std::ostream &out);
		virtual void grounded(Grounder *g);
		virtual void appendLiteral(Literal *l, ExpansionType type);
		/// Destructor
		virtual ~WeightedStatement();
	protected:
		Type type_;
		bool setSemantics_;
		int number_;
		ConditionalLiteralVector literals_;
	};
}

#endif

