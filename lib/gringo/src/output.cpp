#include "output.h"
#include "node.h"
#include "grounder.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;
		
Output::Output(std::ostream *out) : uids_(1), out_(out), pred_(0), hideAll_(false)
{
	
}

void Output::initialize(SignatureVector *pred)
{
	pred_ = pred;
	visible_.reserve(pred_->size());
	for(SignatureVector::const_iterator it = pred_->begin(); it != pred_->end(); it++)
		visible_.push_back(isVisible(it->first, it->second));
	atoms_.resize(pred_->size());
}

std::string Output::atomToString(int id, const ValueVector &values) const
{
	const std::string *name = (*pred_)[id].first;
	std::stringstream ss;
	ss << *name;
	if(values.size() > 0)
	{
		ValueVector::const_iterator it = values.begin();
		ss << "(" << *it;
		for(it++; it != values.end(); it++)
			ss << "," << *it;
		ss << ")";
	}
	return ss.str();
}

bool Output::addAtom(NS_OUTPUT::Atom *r)
{
	int id = r->predUid_;
	AtomHash::iterator res = atoms_[id].find(r->values_);
	if(res == atoms_[id].end())
	{
		res = atoms_[id].insert(std::make_pair(r->values_, newUid())).first;
		r->output_  = this;
		r->uid_     = res->second;
		return true;
	}
	else
	{
		r->output_  = this;
		r->uid_     = res->second;
		return false;
	}
}

int Output::newUid()
{
	return uids_++;
}

void Output::hideAll()
{
	hideAll_ = true;
}

void Output::setVisible(std::string *id, int arity, bool visible)
{
	hide_[std::make_pair(id, arity)] = !visible;
}

bool Output::isVisible(int uid)
{
	return visible_[uid];
}

bool Output::isVisible(std::string *id, int arity)
{
	std::map<Signature, bool>::iterator it = hide_.find(std::make_pair(id, arity));
	if(it == hide_.end())
		return !hideAll_;
	else
		return !it->second;
}

void Output::addSignature()
{
	visible_.push_back(isVisible(pred_->back().first, pred_->back().second));
	atoms_.push_back(AtomHash());
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
Atom::Atom(bool neg, Node *node, int predUid, ValueVector &values) : Object(neg, 0x4), node_(node), predUid_(predUid)
{
	std::swap(values_, values);
}

Atom::Atom(bool neg, int predUid, ValueVector &values) : Object(neg, 0x4), node_(0), predUid_(predUid)
{
	std::swap(values_, values);
}

Atom::Atom(bool neg, int predUid) : Object(neg, 0x4), node_(0), predUid_(predUid)
{
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
	out << (neg_ ? "not " : "") << output_->atomToString(predUid_, values_);
}

void Atom::print(std::ostream &out)
{
	if(print_)
		out << type_ << " " << uid_ << " " << output_->atomToString(predUid_, values_) << " " << (output_->isVisible(predUid_) ? "1 0" : "0") << std::endl;
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
Conjunction::Conjunction() : Object(0x8) 
{
}

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
	assert(type_ != DISJUNCTION);
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights) : 
	Object(neg, type), bounds_(L), lower_(lower), upper_(0)
{
	assert(type_ != DISJUNCTION);
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights, int upper) : 
	Object(neg, type), bounds_(U), lower_(0), upper_(upper)
{
	assert(type_ != DISJUNCTION);
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights) : 
	Object(neg, type), bounds_(N), lower_(0), upper_(0)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type) : Object(neg, type), bounds_(N), lower_(0), upper_(0)
{
}

void Aggregate::print_plain(std::ostream &out)
{
	if(neg_)
		out << "not ";
	if(bounds_ == L || bounds_ == LU)
		out << lower_;
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
		case DISJUNCTION:
			break;
	}
	IntVector::iterator itWeight = weights_.begin();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++, itWeight++)
	{
		if(comma)
			out << (type_ == DISJUNCTION ? " | " : ", ");
		else
			comma = true;
		(*it)->print_plain(out);
		if(type_ != COUNT && type_ != DISJUNCTION)
		{
			out << " = ";
			out << *itWeight;
		}
	}
	if(type_ != DISJUNCTION)
	{
		if(type_ == SUM)
			out << "] ";
		else
			out << "} ";
	}
	if(bounds_ == U || bounds_ == LU)
		out << upper_;
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
	if(type_ != COUNT && type_ != DISJUNCTION)
	{
		out << " " << weights_.size();
		for(IntVector::iterator it = weights_.begin(); it != weights_.end(); it++)
			out << " " << *it;
	}
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

