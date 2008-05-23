#include "grounder.h"
#include "statement.h"
#include "constant.h"
#include "predicateliteral.h"
#include "normalrule.h"
#include "dependencygraph.h"
#include "scc.h"
#include "value.h"
#include "gringooutput.h"
#include "evaluator.h"
#include "node.h"

using namespace NS_GRINGO;

Grounder::Grounder() : internalVars_(0), depGraph_(new DependencyGraph()), output_(0), eval_(0), hideAll_(false)
{
}

void Grounder::addStatement(Statement *rule)
{
	rules_.push_back(rule);
}

void Grounder::addDomains(std::string *id, StringVector* list)
{
	domains_.push_back(DomainPredicate(id, list));
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
					PredicateLiteral *pred = new PredicateLiteral(dp.first, tv);
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
	for(NodeVector::iterator it = depGraph_->getPredNodes().begin(); it != depGraph_->getPredNodes().end(); it++)
	{
		Node *n = *it;
		if(n->complete())
		{
			Signature &sig = (*depGraph_->getPred())[n->getUid()];
			if(warn)
				std::cerr << "Warning: " << *sig.first << "/" << sig.second << " is never defined" << std::endl;
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
	NS_OUTPUT::GrinGoOutput output(&std::cout);
	start(output);
}

void Grounder::start(NS_OUTPUT::Output &output)
{
	output_ = &output;
	std::cerr << "preprocessing ... " << std::endl;
	preprocess();
	std::cerr << "done" << std::endl;
	if(varMap_.size() == 0)
		std::cerr << "got ground program i hope u have enough memory :)" << std::endl;
	std::cerr << "adding domain predicates ... " << std::endl;
	addDomains();
	std::cerr << "done" << std::endl;
	std::cerr << "buidling dependencygraph ... " << std::endl;
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

SignatureVector *Grounder::getPred()
{
	return depGraph_->getPred();
}

void Grounder::addSCC(SCC *scc)
{
	sccs_.push_back(scc);
}

void Grounder::ground()
{
	visible_.reserve(getPred()->size());
	for(SignatureVector::const_iterator it = getPred()->begin(); it != getPred()->end(); it++)
		visible_.push_back(isVisible(it->first, it->second));
	output_->initialize(this);
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
	for(std::map<std::string*, Value*>::iterator it = const_.begin(); it != const_.end(); it++)
		delete it->second;
	// TODO: do i really need pointers????
	StringVector v(stringHash_.begin(), stringHash_.end());
	stringHash_.clear();
	for(StringVector::iterator it = v.begin(); it != v.end(); it++)
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

void Grounder::setConstValue(std::string *id, Value *v)
{
	Value *&ref = const_[id];
	if(ref)
	{
		std::cerr << "warning: multiple definitions of #const " << *id << std::endl;
		delete v;
	}
	else
		ref = v;
}

Evaluator *Grounder::getEvaluator()
{
	return eval_;
}

NS_OUTPUT::Output *Grounder::getOutput()
{
	return output_;
}

Value *Grounder::createConstValue(std::string *id)
{
	std::map<std::string*, Value*>::iterator pos = const_.find(id);
	if(pos != const_.end())
	{
		return new Value(*(pos->second));
	}
	else
	{
		return createStringValue(id);
	}
}

Value *Grounder::createStringValue(std::string *id)
{
	return new Value(id);
}

void Grounder::hideAll()
{
	hideAll_ = true;
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
		PredicateLiteral *p = new PredicateLiteral(pos, tp);
		PredicateLiteral *n = new PredicateLiteral(id, tn);
		LiteralVector *body = new LiteralVector();
		body->push_back(p);
		body->push_back(n);
		NormalRule *r = new NormalRule(0, body);
		addStatement(r);
	}
}

void Grounder::setVisible(std::string *id, int arity, bool visible)
{
	hide_[std::make_pair(id, arity)] = !visible;
}

bool Grounder::isVisible(int uid)
{
	return visible_[uid];
}

bool Grounder::isVisible(std::string *id, int arity)
{
	std::map<Signature, bool>::iterator it = hide_.find(std::make_pair(id, arity));
	if(it == hide_.end())
		return !hideAll_;
	else
		return !it->second;
}

std::string *Grounder::createString(const std::string &s2)
{
	return createString(new std::string(s2));
}

std::string *Grounder::createString(std::string *s)
{
	std::pair<StringHash::iterator, bool> res = stringHash_.insert(s);
	if(!res.second)
		delete s;
	return *res.first;
}

