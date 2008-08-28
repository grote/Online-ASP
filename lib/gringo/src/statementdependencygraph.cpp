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

#include "statementdependencygraph.h"
#include "predicateliteral.h"
#include "domain.h"
#include "grounder.h"
#include "program.h"

using namespace NS_GRINGO;
		
// ==================== SDG::SCC ============================
SDG::SCC::SCC() : type_(FACT), edges_(0)
{
}

// ======================= SDG ==============================

SDG::SDG() : last_(0)
{
}

SDGNode *SDG::createStatementNode(Statement *r, bool preserveOrder)
{
	SDGNode *n = new SDGNode(r);
	ruleNodes_.push_back(n);
	if(preserveOrder)
	{
		if(last_)
			n->addDependency(last_);
		last_ = n;
	}
	return n;
}

SDGNode *SDG::createPredicateNode(PredicateLiteral *pred)
{
	if((int)predicateNodes_.size() < pred->getUid() + 1)
		predicateNodes_.resize(pred->getUid() + 1);
	SDGNode *&n = predicateNodes_[pred->getUid()];
	if(!n)
		n = new SDGNode(pred->getDomain());
	return n;
}

namespace
{
	struct Call
	{
		Call(SDGNode *v, SDGNodeVector::iterator w) : v(v), root(true), w(w)
		{
		}

		SDGNode *v;
		bool root;
		SDGNodeVector::iterator w;
	};
}

void SDG::tarjan(SDGNode *v, int &index, int &back, std::vector<SDGNode*> &stack)
{
	std::vector<Call> callstack;
	callstack.push_back(Call(v, v->getDependency()->begin()));
	v->index_ = index++;
	while(!callstack.empty())
	{
Start:
		Call &c = callstack.back();
		v = c.v;
		for(; c.w != c.v->getDependency()->end(); c.w++)
		{
			SDGNode *w = *c.w;
			if(w->index_ == 0)
			{
				callstack.push_back(Call(w, w->getDependency()->begin()));
				w->index_ = index++;
				goto Start;
			}
			if(w->index_ < v->index_)
			{
				v->index_ = (*c.w)->index_;
				c.root = false;
			}
		}
		if(c.root)
		{
			SCC *scc = new SCC();
			int nodes = 1;
			v->scc_ = scc;
			if(v->getStatement())
				scc->rules_.push_back(v->getStatement());
			index--;
			while(!stack.empty() && v->index_ <= stack.back()->index_)
			{
				stack.back()->index_ = back;
				stack.back()->scc_ = scc;
				if(stack.back()->getStatement())
					scc->rules_.push_back(stack.back()->getStatement());
				stack.pop_back();
				index--;
				nodes++;
			}
			v->index_ = back;
			back--;
			// initialize with fact or basic program
			scc->type_ = nodes == 1 ? SCC::FACT : SCC::BASIC;
			//scc->type_ = nodes == 1 ? SCC::FACT : SCC::NORMAL;
			sccs_.push_back(scc);
			bool root = true;
			// calc type and dependency of program
			calcSCCDep(v, scc, root);
			if(root)
				sccRoots_.insert(scc);
		}
		else
		{
			stack.push_back(v);
		}
		callstack.pop_back();
	}
}

void SDG::calcSCCDep(SDGNode *v1, SCC *scc, bool &root)
{
	// do a depth first search limited to the scc to build a tree of sccs
	v1->done_ = true;
	SDGNodeVector *dep = v1->getDependency();
	// build tree of sccs
	for(SDGNodeVector::iterator it = dep->begin(); it != dep->end(); it++)
	{
		SDGNode *v2 = *it;
		assert(v2->scc_);
		if(v2->scc_ == scc && !v2->done_)
			calcSCCDep(v2, scc, root);
		if(v2->scc_ != scc)
		{
			if(v2->scc_->type_ == SCC::NORMAL)
				scc->type_ = SCC::NORMAL;
			if(v2->scc_->sccs_.insert(scc).second)
				scc->edges_++;
			root = false;
		}
	}
	// try to find neg dep in scc
	if(scc->type_ != SCC::NORMAL)
	{
		SDGNodeVector *negDep = v1->getNegDependency();
		for(SDGNodeVector::iterator it = negDep->begin(); it != negDep->end(); it++)
		{
			SDGNode *v2 = *it;
			if(v2->scc_ == scc)
			{
				scc->type_ = SCC::NORMAL;
				break;
			}
		}
	}
}

void SDG::calcSCCs(Grounder *g)
{
	int index = 1, back = ruleNodes_.size() + predicateNodes_.size() - 1;
	std::vector<SDGNode*> stack;
	/*
	std::cout << "rule nodes: " << ruleNodes_.size() << std::endl;
	std::cout << "pred nodes: " << predicateNodes_.size() << std::endl;
	for(SDGNodeVector::iterator i = ruleNodes_.begin(); i != ruleNodes_.end(); i++)
	{
		std::cout << *i << " =>";
		for(SDGNodeVector::iterator j = (*i)->getDependency()->begin(); j != (*i)->getDependency()->end(); j++)
			std::cout << " " << *j;
		std::cout << std::endl;
	}
	for(SDGNodeVector::iterator i = predicateNodes_.begin(); i != predicateNodes_.end(); i++)
	{
		std::cout << *i << " =>";
		for(SDGNodeVector::iterator j = (*i)->getDependency()->begin(); j != (*i)->getDependency()->end(); j++)
			std::cout << " " << *j;
		std::cout << std::endl;
	}
	*/
	for(SDGNodeVector::iterator it = ruleNodes_.begin(); it != ruleNodes_.end(); it++)
	{
		SDGNode *v = *it;
		if(v->index_ == 0)
			tarjan(v, index, back, stack);
	}
	for(SDGNodeVector::iterator it = predicateNodes_.begin(); it != predicateNodes_.end(); it++)
	{
		SDGNode *v = *it;
		if(v->index_ == 0)
			tarjan(v, index, back, stack);
		// set the type of the domain
		v->getDomain()->setType(static_cast<Domain::Type>(v->scc_->type_));
		if(v->dependency_.size() == 0)
			g->addZeroDomain(v->getDomain());
	}

	// do a topological sort
	std::queue<SCC*> bf;
	for(SCCSet::iterator it = sccRoots_.begin(); it != sccRoots_.end(); it++)
	{
		SCC *scc = *it;
		bf.push(scc);
	}
	while(!bf.empty())
	{
		SCC *top = bf.front();
		assert(top->edges_ == 0);
		bf.pop();
		// if there is something to ground add it to the grounder
		if(top->rules_.size() > 0)
		{
			Program *p = new Program(static_cast<Program::Type>(top->type_), top->rules_);
			//std::cout << pp(g, p) << std::endl;
			g->addProgram(p);
		}
		for(SCCSet::iterator it = top->sccs_.begin(); it != top->sccs_.end(); it++)
		{
			SCC *scc = *it;
			scc->edges_--;
			if(scc->edges_ == 0)
				bf.push(scc);
		}
	}
}

SDG::~SDG()
{
	for(SCCVector::iterator it = sccs_.begin(); it != sccs_.end(); it++)
		delete *it;
	for(SDGNodeVector::iterator it = ruleNodes_.begin(); it != ruleNodes_.end(); it++)
		delete *it;
	for(SDGNodeVector::iterator it = predicateNodes_.begin(); it != predicateNodes_.end(); it++)
		delete *it;
}

// =================================== SDGNode ===========================================

SDGNode::SDGNode(Domain *domain) : index_(0), type_(PREDICATENODE), done_(0), scc_(0), dom_(domain)
{
}

SDGNode::SDGNode(Statement *rule) : index_(0), type_(STATEMENTNODE), done_(0), scc_(0), rule_(rule)
{
}

SDGNodeVector *SDGNode::getNegDependency() const
{
	return const_cast<SDGNodeVector *>(&negDependency_);
}

SDGNodeVector *SDGNode::getDependency() const
{
	return const_cast<SDGNodeVector *>(&dependency_);
}

Domain* SDGNode::getDomain() const
{
	if(type_ == PREDICATENODE)
		return dom_; 
	else
		return 0;
}

Statement* SDGNode::getStatement() const
{ 
	if(type_ == STATEMENTNODE)
		return rule_; 
	else
		return 0;
}

SDGNode::Type SDGNode::getType() const
{
	return static_cast<SDGNode::Type>(type_);
}

void SDGNode::addDependency(SDGNode *n, bool neg)
{
	assert(n);
	if(neg)
		negDependency_.push_back(n);
	dependency_.push_back(n);
}

SDGNode::~SDGNode() 
{
}

