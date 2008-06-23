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

#include "literaldependencygraph.h"
#include "literal.h"

using namespace NS_GRINGO;

///////////////////////////////////// LiteralNode ///////////////////////////////////////////

LDG::LiteralNode::LiteralNode(Literal *l, bool head) : l_(l), done_(0), in_(0), head_(head)
{
}

///////////////////////////////////// VarNode ///////////////////////////////////////////

LDG::VarNode::VarNode(int var) : done_(false), var_(var)
{
}

///////////////////////////////////// Literaldata ///////////////////////////////////////////

LDG::LiteralData::LiteralData(LiteralNode *n) : n_(n)
{
}

///////////////////////////////////// LiteralDependencyGraph ///////////////////////////////////////////

LDG::LDG()
{
}

bool LDG::hasVarNodes() const
{
	return varNodes_.size() > 0;
}

const VarVector &LDG::getGlobalVars() const
{
	return globalVars_;
}

const VarVector &LDG::getParentVars() const
{
	return parentVars_;
}

void LDG::check(VarVector &free)
{
	// propagate the dependencies
	std::queue<LiteralNode*> bf;
	for(LiteralNodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
		if((*it)->in_ == 0)
			bf.push(*it);
	while(bf.size() > 0)
	{
		LiteralNode *top = bf.front();
		bf.pop();
		for(VarNodeVector::iterator it = top->out_.begin(); it != top->out_.end(); it++)
		{
			VarNode *n = *it;
			if(n->done_)
				continue;
			n->done_ = true;
			for(LiteralNodeVector::iterator jt = n->out_.begin(); jt != n->out_.end(); jt++)
			{
				(*jt)->in_--;
				(*jt)->done_++;
				if((*jt)->in_ == 0)
					bf.push(*jt);
			}
		}
		for(LDGVector::iterator it = top->sub_.begin(); it != top->sub_.end(); it++)
			(*it)->check(free);
	}
	// check if there are still variables left which depend on some literals
	for(VarNodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
	{
		if(!(*it)->done_)
			free.push_back((*it)->var_);
		else
			globalVars_.push_back((*it)->var_);
	}
	if(free.size() == 0)
		for(LiteralNodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
			assert((*it)->in_ == 0);
	// reset the graph
	for(VarNodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
		(*it)->done_ = false;
	for(LiteralNodeVector::iterator i = litNodes_.begin(); i != litNodes_.end(); i++)
	{
		(*i)->in_  += (*i)->done_;
		(*i)->done_ = 0;
	}
}

void LDG::start(LiteralSet &list)
{
	assert(globalVars_.size() == varNodes_.size());
	for(LiteralNodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
	{
		litMap_[(*it)->l_] = new LiteralData(*it);
		if((*it)->in_ == 0 && !(*it)->head_)
			list.insert((*it)->l_);
	}
}

void LDG::propagate(Literal *l, LiteralSet &list)
{
	assert(litMap_.find(l) != litMap_.end());
	LiteralData *data = litMap_[l];
	LiteralNode *top = data->n_;
	for(VarNodeVector::iterator it = top->out_.begin(); it != top->out_.end(); it++)
	{
		VarNode *n = *it;
		data->provided_.push_back(n->var_);
		if(n->done_)
			continue;
		n->done_ = true;
		for(LiteralNodeVector::iterator jt = n->out_.begin(); jt != n->out_.end(); jt++)
		{
			(*jt)->in_--;
			(*jt)->done_++;
			assert(litMap_.find((*jt)->l_) != litMap_.end());
			litMap_[(*jt)->l_]->needed_.push_back(n->var_);
			// dont propagate head nodes since they never provide vars
			// and will confuse the dlvgrounder
			if((*jt)->in_ == 0 && !(*jt)->head_)
				list.insert((*jt)->l_);
		}
	}
	list.erase(l);
}

const VarVector &LDG::getNeededVars(Literal *l) const
{
	return litMap_.find(l)->second->needed_;
}

const VarVector &LDG::getProvidedVars(Literal *l) const
{
	return litMap_.find(l)->second->provided_;
}

LDG::~LDG()
{
	for(LiteralDataMap::iterator it = litMap_.begin(); it != litMap_.end(); it++)
		delete it->second;
	for(VarNodeVector::iterator it = varNodes_.begin(); it != varNodes_.end(); it++)
		delete *it;
	for(LiteralNodeVector::iterator it = litNodes_.begin(); it != litNodes_.end(); it++)
		delete *it;
}

///////////////////////////////////// LiteralDependencyGraphBuilder ///////////////////////////////////////////

LDGBuilder::GraphNode::GraphNode(LDG::LiteralNode *n) : n_(n)
{
}

LDGBuilder::LDGBuilder(LDG *dg) : parent_(0), parentNode_(0), dg_(dg)
{
}

void LDGBuilder::addHead(Literal *l)
{
	head_.push_back(l);
}

void LDGBuilder::addToBody(Literal *l)
{
	body_.push_back(l);
}

void LDGBuilder::addGraph(LDGBuilder *dg)
{
	graphNodes_.back()->sub_.push_back(dg);
}

void LDGBuilder::create()
{
	for(LiteralVector::iterator it = head_.begin(); it != head_.end(); it++)
		(*it)->createNode(this, true);
	for(LiteralVector::iterator it = body_.begin(); it != body_.end(); it++)
		(*it)->createNode(this, false);
	for(GraphNodeVector::iterator i = graphNodes_.begin(); i != graphNodes_.end(); i++)
	{
		for(LDGBuilderVector::iterator j = (*i)->sub_.begin(); j != (*i)->sub_.end(); j++)
			(*j)->createSubGraph(this, (*i)->n_);
	}
}

void LDGBuilder::createSubGraph(LDGBuilder *parent, LDG::LiteralNode *n)
{
	parent_     = parent;
	parentNode_ = n;
	create();
	parentNode_->sub_.push_back(dg_);
}

LDG::LiteralNode *LDGBuilder::createLiteralNode(Literal *l, bool head)
{
	LDG::LiteralNode *n = new LDG::LiteralNode(l, head);
	dg_->litNodes_.push_back(n);
	return n;
}

LDG::VarNode *LDGBuilder::createVarNode(int var)
{
	if(parent_)
	{
		VarNodeMap::iterator it = parent_->varNodes_.find(var);
		if(it != parent_->varNodes_.end())
		{
			if(std::find(dg_->parentVars_.begin(), dg_->parentVars_.end(), var) == dg_->parentVars_.end())
			{
				dg_->parentVars_.push_back(var);
				// if it was a parent node the parent literal simply depends on the var
				// and no other deps are inserted
				parentNode_->in_++;
				it->second->out_.push_back(parentNode_);
			}
			return 0;
		}
	}
	LDG::VarNode *&v = varNodes_[var];
	if(v == 0)
	{
		v = new LDG::VarNode(var);
		dg_->varNodes_.push_back(v);
	}
	return v;
}

void LDGBuilder::createNode(Literal *l, bool head, const VarSet &needed, const VarSet &provided, bool graph)
{
	LDG::LiteralNode *n = createLiteralNode(l, head);
	for(VarSet::iterator it = needed.begin(); it != needed.end(); it++)
	{
		LDG::VarNode *v = createVarNode(*it);
		if(v)
		{
			n->in_++;
			v->out_.push_back(n);
		}		
	}
	for(VarSet::iterator it = provided.begin(); it != provided.end(); it++)
	{
		LDG::VarNode *v = createVarNode(*it);
		if(v)
			n->out_.push_back(v);
	}
	if(graph)
		graphNodes_.push_back(new GraphNode(n));
}

LDGBuilder::~LDGBuilder()
{
	for(GraphNodeVector::iterator i = graphNodes_.begin(); i != graphNodes_.end(); i++)
	{
		for(LDGBuilderVector::iterator j = (*i)->sub_.begin(); j != (*i)->sub_.end(); j++)
			delete *j;
		delete *i;
	}
}


