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
#include "dependencygraph.h"
#include "scc.h"
#include "value.h"
#include "evaluator.h"
#include "node.h"
#include "gringoexception.h"

using namespace NS_GRINGO;

Grounder::Grounder(NS_OUTPUT::Output *output) : internalVars_(0), depGraph_(new DependencyGraph()), output_(output), eval_(0)
{
}

void Grounder::addStatement(Statement *rule)
{
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
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
	{
		(*it)->buildDepGraph(depGraph_);
	}
}

void Grounder::addDomains()
{
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
	{
		Statement *rule = *it;
		VarSet vars;
		rule->getVars(vars);
		for(DomainVector::iterator it = domains_.begin(); it != domains_.end(); it++)
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

void Grounder::reset(bool warn = false)
{
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		(*it)->reset();
	// all nodes with zero defines have to be false
	for(int uid = 0; uid < (int)depGraph_->getPredNodes().size(); uid++)
	{
		Node *n = depGraph_->getPredNodes()[uid];
		if(n->complete())
		{
			if(warn)
			{
				Signature &sig = (*getPred())[uid];
				std::cerr << "Warning: " << *sig.first << "/" << sig.second << " is never defined." << std::endl;
			}
			n->setSolved(true);
		}
	}
}

void Grounder::preprocess()
{
	// the size of rules_ may increase during preprocessing make shure the newly inserted rules are preprocessed too
	for(size_t i = 0; i < rules_.size(); i++)
		rules_[i]->preprocess(this);
}

void Grounder::start()
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
	reset(true);
	depGraph_->calcSCCs();
	std::cerr << "done" << std::endl;
	std::cerr << "checking ... " << std::endl;
	if(!depGraph_->check(this))
		return;
	reset(false);
	std::cerr << "done" << std::endl;
	std::cerr << "grounding ... " << std::endl;
	ground();
	std::cerr << "done" << std::endl;
}

void Grounder::addSCC(SCC *scc)
{
	sccs_.push_back(scc);
}

void Grounder::ground()
{
	output_->initialize(getPred());
	substitution_.resize(varMap_.size() + 2);
	binder_.resize(varMap_.size() + 2, -1);
	for(SCCVector::iterator it = sccs_.begin(); it != sccs_.end(); it++)
	{
		SCC *scc = *it;
		//std::cerr << scc << std::endl;
		eval_ = scc->getEvaluator();
		eval_->initialize(this);
		StatementVector *rules = scc->getStatements();
		for(StatementVector::iterator it = rules->begin(); it !=rules->end(); it++)
		{
			Statement *rule = *it;
			rule->ground(this);
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
	delete depGraph_;
	for(DomainVector::iterator it = domains_.begin(); it != domains_.end(); it++)
		delete (*it).second;
	for(StatementVector::iterator it = rules_.begin(); it != rules_.end(); it++)
		delete *it;
	for(ConstTerms::iterator it = constTerms_.begin(); it != constTerms_.end(); it++)
		delete it->second.second;

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
			throw GrinGoException("error cyclic constant definition");
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

