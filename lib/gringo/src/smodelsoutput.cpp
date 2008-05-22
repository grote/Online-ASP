#include "smodelsoutput.h"
#include "grounder.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

SmodelsOutput::SmodelsOutput(std::ostream *out) : Output(out)
{
}

void SmodelsOutput::initialize(Grounder *g)
{
	Output::initialize(g);
	// should get 1 here
	false_  = newUid();
	models_ = 1;
}

void SmodelsOutput::print(NS_OUTPUT::Fact *r)
{
	int head;
	if(dynamic_cast<NS_OUTPUT::Aggregate*>(r->head_))
	{
		head = newUid();
		printHead(head, static_cast<NS_OUTPUT::Aggregate*>(r->head_));
	}
	else
		head = r->head_->getUid();
	*out_ << 1 << " " << head << " " << 0 << " " << 0 << std::endl;
}

void SmodelsOutput::printBody(NS_OUTPUT::Aggregate *r)
{
	// aggregates in bodies with 2 bounds are split:
	//   l { M } u
	// = l { M }, not { M } u + 1
	// = nl, not nu
	// = nl :- l { M }
	// = nu :- { M } u + 1
	int nl, nu;
	switch(r->bounds_)
	{
		case NS_OUTPUT::Aggregate::LU:
			nl = newUid();
			nu = newUid();
			*out_ << 1 << " " << r->getUid() << " " << 2 << " " << 1 << " " << nu << " " << nl << std::endl;
			printWeightRule(nl, r->lower_, r->lits_, r->weights_);
			printWeightRule(nu, r->upper_ + 1, r->lits_, r->weights_);
			break;
		case NS_OUTPUT::Aggregate::L:
			nl = newUid();
			nu = 0;
			*out_ << 1 << " " << r->getUid() << " " << 1 << " " << 0 << " " << nl << std::endl;
			printWeightRule(nl, r->lower_, r->lits_, r->weights_);
			break;
		case NS_OUTPUT::Aggregate::U:
			nl = 0;
			nu = newUid();
			*out_ << 1 << " " << r->getUid() << " " << 1 << " " << 1 << " " << nu << std::endl;
			printWeightRule(nu, r->upper_ + 1, r->lits_, r->weights_);
			break;
		case NS_OUTPUT::Aggregate::N:
			nl = 0;
			nu = 0;
			*out_ << 1 << " " << r->getUid() << " " << 0 << " " << 0 << std::endl;
			break;
	}
}

void SmodelsOutput::printWeightRule(int head, int bound, NS_OUTPUT::ObjectVector &lits, IntVector &weights)
{
	int neg = 0;
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++)
		if((*it)->getUid() < 0)
			neg++;
	*out_ << 5 << " " << head << " " << bound << " " << lits.size() << " " << neg;
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++)
		if((*it)->getUid() < 0)
			*out_ << " " << -(*it)->getUid();
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++)
		if((*it)->getUid() > 0)
			*out_ << " " << (*it)->getUid();
	IntVector::iterator itW = weights.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++, itW++)
		if((*it)->getUid() < 0)
			*out_ << " " << *itW;
	itW = weights.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++, itW++)
		if((*it)->getUid() > 0)
			*out_ << " " << *itW;
	*out_ << std::endl;
}

void SmodelsOutput::printHead(int B, NS_OUTPUT::Aggregate *r)
{
	// aggregates in head:
	//   l { M } u :- B.
	// = { M } :- B, not not l { M } u.
	//    :- not l { M } u, B.
	
	// TODO: atm i take the first transformation even if i have to introduce more symbols here
	// = { M } :- B, not nu
	//   nl :- l { M }
	//   nu :- { M } u + 1
	//   nu :- not nl
	//    :- nu, B.
	// could be optimized if no lower and now upper bound is present

	printBody(r);
	int nn = newUid();
	*out_ << 1 << " " << nn << " " << 1 << " " << 1 << " " << r->uid_ << std::endl;
	*out_ << 1 << " " << false_ << " " << 2 << " " << 1 << " " << r->uid_ << " " << B << std::endl;
	
	if(r->lits_.size() == 0)
		return;

	*out_ << 3 << " " << r->lits_.size();
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
		*out_ << " " << (*it)->getUid();
	*out_ << " " << 2 << " " << 1 << " " << nn << " " << B << std::endl;
}

void SmodelsOutput::printBody(int headId, NS_OUTPUT::ObjectVector &body)
{
	int neg = 0;
	for(NS_OUTPUT::ObjectVector::iterator it = body.begin(); it != body.end(); it++)
	{
		if((*it)->getUid() < 0)
			neg++;
		else if(dynamic_cast<NS_OUTPUT::Aggregate*>(*it))
			printBody(static_cast<NS_OUTPUT::Aggregate*>(*it));
	}
	*out_ << 1 << " " << headId << " " << body.size() << " " << neg;
	for(NS_OUTPUT::ObjectVector::iterator it = body.begin(); it != body.end(); it++)
		if((*it)->getUid() < 0)
			*out_ << " " << -(*it)->getUid();
	for(NS_OUTPUT::ObjectVector::iterator it = body.begin(); it != body.end(); it++)
		if((*it)->getUid() > 0)
			*out_ << " " << (*it)->getUid();
	*out_ << std::endl;
}

void SmodelsOutput::print(NS_OUTPUT::Rule *r)
{
	int head;
	if(dynamic_cast<NS_OUTPUT::Aggregate*>(r->head_))
	{
		head = newUid();
		printHead(head, static_cast<NS_OUTPUT::Aggregate*>(r->head_));
	}
	else
		head = r->head_->getUid();
	assert(dynamic_cast<NS_OUTPUT::Conjunction*>(r->body_));
	printBody(head, static_cast<NS_OUTPUT::Conjunction*>(r->body_)->lits_);
}

void SmodelsOutput::print(NS_OUTPUT::Integrity *r)
{
	assert(dynamic_cast<NS_OUTPUT::Conjunction*>(r->body_));
	printBody(false_, static_cast<NS_OUTPUT::Conjunction*>(r->body_)->lits_);
}

void SmodelsOutput::print(NS_OUTPUT::Compute *r)
{
	models_ = r->models_;
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
		compute_.push_back((*it)->getUid());
}

void SmodelsOutput::print(NS_OUTPUT::Optimize *r)
{
	int inv = r->type_ == NS_OUTPUT::Optimize::MAXIMIZE ? -1 : 1;
	int neg = 0;
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
		if((*it)->getUid() * inv < 0)
			neg++;
	*out_ << 6 << " " << 0 << " " << r->lits_.size() << " " << neg;
	for(NS_OUTPUT::ObjectVector::iterator it =  r->lits_.begin(); it != r->lits_.end(); it++)
		if((*it)->getUid() * inv < 0)
			*out_ << " " << -(*it)->getUid() * inv;
	for(NS_OUTPUT::ObjectVector::iterator it =  r->lits_.begin(); it != r->lits_.end(); it++)
		if((*it)->getUid() * inv > 0)
			*out_ << " " << (*it)->getUid() * inv;
	IntVector::iterator itW = r->weights_.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++, itW++)
		if((*it)->getUid() * inv < 0)
			*out_ << " " << *itW;
	itW = r->weights_.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++, itW++)
		if((*it)->getUid() * inv > 0)
			*out_ << " " << *itW;
	*out_ << std::endl;
}

void SmodelsOutput::print(NS_OUTPUT::Object *r)
{
	if(dynamic_cast<NS_OUTPUT::Fact*>(r))
	{
		print(static_cast<NS_OUTPUT::Fact*>(r));
	}
	else if(dynamic_cast<NS_OUTPUT::Integrity*>(r))
	{
		print(static_cast<NS_OUTPUT::Integrity*>(r));
	}
	else if(dynamic_cast<NS_OUTPUT::Rule*>(r))
	{
		print(static_cast<NS_OUTPUT::Rule*>(r));
	}
	else if(dynamic_cast<NS_OUTPUT::Optimize*>(r))
	{
		print(static_cast<NS_OUTPUT::Optimize*>(r));
	}
	else if(dynamic_cast<NS_OUTPUT::Compute*>(r))
	{
		print(static_cast<NS_OUTPUT::Compute*>(r));
	}
	else
	{
		// TODO: do sth here!
		assert(false);
	}
}

void SmodelsOutput::finalize()
{
	*out_ << 0 << std::endl;
	int uid = 0;
	for(AtomLookUp::iterator it = atoms_.begin(); it != atoms_.end(); it++, uid++)
		if(g_->isVisible(uid))
			for(AtomHash::iterator atom = it->begin(); atom != it->end(); atom++)
			{
				*out_ << atom->second << " " << atomToString(uid, atom->first) << std::endl;
			}
	*out_ << 0 << std::endl;
	*out_ << "B+" << std::endl;
	// compute +
	for(IntVector::iterator it = compute_.begin(); it != compute_.end(); it++)
		if(*it > 0)
			*out_ << *it << std::endl;
	*out_ << 0 << std::endl;
	*out_ << "B-" << std::endl;
	// compute -
	*out_ << false_ << std::endl;
	for(IntVector::iterator it = compute_.begin(); it != compute_.end(); it++)
		if(*it < 0)
			*out_ << -(*it) << std::endl;
	*out_ << 0 << std::endl;
	// number of models
	*out_ << models_ << std::endl;
}

SmodelsOutput::~SmodelsOutput()
{
}

