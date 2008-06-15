#include "smodelsoutput.h"
#include "grounder.h"
#include "gringoexception.h"

using namespace NS_GRINGO;
using namespace NS_OUTPUT;

SmodelsOutput::SmodelsOutput(std::ostream *out) : SmodelsConverter(out)
{
}

void SmodelsOutput::initialize(SignatureVector *pred)
{
	SmodelsConverter::initialize(pred);
	models_ = 1;
}

void SmodelsOutput::printBasicRule(int head, const IntVector &pos, const IntVector &neg)
{
	*out_ << 1 << " " << head << " " << (pos.size() + neg.size()) << " " << neg.size();
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	*out_ << std::endl;
}

void SmodelsOutput::printConstraintRule(int head, int bound, const IntVector &pos, const IntVector &neg)
{
	*out_ << 2 << " " << head << " " << (pos.size() + neg.size()) << " " << neg.size() << " " << bound;
	for(IntVector::const_iterator it = neg.begin(); it != neg.end(); it++)
		*out_ << " " << *it;
	for(IntVector::const_iterator it = pos.begin(); it != pos.end(); it++)
		*out_ << " " << *it;
	*out_ << std::endl;
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
	*out_ << std::endl;
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
	*out_ << std::endl;
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
	*out_ << std::endl;
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
	*out_ << std::endl;
}

void SmodelsOutput::printComputeRule(int models, const IntVector &pos, const IntVector &neg)
{
	models_ = models;
	compPos_.insert(pos.begin(), pos.end());
	compNeg_.insert(neg.begin(), neg.end());
}

void SmodelsOutput::finalize()
{
	*out_ << 0 << std::endl;
	int uid = 0;
	for(AtomLookUp::iterator it = atoms_.begin(); it != atoms_.end(); it++, uid++)
		if(isVisible(uid))
			for(AtomHash::iterator atom = it->begin(); atom != it->end(); atom++)
				*out_ << atom->second << " " << atomToString(uid, atom->first) << std::endl;
	*out_ << 0 << std::endl;
	*out_ << "B+" << std::endl;
	// compute +
	for(IntSet::iterator it = compPos_.begin(); it != compPos_.end(); it++)
		*out_ << *it << std::endl;
	*out_ << 0 << std::endl;
	*out_ << "B-" << std::endl;
	// compute -
	*out_ << getFalse() << std::endl;
	for(IntSet::iterator it = compNeg_.begin(); it != compNeg_.end(); it++)
		*out_ << *it << std::endl;
	*out_ << 0 << std::endl;
	// number of models
	*out_ << models_ << std::endl;
}

SmodelsOutput::~SmodelsOutput()
{
}

