#include "dlvgrounder.h"
#include "grounder.h"
#include "groundable.h"
#include "literal.h"
#include "indexeddomain.h"
#include "value.h"
#include "literaldependencygraph.h"

using namespace NS_GRINGO;

DLVGrounder::DLVGrounder(Grounder *g, Groundable *r, LiteralVector &lit, LDG *dg, const VarVector &relevant) :
	g_(g), r_(r), lit_(lit), dom_(lit.size()), var_(lit.size()), dep_(lit.size()), 
	closestBinderVar_(lit.size()), closestBinderDep_(lit.size()), closestBinderRel_(lit.size() + 1), 
	global_(dg->getGlobalVars()), 
	relevant_(relevant)
{
	sortLiterals(dg);
	calcDependency();
}

DLVGrounder::~DLVGrounder()
{
	for(IndexedDomainVector::iterator it = dom_.begin(); it != dom_.end(); it++)
		delete *it;
}

namespace
{
	struct LiteralCmp
	{
		bool operator()(Literal *a, Literal *b)
		{
			return a->heuristicValue() < b->heuristicValue();
		}
	};
}

void DLVGrounder::sortLiterals(LDG *dg)
{
	VarSet index(dg->getParentVars().begin(), dg->getParentVars().end());
	LiteralSet list;
	dg->start(list);

	for(size_t i = 0; i < lit_.size(); i++)
	{
		assert(list.size() > 0);
		// choose the literal with the least heuristic value
		LiteralCmp cmp;
		Literal *l = *std::min_element(list.begin(), list.end(), cmp);
		dg->propagate(l, list);
		const VarVector &provided = dg->getProvidedVars(l);
		const VarVector &needed   = dg->getNeededVars(l);
		VarSet global;
		global.insert(needed.begin(), needed.end());
		global.insert(provided.begin(), provided.end());
		lit_[i] = l;
		dom_[i] = l->createIndexedDomain(index);
		var_[i].insert(var_[i].end(), global.begin(), global.end());
		index.insert(provided.begin(), provided.end());
	}
}

void DLVGrounder::debug()
{
        int l = lit_.size();
        std::cerr << "predicates: " << std::endl;
	{
		bool comma = false;
		for(int i = 0; i < l; i++)
		{
			if(comma)
				std::cerr << ", ";
			else
				comma = true;
			std::cerr << lit_[i] << " : " << lit_[i]->solved();
		}
	}
	std::cerr << std::endl;
        std::cerr << "variables: " << std::endl;
        for(int i = 0; i < l; i++)
        {
                std::cerr << "  var(" << lit_[i] << ") = { ";
                bool comma = false;
                for(VarVector::iterator it = var_[i].begin(); it != var_[i].end(); it++)
                {
                        if(comma)
                                std::cerr << ", ";
                        else
                                comma = true;
                        std::cerr << *g_->getVarString(*it);
                }
                std::cerr << " }" << std::endl;
        }
        std::cerr << "dependencies: " << std::endl;
        for(int i = 0; i < l; i++)
        {
                std::cerr << "  dep(" << lit_[i] << ") = { ";
                bool comma = false;
                for(VarVector::iterator it = dep_[i].begin(); it != dep_[i].end(); it++)
                {
                        if(comma)
                                std::cerr << ", ";
                        else
                                comma = true;
                        std::cerr << *g_->getVarString(*it);
                }
                std::cerr << " }" << std::endl;
        }
        std::cerr << "relevant: { ";
        bool comma = false;
        for(VarVector::iterator it = relevant_.begin(); it != relevant_.end(); it++)
        {
                if(comma)
                        std::cerr << ", ";
                else
                        comma = true;
                std::cerr << *g_->getVarString(*it);
        }
        std::cerr << " }" << std::endl;
}

void DLVGrounder::calcDependency()
{
	std::map<int,int> firstBinder;
	for(size_t i = 0; i < var_.size(); i++)
	{
		for(VarVector::iterator it = var_[i].begin(); it != var_[i].end(); it++)
			// the nice thing with map::insert is that if the value already existed
			// the new value isnt inserted it behaves exactly like a set
			firstBinder.insert(std::make_pair(*it, i));
	}
	assert(firstBinder.size() == global_.size());
	for(int i = dep_.size() - 1; i >= 0; i--)
	{
		VarSet depi;
		depi.insert(var_[i].begin(), var_[i].end());
		for(int j = dep_.size() - 1; j > i; j--)
		{
			for(VarVector::iterator it = dep_[j].begin(); it != dep_[j].end(); it++)
			{
				if(depi.find(*it) != depi.end())
				{
					depi.insert(dep_[j].begin(), dep_[j].end());
					break;
				}
			}
		}
		dep_[i].resize(depi.size());
		std::copy(depi.begin(), depi.end(), dep_[i].begin());
		
		closestBinderDep_[i] = closestBinder(i, dep_[i], firstBinder);
	}

	for(size_t i = 0; i < var_.size(); i++)
		closestBinderVar_[i] = closestBinder(i, var_[i], firstBinder);
	for(size_t i = 0; i < lit_.size() + 1; i++)
		closestBinderRel_[i] = closestBinder(i, relevant_, firstBinder);
}

int DLVGrounder::closestBinder(int l, VarVector &vars, std::map<int,int> &firstBinder)
{
	int binder = -1;
	for(VarVector::iterator it = vars.begin(); it != vars.end(); it++)
	{
		int newBinder = firstBinder[*it];
		if(newBinder < l)
			binder = std::max(binder, newBinder);
	}
	return binder;
}

void DLVGrounder::ground()
{
	// TODO: i am comparing l and csb at 2 places directly using the position in the list
	//       in the paper it seems like they are using a different relation but i think
	//       this cant be right cause no maximum is defined wrt. to this relation in 
	//       every case and it may be possible that the csb is right to the current literal
	//       what would be very odd
	int csb = -1;
	int l = 0;
	MatchStatus status = SuccessfulMatch;
	while(l != -1)
	{
		if(status == SuccessfulMatch)
			dom_[l]->firstMatch(l, this, status);
		else
			dom_[l]->nextMatch(l, this, status);
		/*
		std::cerr << "matched: " << lit_[l] << std::endl;
		std::cerr << "current binding:";
		for(VarVector::iterator it = global_.begin(); it != global_.end(); it++)
			if(g_->getBinder(*it) != -1 && g_->getBinder(*it) <= l)
				std::cerr << " " << g_->getVarString(*it) << "=" << g_->getValue(*it);
		std::cerr << std::endl;
		*/
		switch(status)
		{
			case SuccessfulMatch:
			{
				if(l + 1 < (int)lit_.size())
				{
					l++;
					//std::cerr << "SuccessfulMatch jt: " << lit_[l] << std::endl;
					break;
				}
				// this causes nextMatch to be called
				status = FailureOnNextMatch;
				r_->grounded(g_);
				csb    = closestBinderRel_[l + 1];
				l      = csb;
				/*
				std::cerr << "found solution backjump to: ";
				if(l == -1)
					std::cerr << -1;
				else
					std::cerr << lit_[l];
				std::cerr << "   csb: ";
				if(csb == -1)
					std::cerr << -1;
				else
					std::cerr << lit_[csb];
				std::cerr << std::endl;
				*/
				break;
			}
			case FailureOnFirstMatch:
			{
				l = closestBinderVar_[l];
				//if(rel_.contains(l, csb))
				if(l < csb)
					csb = l;
				/*
				std::cerr << "FailureOnFirstMatch bj to: ";
				if(l == -1)
					std::cerr << -1;
				else
					std::cerr << lit_[l];
				std::cerr << "   csb: ";
				if(csb == -1)
					std::cerr << -1;
				else
					std::cerr << lit_[csb];
				std::cerr << std::endl;
				*/
				break;
			}
			case FailureOnNextMatch:
			{
				if(l == csb)
					csb = closestBinderRel_[l];
				l = std::max(csb, closestBinderDep_[l]);
				/*
				std::cerr << "FailureOnNextMatch bj to: ";
				if(l == -1)
					std::cerr << -1;
				else
					std::cerr << lit_[l];
				std::cerr << "   csb: ";
				if(csb == -1)
					std::cerr << -1;
				else
					std::cerr << lit_[csb];
				std::cerr << std::endl;
				*/
				break;
			}
		}
	}
	// reset all variables bound!!!
	Value undef;
	for(VarVector::iterator it = global_.begin(); it != global_.end(); it++)
		g_->setValue(*it, undef, -1);
}

