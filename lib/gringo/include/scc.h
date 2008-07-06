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

#ifndef SCC_H
#define SCC_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class SCC : public Printable
	{
		friend class DependencyGraph;
	public:
		enum SCCType {FACT=0, BASIC=1, NORMAL=2};
	public:
		SCC();
		void print(std::ostream &out);
		Evaluator *getEvaluator();
		bool check(Grounder *g);
		StatementVector *getStatements();
		SCCType getType();
		virtual ~SCC();
	protected:
		SCCType type_;
		int edges_;
		StatementVector rules_;
		SCCSet sccs_;
		Evaluator *eval_;
	};
}

#endif
