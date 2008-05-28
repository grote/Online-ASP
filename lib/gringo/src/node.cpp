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

void Node::setSolved(bool solved)
{
	solved_ = solved;
	// when a domain is solved all of its entries are facts
	if(solved_)
	{
		facts_.clear();
		ValueVectorSet facts;
		std::swap(facts, facts_);
	}
}

void Node::evaluate()
{
	setSolved(complete() && (scc_->getType() == SCC::FACT  || scc_->getType() == SCC::BASIC));
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
	// fact programs dont need to store their facts seperatly cause 
	// directly after grounding all values in their domain are facts
	// and there cant be any cycles through fact programs
	// examples:
	// a(1).
	// a(X) :- b(X)
	// b(X) :- a(X)
	// here a(1) is a fact but the rest is a basic program and a/1 is
	// not yet complete
	if(scc_->getType() != SCC::FACT)
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

bool Node::hasFacts()
{
	return facts_.size() > 0;
}

ValueVectorSet &Node::getDomain()
{
	return domain_;
}

int Node::getUid() const
{
	return uid_;
}

Node::~Node() 
{
}

