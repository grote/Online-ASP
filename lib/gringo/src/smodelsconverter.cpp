#include "smodelsconverter.h"
#include "grounder.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

SmodelsConverter::SmodelsConverter(std::ostream *out) : Output(out)
{
}

void SmodelsConverter::initialize(SignatureVector *pred)
{
	Output::initialize(pred);
	// should get 1 here
	false_  = newUid();
}

int SmodelsConverter::getFalse() const
{
	return false_;
}

void SmodelsConverter::handleHead(Object *o)
{
	if(dynamic_cast<Atom*>(o))
	{
		Atom *head = static_cast<Atom*>(o);
		addAtom(head);
		assert(head->getUid() > 0);
		printBasicRule(head->getUid(), pos_, neg_);
	}
	else if(dynamic_cast<Aggregate*>(o))
	{
		Aggregate *head = static_cast<Aggregate*>(o);
		printHead(head);
		for(ObjectVector::iterator it = head->lits_.begin(); it != head->lits_.end(); it++)
		{
			assert(dynamic_cast<Atom*>(*it));
			addAtom(static_cast<Atom*>(*it));
			if((*it)->getUid() > 0)
				head_.push_back((*it)->getUid());
		}
		if(head_.size() > 0)
			printChoiceRule(head_, pos_, neg_);
	}
	else if(dynamic_cast<Disjunction*>(o))
	{
		Disjunction *head = static_cast<Disjunction*>(o);
		for(ObjectVector::iterator it = head->lits_.begin(); it != head->lits_.end(); it++)
		{
			assert(dynamic_cast<Atom*>(*it));
			addAtom(static_cast<Atom*>(*it));
			assert((*it)->getUid() > 0);
			head_.push_back((*it)->getUid());
		}
		assert(head_.size() > 0);
		printDisjunctiveRule(head_, pos_, neg_);
	}
	else
	{
		assert(false);
	}
}

void SmodelsConverter::handleBody(ObjectVector &body)
{
	for(ObjectVector::iterator it = body.begin(); it != body.end(); it++)
	{
		if(dynamic_cast<Atom*>(*it))
		{
			Atom *a = static_cast<Atom*>(*it);
			addAtom(a);
			int uid = a->getUid();
			if(uid > 0)
				pos_.push_back(uid);
			else
				neg_.push_back(-uid);
		}
		else if(dynamic_cast<Aggregate*>(*it))
		{
			printBody(static_cast<Aggregate*>(*it));
		}
		else
		{
			assert(false);
		}
	}
}

void SmodelsConverter::printHead(Aggregate *a)
{
	negA_.clear();
	posA_.clear();
	wNegA_.clear();
	wPosA_.clear();
	switch(a->type_)
	{
		case Aggregate::COUNT:
		{
			int l, u;
			handleCount(a, l, u);
			if(l > 0)
			{
				neg_.push_back(l);
				printBasicRule(false_, pos_, neg_);
				neg_.pop_back();
			}
			if(u > 0)
			{
				pos_.push_back(u);
				printBasicRule(false_, pos_, neg_);
				pos_.pop_back();
			}
			break;
		}
		case Aggregate::SUM:
		{
			int l, u;
			handleSum(a, l, u);
			if(l > 0)
			{
				neg_.push_back(l);
				printBasicRule(false_, pos_, neg_);
				neg_.pop_back();
			}
			if(u > 0)
			{
				pos_.push_back(u);
				printBasicRule(false_, pos_, neg_);
				pos_.pop_back();
			}
			break;
		}
		case Aggregate::MAX:
		case Aggregate::MIN:
			assert(false);
	}
}

void SmodelsConverter::handleAggregate(ObjectVector &lits)
{
	for(ObjectVector::iterator it = lits.begin(); it != lits.end(); it++)
	{
		assert(dynamic_cast<Atom*>(*it));
		Atom *a = static_cast<Atom*>(*it);
		addAtom(a);
		int uid = a->getUid();
		if(uid > 0)
			posA_.push_back(uid);
		else
			negA_.push_back(-uid);
	}
}

void SmodelsConverter::handleAggregate(ObjectVector &lits, IntVector &weights)
{
	IntVector::iterator wIt = weights.begin();
	for(ObjectVector::iterator it = lits.begin(); it != lits.end(); it++, wIt++)
	{
		assert(dynamic_cast<Atom*>(*it));
		Atom *a = static_cast<Atom*>(*it);
		addAtom(a);
		int uid = a->getUid();
		if(uid > 0)
		{
			posA_.push_back(uid);
			wPosA_.push_back(*wIt);
		}
		else
		{
			negA_.push_back(-uid);
			wNegA_.push_back(*wIt);
		}
	}
}

void SmodelsConverter::handleCount(Aggregate *a, int &l, int &u)
{
	switch(a->bounds_)
	{
		case Aggregate::LU:
		{
			handleAggregate(a->lits_);
			l = newUid();
			u = newUid();
			printConstraintRule(l, a->lower_, posA_, negA_);
			printConstraintRule(u, a->upper_ + 1, posA_, negA_);
			break;
		}
		case Aggregate::L:
		{
			handleAggregate(a->lits_);
			l = newUid();
			u = 0;
			printConstraintRule(l, a->lower_, posA_, negA_);
			break;
		}
		case Aggregate::U:
		{
			handleAggregate(a->lits_);
			l = 0;
			u = newUid();
			printConstraintRule(u, a->upper_ + 1, posA_, negA_);
			break;
		}
		case Aggregate::N:
		{
			l = 0;
			u = 0;
			break;
		}
	}
}

void SmodelsConverter::handleSum(Aggregate *a, int &l, int &u)
{
	switch(a->bounds_)
	{
		case Aggregate::LU:
		{
			handleAggregate(a->lits_, a->weights_);
			l = newUid();
			u = newUid();
			printWeightRule(l, a->lower_, posA_, negA_, wPosA_, wNegA_);
			printWeightRule(u, a->upper_ + 1, posA_, negA_, wPosA_, wNegA_);
			break;
		}
		case Aggregate::L:
		{
			handleAggregate(a->lits_, a->weights_);
			l = newUid();
			u = 0;
			printWeightRule(l, a->lower_, posA_, negA_, wPosA_, wNegA_);
			break;
		}
		case Aggregate::U:
		{
			handleAggregate(a->lits_, a->weights_);
			l = 0;
			u = newUid();
			printWeightRule(u, a->upper_ + 1, posA_, negA_, wPosA_, wNegA_);
			break;
		}
		case Aggregate::N:
		{
			l = 0;
			u = 0;
			break;
		}
	}
}

void SmodelsConverter::printBody(Aggregate *a)
{
	negA_.clear();
	posA_.clear();
	wNegA_.clear();
	wPosA_.clear();
	switch(a->type_)
	{
		case Aggregate::COUNT:
		{
			int l, u;
			handleCount(a, l, u);
			if(l > 0)
				pos_.push_back(l);
			if(u > 0)
				neg_.push_back(u);
			break;
		}
		case Aggregate::SUM:
		{
			int l, u;
			handleSum(a, l, u);
			if(l > 0)
				pos_.push_back(l);
			if(u > 0)
				neg_.push_back(u);
			break;
		}
		case Aggregate::MAX:
		case Aggregate::MIN:
			assert(false);
	}
}

void SmodelsConverter::print(Fact *r)
{
	handleHead(r->head_);
}

void SmodelsConverter::print(Rule *r)
{
	assert(dynamic_cast<Conjunction*>(r->body_));
	handleBody(static_cast<Conjunction*>(r->body_)->lits_);
	handleHead(r->head_);
}

void SmodelsConverter::print(Integrity *r)
{
	handleBody(static_cast<Conjunction*>(r->body_)->lits_);
	printBasicRule(false_, pos_, neg_);
}

void SmodelsConverter::print(Optimize *r)
{
	int inv = r->type_ == Optimize::MINIMIZE ? 1 : -1;
	IntVector::iterator wIt = r->weights_.begin();
	for(ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++, wIt++)
	{
		assert(dynamic_cast<Atom*>(*it));
		Atom *a = static_cast<Atom*>(*it);
		addAtom(a);
		int uid = inv * a->getUid();
		if(uid > 0)
		{
			pos_.push_back(uid);
			wPos_.push_back(*wIt);
		}
		else
		{
			neg_.push_back(-uid);
			wNeg_.push_back(*wIt);
		}
	}
	printMinimizeRule(pos_, neg_, wPos_, wNeg_);
}

void SmodelsConverter::print(Compute *r)
{
	for(ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
	{
		assert(dynamic_cast<Atom*>(*it));
		Atom *a = static_cast<Atom*>(*it);
		int uid = a->getUid();
		if(uid > 0)
			pos_.push_back(uid);
		else
			neg_.push_back(-uid);
	}
	printComputeRule(r->models_, pos_, neg_);
}

void SmodelsConverter::print(Object *r)
{
	pos_.clear();
	neg_.clear();
	head_.clear();
	wPos_.clear();
	wNeg_.clear();
	if(dynamic_cast<Fact*>(r))
	{
		print(static_cast<Fact*>(r));
	}
	else if(dynamic_cast<Integrity*>(r))
	{
		print(static_cast<Integrity*>(r));
	}
	else if(dynamic_cast<Rule*>(r))
	{
		print(static_cast<Rule*>(r));
	}
	else if(dynamic_cast<Optimize*>(r))
	{
		print(static_cast<Optimize*>(r));
	}
	else if(dynamic_cast<Compute*>(r))
	{
		print(static_cast<Compute*>(r));
	}
	else
	{
		assert(false);
	}
}

SmodelsConverter::~SmodelsConverter()
{
}

