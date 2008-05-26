#include "indexeddomain.h"
#include "value.h"
#include "grounder.h"
#include "dlvgrounder.h"
#include "literal.h"
#include "constant.h"

using namespace NS_GRINGO;

IndexedDomain::IndexedDomain()
{
}

IndexedDomain::~IndexedDomain()
{
}

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

//////////////////////////////// IndexedDomainDefault ///////////////////////////////////////

IndexedDomainDefault::IndexedDomainDefault(ValueVectorSet &domain, VarSet &index, ConstantVector &param)
{
	// TODO: predicates like p(X,X) are not handles correctly
	std::vector<std::pair<int, Value> > constant;
	std::map<int, std::vector<int> > equal;

	for(int i = 0; i < (int)param.size(); i++)
	{
		int uid = param[i]->getUID();
		if(uid)
			if(index.find(uid) != index.end())
				index_.push_back(std::make_pair(i, uid));
			else
			{
				// insert binders only once
				equal[uid].push_back(i);
				if(equal[uid].size() == 1)
					bind_.push_back(std::make_pair(i, uid));
			}
		else
			constant.push_back(std::make_pair(i, param[i]->getValue()));
	}

	std::map<int, std::vector<int> >::iterator equalIt = equal.begin(), eraseIt;
	while(equalIt != equal.end())
	{
		if(equalIt->second.size() == 1)
		{
			eraseIt = equalIt;
			equalIt++;
			equal.erase(eraseIt);
		}
		else
			equalIt++;
	}

	for(ValueVectorSet::iterator it = domain.begin(); it != domain.end(); it++)
	{
		const ValueVector &val = (*it);
		ValueVector curIndex;
		for(equalIt = equal.begin(); equalIt != equal.end(); equalIt++)
		{
			std::vector<int> &eq = equalIt->second;
			Value v = val[*(eq.begin())];
			for(std::vector<int>::iterator it = eq.begin() + 1; it != eq.end(); it++)
			{
				if(v != val[*it])
					goto skip;
			}
		}
		for(std::vector<std::pair<int, Value> >::iterator it = constant.begin(); it != constant.end(); it++)
			if(val[it->first] != it->second)
				goto skip;
		for(std::vector<std::pair<int, int> >::iterator it = index_.begin(); it != index_.end(); it++)
			curIndex.push_back(val[it->first]);
		domain_[curIndex].push_back(&val);
skip:;
	}
}

void IndexedDomainDefault::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	currentIndex_.clear();
	for(std::vector<std::pair<int, int> >::iterator it = index_.begin(); it != index_.end(); it++)
		currentIndex_.push_back(g->g_->getValue(it->second));
	ValueVectorMap::iterator it = domain_.find(currentIndex_);
	if(it != domain_.end())
	{
		current_ = it->second.begin();
		end_     = it->second.end();
		assert(current_ != end_);
		for(std::vector<std::pair<int, int> >::iterator it = bind_.begin(); it != bind_.end(); it++)
		{
			g->g_->setValue(it->second, (**current_)[it->first], binder);
		}
		status = SuccessfulMatch;
	}
	else
		status = FailureOnFirstMatch;
}

void IndexedDomainDefault::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	current_++;
	if(current_ != end_)
	{
		for(std::vector<std::pair<int, int> >::iterator it = bind_.begin(); it != bind_.end(); it++)
		{
			g->g_->setValue(it->second, (**current_)[it->first], binder);
		}
		status = SuccessfulMatch;
	}
	else
		status = FailureOnNextMatch;
}

IndexedDomainDefault::~IndexedDomainDefault()
{
}

//////////////////////////////// IndexedDomainFullMatch ///////////////////////////////////////

IndexedDomainFullMatch::IndexedDomainFullMatch(ValueVectorSet &domain, ConstantVector &param) : domain_(domain)
{
	for(ConstantVector::iterator it = param.begin(); it != param.end(); it++)
		bind_.push_back((*it)->getUID());
}

void IndexedDomainFullMatch::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	current_ = domain_.begin();
	if(current_ != domain_.end())
	{
		for(size_t i = 0; i < bind_.size(); i++)
			g->g_->setValue(bind_[i], (*current_)[i], binder);
		status = SuccessfulMatch;
	}
	else
		status = FailureOnFirstMatch;
}

void IndexedDomainFullMatch::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	current_++;
	if(current_ != domain_.end())
	{
		for(size_t i = 0; i < bind_.size(); i++)
			g->g_->setValue(bind_[i], (*current_)[i], binder);
		status = SuccessfulMatch;
	}
	else
		status = FailureOnNextMatch;
}

IndexedDomainFullMatch::~IndexedDomainFullMatch()
{
}

