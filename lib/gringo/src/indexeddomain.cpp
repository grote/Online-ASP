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














//////////////////////////////// IndexedDomainNewDefault ///////////////////////////////////////

IndexedDomainNewDefault::IndexedDomainNewDefault(ValueVectorSet &domain, VarSet &index, ConstantVector &param)
{
	TermVector paramNew;
	paramNew.resize(param.size());
	for(ConstantVector::iterator i = param.begin(); i != param.end(); ++i)
	{
		paramNew.push_back(*i);
	}
	std::cout << "IndexedDomain Default ctor" << std::endl;
	std::cout << "index= ";
	for (VarSet::const_iterator i = index.begin(); i != index.end(); ++i)
		std::cout << *i << " ";
	std::cout << std::endl;
	std::cout << "Domain: " << std::endl;
	for (ValueVectorSet::const_iterator i = domain.begin(); i != domain.end(); ++i)
	{
		for (ValueVector::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			if (j->type_ == Value::INT)
				std::cout << j->intValue_ << " ";
			else
				std::cout << *j->stringValue_ << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	// TODO: predicates like p(X,X) are not handles correctly
	std::vector<std::pair<int, Value> > constant;
	std::map<int, std::vector<int> > equal;

	for(int i = 0; i < (int)paramNew.size(); i++)
	{
		VarSet variables;
		param[i]->getVars(variables);
		if (variables.size() > 0)
			for (VarSet::const_iterator j = variables.begin(); j != variables.end(); ++j)
			{
				//add the UID of bound variables
				if (index.find(*j) != index.end())
					indexNew_.insert(*j);
				else
				{
					// what about equal variables
					bindNew_.insert(*j);
				}
			}
		else
			constant.push_back(std::make_pair(i, param[i]->getValue()));

//		int uid = param[i]->getUID();
//		if(uid)
//			if(index.find(uid) != index.end())
//				index_.push_back(std::make_pair(i, uid));
//			// in index_ sind alle Variablen die gebunden sind, index_[stelle, uid]
//			// die Stelle ist die nummer der Variable im Prädikat, angefangen mit 0
//			else
//			{
//				// insert binders only once
//				equal[uid].push_back(i);
//				if(equal[uid].size() == 1)
//					bind_.push_back(std::make_pair(i, uid));
//				//bind_ sind alle Variablen [stelle, uid] die jetzt gebunden werden müssen (noch frei sind)
//			}
//		else
//			constant.push_back(std::make_pair(i, param[i]->getValue()));
	}

//	std::map<int, std::vector<int> >::iterator equalIt = equal.begin(), eraseIt;
//	while(equalIt != equal.end())
//	{
//		if(equalIt->second.size() == 1)
//		{
//			eraseIt = equalIt;
//			equalIt++;
//			equal.erase(eraseIt);
//		}
//		else
//			equalIt++;
//	}

	for(ValueVectorSet::iterator it = domain.begin(); it != domain.end(); it++)
	{
		const ValueVector &val = (*it);
		ValueVector curIndex;
		// diese Schleife geht durch alle equal, das ist eine Liste mit Variablen X die doppelt vorkommen im Predikat
		// p(X,Y,4,X)
		// Es werden alle domaininhalte (instances) übersprungen die an diesen Stellen nicht gleich sind.
		// p(1,2,4,2) wird übersprungen
//		for(equalIt = equal.begin(); equalIt != equal.end(); equalIt++)
//		{
//			std::vector<int> &eq = equalIt->second;
//			Value v = val[*(eq.begin())];
//			for(std::vector<int>::iterator it = eq.begin() + 1; it != eq.end(); it++)
//			{
//				if(v != val[*it])
//					goto skip;
//			}
//		}
		// auch alle die die gesetzte Konstante vom Prädikat p(X,2,Y) nicht an dieser Stelle haben werden übersprungen
		for(std::vector<std::pair<int, Value> >::iterator it = constant.begin(); it != constant.end(); it++)
			if(val[it->first] != it->second)
				goto skip;

		// in index sind alle gebundenen Variablen (a(X) :- r(X), p(X,Y) == X) einmal vorhanden index_[stelle,uid]
		// curIndex ist also danach eine Liste mit Werten die durch Variablen gebunden werden
		for(std::vector<std::pair<int, int> >::iterator it = index_.begin(); it != index_.end(); it++)
		{
			curIndex.push_back(val[it->first]);
		}

		//if (val.unifies(paramNew, indexNew_, curIndex))
		//die indexedDomain mit dem Index aller einer Instanz aller gebundenen Variablen ist gleich der Instanz aus der Domain
		// (mehrere Instanzen)
		domain_[curIndex].push_back(&val);
skip:;
	}
}

void IndexedDomainNewDefault::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
{
	std::cout << "IndexedDomain firstMatch: " << std::endl;
	for (ValueVectorMap::const_iterator i = domain_.begin(); i != domain_.end(); ++i)
	{
		std::cout << "domain_[";
		for (ValueVector::const_iterator j = i->first.begin(); j != i->first.end(); ++j)
		{
			if (j->type_ == Value::INT)
				std::cout << j->intValue_ << " ";
			else
				std::cout << *j->stringValue_ << " ";
		}
		std::cout << "]" << std::endl;

		for (std::vector<const ValueVector*>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			for (ValueVector::const_iterator k = (*j)->begin(); k != (*j)->end(); ++k)
			{
				if (k->type_ == Value::INT)
					std::cout << k->intValue_ << " ";
				else
					std::cout << *k->stringValue_ << " ";
			}
			std::cout << " | ";
		}
		std::cout << std::endl;
	}

	std::cout << "index_[stelle, uid], alle Variablen die gebunden sind";
	for (std::vector<std::pair<int, int> >::const_iterator i = index_.begin(); i != index_.end(); ++i)
	{
		std::cout << "[" << i->first << "," << i->second << "]";
	}
	std::cout << std::endl;
	currentIndex_.clear();
	for(std::vector<std::pair<int, int> >::iterator it = index_.begin(); it != index_.end(); it++)
	{
		std::cout << "getValue auf " << it->second << " liefert ";
       		Value fuck = g->g_->getValue(it->second);
		if (fuck.type_ == Value::INT)
			std::cout << fuck.intValue_;
		else
			std::cout << *fuck.stringValue_;
		currentIndex_.push_back(g->g_->getValue(it->second));
	}
	std::cout << std::endl;
	std::cout << "currentIndex Size: " << currentIndex_.size() << std::endl;
	ValueVectorMap::iterator it = domain_.find(currentIndex_);
	std::cout << "IndexedDomain and stelle des currentIndexes: ";
	for (std::vector<const ValueVector*>::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		for (ValueVector::const_iterator j = (*i)->begin(); j != (*i)->end(); ++j)
		{
			if (j->type_ == Value::INT)
				std::cout << j->intValue_ << " ";
			else
				std::cout << *(j->stringValue_) << " ";
		}
		std::cout << " | " << std::endl;
	}
	std::cout << std::endl;
	if(it != domain_.end())
	{
		current_ = it->second.begin();
		end_     = it->second.end();
		assert(current_ != end_);
		//Vorsicht: it wird überschrieben !
		for(std::vector<std::pair<int, int> >::iterator it = bind_.begin(); it != bind_.end(); it++)
		{
			// setze freie Variable X(it->second) auf currentDomain[1 >1< 2], weil X an stelle 2 ist
			g->g_->setValue(it->second, (**current_)[it->first], binder);
		}
		status = SuccessfulMatch;
	}
	else
		status = FailureOnFirstMatch;
}

void IndexedDomainNewDefault::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
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

IndexedDomainNewDefault::~IndexedDomainNewDefault()
{
}

