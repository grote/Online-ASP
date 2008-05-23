#include "literaldependencygraph.h"
#include "literal.h"

using namespace NS_GRINGO;

LiteralDependencyGraph::LiteralDependencyGraph(Literal *head, LiteralVector *body)
{
	// TODO: here could be used a similar datastructure like in basic brograms cause
	// checking the dependencies between literals and variables is in principle the same
	// as propagating in basic programs
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
			node->out_.insert(ln);
			ln->in_.insert(node);
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
				node->out_.insert(ln);
				ln->in_.insert(node);
			}
			for(VarSet::iterator it = provided.begin(); it != provided.end(); it++)
			{
				Node *&node = varMap_[*it];
				if(node == 0)
				{
					node = new Node(*it);
					varNodes_.push_back(node);
				}
				ln->out_.insert(node);
				node->in_.insert(ln);
			}
		}
	}
}

void LiteralDependencyGraph::getUnboundVars(VarVector &free)
{
	// TODO: this could be done without sets but i think this is no speed critical code

	// propagate the dependencies
	std::queue<Node*> bf;
	for(NodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
		if((*it)->in_.size() == 0)
			bf.push(*it);
	while(bf.size() > 0)
	{
		Node *top = bf.front();
		bf.pop();
		while(top->out_.size() > 0)
		{
			Node *n = *top->out_.begin();
			for(NodeSet::iterator j = n->in_.begin(); j != n->in_.end(); j++)
				(*j)->out_.erase(n);
			n->in_.clear();
			bf.push(n);
		}

	}
	// check if there are still variables left which depend on some literals
	for(NodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
	{
		assert((*it)->var_ > 0);
		if((*it)->out_.size() > 0)
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

