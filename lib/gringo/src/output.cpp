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
		r->uid_     = res->second;
		return true;
	}
	else
	{
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
Object::Object() : neg_(false) 
{
}

Object::Object(bool neg) : neg_(neg) 
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
Atom::Atom(bool neg, Node *node, int predUid, const ValueVector &values) : Object(neg), node_(node), predUid_(predUid), values_(values)
{
}

Atom::Atom(bool neg, int predUid, const ValueVector &values) : Object(neg), node_(0), predUid_(predUid), values_(values)
{
}

Atom::Atom(bool neg, int predUid) : Object(neg), node_(0), predUid_(predUid)
{
}

void Atom::addDomain(bool fact)
{
	if(fact)
		node_->addFact(values_);
	node_->addDomain(values_);
}

void Atom::print_plain(Output *o, std::ostream &out)
{
	out << (neg_ ? "not " : "") << o->atomToString(predUid_, values_);
}

void Atom::print(Output *o, std::ostream &out)
{
	if(o->addAtom(this))
		out << 0x4 << " " << uid_ << " " << o->atomToString(predUid_, values_) << " " << (o->isVisible(predUid_) ? "1 0" : "0") << NL;
}
	
// =============== NS_OUTPUT::Rule ===============
Rule::Rule(Object* head, Object *body) : head_(head), body_(body) 
{
}

void Rule::print_plain(Output *o, std::ostream &out)
{
	head_->print_plain(o, out);
	out << " :- ";
	body_->print_plain(o, out);
	out << "." << NL;
}

void Rule::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	head_->print(o, out);
	body_->print(o, out);
	out << 0x5 << " " << uid_ << " " << head_->getUid() << " " << body_->getUid() << NL;
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

// =============== NS_OUTPUT::Fact ===============
Fact::Fact(Object *head) : head_(head) 
{
}

void Fact::print_plain(Output *o, std::ostream &out)
{
	head_->print_plain(o, out);
	out << "." << NL;
}

void Fact::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	head_->print(o, out);
	out << 0x6 << " " << uid_ << " " << head_->getUid() << NL;
}

Fact::~Fact()
{
	delete head_;
}

void Fact::addDomain(bool fact)
{
	head_->addDomain(fact);
}

// =============== NS_OUTPUT::Integrity ===============
Integrity::Integrity(Object *body) : body_(body) 
{
}

void Integrity::print_plain(Output *o, std::ostream &out)
{
	out << " :- ";
	body_->print_plain(o, out);
	out << "." << NL;

}

void Integrity::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	body_->print(o, out);
	out << 0x7 << " " << uid_ << " " << body_->getUid() << NL;
}

void Integrity::addDomain(bool fact)
{
}

Integrity::~Integrity()
{
	delete body_;
}

// =============== NS_OUTPUT::Conjunction ===============
Conjunction::Conjunction() 
{
}

Conjunction::Conjunction(ObjectVector &lits) 
{
	std::swap(lits, lits_);
}

void Conjunction::print_plain(Output *o, std::ostream &out)
{
	bool comma = false;
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(o, out);
	}
}

void Conjunction::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(o, out);
	out << 0x8 << " " << uid_ << " " << lits_.size();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		out << " " << (*it)->getUid();
	out << NL;
}

void Conjunction::addDomain(bool fact)
{
}

Conjunction::~Conjunction()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

// =============== NS_OUTPUT::Disjunction ===============
Disjunction::Disjunction() 
{
}

Disjunction::Disjunction(ObjectVector &lits) 
{
	std::swap(lits, lits_);
}

void Disjunction::print_plain(Output *o, std::ostream &out)
{
	bool comma = false;
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
	{
		if(comma)
			out << " | ";
		else
			comma = true;
		(*it)->print_plain(o, out);
	}
}

void Disjunction::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(o, out);
	out << 0x13 << " " << uid_ << " " << lits_.size();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		out << " " << (*it)->getUid();
	out << NL;
}

void Disjunction::addDomain(bool fact)
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addDomain(false);
}

Disjunction::~Disjunction()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

// =============== NS_OUTPUT::Aggregate ===============
Aggregate::Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights, int upper) : 
	Object(neg), type_(type), bounds_(LU), lower_(lower), upper_(upper)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, int lower, ObjectVector lits, IntVector weights) : 
	Object(neg), type_(type), bounds_(L), lower_(lower), upper_(0)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights, int upper) : 
	Object(neg), type_(type), bounds_(U), lower_(0), upper_(upper)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type, ObjectVector lits, IntVector weights) : 
	Object(neg), type_(type), bounds_(N), lower_(0), upper_(0)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

Aggregate::Aggregate(bool neg, Type type) : Object(neg), type_(type), bounds_(N), lower_(0), upper_(0)
{
}

void Aggregate::print_plain(Output *o, std::ostream &out)
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
			out << " max [";
			break;
		case MIN:
			out << " min [";
			break;
		case TIMES:
			out << " times [";
			break;
	}
	IntVector::iterator itWeight = weights_.begin();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++, itWeight++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(o, out);
		if(type_ != COUNT)
		{
			out << " = ";
			out << *itWeight;
		}
	}
	if(type_ == COUNT)
		out << "} ";
	else
		out << "] ";
	if(bounds_ == U || bounds_ == LU)
		out << upper_;
}

void Aggregate::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(o, out);
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
	if(type_ != COUNT)
	{
		out << " " << weights_.size();
		for(IntVector::iterator it = weights_.begin(); it != weights_.end(); it++)
			out << " " << *it;
	}
	out << NL;
}

void Aggregate::addDomain(bool fact)
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->addDomain(false);
}

Aggregate::~Aggregate()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

// =============== NS_OUTPUT::Compute ===============
Compute::Compute(ObjectVector &lits, int models) : models_(models)
{
	std::swap(lits, lits_);
}

void Compute::print_plain(Output *o, std::ostream &out)
{
	out << "compute " << models_ << " { ";
	bool comma = false;
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		(*it)->print_plain(o, out);
	}
	out << " }." << NL;
}

void Compute::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		(*it)->print(o, out);
	out << 0x42 << " " << uid_ << " " << lits_.size();
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		out << " " << (*it)->getUid();
	out << NL;
}

Compute::~Compute()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

void Compute::addDomain(bool fact)
{
}

// =============== NS_OUTPUT::Optimize ===============
Optimize::Optimize(Type type, ObjectVector &lits, IntVector &weights) : type_(type)
{
	std::swap(lits, lits_);
	std::swap(weights, weights_);
}

void Optimize::print_plain(Output *o, std::ostream &out)
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
		(*it)->print_plain(o, out);
		out << " = " << *itWeights;
	}
	out << " ]." << NL;
}

void Optimize::print(Output *o, std::ostream &out)
{
	uid_ = o->newUid();
	// TODO:
}

Optimize::~Optimize()
{
	for(ObjectVector::iterator it = lits_.begin(); it != lits_.end(); it++)
		delete *it;
}

void Optimize::addDomain(bool fact)
{
}

