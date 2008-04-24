#include "dlvgrounder.h"
#include "grounder.h"
#include "groundable.h"
#include "literal.h"
#include "indexeddomain.h"
#include "value.h"

using namespace NS_GRINGO;

DLVGrounder::DLVGrounder(Grounder *g, Groundable *r, LiteralVector &lit, VarSet &glob, VarSet &relevant) : 
	g_(g), r_(r), lit_(lit), dom_(lit.size()), var_(lit.size()), dep_(lit.size()), 
	closestBinderVar_(lit.size()), closestBinderDep_(lit.size()), closestBinderRel_(lit.size() + 1), 
	global_(glob.begin(), glob.end()),
	relevant_(relevant.begin(), relevant.end())
{
	sortLiterals();
	VarSet index;
	int l = lit_.size();
	for(int i = 0; i < l; i++)
	{
		dom_[i] = lit_[i]->createIndexedDomain(index);
		lit_[i]->getVars(index, VARS_PROVIDED);
	}
	cacheVariables();
	calcDependency();
}

DLVGrounder::DLVGrounder(Grounder *g, Groundable *r, LiteralVector &lit, VarSet &glob, VarSet &relevant, VarSet &index) : 
	g_(g), r_(r), lit_(lit), dom_(lit.size()), var_(lit.size()), dep_(lit.size()), 
	closestBinderVar_(lit.size()), closestBinderDep_(lit.size()), closestBinderRel_(lit.size() + 1), 
	global_(glob.begin(), glob.end()), 
	relevant_(relevant.begin(), relevant.end())
{
	sortLiterals();
	int l = lit_.size();
	for(int i = 0; i < l; i++)
	{
		dom_[i] = lit_[i]->createIndexedDomain(index);
		lit_[i]->getVars(index, VARS_PROVIDED);
	}
	cacheVariables();
	calcDependency();
}

DLVGrounder::~DLVGrounder()
{
	for(IndexedDomainVector::iterator it = dom_.begin(); it != dom_.end(); it++)
		delete *it;
}

void DLVGrounder::sortLiterals()
{
	// TODO: it would be better to cache the variables than calculating them all the times
	// TODO: maybe this can be improved with a clever heuristic ????
	LiteralVector::iterator sorted = lit_.begin();
	// sort the literals in body: literals which bind variables first
	VarSet provided;
	while(sorted != lit_.end())
	{
		for(LiteralVector::iterator it = sorted; it != lit_.end(); it++)
		{
			Literal *l = *it;
			VarSet needed;
			l->getVars(needed, VARS_NEEDED, global_);
			for(VarSet::const_iterator itNeeded = needed.begin(); itNeeded != needed.end(); itNeeded++)
			{
				// found a var that is not yet provided
				if(provided.find(*itNeeded) == provided.end())
					goto skip;
			}
			l->getVars(provided, VARS_PROVIDED);
			std::swap(*sorted, *it);
			sorted++;
skip:			;
		}
	}
}

void DLVGrounder::cacheVariables()
{
	// store variables of literals in vector for faster access
	std::vector<VarVector>::iterator varIt = var_.begin();
	for(LiteralVector::iterator it = lit_.begin(); it != lit_.end(); it++, varIt++)
	{
		VarSet vars;
		(*it)->getVars(vars, VARS_GLOBAL, global_);
		(*varIt).resize(vars.size());
		std::copy(vars.begin(), vars.end(), (*varIt).begin());
	}
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

void DLVGrounder::debug()
{
	std::cerr << "dependencies: " << std::endl;
	int l = lit_.size();
	for(int i = 0; i < l; i++)
	{
		std::cerr << "	dep(" << lit_[i] << ") = { ";
		bool comma = false;
		for(VarVector::iterator it = dep_[i].begin(); it != dep_[i].end(); it++)
		{
			if(comma)
				std::cerr << ", ";
			else
				comma = true;
			std::cerr << g_->getVarString(*it);
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
		std::cerr << g_->getVarString(*it);
	}
	std::cerr << " }" << std::endl;
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

