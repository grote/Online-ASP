#include "indexeddomain.h"
#include "value.h"
#include "grounder.h"
#include "dlvgrounder.h"
#include "literal.h"
#include "constant.h"

using namespace NS_GRINGO;

//////////////////////////////// IndexedDomain ///////////////////////////////////////

IndexedDomain::IndexedDomain()
{
}

IndexedDomain::~IndexedDomain()
{
}

//////////////////////////////// IndexedDomainMatchOnly ///////////////////////////////////////

IndexedDomainMatchOnly::IndexedDomainMatchOnly(Literal *l) : IndexedDomain(), l_(l)
{
}

void IndexedDomainMatchOnly::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	if(l_->match(g->g_))
		status = SuccessfulMatch;
	else
		status = FailureOnFirstMatch;
}

void IndexedDomainMatchOnly::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	assert(false);
}

IndexedDomainMatchOnly::~IndexedDomainMatchOnly()
{
}

//////////////////////////////// IndexedDomainNewDefault ///////////////////////////////////////

IndexedDomainNewDefault::IndexedDomainNewDefault(ValueVectorSet &domain, VarSet &index, const TermVector &paramNew)
{
	for(int i = 0; i < (int)paramNew.size(); i++)
	{
		VarSet variables;
		paramNew[i]->getVars(variables);
		for (VarSet::const_iterator j = variables.begin(); j != variables.end(); ++j)
		{
			//add the UID of bound variables
			if (index.find(*j) != index.end())
				index_.push_back(*j);
			else
			{
				//and unbound variables
				bind_.push_back(*j);
			}
		}
	}

	//make unique
	sort(index_.begin(), index_.end());
	VarVector::iterator newEnd = std::unique(index_.begin(), index_.end());
	index_.erase(newEnd, index_.end());

	sort(bind_.begin(), bind_.end());
	newEnd = std::unique(bind_.begin(), bind_.end());
	bind_.erase(newEnd, bind_.end());


	for(ValueVectorSet::iterator it = domain.begin(); it != domain.end(); it++)
	{
		const ValueVector &val = (*it);
		ValueVector curIndex(index_.size(),Value());
		ValueVector curValue(bind_.size(), Value());
		bool doContinue = false;


		assert(paramNew.size() == val.size());
		TermVector::const_iterator p = paramNew.begin();
		for (ValueVector::const_iterator i = val.begin(); i != val.end(); ++i, ++p)
		{
			if (!(*p)->unify(*i, index_, bind_, curIndex, curValue))
			{
				doContinue = true;
				break;
			}
		}

		if (doContinue) continue;


		//die indexedDomain mit dem Index aller einer Instanz aller gebundenen Variablen ist gleich der Instanz aus der Domain
		// (mehrere Instanzen)
		domain_[curIndex].insert(domain_[curIndex].end(),curValue.begin(), curValue.end());
	}
}

void IndexedDomainNewDefault::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
{

	currentIndex_.clear();
	for (VarVector::const_iterator i = index_.begin(); i != index_.end(); ++i)
	{
		currentIndex_.push_back(g->g_->getValue(*i));
	}


	ValueVectorMap::iterator it = domain_.find(currentIndex_);



	if(it != domain_.end())
	{
		current_ = it->second.begin();
		end_     = it->second.end();
		for(unsigned int i = 0; i < bind_.size(); ++i)
		{
			assert(current_ != end_);
			// setze freie Variable X(bind_[i]) auf currentDomain[1 >1< 2], weil X an stelle 2(i) ist
			g->g_->setValue(bind_[i], (*current_), binder);
			++current_;
		}
		status = SuccessfulMatch;
	}
	else
		status = FailureOnFirstMatch;
}

void IndexedDomainNewDefault::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	if(current_ != end_)
	{
		for(unsigned int i = 0; i < bind_.size(); ++i)
		{
			assert(current_ != end_);
			// setze freie Variable X(it->second) auf currentDomain[1 >1< 2], weil X an stelle 2 ist
			g->g_->setValue(bind_[i], (*current_), binder);
			++current_;
		}
		status = SuccessfulMatch;
	}
	else
		status = FailureOnNextMatch;

}

IndexedDomainNewDefault::~IndexedDomainNewDefault()
{
}

