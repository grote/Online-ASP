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

#include "node.h"
#include "scc.h"

using namespace NS_GRINGO;

Node::Node(Domain *domain) : lowlink_(-1), index_(-1), type_(PREDICATENODE), stacked_(0), done_(0), scc_(0), dom_(domain)
{
}

Node::Node(Statement *rule) : lowlink_(-1), index_(-1), type_(STATEMENTNODE), stacked_(0), done_(0), scc_(0), rule_(rule)
{
}

NodeVector *Node::getNegDependency() const
{
	return const_cast<NodeVector *>(&negDependency_);
}

NodeVector *Node::getDependency() const
{
	return const_cast<NodeVector *>(&dependency_);
}

Domain* Node::getDomain() const
{
	if(type_ == PREDICATENODE)
		return dom_; 
	else
		return 0;
}

Statement* Node::getStatement() const
{ 
	if(type_ == STATEMENTNODE)
		return rule_; 
	else
		return 0;
}

Node::Type Node::getType() const
{
	return static_cast<Node::Type>(type_);
}

void Node::addDependency(Node *n, bool neg)
{
	assert(n);
	if(neg)
		negDependency_.push_back(n);
	dependency_.push_back(n);
}

Node::~Node() 
{
}

