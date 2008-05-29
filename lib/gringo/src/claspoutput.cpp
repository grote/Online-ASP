#include "claspoutput.h"
#include "grounder.h"
#include <clasp/include/program_builder.h>

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

ClaspOutput::ClaspOutput(Clasp::ProgramBuilder *b, Clasp::LparseReader::TransformMode tf) : Output(&std::cout), b_(b), tf_(tf)
{
}

void ClaspOutput::initialize(Grounder *g)
{
	Output::initialize(g);
	// should get 1 here
	false_  = newUid();
	models_ = 1;
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
	stats_.rules[Clasp::BASICRULE]++;
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
			stats_.rules[Clasp::BASICRULE]++;
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
			stats_.rules[Clasp::BASICRULE]++;
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
			stats_.rules[Clasp::BASICRULE]++;
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
			stats_.rules[Clasp::BASICRULE]++;
			b_->startRule();
			b_->addHead(r->getUid());
			b_->endRule();
			break;
	}
}

void ClaspOutput::printWeightRule(int head, int bound, NS_OUTPUT::ObjectVector &lits, IntVector &weights)
{
	stats_.rules[Clasp::WEIGHTRULE]++;
	Clasp::PrgRule r(Clasp::WEIGHTRULE);
	r.setBound(bound);
	r.addHead(head);
	IntVector::iterator itW = weights.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++, itW != weights.end() ? itW++ : itW)
		r.addToBody(abs((*it)->getUid()), (*it)->getUid() > 0, itW != weights.end() ? *itW : 1);
	if(Clasp::LparseReader::transform_weight & tf_)
		b_->addAsNormalRules(r);
	else
		b_->addRule(r);
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
	stats_.rules[Clasp::BASICRULE]++;
	b_->startRule();
	b_->addHead(nn);
	b_->addToBody(r->uid_, false);
	b_->endRule();
	//*out_ << 1 << " " << false_ << " " << 2 << " " << 1 << " " << r->uid_ << " " << B << std::endl;
	stats_.rules[Clasp::BASICRULE]++;
	b_->startRule();
	b_->addHead(false_);
	b_->addToBody(r->uid_, false);
	b_->addToBody(B, true);
	b_->endRule();
	
	if(r->lits_.size() == 0)
		return;
	
	stats_.rules[Clasp::CHOICERULE]++;
	Clasp::PrgRule ru(Clasp::CHOICERULE);
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
		ru.addHead((*it)->getUid());
	ru.addToBody(B, true);
	ru.addToBody(nn, false);
	if(Clasp::LparseReader::transform_choice & tf_)
		b_->addAsNormalRules(ru);
	else
		b_->addRule(ru);
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
	stats_.rules[Clasp::BASICRULE]++;
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

void ClaspOutput::print(NS_OUTPUT::Compute *r)
{
	models_ = r->models_;
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++)
	{
		b_->setCompute(abs((*it)->getUid()), (*it)->getUid() > 0);
	}
}

void ClaspOutput::print(NS_OUTPUT::Optimize *r)
{
	int inv = r->type_ == NS_OUTPUT::Optimize::MAXIMIZE ? -1 : 1;
	stats_.rules[Clasp::OPTIMIZERULE]++;
	b_->startRule(Clasp::OPTIMIZERULE, 0);
	IntVector::iterator itW = r->weights_.begin();
	for(NS_OUTPUT::ObjectVector::iterator it = r->lits_.begin(); it != r->lits_.end(); it++, itW++)
	{
		int uid = (*it)->getUid() * inv;
		b_->addToBody(abs(uid), uid > 0, *itW);
	}
	b_->endRule();
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
	else if(dynamic_cast<NS_OUTPUT::Compute*>(r))
	{
		print(static_cast<NS_OUTPUT::Compute*>(r));
	}
	else if(dynamic_cast<NS_OUTPUT::Optimize*>(r))
	{
		print(static_cast<NS_OUTPUT::Optimize*>(r));
	}
	else
	{
		assert(false);
	}
}

Clasp::LparseStats &ClaspOutput::getStats()
{
	return stats_;
}

void ClaspOutput::finalize()
{
	int uid = 0;
	for(AtomLookUp::iterator it = atoms_.begin(); it != atoms_.end(); it++, uid++)
		if(g_->isVisible(uid))
			for(AtomHash::iterator atom = it->begin(); atom != it->end(); atom++)
				b_->setAtomName(atom->second, atomToString(uid, atom->first).c_str());
	b_->setCompute(false_, false);
	stats_.atoms[0] = uids_ - 1;
	stats_.atoms[1] = b_->numAtoms() - uids_;
}

int ClaspOutput::newUid()
{
	uids_++;
	return b_->newAtom();
}

ClaspOutput::~ClaspOutput()
{
}

