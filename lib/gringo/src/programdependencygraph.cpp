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

#include "programdependencygraph.h"
#include "literal.h"
#include "domain.h"
#include "grounder.h"
#include "statement.h"

using namespace NS_GRINGO;

//////////////////////////// PDG::VarNodeCmp /////////////////////////////////
bool PDG::VarNodeCmp::operator() (const VarNode *a, const VarNode *b) const
{
	return a->uid_ < b->uid_;
}

///////////////////////////// PDG::VarNode ///////////////////////////////////
PDG::VarNode::VarNode(int uid, StatementNode *s) : uid_(uid), in_(0), complete_(false), s_(s)
{
}

/////////////////////////// PDG::DomainNode ///////////////////////////////
bool PDG::DomainNode::complete()
{
	return defines_ == 0;
}

/////////////////////////// PDG::StatementNode ///////////////////////////////
PDG::StatementNode::StatementNode(Statement *s) : open_(0), s_(s)
{
}

/////////////////////////// PDG::DomainNodes /////////////////////////////////
PDG::DomainNodes::DomainNodes(DomainVector &doms) : doms_(doms.size() + 1)
{
	DomainNodeVector::iterator i = doms_.begin();
	for(DomainVector::iterator j = doms.begin(); j != doms.end(); i++, j++)
	{
		i->defines_ = (*j)->getDefines();
		i->queued_  = false;
	}
	i->defines_ = 0;
	i->queued_  = false;
	
}

///////////////////////////////////// PDG ////////////////////////////////////
PDG::PDG(Program *p, DomainNodes &doms) : p_(p), pos_(0), doms_(doms.doms_)
{
}

bool PDG::check(Grounder *g)
{
	while(queue_.size() > 0)
	{
		int dom = queue_.back();
		doms_[dom].queued_ = false;
		queue_.pop_back();
		for(VarNodeVector::iterator i = doms_[dom].nodes_.begin(); i != doms_[dom].nodes_.end(); i++)
		{
			propagate(*i);
			StatementNode *s = (*i)->s_;
			if(s->open_ == 0)
			{
				for(IntVector::iterator j = s->heads_.begin(); j != s->heads_.end(); j++)
					if(--doms_[*j].defines_ == 0)
						enqueue(*j);
				// TODO: inform program about rule
				//dg_->p->...
				pos_++;
			}
		}
	}

	if(pos_ != (int)statementNodes_.size())
	{
		for(std::vector<StatementNode>::iterator i = statementNodes_.begin(); i != statementNodes_.end(); i++)
		{
			if(i->open_ > 0)
			{
				std::cerr << "the following rule cannot be grounded, ";
				std::cerr << "weakly restricted varibles: { ";
				bool comma = false;
				for(std::vector<VarNode>::iterator j = i->nodes_.begin(); j != i->nodes_.end(); j++)
				{
					if(comma)
						std::cerr << ", ";
					else
						comma = true;
					std::cerr << *g->getVarString(j->uid_);
				}
				std::cerr << " }" << std::endl;
				std::cerr << "	" << i->s_ << std::endl;
				break;
			}
		}
		return false;
	}
	return true;
}

void PDG::propagate(VarNode *n)
{
	if(!n->complete_)
	{
		n->complete_ = true;
		n->s_->open_--;
		for(VarNodeVector::iterator i = n->out_.begin(); i != n->out_.end(); i++)
			if(--((*i)->in_) == 0)
				propagate(*i);
	}
}

void PDG::enqueue(int dom)
{
	if(!doms_[dom].queued_)
	{
		queue_.push_back(dom);
		doms_[dom].queued_ = true;
	}
}

///////////////////////////// PDGBuilder::Delay //////////////////////////////
PDGBuilder::Delay::Delay(Literal *l, bool head, bool defining) : l_(l), head_(head), defining_(defining)
{
}

//////////////////////////////// PDGBuilder //////////////////////////////////
PDGBuilder::PDGBuilder(PDGBuilder *parent) : dg_(parent->dg_), d_(0), parent_(parent)
{
}

PDGBuilder::PDGBuilder(PDG *dg, Statement *s) : dg_(dg), d_(0), parent_(0)
{
	dg_->statementNodes_.push_back(PDG::StatementNode(s));
	dg_->statementNodes_.back().nodes_.push_back(PDG::VarNode(0, &dg_->statementNodes_.back()));
}

void PDGBuilder::add(Literal *l, bool head, bool defining)
{
	l->createNode(this, head, defining, false);
}

int PDGBuilder::getSentinelDomain()
{
	return dg_->doms_.size() - 1;
}

void PDGBuilder::createDelayedNode(Literal *l, bool head, bool defining, const VarSet &needed, const VarSet &provided)
{
	delayed_.push_back(Delay(l, head, defining));
	d_ = &delayed_.back();
	for(VarSet::iterator it = provided.begin(); it != provided.end(); it++)
	{
		std::pair<PDG::VarNode*, bool> res = createVarNode(*it);
		if(res.second)
			d_->provided_.push_back(res.first);
	}
	for(VarSet::iterator it = needed.begin(); it != needed.end(); it++)
		createVarNode(*it);
	d_ = 0;
}

void PDGBuilder::createHeadNode(int domain)
{
	dg_->statementNodes_.back().heads_.push_back(domain);
}

void PDGBuilder::createNode(int domain, const VarSet &provided)
{
	bool inserted = false;
	for(VarSet::iterator it = provided.begin(); it != provided.end(); it++)
	{
		std::pair<PDG::VarNode*, bool> res = createVarNode(*it);
		if(res.second)
			dg_->doms_[domain].nodes_.push_back(res.first);
	}
	if(inserted && dg_->doms_[domain].complete())
		dg_->enqueue(domain);
}

void PDGBuilder::createNode(const VarSet &needed, const VarSet &provided)
{
	std::vector<PDG::VarNode*> n;
	for(VarSet::iterator i = needed.begin(); i != needed.end(); i++)
		n.push_back(createVarNode(*i).first);
	for(VarSet::iterator i = provided.begin(); i != provided.end(); i++)
	{
		std::pair<PDG::VarNode*, bool> res = createVarNode(*i);
		if(res.second)
		{
			PDG::VarNode *p = res.first;
			if(n.size() > 0)
			{
				for(std::vector<PDG::VarNode*>::iterator j = n.begin(); j != n.end(); j++)
				{
					(*j)->out_.push_back(p);
					p->in_++;
				}
			}
			else
			{
				// if the the provided vars depend on no other vars
				// the provided vars will be propagated by the sentinel domain
				// which is always complete
				dg_->enqueue(getSentinelDomain());
				dg_->doms_[getSentinelDomain()].nodes_.push_back(p);
			}
		}
	}
}

void PDGBuilder::finalize()
{
	for(DelayVector::iterator it = delayed_.begin(); it != delayed_.end(); it++)
	{
		d_ = &(*it);
		d_->l_->createNode(this, d_->head_, d_->defining_, true);
	}
	if(!parent_)
		dg_->statementNodes_.back().nodes_.pop_back();
	// TODO: optimization for rules that are already ground
	if(!dg_->statementNodes_.back().nodes_.size() == 0)
	{
		assert(false);
		//dg_->statementNodes_.pop_back();
		//dg_->p->...
	}
}

PDG::VarNode* PDGBuilder::addParentDep(PDG::VarNode* n)
{
	if(!parent_ || !parent_->d_)
		return n;
	for(PDG::VarNodeVector::iterator i = parent_->d_->provided_.begin(); i != parent_->d_->provided_.end(); i++)
	{
		n->out_.push_back(*i);
		(*i)->in_++;
	}
	return n;
}

std::pair<PDG::VarNode*, bool> PDGBuilder::createVarNode(int var)
{
	PDG::VarNode *back = &dg_->statementNodes_.back().nodes_.back();
	back->uid_ = var;
	if(parent_)
	{
		VarNodeSet::iterator i = parent_->varNodes_.find(back);
		if(i != parent_->varNodes_.end())
			return std::make_pair(addParentDep(*i), false);
	}
	std::pair<VarNodeSet::iterator, bool> res = varNodes_.insert(back);
	if(res.second)
		dg_->statementNodes_.back().nodes_.push_back(PDG::VarNode(0, &dg_->statementNodes_.back()));
	return std::make_pair(addParentDep(*res.first), true);
}

