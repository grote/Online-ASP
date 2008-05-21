#include "dependencygraph.h"
#include "predicateliteral.h"
#include "node.h"
#include "grounder.h"
#include "scc.h"

using namespace NS_GRINGO;
		
DependencyGraph::DependencyGraph() : last_(0)
{
}

Node *DependencyGraph::createStatementNode(Statement *r, bool preserveOrder)
{
	Node *n = new Node(0, r);
	ruleNodes_.push_back(n);
	if(preserveOrder)
	{
		if(last_)
			n->addDependency(last_);
		last_ = n;
	}
	return n;
}

Node *DependencyGraph::createPredicateNode(PredicateLiteral *pred)
{
	Node *&n = predicateMap_[std::make_pair(pred->getId(), pred->getArity())];
	if(!n)
	{
		n = new Node(pred_.size());
		pred_.push_back(std::make_pair(pred->getId(), pred->getArity()));
		predicateNodes_.push_back(n);
	}
	return n;
}

Node *DependencyGraph::createPredicateNode()
{
	Node *n = new Node(0);
	predicateNodes_.push_back(n);
	return n;
}

void DependencyGraph::tarjan(Node *v1, std::stack<Node*> &s, int &index)
{
	// do a depth first search to find the sccs
	v1->index_ = index;
	v1->lowlink_ = index;
	index = index + 1;
	s.push(v1);
	v1->stacked_ = true;
	NodeVector *dep = v1->getDependency();
	for(NodeVector::iterator it = dep->begin(); it != dep->end(); it++)
	{
		Node *v2 = *it;
		if(v2->done_)
			continue;
		if(v2->index_ == -1)
		{
			tarjan(v2, s, index);
			v1->lowlink_ = std::min(v1->lowlink_, v2->lowlink_);
		}
		else if (v2->stacked_)
		{
			v1->lowlink_ = std::min(v1->lowlink_, v2->index_);
		}
	}
	if (v1->lowlink_ == v1->index_)
	{
		SCC *scc = new SCC();
		int nodes = 0;
		Node *v2;
		do
		{
			v2 = s.top();
			v2->scc_ = scc;
			if(v2->getStatement())
				scc->rules_.push_back(v2->getStatement());
			s.pop();
			v2->stacked_ = false;
			nodes++;
		}
		while(v1 != v2);
		// initialize with fact or basic program
		// TODO: as long as basic programs are not solved by the grounder
		//       they are assumed to be normal programs and the solver has to :)
		scc->type_ = nodes == 1 ? SCC::FACT : SCC::BASIC;
		//scc->type_ = nodes == 1 ? SCC::FACT : SCC::NORMAL;
		sccs_.push_back(scc);
		bool root = true;
		// calc type and dependency of program
		calcSCCDep(v1, scc, root);
		if(root)
			sccRoots_.insert(scc);
	}
}

void DependencyGraph::calcSCCDep(Node *v1, SCC *scc, bool &root)
{
	// do a depth first search limited to the scc to build a tree of sccs
	v1->done_ = true;
	NodeVector *dep = v1->getDependency();
	// build tree of sccs
	for(NodeVector::iterator it = dep->begin(); it != dep->end(); it++)
	{
		Node *v2 = *it;
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
		NodeVector *negDep = v1->getNegDependency();
		for(NodeVector::iterator it = negDep->begin(); it != negDep->end(); it++)
		{
			Node *v2 = *it;
			if(v2->scc_ == scc)
			{
				scc->type_ = SCC::NORMAL;
				break;
			}
		}
	}
}

bool DependencyGraph::check(Grounder *g)
{
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
		if(!top->check(g))
			return false;
		bf.pop();
		// if there is something to ground add it to the grounder
		if(top->rules_.size() > 0)
			g->addSCC(top);
		for(SCCSet::iterator it = top->sccs_.begin(); it != top->sccs_.end(); it++)
		{
			SCC *scc = *it;
			scc->edges_--;
			if(scc->edges_ == 0)
				bf.push(scc);
		}
	}
	return true;
}

void DependencyGraph::calcSCCs()
{
	for(NodeVector::iterator it = ruleNodes_.begin(); it != ruleNodes_.end(); it++)
	{
		Node *v = *it;
		if(!v->done_)
		{
			int index = 0;
			std::stack<Node*> stack;
			tarjan(v, stack, index);
		}
	}
	for(NodeVector::iterator it = predicateNodes_.begin(); it != predicateNodes_.end(); it++)
	{
		Node *v = *it;
		if(!v->done_)
		{
			int index = 0;
			std::stack<Node*> stack;
			tarjan(v, stack, index);
		}
	}
}

NodeVector &DependencyGraph::getPredNodes()
{
	return predicateNodes_;
}

SignatureVector *DependencyGraph::getPred()
{
	return &pred_;
}

DependencyGraph::~DependencyGraph()
{
	for(SCCVector::iterator it = sccs_.begin(); it != sccs_.end(); it++)
		delete *it;
	for(NodeVector::iterator it = ruleNodes_.begin(); it != ruleNodes_.end(); it++)
		delete *it;
	for(NodeVector::iterator it = predicateNodes_.begin(); it != predicateNodes_.end(); it++)
		delete *it;
}

