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

#ifndef PROGRAMDEPENDENCYGRAPH_H
#define PROGRAMDEPENDENCYGRAPH_H

#include <gringo.h>

namespace NS_GRINGO
{
	class PDG
	{
		friend class PDGBuilder;
	private:
		struct VarNode;
		struct StatementNode;

		typedef std::vector<VarNode*> VarNodeVector;
		typedef std::vector<VarNodeVector> DomainVector;
	private:
		struct VarNode
		{
			int uid_;
			int in_;
			VarNodeVector out_;
			StatementNode *n_;
		};

		struct StatementNode
		{
			int open_;
			VarNodeVector nodes_;
			Statement *s_;
		};
	public:
		class DomainNodes
		{
		public:
			DomainNodes(int doms);
		private:
			DomainVector doms_;
		};
	public:
		PDG(Program *p, DomainNodes &doms);
		bool check();
	private:
		Program *p_;
		int pos_;
		DomainVector &doms_;
		std::vector<VarNode> pdgNodes_;
		std::vector<StatementNode> statementNodes_;
	};

	class PDGBuilder
	{
	private:
		struct Delay;
		typedef std::vector<Delay> DelayVector;

		struct Delay
		{
			Literal *l;
			bool defining;
		};
	public:
		PDGBuilder(PDG *dg, Statement *s);
		void add(Literal *l, bool defining, bool delay = false);
		void createNode(int domain, const VarSet &provided);
		void createNode(const VarSet &needed, const VarSet &provided);
	private:
		PDG *dg_;
		PDGBuilder *parent_;
		DelayVector delayed_;
	};
}

#endif

