#include "node.h"
#include "scc.h"
#include "value.h"

using namespace NS_GRINGO;

Node::Node(int uid, Statement *rule) : uid_(uid), defines_(0), lowlink_(-1), index_(-1), stacked_(false), done_(false), scc_(0), rule_(rule), solved_(false)
{
}

NodeVector *Node::getNegDependency()
{
	return &negDependency_;
}

NodeVector *Node::getDependency()
{
	return &dependency_;
}

Statement* Node::getStatement() 
{ 
	return rule_; 
}

void Node::addDependency(Node *n, bool neg)
{
	assert(n);
	if(neg)
		negDependency_.push_back(n);
	dependency_.push_back(n);
}

bool Node::complete()
{
	return defines_ == 0;
}

bool Node::solved()
{
	return solved_;
}

void Node::evaluate()
{
	solved_ = complete() && (scc_->getType() == SCC::FACT  || scc_->getType() == SCC::BASIC);
}

void Node::reset()
{
	defines_++;
	solved_ = false;
}

void Node::finish()
{
	defines_--;
}

bool Node::isFact(const ValueVector &values)
{
	return facts_.find(values) != facts_.end();
}

void Node::addFact(const ValueVector &values)
{
	facts_.insert(values);
}

bool Node::inDomain(const ValueVector &values)
{
	return domain_.find(values) != domain_.end();
}

void Node::removeDomain(const ValueVector &values)
{
	domain_.erase(values);
}

void Node::addDomain(const ValueVector &values)
{
	domain_.insert(values);
}

ValueVectorSet &Node::getDomain()
{
	return domain_;
}

int Node::getUid()
{
	return uid_;
}

Node::~Node() 
{
}

