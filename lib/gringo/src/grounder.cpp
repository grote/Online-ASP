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
#include "constant.h"
#include "predicateliteral.h"
#include "normalrule.h"
#include "statementdependencygraph.h"
#include "program.h"
#include "value.h"
#include "evaluator.h"
#include "domain.h"
#include "gringoexception.h"

using namespace NS_GRINGO;

Grounder::Grounder() : inc_(false), incStep_(1), internalVars_(0), output_(0), eval_(0)
{
}

void Grounder::setOutput(NS_OUTPUT::Output *output)
{
	output_ = output;
}

void Grounder::addStatement(Statement *rule)
{
#ifdef WITH_ICLASP
	if(incParts_.size() > 0)
		incParts_.back().second++;
#endif
	rules_.push_back(rule);
}

void Grounder::addDomains(std::string *id, std::vector<StringVector*>::iterator pos, std::vector<StringVector*>::iterator end, StringVector &list)
{
	if(pos == end)
	{
		domains_.push_back(DomainPredicate(id, new StringVector(list)));
	}
	else
	{
		for(StringVector::iterator it = (*pos)->begin(); it != (*pos)->end(); it++)
		{
			list.push_back(*it);
			addDomains(id, pos + 1, end, list);
			list.pop_back();
		}
		delete (*pos);
	}
}

void Grounder::addDomains(std::string *id, std::vector<StringVector*>* list)
{
	StringVector empty;
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
			for(StringVector::iterator it = dp.second->begin(); it != dp.second->end(); it++)
			{
				if(vars.find(getVar(*it)) != vars.end())
				{
					// construct predicate literal
					TermVector *tv = new TermVector();
					for(StringVector::iterator vars = dp.second->begin(); vars != dp.second->end(); vars++)
						tv->push_back(new Constant(Constant::VAR, this, *vars));
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
			std::cerr << "Warning: " << *sig.first << "/" << sig.second << " is never defined." << std::endl;
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
#ifdef WITH_ICLASP
	StatementVector::iterator r = rules_.begin();
	for(IncParts::iterator it = incParts_.begin(); it != incParts_.end(); it++)
		for(int j = 0; j < it->second; j++, r++)
			(*r)->setIncPart(this, it->first.first, it->first.second);
#else
	if(inc_)
		throw GrinGoException("Error: GrinGo has not been compiled with incremental clasp interface.");
#endif
	// the size of rules_ may increase during preprocessing make shure the newly inserted rules are preprocessed too
	for(size_t i = 0; i < rules_.size(); i++)
		rules_[i]->preprocess(this);
}

void Grounder::start()
{
#ifdef WITH_ICLASP
	if(inc_)
		throw GrinGoException("Error: Use the option -i to ground an incremental program.");
#endif
	std::cerr << "preprocessing ... " << std::endl;
	preprocess();
	std::cerr << "done" << std::endl;
	if(varMap_.size() == 0)
		std::cerr << "got ground program i hope u have enough memory :)" << std::endl;
	std::cerr << "adding domain predicates ... " << std::endl;
	addDomains();
	std::cerr << "done" << std::endl;
	std::cerr << "building dependencygraph ... " << std::endl;
	buildDepGraph();
	std::cerr << "done" << std::endl;
	std::cerr << "checking ... " << std::endl;
	check();
	reset();
	std::cerr << "done" << std::endl;
	std::cerr << "grounding ... " << std::endl;
	output_->initialize(getPred());
	substitution_.resize(varMap_.size() + 2);
	binder_.resize(varMap_.size() + 2, -1);
	ground();
	std::cerr << "done" << std::endl;
}

#ifdef WITH_ICLASP
int Grounder::getIncStep() const
{
	return incStep_;
}

void Grounder::iground()
{
	if(incStep_ == 1)
	{
		std::cerr << "preprocessing ... " << std::endl;
		preprocess();
		std::cerr << "done" << std::endl;
		if(varMap_.size() == 0)
			std::cerr << "got ground program i hope u have enough memory :)" << std::endl;
		std::cerr << "adding domain predicates ... " << std::endl;
		addDomains();
		std::cerr << "done" << std::endl;
		std::cerr << "building dependencygraph ... " << std::endl;
		buildDepGraph();
		std::cerr << "done" << std::endl;
		std::cerr << "checking ... " << std::endl;
		check();
		std::cerr << "done" << std::endl;
	}
	std::cerr << "grounding step: " << incStep_ << " ... " << std::endl;
	reset();
	ground();
	std::cerr << "done" << std::endl;
	incStep_++;
}
#endif

void Grounder::setIncPart(IncPart part, std::string *var)
{
	inc_     = true;
	if(incParts_.size() == 0 && rules_.size() > 0)
	{
		incParts_.push_back(make_pair(std::make_pair(BASE, static_cast<std::string*>(0)), rules_.size()));
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
#ifdef WITH_ICLASP
	if(incStep_ == 1 || !inc_)
		output_->initialize(getPred());
	else
		output_->reinitialize();
#else
	output_->initialize(getPred());
#endif
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
#ifdef WITH_ICLASP
			if(incStep_ == 1 || !inc_)
				rule->ground(this, PREPARE);
			else
				rule->ground(this, REINIT);
#else
			rule->ground(this, PREPARE);
#endif
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
	output_->finalize();
}

std::string *Grounder::createUniqueVar()
{
	std::string *uid;
	do
	{
		std::stringstream ss;
		ss << "I_" << internalVars_++;
		uid = createString(ss.str());
	}
	while(varMap_.find(uid) != varMap_.end());
	return uid;
}

std::string *Grounder::getVarString(int uid)
{
	// inefficient but we need it only for error messages
	for(VariableMap::iterator it = varMap_.begin(); it != varMap_.end(); it++)
		if(it->second == uid)
			return it->first;
	// we should get a string for every variable
	assert(false);
}

int Grounder::getVar(std::string *var)
{
	VariableMap::iterator it = varMap_.find(var);
	if(it != varMap_.end())
		return it->second;
	else
		return 0;
}

int Grounder::registerVar(std::string *var)
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

int Grounder::getBinder(int var)
{
	return binder_[var];
}

void Grounder::setConstValue(std::string *id, Term *t)
{
	std::pair<ConstTerms::iterator, bool> res = constTerms_.insert(std::make_pair(id, std::make_pair(false, t)));
	if(!res.second)
	{
		std::cerr << "Warning: multiple definitions of #const " << *id << std::endl;
		delete t;
	}
}

Value Grounder::getConstValue(std::string *id)
{
	ConstTerms::iterator it = constTerms_.find(id);
	if(it != constTerms_.end())
	{
		if(it->second.first)
			throw GrinGoException("Error: cyclic constant definition.");
		it->second.first = true;
		Value v = it->second.second->getConstValue();
		it->second.first = false;
		return v;
	}
	else
		return Value(id);
}

Evaluator *Grounder::getEvaluator()
{
	return eval_;
}

NS_OUTPUT::Output *Grounder::getOutput()
{
	return output_;
}

void Grounder::addTrueNegation(std::string *id, int arity)
{
#ifdef WITH_ICLASP
	// TODO: this is ugly
	if(inc_)
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
			std::string *var = createUniqueVar();
			tp->push_back(new Constant(Constant::VAR, this, var));
			tn->push_back(new Constant(Constant::VAR, this, var));
		}
		std::string *pos = createString(id->substr(1));
		PredicateLiteral *p = new PredicateLiteral(this, pos, tp);
		PredicateLiteral *n = new PredicateLiteral(this, id, tn);
		LiteralVector *body = new LiteralVector();
		body->push_back(p);
		body->push_back(n);
		NormalRule *r = new NormalRule(0, body);
		addStatement(r);
	}
}

