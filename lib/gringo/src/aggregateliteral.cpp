#include "aggregateliteral.h"
#include "predicateliteral.h"
#include "conditionalliteral.h"
#include "term.h"
#include "node.h"
#include "value.h"
#include "grounder.h"
#include "dependencygraph.h"
#include "literaldependencygraph.h"
#include "indexeddomain.h"
#include "output.h"
#include "evaluator.h"
#include "constant.h"
#include "dlvgrounder.h"

using namespace NS_GRINGO;

AggregateLiteral::AggregateLiteral(AggregateType type, ConditionalLiteralVector *literals) : Literal(), type_(type), literals_(literals), lower_(0), upper_(0), equal_(0)
{
}

void AggregateLiteral::setBounds(Term *lower, Term *upper)
{
	lower_ = lower;
	upper_ = upper;
}

void AggregateLiteral::setEqual(Constant *equal)
{
	equal_ = equal;
	lower_ = equal_->clone();
	upper_ = equal_->clone();
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
	// TODO: for now !!!!
	return false;
}

bool AggregateLiteral::solved()
{
	/*
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		if(!(*it)->solved())
			return false;
	return true;
	*/
	return false;
}

bool AggregateLiteral::matchSum(Grounder *g, int &lower, int &upper, bool checkBounds)
{
	lower = 0;
	upper = 0;
	int fixed = 0;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match(g))
				continue;
			int weight = p->getWeight();
			if(p->isFact())
				fixed+= weight;
			else
			{
				if(weight > 0)
					upper+= weight;
				else
					lower+= weight;
			}
		}
	}
	lower+= fixed;
	upper+= fixed;
	if(checkBounds)
		return this->checkBounds(lower, upper);
	else
		return true;
}

bool AggregateLiteral::matchTimes(Grounder *g, int &lower, int &upper, bool checkBounds)
{
	lower = 1;
	upper = 1;
	int fixed = 1;
	bool hasZero = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match(g))
				continue;
			int weight = p->getWeight();
			if(p->isFact())
				fixed*= weight;
			else
			{
				if(weight == 0)
					hasZero = true;
				else if(weight > 0)
					upper*= weight;
				else
					lower*= weight;
			}
		}
	}
	lower*= fixed;
	upper*= fixed;
	if(hasZero && lower > 0)
		lower = 0;
	if(hasZero && upper < 0)
		upper = 0;
	if(checkBounds)
		return this->checkBounds(lower, upper);
	else
		return true;
}

bool AggregateLiteral::matchMin(Grounder *g, int &lower, int &upper, bool checkBounds)
{
	lower = INT_MAX;
	upper = INT_MIN;
	int fixed = INT_MAX;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match(g))
				continue;
			int weight = p->getWeight();
			if(p->isFact())
				fixed = std::min(fixed, weight);
			else
			{
				lower = std::min(lower, weight);
				upper = std::max(upper, weight);
			}
		}
	}
	lower = std::min(lower, fixed);
	if(upper > fixed || upper == INT_MIN)
		upper = fixed;
	if(checkBounds)
		return this->checkBounds(lower, upper);
	else
		return true;
}

bool AggregateLiteral::matchMax(Grounder *g, int &lower, int &upper, bool checkBounds)
{
	lower = INT_MAX;
	upper = INT_MIN;
	int fixed = INT_MIN;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match(g))
				continue;
			int weight = p->getWeight();
			if(p->match(g))
			{
				if(p->isFact())
					fixed = std::max(fixed, weight);
				else
				{
					lower = std::min(lower, weight);
					upper = std::max(upper, weight);
				}
			}
		}
	}
	if(lower < fixed || lower == INT_MAX)
		lower = fixed;
	upper = std::max(upper, fixed);
	if(checkBounds)
		return this->checkBounds(lower, upper);
	else
		return true;
}

bool AggregateLiteral::checkBounds(int lower, int upper)
{
	int lowerBound = lower_ ? (int)lower_->getValue() : lower;
	int upperBound = upper_ ? (int)upper_->getValue() : upper;
	if(lowerBound > upperBound)
		return false;
	if(lower >= lowerBound && upper <= upperBound)
	{
		fact_ = true;
		return true;
	}
	fact_ = false;
	if(upper < lowerBound || lower > upperBound)
		return false;
	return true;
}

bool AggregateLiteral::matchConjunction(Grounder *g)
{
	fact_ = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(!p->match(g))
				return false;
		}
	}
	return true;
}

bool AggregateLiteral::matchDisjunction(Grounder *g)
{
	fact_ = false;
	bool matchFound = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		ConditionalLiteral *p = *it;
		p->ground(g);
		for(p->start(); p->hasNext(); p->next())
		{
			if(p->match(g))
				matchFound = true;
		}
	}
	return matchFound;
}

bool AggregateLiteral::match(Grounder *g)
{
	int upper, lower;
	return match(g, lower, upper, true);
}

bool AggregateLiteral::match(Grounder *g, int &lower, int &upper, bool checkBounds)
{
	switch(type_)
	{
		case COUNT:
		case SUM:
			return matchSum(g, upper, lower, checkBounds);
		case TIMES:
			return matchTimes(g, upper, lower, checkBounds);
		case MIN:
			return matchMin(g, upper, lower, checkBounds);
		case MAX:
			return matchMax(g, upper, lower, checkBounds);
		case DISJUNCTION:
			return matchDisjunction(g);
		case CONJUNCTION:
			return matchConjunction(g);
	}
	assert(false);
}

void AggregateLiteral::getVars(VarSet &vars) const
{
	if(equal_)
		equal_->getVars(vars);
	if(lower_)
		lower_->getVars(vars);
	if(upper_)
		upper_->getVars(vars);
	for(ConditionalLiteralVector::const_iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->getVars(vars);
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

void AggregateLiteral::createNode(LDGBuilder *dg, bool head)
{
	VarSet needed, provided;
	if(equal_)
		equal_->getVars(provided);
	else
	{
		if(lower_)
			lower_->getVars(needed);
		if(upper_)
			upper_->getVars(needed);
	}
	dg->createNode(this, head, needed, provided, true);
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
		(*it)->createNode(dg, head);
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
		case DISJUNCTION:
		case CONJUNCTION:
			break;
			
	}
	if(type_ != DISJUNCTION && type_ != CONJUNCTION)
		out << "{";
	bool comma = false;
	for(ConditionalLiteralVector::iterator it = literals_->begin(); it != literals_->end(); it++)
	{
		if(comma)
			out << (type_ == DISJUNCTION ? " | " : ", ");
		else
			comma = true;
		out << *it;
	}
	if(type_ != DISJUNCTION && type_ != CONJUNCTION)
		out << "}";
	if(upper_)
		out << " " << upper_;
}

namespace
{
	class IndexedDomainAggregate : public IndexedDomain
	{
	public:
		IndexedDomainAggregate(AggregateLiteral *l, int var);
		virtual void firstMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual void nextMatch(int binder, DLVGrounder *g, MatchStatus &status);
		virtual ~IndexedDomainAggregate();
	protected:
		AggregateLiteral *l_;
		int var_;
		int current_;
		int lower_;
		int upper_;
	};

	IndexedDomainAggregate::IndexedDomainAggregate(AggregateLiteral *l, int var) : l_(l), var_(var)
	{
	}

	void IndexedDomainAggregate::firstMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		l_->match(g->g_, lower_, upper_, false);
		current_ = lower_;
		if(current_ <= upper_)
		{
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnFirstMatch;
	}

	void IndexedDomainAggregate::nextMatch(int binder, DLVGrounder *g, MatchStatus &status)
	{
		if(current_ < upper_)
		{
			current_++;
			g->g_->setValue(var_, Value(current_), binder);
			status = SuccessfulMatch;
		}
		else
			status = FailureOnNextMatch;
	}

	IndexedDomainAggregate::~IndexedDomainAggregate()
	{
	}
	
}

IndexedDomain *AggregateLiteral::createIndexedDomain(VarSet &index)
{
	if(equal_)
	{
		if(index.find(equal_->getUID()) != index.end())
		{
			return new IndexedDomainMatchOnly(this);
		}
		else
		{
			return new IndexedDomainAggregate(this, equal_->getUID());
		}
	}
	else
		return new IndexedDomainMatchOnly(this);
}

AggregateLiteral::AggregateLiteral(const AggregateLiteral &a) : type_(a.type_), lower_(a.lower_ ? a.lower_->clone() : 0), upper_(a.upper_ ? a.upper_->clone() : 0), equal_(a.equal_ ? static_cast<Constant*>(equal_->clone()) : 0)
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

void AggregateLiteral::appendLiteral(Literal *l, ExpansionType type)
{
	if(!literals_)
		literals_ = new ConditionalLiteralVector();
	assert(dynamic_cast<ConditionalLiteral*>(l));
	literals_->push_back((ConditionalLiteral*)l);
}

void AggregateLiteral::preprocess(Grounder *g, Expandable *e)
{
	if(type_ == DISJUNCTION)
	{
		assert(literals_);
		for(size_t i = 0; i < literals_->size(); i++)
			(*literals_)[i]->preprocessDisjunction(g, this, e);
	}
	else if(literals_)
		for(size_t i = 0; i < literals_->size(); i++)
			(*literals_)[i]->preprocess(g, this);
	if(upper_)
		upper_->preprocess(this, upper_, g, e);
	if(lower_)
		lower_->preprocess(this, lower_, g, e);
}

ConditionalLiteralVector *AggregateLiteral::getLiterals()
{
	return literals_;
}

Literal* AggregateLiteral::clone() const
{
	return new AggregateLiteral(*this);
}

Term *AggregateLiteral::getLower() const
{
	return lower_;
}

Term *AggregateLiteral::getUpper() const
{
	return upper_;
}

AggregateLiteral::AggregateType AggregateLiteral::getType() const
{
	return type_;
}

NS_OUTPUT::Object *AggregateLiteral::convert()
{
	if(type_ == CONJUNCTION)
	{
		NS_OUTPUT::ObjectVector lits;
		for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
		{
			ConditionalLiteral *p = *it;
			for(p->start(); p->hasNext(); p->next())
				lits.push_back(p->convert());
		}
		return new NS_OUTPUT::Conjunction(lits);
	}
	NS_OUTPUT::ObjectVector lits;
	IntVector weights;
	for(ConditionalLiteralVector::iterator it = getLiterals()->begin(); it != getLiterals()->end(); it++)
	{
		ConditionalLiteral *p = *it;
		for(p->start(); p->hasNext(); p->next())
		{
			lits.push_back(p->convert());
			if(type_ != DISJUNCTION && type_ != COUNT)
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
		case DISJUNCTION:
			type = NS_OUTPUT::Aggregate::DISJUNCTION;
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

double AggregateLiteral::heuristicValue()
{
	return DBL_MAX;
}

AggregateLiteral::~AggregateLiteral()
{
	if(equal_)
		delete equal_;
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

Literal *AggregateLiteral::createHead(ConditionalLiteralVector *list)
{
	if(list->size() == 1 && !list->front()->hasConditionals())
	{
		Literal *l = list->front()->toPredicateLiteral();
		delete list;
		return l;
	}
	else
		return new AggregateLiteral(DISJUNCTION, list);
}

Literal *AggregateLiteral::createBody(PredicateLiteral *pred, LiteralVector *list)
{
	if(list == 0 || list->size() == 0)
		return pred;
	else
	{
		ConditionalLiteralVector *clv = new ConditionalLiteralVector();
		clv->push_back(new ConditionalLiteral(pred, list));
		return new AggregateLiteral(CONJUNCTION, clv);
	}
}

