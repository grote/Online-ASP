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
	private:
		struct DomainNode
		{
			bool complete();

			int defines_;
			bool queued_;
			VarNodeVector nodes_;
		};
		typedef std::vector<DomainNode> DomainNodeVector;

		struct VarNode
		{
			VarNode(int uid_, StatementNode *s);

			int uid_;
			int in_;
			bool complete_;
			VarNodeVector out_;
			StatementNode *s_;
		};

		struct VarNodeCmp
		{
			bool operator() (const VarNode *a, const VarNode *b) const;
		};

		struct StatementNode
		{
			StatementNode(Statement *s_);
			int open_;
			Statement *s_;
			std::vector<VarNode> nodes_;
			IntVector heads_;
		};
	public:
		class DomainNodes
		{
			friend class PDG;
		public:
			DomainNodes(DomainVector &doms);
		private:
			DomainNodeVector doms_;
		};
	public:
		PDG(Program *p, DomainNodes &doms);
		bool check(Grounder *g);
	private:
		void enqueue(int dom);
		void propagate(VarNode *n);
	private:
		Program *p_;
		int pos_;
		DomainNodeVector &doms_;
		std::vector<StatementNode> statementNodes_;
		IntVector queue_;
	};

	class PDGBuilder
	{
	private:
		struct Delay;
		typedef std::vector<Delay> DelayVector;
		typedef std::set<PDG::VarNode*, PDG::VarNodeCmp> VarNodeSet;

		struct Delay
		{
			Delay(Literal *l, bool head, bool defining);
			Literal *l_;
			bool head_;
			bool defining_;
			PDG::VarNodeVector provided_;
		};
	public:
		PDGBuilder(PDGBuilder *parent);
		PDGBuilder(PDG *dg, Statement *s);
		void add(Literal *l, bool head, bool defining);
		
		int getSentinelDomain();
		// The domian the statement provides if all its variables are bound
		void createHeadNode(int domain);
		// Variables needed/provided by a literal that needs loclas grounding
		void createDelayedNode(Literal *l, bool head, bool defining, const VarSet &needed, const VarSet &provided);
		// Variables provided by a domain
		void createNode(int domain, const VarSet &provided);
		// Variables provided by other variables (may be empty)
		void createNode(const VarSet &needed, const VarSet &provided);

		void finalize();
	private:
		PDG::VarNode* addParentDep(PDG::VarNode* n);
		std::pair<PDG::VarNode*, bool> createVarNode(int var);
	private:
		PDG *dg_;
		Delay *d_;
		PDGBuilder *parent_;
		DelayVector delayed_;
		VarNodeSet varNodes_;
	};
}

#endif

