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

#include "bindersplitter.h"
#include "literaldependencygraph.h"
#include "grounder.h"
#include "indexeddomain.h"
#include "dlvgrounder.h"
#include "value.h"
#include "output.h"
#include "term.h"
#include "domain.h"

using namespace NS_GRINGO;
		
BinderSplitter::BinderSplitter(Domain *domain, TermVector *param, const VarVector &relevant) : domain_(domain), param_(param), relevant_(relevant)
{
}

bool BinderSplitter::checkO(LiteralVector &unsolved)
{
	assert(false);
}

SDGNode *BinderSplitter::createNode(SDG *dg, SDGNode *prev, DependencyAdd todo)
{
	assert(false);
}

void BinderSplitter::createNode(LDGBuilder *dg, bool head)
{
	VarSet needed, provided(relevant_.begin(), relevant_.end());
	dg->createNode(this, head, needed, provided);
}

void BinderSplitter::createNode(StatementChecker *dg, bool head, bool delayed)
{
	assert(false);
}

void BinderSplitter::print(std::ostream &out)
{
	out << "true";
}

void BinderSplitter::reset()
{
	assert(false);
}

void BinderSplitter::finish()
{
	assert(false);
}

void BinderSplitter::evaluate()
{
	assert(false);
}

bool BinderSplitter::isFact(Grounder *g)
{
	return true;
}

bool BinderSplitter::solved()
{
	return true;
}

bool BinderSplitter::match(Grounder *g)
{
	// theres always a constructed indexed domain
	assert(false);
}

NS_OUTPUT::Object *BinderSplitter::convert()
{
	assert(false);
}

namespace NS_GRINGO
{
	namespace
	{
		/*
		std::string print(const ValueVector &v)
		{
			std::stringstream ss;
			bool comma = false;
			for(ValueVector::const_iterator i = v.begin(); i != v.end(); i++)
			{
				if(!comma)
					comma = true;
				else
					ss << ", ";
				ss << *i;
			}
			return ss.str();
		}
		*/

		class IndexedDomainBS : public IndexedDomain
		{
		private:
			typedef __gnu_cxx::hash_map<ValueVector, ValueVector, Value::VectorHash, Value::VectorEqual> ValueVectorMap;
		public:
			IndexedDomainBS(ValueVectorSet &domain, VarSet &index, TermVector &param, VarVector &relevant)
			{
				VarVector bind;
				for(int i = 0; i < (int)param.size(); i++)
				{
					VarSet variables;
					param[i]->getVars(variables);
					for (VarSet::const_iterator j = variables.begin(); j != variables.end(); ++j)
					{
						if(std::binary_search(relevant.begin(), relevant.end(), *j))
						{
							//add the UID of bound variables
							if (index.find(*j) != index.end())
								index_.push_back(*j);
							else
							{
								//and unbound variables
								bind_.push_back(*j);
								bind.push_back(*j);
							}
						}
						else
						{
							bind.push_back(*j);
						}
					}
				}

				//make unique
				sort(index_.begin(), index_.end());
				VarVector::iterator newEnd = std::unique(index_.begin(), index_.end());
				index_.erase(newEnd, index_.end());

				sort(bind.begin(), bind.end());
				newEnd = std::unique(bind.begin(), bind.end());
				bind.erase(newEnd, bind.end());

				sort(bind_.begin(), bind_.end());
				newEnd = std::unique(bind_.begin(), bind_.end());
				bind_.erase(newEnd, bind_.end());

				ValueVectorSet domainsBS;
				for(ValueVectorSet::iterator it = domain.begin(); it != domain.end(); it++)
				{
					const ValueVector &val = (*it);
					ValueVector curIndex(index_.size(),Value());
					ValueVector curValue(bind.size(), Value());
					bool doContinue = false;

					assert(param.size() == val.size());
					TermVector::const_iterator p = param.begin();
					for (ValueVector::const_iterator i = val.begin(); i != val.end(); ++i, ++p)
					{
						if (!(*p)->unify(*i, index_, bind, curIndex, curValue))
						{
							doContinue = true;
							break;
						}
					}

					if (doContinue) continue;

					//std::cerr << "unified with: " << print(curIndex) << " | " << print(curValue) << std::endl;
					
					ValueVector v = curIndex;
					ValueVector curValue2;
					ValueVector::iterator l = curValue.begin();
					for(VarVector::iterator j = bind.begin(), k = bind_.begin(); j != bind.end() && k != bind_.end(); j++, l++)
						if(*j == *k)
						{
							curValue2.push_back(*l);
							k++;
						}
					v.insert(v.end(), curValue2.begin(), curValue2.end());
					if(!domainsBS.insert(v).second)
					{
						//std::cerr << " <- already contained!" << std::endl;;
						continue;
					}
					//else
						//std::cerr << " <- inserted!" << std::endl;;
						
					//die indexedDomain mit dem Index aller einer Instanz aller gebundenen Variablen ist gleich der Instanz aus der Domain
					// (mehrere Instanzen)
					domain_[curIndex].insert(domain_[curIndex].end(), curValue2.begin(), curValue2.end());
				}
			}

			void firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
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
						//std::cerr << "setting(" << bind_[i] << ") " << *g->g_->getVarString(bind_[i]) << " = " << *current_ << std::endl;
						++current_;
					}
					status = SuccessfulMatch;
				}
				else
					status = FailureOnFirstMatch;
			}

			void nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
			{
				if(current_ != end_)
				{
					for(unsigned int i = 0; i < bind_.size(); ++i)
					{
						assert(current_ != end_);
						// setze freie Variable X(it->second) auf currentDomain[1 >1< 2], weil X an stelle 2 ist
						g->g_->setValue(bind_[i], (*current_), binder);
						//std::cerr << "setting " << *g->g_->getVarString(bind_[i]) << " = " << *current_ << std::endl;
						++current_;
					}
					status = SuccessfulMatch;
				}
				else
					status = FailureOnNextMatch;
			}

			~IndexedDomainBS()
			{
			}
		private:
			ValueVector currentIndex_;
			ValueVectorMap domain_;
			VarVector bind_;
			VarVector index_;
			ValueVector::iterator current_, end_;
		};
	}
}

IndexedDomain *BinderSplitter::createIndexedDomain(VarSet &index)
{
	return new IndexedDomainBS(domain_->getDomain(), index, *param_, relevant_);
}

Literal* BinderSplitter::clone() const
{
	assert(false);
}

BinderSplitter::~BinderSplitter()
{
}

void BinderSplitter::preprocess(Grounder *g, Expandable *e)
{
	assert(false);
}

double BinderSplitter::heuristicValue()
{
	// this selects splitted domains as soon as possible 
	// dont know if this is good but it is simple :)
	return 0;
}

void BinderSplitter::getVars(VarSet &vars) const
{
	vars.insert(relevant_.begin(), relevant_.end());
}

