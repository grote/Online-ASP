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

#include "grounder.h"
#include "statement.h"
#include "variable.h"
#include "predicateliteral.h"
#include "normalrule.h"
#include "statementdependencygraph.h"
#include "program.h"
#include "value.h"
#include "evaluator.h"
#include "domain.h"
#include "gringoexception.h"

using namespace NS_GRINGO;

Grounder::Grounder(const Options &opts) : opts_(opts), incremental_(false), incStep_(1), internalVars_(0), output_(0), eval_(0)
{
}

void Grounder::setOutput(NS_OUTPUT::Output *output)
{
	output_ = output;
}

void Grounder::addStatement(Statement *rule)
{
	if(incParts_.size() > 0)
		incParts_.back().second++;
	rules_.push_back(rule);
}

void Grounder::addDomains(int id, std::vector<IntVector*>::iterator pos, std::vector<IntVector*>::iterator end, IntVector &list)
{
	if(pos == end)
	{
		domains_.push_back(DomainPredicate(id, new IntVector(list)));
	}
	else
	{
		for(IntVector::iterator it = (*pos)->begin(); it != (*pos)->end(); it++)
		{
			list.push_back(*it);
			addDomains(id, pos + 1, end, list);
			list.pop_back();
		}
		delete (*pos);
	}
}

void Grounder::addDomains(int id, std::vector<IntVector*>* list)
{
	IntVector empty;
	addDomains(id, list->begin(), list->end(), empty);
	delete list;
}

void Grounder::buildDepGraph()
{
	SDG dg;
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		(*it)->buildDepGraph(&dg);
	reset();
	dg.calcSCCs(this);
}

void Grounder::check()
{
	for(ProgramVector::iterator it = sccs_.begin(); it != sccs_.end(); it++)
		if(!(*it)->check(this))
			throw GrinGoException("Error: the program is not groundable.");
	substitution_.resize(varMap_.size() + 2);
	binder_.resize(varMap_.size() + 2, -1);
}

void Grounder::addDomains()
{
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
	{
		Statement *rule = *it;
		VarSet vars;
		rule->getVars(vars);
		for(DomainPredicateVector::iterator it = domains_.begin(); it != domains_.end(); it++)
		{
			DomainPredicate &dp = *it;
			for(IntVector::iterator it = dp.second->begin(); it != dp.second->end(); it++)
			{
				if(vars.find(getVar(*it)) != vars.end())
				{
					// construct predicate literal
					TermVector *tv = new TermVector();
					for(IntVector::iterator vars = dp.second->begin(); vars != dp.second->end(); vars++)
						tv->push_back(new Variable(this, *vars));
					PredicateLiteral *pred = new PredicateLiteral(this, dp.first, tv);
					rule->addDomain(pred);
					break;
				}
			}
		}
	}
}

void Grounder::addZeroDomain(Domain *d)
{
	d->setSolved(true);
	for(int uid = 0; uid < (int)getDomains()->size(); uid++)
	{
		if(d == (*getDomains())[uid])
		{
			Signature &sig = (*getPred())[uid];
			std::cerr << "Warning: " << *getString(sig.first) << "/" << sig.second << " is never defined." << std::endl;
			break;
		}
	}
}

void Grounder::reset()
{
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		(*it)->reset();
}

void Grounder::preprocess()
{
	StatementVector::iterator r = rules_.begin();
	for(IncParts::iterator it = incParts_.begin(); it != incParts_.end(); it++)
		for(int j = 0; j < it->second; j++, r++)
			(*r)->setIncPart(this, it->first.first, it->first.second);

	// the size of rules_ may increase during preprocessing make shure the newly inserted rules are preprocessed too
	for(size_t i = 0; i < rules_.size(); i++)
		rules_[i]->preprocess(this);
}

void Grounder::start()
{
	if(incParts_.size() > 0 && opts_.ifixed < 0)
		throw GrinGoException("Error: A fixed number of incremtal steps is needed to ground the program.");
	if(incParts_.size() > 0)
	{
		do
		{
			iground();
		}
		while(incStep_ <= opts_.ifixed);
		output_->finalize(true);
	}
	else
	{
		if(options().verbose)
			std::cerr << "preprocessing ... " << std::endl;
		preprocess();
		if(options().verbose)
			std::cerr << "done" << std::endl;
		if(options().verbose)
			std::cerr << "adding domain predicates ... " << std::endl;
		addDomains();
		if(options().verbose)
		{
			std::cerr << "done" << std::endl;
			std::cerr << "building dependencygraph ... " << std::endl;
		}
		buildDepGraph();
		if(options().verbose)
		{
			std::cerr << "done" << std::endl;
			std::cerr << "checking ... " << std::endl;
		}
		check();
		reset();
		if(options().verbose)
		{
			std::cerr << "done" << std::endl;
			std::cerr << "grounding ... " << std::endl;
		}
		substitution_.resize(varMap_.size() + 2);
		binder_.resize(varMap_.size() + 2, -1);
		ground();
		if(options().verbose)
			std::cerr << "done" << std::endl;
		output_->finalize(true);
	}
}

int Grounder::getIncStep() const
{
	return incStep_;
}

bool Grounder::isIncGrounding() const
{
	return incremental_;
}

void Grounder::iground()
{
	incremental_ = true;
	if(incParts_.size() == 0)
	{
		incParts_.push_back(make_pair(std::make_pair(BASE, 0), rules_.size()));
		std::cerr << "Warning: There are no #base, #lambda or #delta sections." << std::endl;
	}

	if(incStep_ == 1)
	{
		if(options().verbose)
			std::cerr << "preprocessing ... " << std::endl;
		preprocess();
		if(options().verbose)
		{
			std::cerr << "done" << std::endl;
			std::cerr << "adding domain predicates ... " << std::endl;
		}
		addDomains();
		if(options().verbose)
		{
			std::cerr << "done" << std::endl;
			std::cerr << "building dependencygraph ... " << std::endl;
		}
		buildDepGraph();
		if(options().verbose)
		{
			std::cerr << "done" << std::endl;
			std::cerr << "checking ... " << std::endl;
		}
		check();
		if(options().verbose)
			std::cerr << "done" << std::endl;
	}
	if(options().verbose)
		std::cerr << "grounding step: " << incStep_ << " ... " << std::endl;
	reset();
	ground();
	if(options().verbose)
		std::cerr << "done" << std::endl;
	incStep_++;
}

void Grounder::setIncPart(IncPart part, int var)
{
	if(incParts_.size() == 0 && rules_.size() > 0)
	{
		incParts_.push_back(make_pair(std::make_pair(BASE, 0), rules_.size()));
		std::cerr << "Warning: There are statements not within a #base, #lambda or #delta section." << std::endl;
		std::cerr << "         These Statements are put into the #base section." << std::endl;
	}
	incParts_.push_back(make_pair(std::make_pair(part, var), 0));
}


void Grounder::addProgram(Program *scc)
{
	sccs_.push_back(scc);
}

void Grounder::ground()
{
	if(incStep_ == 1 || !incremental_)
		output_->initialize(this, getPred());
	else
		output_->reinitialize();
	
	for(ProgramVector::iterator it = sccs_.begin(); it != sccs_.end(); it++)
	{
		Program *scc = *it;
		//std::cerr << scc << std::endl;
		eval_ = scc->getEvaluator();
		eval_->initialize(this);
		StatementVector *rules = scc->getStatements();
		for(StatementVector::iterator it = rules->begin(); it !=rules->end(); it++)
		{
			Statement *rule = *it;
			if(incStep_ == 1 || !incremental_)
				rule->ground(this, PREPARE);
			else
				rule->ground(this, REINIT);

			rule->ground(this, GROUND);
			rule->ground(this, RELEASE);
			rule->finish();
		}
		// this solves the domains
		for(StatementVector::iterator it = rules->begin(); it !=rules->end(); it++)
		{
			Statement *rule = *it;
			rule->evaluate();
		}
		eval_->evaluate();
		eval_ = 0;
	}
	output_->finalize(false);
}

int Grounder::createUniqueVar()
{
	int uid;
	do
	{
		std::stringstream ss;
		ss << "I_" << internalVars_++;
		uid = createString(ss.str());
	}
	while(varMap_.find(uid) != varMap_.end());
	return uid;
}

const std::string *Grounder::getVarString(int uid)
{
	// inefficient but we need it only for error messages
	for(VariableMap::iterator it = varMap_.begin(); it != varMap_.end(); it++)
		if(it->second == uid)
			return getString(it->first);
	// we should get a string for every variable
	assert(false);
}

int Grounder::getVar(int var)
{
	VariableMap::iterator it = varMap_.find(var);
	if(it != varMap_.end())
		return it->second;
	else
		return 0;
}

int Grounder::registerVar(int var)
{
	int &uid = varMap_[var];
	if(uid == 0)
		uid = varMap_.size();
	return uid;
}

Grounder::~Grounder()
{
	for(DomainPredicateVector::iterator it = domains_.begin(); it != domains_.end(); it++)
		delete (*it).second;
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		delete *it;
	for(ConstTerms::iterator it = constTerms_.begin(); it != constTerms_.end(); it++)
		delete it->second.second;
	for(ProgramVector::iterator it = sccs_.begin(); it != sccs_.end(); it++)
		delete *it;
}

Value Grounder::getValue(int var)
{
	return substitution_[var];
}

void Grounder::setTempValue(int var, const Value &val)
{
	substitution_[var] = val;
}

void Grounder::setValue(int var, const Value &val, int binder)
{
	substitution_[var] = val;
	binder_[var] = binder;
}

int Grounder::getBinder(int var) const
{
	return binder_[var];
}

void Grounder::setConstValue(int id, Term *t)
{
	std::pair<ConstTerms::iterator, bool> res = constTerms_.insert(std::make_pair(id, std::make_pair(false, t)));
	if(!res.second)
	{
		std::cerr << "Warning: multiple definitions of #const " << *getString(id) << std::endl;
		delete t;
	}
}

Value Grounder::getConstValue(int id)
{
	ConstTerms::iterator it = constTerms_.find(id);
	if(it != constTerms_.end())
	{
		if(it->second.first)
			throw GrinGoException("Error: cyclic constant definition.");
		it->second.first = true;
		Value v = it->second.second->getConstValue(this);
		it->second.first = false;
		return v;
	}
	else
		return Value(Value::STRING, id);
}

Evaluator *Grounder::getEvaluator()
{
	return eval_;
}

NS_OUTPUT::Output *Grounder::getOutput()
{
	return output_;
}

const Grounder::Options &Grounder::options() const
{
	return opts_;
}

void Grounder::addTrueNegation(int id, int arity)
{
#ifdef WITH_ICLASP
	// TODO: this is ugly
	if(incremental_)
	{
		static bool warn = true;
		if(!warn)
			return;
		warn = false;
		std::cerr << "Warning: Classical negation is not handled correctly in combination with the icremental output." << std::endl;
		std::cerr << "         You have to add rules like: :- a, -a. on your own! (at least for now)" << std::endl;
		return;
	}
#endif
	if(trueNegPred_.insert(Signature(id, arity)).second)
	{
		TermVector *tp = new TermVector();
		TermVector *tn = new TermVector();
		for(int i = 0; i < arity; i++)
		{
			// in theory existing vars could be reused
			int var = createUniqueVar();
			tp->push_back(new Variable(this, var));
			tn->push_back(new Variable(this, var));
		}
		int pos = createString(getString(id)->substr(1));
		PredicateLiteral *p = new PredicateLiteral(this, pos, tp);
		PredicateLiteral *n = new PredicateLiteral(this, id, tn);
		LiteralVector *body = new LiteralVector();
		body->push_back(p);
		body->push_back(n);
		NormalRule *r = new NormalRule(0, body);
		addStatement(r);
	}
}

