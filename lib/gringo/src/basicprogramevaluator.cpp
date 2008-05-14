#include "basicprogramevaluator.h"
#include "grounder.h"
#include "node.h"
#include "output.h"

using namespace NS_GRINGO;
		
BasicProgramEvaluator::BasicProgramEvaluator() : Evaluator()
{
}

void BasicProgramEvaluator::initialize(Grounder *g)
{
	//std::cerr << "starting basic program" << std::endl;
	Evaluator::initialize(g);
	atomHash_.resize(g->getPred()->size());
}

int BasicProgramEvaluator::add(NS_OUTPUT::Atom *r)
{
	assert(!r->neg_);
	int id = r->node_->getUid();
	AtomHash::iterator res = atomHash_[id].find(r->values_);
	if(res == atomHash_[id].end())
	{
		int uid = facts_.size();
		atomHash_[id].insert(std::make_pair(r->values_, std::make_pair(r->node_, uid)));
		facts_.push_back(false);
		watches_.push_back(IntVector());
		/*
		// debug: 
		const std::string &name = (*g_->getPred())[id].first;
		std::stringstream ss;
		ss << name;
		if(r->values_.size() > 0)
		{
			ValueVector::iterator it = r->values_.begin();
			ss << "(" << *it;
			for(it++; it != r->values_.end(); it++)
				ss << "," << *it;
			ss << ")";
		}
		debug_.push_back(ss.str());
		*/
		//std::cout << "got atom: " << debug_[uid] << " = " << facts_[uid] << std::endl;
		return uid;
	}
	else
		return res->second.second;
}

void BasicProgramEvaluator::add(NS_OUTPUT::Fact *r)
{
	assert(dynamic_cast<NS_OUTPUT::Atom*>(r->head_));
	int uid = add(static_cast<NS_OUTPUT::Atom*>(r->head_));
	
	//std::cerr << "got fact: " << debug_[uid] << std::endl;

	facts_[uid] = true;
	propQ_.push_back(uid);
}

void BasicProgramEvaluator::add(NS_OUTPUT::Rule *r)
{
	assert(dynamic_cast<NS_OUTPUT::Atom*>(r->head_));
	assert(dynamic_cast<NS_OUTPUT::Conjunction*>(r->body_));
	int uid = add(static_cast<NS_OUTPUT::Atom*>(r->head_));
	//std::cerr << "starting rule with head: " << debug_[uid] << std::endl;
	if(!facts_[uid])
	{
		NS_OUTPUT::ObjectVector &lits = static_cast<NS_OUTPUT::Conjunction*>(r->body_)->lits_;
		assert(lits.size() > 0);
		rules_.push_back(IntVector());
		IntVector &rule = rules_.back();
		rule.reserve(1 + lits.size());
		rule.push_back(uid);
		//std::cerr << debug_[uid] << " :- ";
		for(NS_OUTPUT::ObjectVector::iterator it = lits.begin(); it != lits.end(); it++)
		{
			assert(dynamic_cast<NS_OUTPUT::Atom*>(*it));
			int bodyUid = add(static_cast<NS_OUTPUT::Atom*>(*it));
			//std::cerr << debug_[bodyUid] << " = " << facts_[bodyUid] << ", ";
			if(!facts_[bodyUid])
				rule.push_back(bodyUid);
		}
		//std::cerr << "." << std::endl;
		if(rule.size() == 1)
		{
			facts_[uid] = true;
			propQ_.push_back(uid);
			rules_.pop_back();
			//std::cerr << "got fact: " << debug_[uid] << std::endl;
		}
		else
			watches_[rule[1]].push_back(rules_.size() - 1);
	}
}

void BasicProgramEvaluator::add(NS_OUTPUT::Object *r)
{
	if(dynamic_cast<NS_OUTPUT::Rule*>(r))
	{
		add(static_cast<NS_OUTPUT::Rule*>(r));
	}
	else if(dynamic_cast<NS_OUTPUT::Fact*>(r))
	{
		add(static_cast<NS_OUTPUT::Fact*>(r));
	}
	else
		assert(false);
	// for grounding it is essentiell to add the domains
	r->addDomain();
	delete r;
}

void BasicProgramEvaluator::evaluate()
{
	//std::cerr << "evaluating basic program" << std::endl;
	while(propQ_.size() > 0)
	{
		int i = propQ_.back();
		propQ_.pop_back();
		assert(i < (int)watches_.size());
		for(IntVector::iterator it = watches_[i].begin(); it != watches_[i].end(); it++)
		{
			assert(*it < (int)rules_.size());
			IntVector &rule = rules_[*it];
			assert(rule[0] < (int)facts_.size());
			if(!facts_[rule[0]])
			{
				if(rule.size() == 2)
				{
					facts_[rule[0]] = true;
					propQ_.push_back(rule[0]);
				}
				else
				{
					int newsize = 1;
					IntVector::iterator solved = rule.begin() + 1;
					for(IntVector::iterator lit = rule.begin() + 2; lit != rule.end(); lit++)
						if(!facts_[*lit])
							*solved++ = *lit, newsize++;
					rule.resize(newsize);
					if(rule.size() == 1)
					{
						facts_[rule[0]] = true;
						propQ_.push_back(rule[0]);
					}
					else
						watches_[rule[1]].push_back(*it);
				}
			}
		}
	}
	for(AtomLookUp::iterator itHash = atomHash_.begin(); itHash != atomHash_.end(); itHash++)
	{
		for(AtomHash::iterator it = itHash->begin(); it != itHash->end(); it++)
		{
			if(facts_[it->second.second])
			{
				ValueVector values(it->first);
				NS_OUTPUT::Atom *a = new NS_OUTPUT::Atom(false, it->second.first, values);
				NS_OUTPUT::Fact f(a);
				f.addUid(o_);
				f.addDomain(true);
				o_->print(&f);
			}
			else
				// remove all atoms that couldnt be derived from the domain
				it->second.first->removeDomain(it->first);
		}
	}
	// cleanup but if fear even though clear is called 
	// the vectors still waste a lot of memory
	atomHash_.clear();
	facts_.clear();
	watches_.clear();
	rules_.clear();

	//std::cerr << "evaluated basic program" << std::endl;
}

BasicProgramEvaluator::~BasicProgramEvaluator()
{
}

