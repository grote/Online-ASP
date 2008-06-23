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

#ifndef NODE_H
#define NODE_H

#include <gringo.h>
#include <value.h>

namespace NS_GRINGO
{
	class Node
	{
		friend class DependencyGraph;
	public:
		Node(Statement *rule = 0);
		NodeVector *getDependency();
		NodeVector *getNegDependency();
		Statement* getStatement();
		void addDependency(Node *n, bool neg = false);
		bool complete();
		bool solved();
		void setSolved(bool solved);
		void reset();
		void finish();
		void evaluate();
		bool hasFacts();
		void addFact(const ValueVector &values);
		bool isFact(const ValueVector &values);
		bool inDomain(const ValueVector &values);
		void addDomain(const ValueVector &values);
		void removeDomain(const ValueVector &values);
		ValueVectorSet &getDomain();
		virtual ~Node();
	protected:
		int defines_;
		int lowlink_;
		int index_;
		bool stacked_;
		bool done_;
		SCC *scc_;
		Statement  *rule_;
		NodeVector dependency_;
		NodeVector negDependency_;
		ValueVectorSet facts_;
		ValueVectorSet domain_;
		bool solved_;
	};
}

#endif

