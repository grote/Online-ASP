#include "output.h"
#include "node.h"
#include "grounder.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;
		
Output::Output(std::ostream *out) : uids_(1), out_(out), g_(0)
{
	
}

void Output::initialize(Grounder *g)
{
	g_ = g;
	atoms_.resize(g->getPred()->size());
}

bool Output::addAtom(NS_OUTPUT::Atom *r)
{
	// TODO: is this a good or a bad place for this method
	int id = r->node_->getUid();
	AtomHash::iterator res = atoms_[id].find(r->values_);
	if(res == atoms_[id].end())
	{
		const std::string &name = (*g_->getPred())[id].first;
		std::stringstream ss;
		ss << name;
		if(r->values_.size() > 0)
		{
			ValueVector::iterator it = r->values_.begin();
			ss << "(" << *it;
			for(it++; it != r->values_.end(); it++)
				ss << "," << *it;
			ss << ")";
		}

		res = atoms_[id].insert(std::make_pair(r->values_, std::make_pair(ss.str(), newUid()))).first;
		r->name_    = &res->second.first;
		r->uid_     = res->second.second;
		r->visible_ = false;
		r->visible_ = g_->isVisible(id);
		return true;
	}
	else
	{
		r->name_    = &res->second.first;
		r->uid_     =  res->second.second;
		r->visible_ =  false;
		return false;
	}
}

int Output::newUid()
{
	return uids_++;
}

Output::~Output()
{
}

// =============== NS_OUTPUT::Object ===============
Object::Object(int type) : neg_(false), type_(type) 
{
}

Object::Object(bool neg, int type) : neg_(neg), type_(type) 
{
}

int Object::getUid()
{
	return neg_ ? -uid_ : uid_;
}

Object::~Object() 
{
}

// =============== NS_OUTPUT::Atom ===============
Atom::Atom(bool neg, Node *node, ValueVector &values) : Object(neg, 0x4), node_(node)
{
	std::swap(values_, values);
}

void Atom::addDomain(bool fact)
{
	if(fact)
		node_->addFact(values_);
	node_->addDomain(values_);
}

void Atom::addUid(Output *o)
{
	print_ = o->addAtom(this);
}

void Atom::print_plain(std::ostream &out)
{
	out << (neg_ ? "not " : "") << *name_;
}

void Atom::print(std::ostream &out)
{
	if(print_)
		out << type_ << " " << uid_ << " " << *name_ << " " << (visible_ ? "1 0" : "0") << std::endl;
}
	
// =============== NS_OUTPUT::Rule ===============
Rule::Rule(Object* head, Object *body) : Object(0x5), head_(head), body_(body) 
{
}

void Rule::print_plain(std::ostream &out)
{
	head_->print_plain(out);
	out << " :- ";
	body_->print_plain(out);
	out << "." << std::endl;
}

void Rule::print(std::ostream &out)
{
	head_->print(out);
	body_->print(out);
	out << type_ << " " << uid_ << " " << head_->getUid() << " " << body_->getUid() << std::endl;
}

Rule::~Rule()
{
	delete head_;
	delete body_;
}

void Rule::addDomain(bool fact)
{
	head_->addDomain(false);
}

void Rule::addUid(Output *o)
{
	uid_ = 0;
	head_->addUid(o);
	body_->addUid(o);
}

// =============== NS_OUTPUT::Fact ===============
Fact::Fact(Object *head) : Object(0x6), head_(head) 
{
}

void Fact::print_plain(std::ostream &out)
{
	head_->print_plain(out);
	out << "." << std::endl;
}

void Fact::print(std::ostream &out)
{
	head_->print(out);
	out << type_ << " " << uid_ << " " << head_->getUid() << std::endl;
}

Fact::~Fact()
{
	delete head_;
}

void Fact::addDomain(bool fact)
{
	head_->addDomain(fact);
}

void Fact::addUid(Output *o)
{
	uid_ = 0;
	head_->addUid(o);
}

// =============== NS_OUTPUT::Integrity ===============
Integrity::Integrity(Object *body) : Object(0x5), body_(body) 
{
}

void Integrity::print_plain(std::ostream &out)
{
	out << " :- ";
	body_->print_plain(out);
	out << "." << std::endl;

}

void Integrity::print(std::ostream &out)
{
	body_->print(out);
	out << type_ << " " << uid_ << " " << body_->getUid() << std::endl;
}

void Integrity::addDomain(bool fact)
{
}

void Integrity::addUid(Output *o)
{
	uid_ = 0;
	body_->addUid(o);
}

Integrity::~Integrity()
{
	delete body_;
}

// =============== NS_OUTPUT::Conjunction ===============
Conjunction::Conjunction(ObjectVector &lits) : Object(0x8) 
{
	std::swap(lits, lits_);
}
void Conjunction::print_plain(std::ostream &out)
{
	bool comma = false;
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(out);
	}
}

void Conjunction::print(std::ostream &out)
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(out);
	out << type_ << " " << uid_ << " " << lits_.size();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		out << " " << (*it)->getUid();
	out << std::endl;
}
Conjunction::~Conjunction()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}
void Conjunction::addDomain(bool fact)
{
}

void Conjunction::addUid(Output *o)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addUid(o);
}

// =============== NS_OUTPUT::Aggregate ===============
Aggregate::Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights, int upper) : 
	Object(neg, type), bounds_(LU), lower_(lower), upper_(upper)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights) : 
	Object(neg, type), bounds_(L), lower_(lower), upper_(0)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights, int upper) : 
	Object(neg, type), bounds_(U), lower_(0), upper_(upper)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights) : 
	Object(neg, type), bounds_(N), lower_(0), upper_(0)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

void Aggregate::print_plain(std::ostream &out)
{
	if(neg_)
		out << "not ";
	if(bounds_ == L || bounds_ == LU)
		std::cout << lower_;
	bool comma = false;
	switch(type_)
	{
		case SUM:
			out << " [";
			break;
		case COUNT:
			out << " {";
			break;
		case MAX:
			out << " max {";
			break;
		case MIN:
			out << " min {";
			break;
		case TIMES:
			out << " times {";
			break;
	}
	IntVector::iterator itWeight = weights_.begin();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++, itWeight++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(out);
		if(type_ != COUNT)
		{
			out << " = ";
			out << *itWeight;
		}
	}
	if(type_ == SUM)
		out << "] ";
	else
		out << "} ";
	if(bounds_ == U || bounds_ == LU)
		std::cout << upper_;
}

void Aggregate::print(std::ostream &out)
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(out);
	out << type_ << " " << uid_ << " " << bounds_;
	switch(bounds_)
	{
		case LU:
			out << "" << lower_ << " " << upper_;
			break;
		case L:
			out << "" << lower_;
			break;
		case U:
			out << "" << upper_;
			break;
		case N:
			break;
	}
	out << " " << lits_.size();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		out << " " << (*it)->getUid();
	std::cout << " " << weights_.size();
	for(IntVector::iterator it = weights_.begin(); it != weights_.end(); it++)
		out << " " << *it;
	out << std::endl;
}

void Aggregate::addDomain(bool fact)
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addDomain(false);
}

void Aggregate::addUid(Output *o)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addUid(o);
}

Aggregate::~Aggregate()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

// =============== NS_OUTPUT::Compute ===============
Compute::Compute(ObjectVector &lits, int models) : Object(0x42), models_(models)
{
	std::swap(lits, lits_);
}

void Compute::print_plain(std::ostream &out)
{
	out << "compute " << models_ << " { ";
	bool comma = false;
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(out);
	}
	out << " }." << std::endl;
}

void Compute::print(std::ostream &out)
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(out);
	out << type_ << " " << uid_ << " " << lits_.size();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		out << " " << (*it)->getUid();
	out << std::endl;
}

Compute::~Compute()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

void Compute::addDomain(bool fact)
{
}

void Compute::addUid(Output *o)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addUid(o);
}
		
// =============== NS_OUTPUT::Optimize ===============
Optimize::Optimize(Type type, ObjectVector &lits, IntVector &weights) : Object(type)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

void Optimize::print_plain(std::ostream &out)
{
	switch(type_)
	{
		case MINIMIZE:
			out << "minimize [ ";
			break;
		case MAXIMIZE:
			out << "maximize [ ";
			break;
	}
	bool comma = false;
	IntVector::iterator itWeights = weights_.begin();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++, itWeights++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(out);
		out << " = " << *itWeights;
	}
	out << " ]." << std::endl;
}

void Optimize::print(std::ostream &out)
{
}

Optimize::~Optimize()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

void Optimize::addDomain(bool fact)
{
}

void Optimize::addUid(Output *o)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addUid(o);
}

