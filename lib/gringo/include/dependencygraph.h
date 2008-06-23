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

#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#include <gringo.h>
#include <printable.h>

namespace NS_GRINGO
{
	class DependencyGraph
	{
	public:
		DependencyGraph();
		Node *createStatementNode(Statement *r, bool preserveOrder = false);
		Node *createPredicateNode(PredicateLiteral *pred);
		NodeVector &getPredNodes();
		void calcSCCs();
		bool check(Grounder *g);
		virtual ~DependencyGraph();
	protected:
		void calcSCCDep(Node *v1, SCC *scc, bool &root);
		void tarjan(Node *v1, std::stack<Node*> &stack, int &index);
	protected:
		int uids_;
		NodeVector ruleNodes_;
		NodeVector predicateNodes_;
		SCCVector sccs_;
		SCCSet sccRoots_;
		Node *last_;
	};
}

#endif

