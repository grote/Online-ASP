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

#ifndef LITERALSTATEMENT_H
#define LITERALSTATEMENT_H

#include <gringo.h>
#include <statement.h>
#include <expandable.h>

namespace NS_GRINGO
{
	/**
	 * \brief A statement that contains exactly one literal
	 *
	 * Includes compute, minimize and maximize statements.
	 */
	class LiteralStatement : public Statement
	{
	public:
		enum Type { COMPUTE, MINIMIZE, MAXIMIZE };
	public:
		/// Constructor
		LiteralStatement(Literal *lit, bool preserveOrder);
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
		/// Destructor
		virtual ~LiteralStatement();
	protected:
		Literal *lit_;
		bool preserveOrder_;
	};
}

#endif
