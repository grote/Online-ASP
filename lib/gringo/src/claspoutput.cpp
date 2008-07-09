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

#ifdef WITH_CLASP

#include "claspoutput.h"
#include "gringoexception.h"
#include "grounder.h"
#include <clasp/include/program_builder.h>

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

ClaspOutput::ClaspOutput(Clasp::ProgramBuilder *b, Clasp::LparseReader::TransformMode tf) : SmodelsConverter(&std::cout), b_(b), tf_(tf)
{
}

void ClaspOutput::initialize(SignatureVector *pred)
{
	SmodelsConverter::initialize(pred);
	b_->setCompute(getFalse(), false);
}

void ClaspOutput::printBasicRule(int head, const IntVector &pos, const IntVector &neg)
{
	stats_.rules[Clasp::BASICRULE]++;
	b_->startRule();
	b_->addHead(head);
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		b_->addToBody(*it, false);
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		b_->addToBody(*it, true);
	b_->endRule();
}

void ClaspOutput::printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg)
{
	stats_.rules[Clasp::CONSTRAINTRULE]++;
	Clasp::PrgRule r(Clasp::CONSTRAINTRULE);
	r.setBound(bound);
	r.addHead(head);
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		r.addToBody(*it, false);
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		r.addToBody(*it, true);
	if(Clasp::LparseReader::transform_weight & tf_)
		b_->addAsNormalRules(r);
	else
		b_->addRule(r);
}

void ClaspOutput::printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg)
{
	stats_.rules[Clasp::CHOICERULE]++;
	Clasp::PrgRule r(Clasp::CHOICERULE);
	for(IntVector::const_iterator it = head.begin(); it != head.end(); it++)
		r.addHead(*it);
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		r.addToBody(*it, false);
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		r.addToBody(*it, true);
	if(Clasp::LparseReader::transform_choice & tf_)
		b_->addAsNormalRules(r);
	else
		b_->addRule(r);
}

void ClaspOutput::printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg)
{
	stats_.rules[Clasp::WEIGHTRULE]++;
	Clasp::PrgRule r(Clasp::WEIGHTRULE);
	r.setBound(bound);
	r.addHead(head);
	IntVector::const_iterator itW = wNeg.begin();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++, itW++)
		r.addToBody(*it, false, *itW);
	itW = wPos.begin();
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++, itW++)
		r.addToBody(*it, true, *itW);
	if(Clasp::LparseReader::transform_weight & tf_)
		b_->addAsNormalRules(r);
	else
		b_->addRule(r);
}

void ClaspOutput::printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg)
{
	stats_.rules[Clasp::OPTIMIZERULE]++;
	b_->startRule(Clasp::OPTIMIZERULE);
	IntVector::const_iterator itW = wNeg.begin();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++, itW++)
		b_->addToBody(*it, false, *itW);
	itW = wPos.begin();
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++, itW++)
		b_->addToBody(*it, true, *itW);
	b_->endRule();
}

void ClaspOutput::printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg)
{
	throw GrinGoException("error: sorry clasp cannot handle disjunctive rules!");
}

void ClaspOutput::printComputeRule(int models, const IntVector &pos, const IntVector &neg)
{
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		b_->setCompute(*it, false);
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		b_->setCompute(*it, true);
}

void ClaspOutput::finalize()
{
	stats_.atoms[0] = uids_ - 1;
	stats_.atoms[1] = b_->numAtoms() - uids_;
}

int ClaspOutput::newUid()
{
	uids_++;
	return b_->newAtom();
}

Clasp::LparseStats &ClaspOutput::getStats()
{
	return stats_;
}

bool ClaspOutput::addAtom(NS_OUTPUT::Atom *r)
{
	bool ret = Output::addAtom(r);
	if(ret && isVisible(r->predUid_))
		b_->setAtomName(r->uid_, atomToString(r->predUid_, r->values_).c_str());
	return ret;
}

ClaspOutput::~ClaspOutput()
{
}

#endif

#ifdef WITH_ICLASP

IClaspOutput::IClaspOutput(Clasp::ProgramBuilder *b, Clasp::LparseReader::TransformMode tf) : ClaspOutput(b, tf), incUid_(0)
{
}

void IClaspOutput::initialize(SignatureVector *pred)
{
	ClaspOutput::initialize(pred);
	incUid_ = 0;
	reinitialize();
}

void IClaspOutput::reinitialize()
{
	// set the previous incUid to false
	if(incUid_)
		b_->setCompute(incUid_, false);
	// create a new uid
	incUid_ = newUid();
	IntVector empty, head;
	head.push_back(incUid_);
	printChoiceRule(head, empty, empty);
}

void IClaspOutput::finalize()
{
	ClaspOutput::finalize();
}

int IClaspOutput::getIncUid()
{
	return incUid_;
}

void IClaspOutput::print(NS_OUTPUT::Object *o)
{
	//o->print_plain(this, std::cout);
	SmodelsConverter::print(o);
}

#endif

