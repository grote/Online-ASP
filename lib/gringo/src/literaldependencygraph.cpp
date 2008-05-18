#include "literaldependencygraph.h"
#include "literal.h"

using namespace NS_GRINGO;

LiteralDependencyGraph::LiteralDependencyGraph(Literal *head, LiteralVector *body)
{
	if(head)
	{
		Node *ln = new Node(head);
		litNodes_.push_back(ln);
		VarSet global;
		head->getVars(global, VARS_GLOBAL);
		for(VarSet::iterator it = global.begin(); it != global.end(); it++)
		{
			Node *&node = varMap_[*it];
			if(node == 0)
			{
				node = new Node(*it);
				varNodes_.push_back(node);
			}
			node->dep_.push_back(ln);
			ln->in_++;
		}
	}
	if(body)
	{
		for(LiteralVector::iterator it = body->begin(); it != body->end(); it++)
		{
			Literal *l = *it;
			Node *ln = new Node(l);
			litNodes_.push_back(ln);
			VarSet needed, provided;
			l->getVars(needed, VARS_NEEDED);
			l->getVars(provided, VARS_PROVIDED);
			for(VarSet::iterator it = needed.begin(); it != needed.end(); it++)
			{
				Node *&node = varMap_[*it];
				if(node == 0)
				{
					node = new Node(*it);
					varNodes_.push_back(node);
				}
				node->dep_.push_back(ln);
				ln->in_++;
			}
			for(VarSet::iterator it = provided.begin(); it != provided.end(); it++)
			{
				Node *&node = varMap_[*it];
				if(node == 0)
				{
					node = new Node(*it);
					varNodes_.push_back(node);
				}
				ln->dep_.push_back(node);
				node->in_++;
			}
		}
	}
}

void LiteralDependencyGraph::getUnboundVars(VarVector &free)
{
	for(NodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
	{
		if((*it)->in_ == 0)
			free.push_back((*it)->var_);
	}
	// make a top sort
	std::queue<Node*> bf;
	for(NodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
		if((*it)->in_ == 0)
			bf.push(*it);

	while(bf.size() > 0)
	{
		Node *top = bf.front();
		bf.pop();
		for(NodeVector::iterator it = top->dep_.begin(); it != top->dep_.end(); it++)
		{
			(*it)->in_--;
			if((*it)->in_ == 0)
				bf.push(*it);
		}
	}
	for(NodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
	{
		if((*it)->in_ > 0 && (*it)->var_ > 0)
			free.push_back((*it)->var_);
	}
}

LiteralDependencyGraph::~LiteralDependencyGraph()
{
	for(NodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
		delete *it;
	for(NodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
		delete *it;
}

