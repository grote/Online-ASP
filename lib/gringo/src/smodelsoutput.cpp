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

#include "smodelsoutput.h"
#include "grounder.h"
#include "gringoexception.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

SmodelsOutput::SmodelsOutput(std::ostream *out) : SmodelsConverter(out)
{
}

void SmodelsOutput::initialize(GlobalStorage *g, SignatureVector *pred)
{
	SmodelsConverter::initialize(g, pred);
	models_ = 1;
}

void SmodelsOutput::printBasicRule(int head, const IntVector &pos, const IntVector &neg)
{
	*out_ << 1 << " " << head << " " << (pos.size() + neg.size()) << " " << neg.size();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	*out_ << NL;
}

void SmodelsOutput::printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg)
{
	*out_ << 2 << " " << head << " " << (pos.size() + neg.size()) << " " << neg.size() << " " << bound;
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	*out_ << NL;
}

void SmodelsOutput::printChoiceRule(const IntVector &head, const IntVector &pos, const IntVector &neg)
{
	*out_ << 3 << " " << head.size();
	for(IntVector::const_iterator it = head.begin(); it != head.end(); it++)
		*out_ << " " << *it;
	*out_ << " " << (pos.size() + neg.size()) << " " << neg.size();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	*out_ << NL;
}

void SmodelsOutput::printWeightRule(int head, int bound, const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg)
{
	*out_ << 5 << " " << head << " " << bound << " " << (pos.size() + neg.size()) << " " << neg.size();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = wNeg.begin(); it != wNeg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = wPos.begin(); it != wPos.end(); it++)
		*out_ << " " << *it;
	*out_ << NL;
}

void SmodelsOutput::printMinimizeRule(const IntVector &pos, const IntVector &neg, const IntVector &wPos, const IntVector &wNeg)
{
	*out_ << 6 << " " << 0 << " " << (pos.size() + neg.size()) << " " << neg.size();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = wNeg.begin(); it != wNeg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = wPos.begin(); it != wPos.end(); it++)
		*out_ << " " << *it;
	*out_ << NL;
}

void SmodelsOutput::printDisjunctiveRule(const IntVector &head, const IntVector &pos, const IntVector &neg)
{
	*out_ << 8 << " " << head.size();
	for(IntVector::const_iterator it = head.begin(); it != head.end(); it++)
		*out_ << " " << *it;
	*out_ << " " << (pos.size() + neg.size()) << " " << neg.size();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	*out_ << NL;
}

void SmodelsOutput::printComputeRule(int models, const IntVector &pos, const IntVector &neg)
{
	models_ = models;
	compPos_.insert(pos.begin(), pos.end());
	compNeg_.insert(neg.begin(), neg.end());
}

void SmodelsOutput::finalize(bool last)
{
	if(last)
	{
		*out_ << 0 << NL;
		int uid = 0;
		for(AtomLookUp::iterator it = atoms_.begin(); it != atoms_.end(); it++, uid++)
			if(isVisible(uid))
				for(AtomHash::iterator atom = it->begin(); atom != it->end(); atom++)
					*out_ << atom->second << " " << atomToString(uid, atom->first) << NL;
		*out_ << 0 << NL;
		*out_ << "B+" << NL;
		// compute +
		for(IntSet::iterator it = compPos_.begin(); it != compPos_.end(); it++)
			*out_ << *it << NL;
		*out_ << 0 << NL;
		*out_ << "B-" << NL;
		// compute -
		*out_ << getFalse() << NL;
		for(IntSet::iterator it = compNeg_.begin(); it != compNeg_.end(); it++)
			*out_ << *it << NL;
		*out_ << 0 << NL;
		// number of models
		*out_ << models_ << NL;
		out_->flush();
	}
}

SmodelsOutput::~SmodelsOutput()
{
}

