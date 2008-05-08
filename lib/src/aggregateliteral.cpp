#include "aggregateliteral.h"
#include "conditionalliteral.h"
#include "term.h"
#include "node.h"
#include "value.h"
#include "grounder.h"
#include "dependencygraph.h"
#include "indexeddomain.h"
#include "output.h"
#include "evaluator.h"

using namespace NS_GRINGO;

AggregateLiteral::AggregateLiteral(AggregateType type, ConditionalLiteralVector *literals) : Literal(), type_(type), literals_(literals), lower_(0), upper_(0)
{
}

void AggregateLiteral::setBounds(Term *lower, Term *upper)
{
	lower_ = lower;
	upper_ = upper;
}

bool AggregateLiteral::checkO(LiteralVector &unsolved)
{
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(!(*it)->checkO(unsolved))
			return false;
	}
	return true;
}

bool AggregateLiteral::isFact()
{
	return false;
}

bool AggregateLiteral::solved()
{
	return false;
}

bool AggregateLiteral::match(Grounder *g)
{
	// TODO: needs improvement
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		(*it)->ground(g);
	}
	return true;
}

void AggregateLiteral::normalize(Grounder *g, Expandable *r)
{
	// since aggregateliterals dont provide vars we dont need to normalize them
	// but the conditionals of predicates have to be normalized
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->normalize(g, this);
}

void AggregateLiteral::getVars(VarSet &vars, VarsType type, VarVector &glob)
{
	// aggregate literals are the only literals with local vars
	// so all global bound vars are needed to ground the aggregate
	// only the local vars are not needed
	Literal::getVars(vars, type == VARS_NEEDED ? VARS_ALL : type, glob);
}

void AggregateLiteral::getVars(VarSet &vars, VarsType type)
{
	switch(type)
	{
		case VARS_PROVIDED:
			// aggregate literals never!!! povide vars (even though in lparse they do)
			break;
		case VARS_NEEDED:
			// at least the global vars are needed
		case VARS_GLOBAL:
			if(lower_)
				lower_->getVars(vars);
			if(upper_)
				upper_->getVars(vars);
			for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
				(*it)->getVars(vars, VARS_GLOBAL);
			break;
		case VARS_ALL:
			if(lower_)
				lower_->getVars(vars);
			if(upper_)
				upper_->getVars(vars);
			for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
				(*it)->getVars(vars, VARS_ALL);
			break;
	}
}

Node *AggregateLiteral::createNode(DependencyGraph *dg, Node *prev, DependencyAdd todo)
{
	// TODO: this is only needed as long as the truth value of aggregates is not determined
	if(todo == ADD_BODY_DEP)
		prev->addDependency(prev, true);
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		// aggregate literals always depend negativly on its literals 
		Node *p = (*it)->createNode(dg, prev, ADD_NOTHING);
		assert(p);
		prev->addDependency(p, true);
		// if used in the head they it also depends cyclically on its literals
		if(todo == ADD_HEAD_DEP)
			p->addDependency(prev);
	}
	return 0;
}

void AggregateLiteral::reset()
{
	// aggregate literals should never occur negativly in heads
	assert(!getNeg());
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->reset();
}

void AggregateLiteral::finish()
{
	// aggregate literals should never occur negativly in heads
	assert(!getNeg());
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->finish();
}

void AggregateLiteral::print(std::ostream &out)
{
	if(lower_)
		out << lower_ << " ";
	switch(type_)
	{
		case SUM:
			out << "sum ";
			break;
		case TIMES:
			out << "times ";
			break;
		case COUNT:
			out << "count ";
			break;
		case MIN:
			out << "min ";
			break;
		case MAX:
			out << "max ";
			break;
	}
	out << "{";
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << ", ";
		else
			comma = true;
		out << *it;
	}
	out << "}";
	if(upper_)
		out << " " << upper_;
}

IndexedDomain *AggregateLiteral::createIndexedDomain(VarSet &index)
{
	return new IndexedDomainMatchOnly(this);
}

AggregateLiteral::AggregateLiteral(AggregateLiteral &a) : type_(a.type_), lower_(a.lower_ ? a.lower_->clone() : 0), upper_(a.upper_ ? a.upper_->clone() : 0)
{
	if(a.literals_)
	{
		literals_ = new ConditionalLiteralVector();
		for(ConditionalLiteralVector::iterator it = a.literals_->begin(); it != a.literals_->end(); it++)
			literals_->push_back((ConditionalLiteral*)(*it)->clone());
	}
	else
		literals_ = 0;
}

void AggregateLiteral::appendLiteral(Literal *l)
{
	if(!literals_)
		literals_ = new ConditionalLiteralVector();
	assert(dynamic_cast<ConditionalLiteral*>(l));
	literals_->push_back((ConditionalLiteral*)l);
}

void AggregateLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(literals_)
		for(size_t i = 0; i < literals_->size(); i++)
			(*literals_)[i]->preprocess(g, this);
}

ConditionalLiteralVector *AggregateLiteral::getLiterals()
{
	return literals_;
}

Literal* AggregateLiteral::clone()
{
	return new AggregateLiteral(*this);
}

Term *AggregateLiteral::getLower()
{
	return lower_;
}

Term *AggregateLiteral::getUpper()
{
	return upper_;
}

AggregateLiteral::AggregateType AggregateLiteral::getType()
{
	return type_;
}

void AggregateLiteral::ground(Grounder *g)
{
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
		(*it)->ground(g);
}

NS_OUTPUT::Object *AggregateLiteral::convert()
{	
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
		{
			lits.push_back(p->convert());
			weights.push_back(p->getWeight());
		}
	}
	NS_OUTPUT::Aggregate::Type type;
	switch(type_)
	{
		case SUM:
			type = NS_OUTPUT::Aggregate::SUM;
			break;
		case COUNT:
			type = NS_OUTPUT::Aggregate::COUNT;
			break;
		case TIMES:
			type = NS_OUTPUT::Aggregate::TIMES;
			break;
		case MIN:
			type = NS_OUTPUT::Aggregate::MIN;
			break;
		case MAX:
			type = NS_OUTPUT::Aggregate::MAX;
			break;
		default:
			assert(false);
			break;
	}
	NS_OUTPUT::Aggregate *a;
	if(lower_ && upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), type, lower_->getValue(), lits, weights, upper_->getValue());
	else if(lower_)
		a = new NS_OUTPUT::Aggregate(getNeg(), type, lower_->getValue(), lits, weights);
	else if(upper_)
		a = new NS_OUTPUT::Aggregate(getNeg(), type, lits, weights, upper_->getValue());
	else
		a = new NS_OUTPUT::Aggregate(getNeg(), type, lits, weights);

	return a;
}

AggregateLiteral::~AggregateLiteral()
{
	if(lower_)
		delete lower_;
	if(upper_)
		delete upper_;
	if(literals_)
	{
		for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
			delete *it;
		delete literals_;
	}
}

