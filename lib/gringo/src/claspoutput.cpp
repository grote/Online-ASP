#include "claspoutput.h"
#include "grounder.h"
#include <clasp/include/program_builder.h>

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

ClaspOutput::ClaspOutput(Clasp::ProgramBuilder *b) : Output(&std::cout), b_(b)
{
}

void ClaspOutput::initialize(Grounder *g)
{
	Output::initialize(g);
	// should get 1 here
	false_ = newUid();
}

void ClaspOutput::print(NS_OUTPUT::Fact *r)
{
	int head;
	if(dynamic_cast<NS_OUTPUT::Aggregate*>(r->head_))
	{
		head = newUid();
		printHead(head, static_cast<NS_OUTPUT::Aggregate*>(r->head_));
	}
	else
		head = r->head_->getUid();
	//*out_ << 1 << " " << head << " " << 0 << " " << 0 << std::endl;
	b_->startRule();
	b_->addHead(head);
	b_->endRule();
}

void ClaspOutput::printBody(NS_OUTPUT::Aggregate *r)
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
			//*out_ << 1 << " " << r->getUid() << " " << 2 << " " << 1 << " " << nu << " " << nl << std::endl;
			b_->startRule();
			b_->addHead(r->getUid());
			b_->addToBody(nu, false);
			b_->addToBody(nl, true);
			b_->endRule();
			printWeightRule(nl, r->lower_, r->lits_, r->weights_);
			printWeightRule(nu, r->upper_ + 1, r->lits_, r->weights_);
			break;
		case NS_OUTPUT::Aggregate::L:
			nl = newUid();
			nu = 0;
			//*out_ << 1 << " " << r->getUid() << " " << 1 << " " << 0 << " " << nl << std::endl;
			b_->startRule();
			b_->addHead(r->getUid());
			b_->addToBody(nl, true);
			b_->endRule();
			printWeightRule(nl, r->lower_, r->lits_, r->weights_);
			break;
		case NS_OUTPUT::Aggregate::U:
			nl = 0;
			nu = newUid();
			//*out_ << 1 << " " << r->getUid() << " " << 1 << " " << 1 << " " << nu << std::endl;
			b_->startRule();
			b_->addHead(r->getUid());
			b_->addToBody(nu, false);
			b_->endRule();
			printWeightRule(nu, r->upper_ + 1, r->lits_, r->weights_);
			break;
		case NS_OUTPUT::Aggregate::N:
			nl = 0;
			nu = 0;
			//*out_ << 1 << " " << r->getUid() << " " << 0 << " " << 0 << std::endl;
			b_->startRule();
			b_->addHead(r->getUid());
			b_->endRule();
			break;
	}
}

void ClaspOutput::printWeightRule(int head, int bound, NS_OUTPUT::ObjectVector &lits, IntVector &weights)
{
	b_->startRule(Clasp::WEIGHTRULE, bound);
	b_->addHead(head);
	IntVector::iterator wIt = weights.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++, wIt++)
		b_->addToBody(abs((*it)->getUid()), (*it)->getUid() > 0, *wIt);
	b_->endRule();
}

void ClaspOutput::printHead(int B, NS_OUTPUT::Aggregate *r)
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
	//*out_ << 1 << " " << nn << " " << 1 << " " << 1 << " " << r->uid_ << std::endl;
	b_->startRule();
	b_->addHead(nn);
	b_->addToBody(r->uid_, false);
	b_->endRule();
	//*out_ << 1 << " " << false_ << " " << 2 << " " << 1 << " " << r->uid_ << " " << B << std::endl;
	b_->startRule();
	b_->addHead(false_);
	b_->addToBody(B, true);
	b_->addToBody(r->uid_, false);
	b_->endRule();
	
	if(r->lits_.size() == 0)
		return;
	
	b_->startRule(Clasp::CHOICERULE);
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
		b_->addHead((*it)->getUid());
	b_->addToBody(B, true);
	b_->addToBody(nn, false);
	b_->endRule();
}

void ClaspOutput::printBody(int headId, NS_OUTPUT::ObjectVector &body)
{
	int neg = 0;
	for(NS_OUTPUT::ObjectVector::iterator it = body.begin(); it != body.end(); it++)
	{
		if((*it)->getUid() < 0)
			neg++;
		else if(dynamic_cast<NS_OUTPUT::Aggregate*>(*it))
			printBody(static_cast<NS_OUTPUT::Aggregate*>(*it));
	}
	b_->startRule();
	b_->addHead(headId);
	for(NS_OUTPUT::ObjectVector::iterator it = body.begin(); it != body.end(); it++)
		b_->addToBody(abs((*it)->getUid()), (*it)->getUid() > 0);
	b_->endRule();
}

void ClaspOutput::print(NS_OUTPUT::Rule *r)
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

void ClaspOutput::print(NS_OUTPUT::Integrity *r)
{
	assert(dynamic_cast<NS_OUTPUT::Conjunction*>(r->body_));
	printBody(false_, static_cast<NS_OUTPUT::Conjunction*>(r->body_)->lits_);
}

void ClaspOutput::print(NS_OUTPUT::Object *r)
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
	else
	{
		assert(false);
	}
}

void ClaspOutput::finalize()
{
	int uid = 0;
	for(AtomLookUp::iterator it = atoms_.begin(); it != atoms_.end(); it++, uid++)
		if(g_->isVisible(uid))
			for(AtomHash::iterator atom = it->begin(); atom != it->end(); atom++)
				b_->setAtomName(atom->second.second, atom->second.first.c_str());
	b_->setCompute(false_, false);
}

int ClaspOutput::newUid()
{
	return b_->newAtom();
}

ClaspOutput::~ClaspOutput()
{
}

