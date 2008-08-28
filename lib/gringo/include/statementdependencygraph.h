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
	class SDG
	{
		friend class SDGNode;
	private:
		class SCC;
		typedef std::vector<SCC*> SCCVector;
		typedef std::set<SCC*> SCCSet;

		class SCC
		{
		public:
			enum Type {FACT=0, BASIC=1, NORMAL=2};
		public:
			SCC();
		public:
			Type type_;
			int edges_;
			StatementVector rules_;
			SCCSet sccs_;
		};
	public:
		SDG();
		SDGNode *createStatementNode(Statement *r, bool preserveOrder = false);
		SDGNode *createPredicateNode(PredicateLiteral *pred);
		void calcSCCs(Grounder *g);
		~SDG();
	private:
		void calcSCCDep(SDGNode *v1, SCC *scc, bool &root);
		void tarjan(SDGNode *v, int &index, int &back, std::vector<SDGNode*> &stack);
	private:
		int uids_;
		SDGNodeVector ruleNodes_;
		SDGNodeVector predicateNodes_;
		SCCVector sccs_;
		SCCSet sccRoots_;
		SDGNode *last_;
	};
	
	class SDGNode
	{
		friend class SDG;
	public:
		enum Type {STATEMENTNODE = 0, PREDICATENODE = 1};
	public:
		SDGNode(Statement *rule);
		SDGNode(Domain *dom);
		SDGNodeVector *getDependency() const;
		SDGNodeVector *getNegDependency() const;
		Statement *getStatement() const;
		Domain *getDomain() const;
		Type getType() const;

		void addDependency(SDGNode *n, bool neg = false);
		~SDGNode();
	private:
		int index_;
		unsigned int type_ : 1;
		unsigned int done_ : 1;
		SDG::SCC *scc_;
		union
		{
			Statement  *rule_;
			Domain     *dom_;
		};
		SDGNodeVector dependency_;
		SDGNodeVector negDependency_;
	};
}

#endif

